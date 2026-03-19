/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/threadverify.h>

struct ev_loop;
struct ev_child;

namespace cflib::util {

class Mail
{
public:
    String from;
    String to;
    String subject;
    String text;

public:
    Mail() = default;
    Mail(const String & to, const String & subject, const String & text, const String & from) :
        from(from), to(to), subject(subject), text(text) {}

    bool isValid() const;

    ByteArray raw(String & fromAddr, String & toAddr) const;
};

class Mailer : public ThreadVerify
{
    CF_DISABLE_COPY(Mailer)
public:
    Mailer(bool isEnabled = true);
    ~Mailer();

    static void send(const Mail & mail);

protected:
    virtual void deleteThreadData();

private:
    void initThreadData();
    void doSend(const Mail & mail);
    void startProcess();
    static void childCallback(ev_loop * loop, ev_child * w, int revents);
    void childExited(int status);

private:
    static Mailer * instance_;

    String sendmailPath_;
    List<Mail> queue_;
    pid_t childPid_;
};

} // namespace
