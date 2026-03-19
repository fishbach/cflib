/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/tcpconn.h>
#include <cflib/util/threadverify.h>

namespace cflib::net {

class PassThroughHandler;
class RequestHandler;

namespace impl {

class HttpThread;

class RequestParser : public util::ThreadVerify, public TCPConn
{
public:
    RequestParser(TCPConnData * data,
        const CFList<RequestHandler *> & handlers, HttpThread * thread);
    ~RequestParser();

    void sendReply(int id, const ByteArray & reply);

    void detachRequest();
    void setPassThroughHandler(PassThroughHandler * hdl);
    ByteArray readPassThrough(bool & isLast);
    TCPConnData * detach();

protected:
    virtual void newBytesAvailable();
    virtual void closed(CloseType type);

private:
    void parseRequest();
    bool parseHeader();
    bool handleRequestLine(const ByteArray & line);
    void writeReply(const ByteArray & reply);

private:
    const CFList<RequestHandler *> & handlers_;
    HttpThread * thread_;
    const int id_;

    ByteArray header_;

    cfint64 contentLength_;
    CFMap<ByteArray, ByteArray> headerFields_;
    int method_;
    ByteArray uri_;
    ByteArray body_;

    int requestCount_;
    int nextReplyId_;
    CFMap<int, ByteArray> replies_;

    int attachedRequests_;
    bool detached_;
    bool passThrough_;
    PassThroughHandler * passThroughHandler_;
};

}} // namespace
