/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cflib/crypt/util.h>
#include <cflib/net/websocketservice.h>
#include <cflib/serialize/util.h>

namespace cflib { namespace net {

template <class C>
class WSCommStateListener
{
public:
	virtual void newClient(uint connId) { Q_UNUSED(connId); }
	virtual void connDataChange(const C & connData, uint connId) { Q_UNUSED(connData); Q_UNUSED(connId); }
	virtual bool handleTextMsg(const QByteArray & data, const C & connData, uint connId)
	{
		Q_UNUSED(data); Q_UNUSED(connData); Q_UNUSED(connId);
		return false;
	}
	virtual bool handleBinaryMsg(uint type, const QByteArray & data, const C & connData, uint connId)
	{
		Q_UNUSED(type); Q_UNUSED(data); Q_UNUSED(connData); Q_UNUSED(connId);
		return false;
	}
};

template <class C>
class WSCommManager : public WebSocketService
{
public:
	typedef WSCommStateListener<C> StateListener;

public:
	WSCommManager(const QString & path) : WebSocketService(path) {}
	~WSCommManager() { stopVerifyThread(); }

protected:
	virtual void newConnection(uint connId)
	{
		QListIterator<StateListener *> it(stateListener_);
		while (it.hasNext()) it.next()->newClient(connId);
	}

	virtual void newMsg(uint connId, const QByteArray & data, bool isBinary)
	{
		if (!isBinary) {
			QListIterator<StateListener *> it(stateListener_);
			while (it.hasNext()) {
				if (it.next()->handleTextMsg(data, connData_.value(connId), connId)) break;
			}
			return;
		}
		quint64 tag;
		int tagLen;
		int lengthSize;
		qint32 tlvLen = serialize::getTLVLength(data, tag, tagLen, lengthSize);
		if (tlvLen < 0) {
			close(TCPConn::HardClosed);
			return;
		}
		switch (tag) {
			case 1: send(connId, serialize::toByteArray(crypt::random(20)), true); return;
		}

	}

	virtual void closed(uint connId, TCPConn::CloseType type)
	{
	}

private:
	QList<StateListener *> stateListener_;
	QHash<uint, C> connData_;
};

}}	// namespace
