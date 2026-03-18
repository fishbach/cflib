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

namespace cflib { namespace util {

class Mail
{
public:
    CFString from;
    CFString to;
    CFString subject;
    CFString text;

public:
    Mail() = default;
    Mail(const CFString & to, const CFString & subject, const CFString & text, const CFString & from) :
        from(from), to(to), subject(subject), text(text) {}

    bool isValid() const;

    CFByteArray raw(CFString & fromAddr, CFString & toAddr) const;
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

    CFString sendmailPath_;
    CFList<Mail> queue_;
    pid_t childPid_;
};

}}    // namespace
