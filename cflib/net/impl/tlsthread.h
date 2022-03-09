/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/tcpconn.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class TCPConnData;

namespace impl {

class TCPManagerImpl;

class TLSThread : public util::ThreadVerify
{
public:
	TLSThread(TCPManagerImpl & impl, uint no, uint total);
	~TLSThread();

	void startReadWatcher(TCPConnData * conn);
	void read(TCPConnData * conn);
	void write(TCPConnData * conn, const QByteArray & data, bool notifyFinished);
	void closeConn(TCPConnData * conn, TCPConn::CloseType type, bool notifyClose);
	void deleteOnFinish(TCPConnData * conn);
	void callClosed(TCPConnData * conn);

private:
	TCPManagerImpl & impl_;
};

}}}	// namespace
