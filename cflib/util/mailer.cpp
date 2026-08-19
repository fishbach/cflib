/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "mailer.h"

#include <cflib/util/libev.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <cstdlib>
#include <format>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

USE_LOG(LogCat::Etc)

namespace cflib::util {

Mailer * Mailer::instance_ = nullptr;

Mailer::Mailer(bool isEnabled) :
    ThreadVerify("Mailer", ThreadVerify::Worker),
    childPid_(-1)
{
    if (instance_) logWarn("It makes no sense to have two Mailer instances!");
    instance_ = this;

    if (!isEnabled) {
        logInfo("emailing disabled");
        return;
    }

    // Search for sendmail
    const char * pathEnv = getenv("PATH");
    String pathStr(pathEnv ? pathEnv : "");
    auto paths = pathStr.split(':');
    // Add extra search paths
    paths.push_back(String("/usr/lib"));
    paths.push_back(String("/usr/sbin"));

    for (const String & path : paths) {
        String candidate = path;
        if (!candidate.endsWith("/")) candidate += "/";
        candidate += "sendmail";
        struct stat st;
        if (stat(candidate.toStdString().c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
            sendmailPath_ = candidate;
            break;
        }
    }

    if (sendmailPath_.isNull()) {
        String searchedPaths = String(", ").join(paths);
        logWarn("cannot find sendmail executable (searched: %1)", searchedPaths);
    } else {
        logDebug("found sendmail at: %1", sendmailPath_);
    }

    initThreadData();
}

Mailer::~Mailer()
{
    stopVerifyThread();
    instance_ = nullptr;
}

void Mailer::send(const Mail & mail)
{
    if (!instance_) {
        logWarn("no Mailer instance available");
        return;
    }
    instance_->doSend(mail);
}

void Mailer::initThreadData()
{
    if (!verifyThreadCall(&Mailer::initThreadData)) return;
    logFunctionTrace
}

void Mailer::deleteThreadData()
{
    if (childPid_ > 0) {
        logWarn("mail process still running");
        kill(childPid_, SIGKILL);
        childPid_ = -1;
    }
}

void Mailer::doSend(const Mail & mail)
{
    logFunctionTraceParam("new mail to: %1", mail.to);
    if (!verifyThreadCall(&Mailer::doSend, mail)) return;

    String fromAddr;
    String toAddr;
    if (!mail.isValid()) {
        logWarn("invalid mail: %1", mail.raw(fromAddr, toAddr));
        return;
    }

    if (sendmailPath_.isNull()) {
        logInfo("mailer not active, mail from %1 to %2 dropped", mail.from, mail.to);
        std::cout << std::format("--------\n{}\n--------\n", mail.raw(fromAddr, toAddr).sv());
        return;
    }

    queue_.push_back(mail);
    if (queue_.size() == 1) startProcess();
}

namespace {

ByteArray encodeAddress(const String & address, String & plain)
{
    // Simple parse: look for "name <addr>" pattern
    ssize_t lt = address.indexOf('<');
    ssize_t gt = address.indexOf('>');
    if (lt >= 0 && gt > lt) {
        String name = address.left(lt).trimmed();
        plain = address.mid(lt + 1, gt - lt - 1).trimmed();
        return cflib::util::encodeWord(name, true) + " <" + plain.toUtf8() + ">";
    } else {
        plain = address.trimmed();
        return plain.toUtf8();
    }
}

}

void Mailer::startProcess()
{
    const Mail & mail = queue_.front();
    String from;
    String to;
    const ByteArray raw = mail.raw(from, to);
    logDebug("exec: %1 -f %2 %3", sendmailPath_, from, to);

    // C strings for the child's execl, made before the fork
    const std::string sendmailC = sendmailPath_.toStdString();
    const std::string fromC     = from.toStdString();
    const std::string toC       = to.toStdString();

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        logWarn("pipe failed for sendmail");
        queue_.erase(queue_.begin());
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        logWarn("fork failed for sendmail");
        close(pipefd[0]);
        close(pipefd[1]);
        queue_.erase(queue_.begin());
        return;
    }

    if (pid == 0) {
        // Child process
        close(pipefd[1]); // close write end
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Close stdout/stderr
        int devnull = open("/dev/null", 0);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        execl(sendmailC.c_str(), "sendmail", "-f", fromC.c_str(), toC.c_str(), (char *)nullptr);
        _exit(127);
    }

    // Parent
    close(pipefd[0]); // close read end
    // Write mail data to pipe
    const char * data = raw.constData();
    size_t remaining = raw.size();
    while (remaining > 0) {
        ssize_t written = write(pipefd[1], data, remaining);
        if (written <= 0) break;
        data += written;
        remaining -= written;
    }
    close(pipefd[1]);

    childPid_ = pid;

    // Watch for child exit via libev
    ev_loop * loop = libEVLoop();
    if (loop) {
        ev_child * cw = new ev_child;
        ev_child_init(cw, &Mailer::childCallback, pid, 0);
        cw->data = this;
        ev_child_start(loop, cw);
    }
}

void Mailer::childCallback(ev_loop * loop, ev_child * w, int)
{
    Mailer * self = (Mailer *)w->data;
    int status = w->rstatus;
    ev_child_stop(loop, w);
    delete w;
    self->childExited(status);
}

void Mailer::childExited(int status)
{
    childPid_ = -1;

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        logInfo("email from %1 to %2 sent", queue_.front().from, queue_.front().to);
    } else {
        int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        logWarn("could not send email to %1 (exitCode: %2)", queue_.front().to, exitCode);
    }

    queue_.erase(queue_.begin());
    if (!queue_.isEmpty()) execLater([this]() { startProcess(); });
}

bool Mail::isValid() const
{
    return !from.isEmpty() && !to.isEmpty();
}

ByteArray Mail::raw(String & fromAddr, String & toAddr) const
{
    ByteArray rv;
    rv  << "Content-type: text/plain; charset=utf-8\r\n"
        << "Content-transfer-encoding: quoted-printable\r\n"
        << "From: "    << encodeAddress(from, fromAddr)           << "\r\n"
        << "To: "      << encodeAddress(to, toAddr)               << "\r\n"
        << "Subject: " << cflib::util::encodeWord(subject, false) << "\r\n"
        << "\r\n"
        << cflib::util::encodeQuotedPrintable(text);
    return rv;
}


} // namespace
