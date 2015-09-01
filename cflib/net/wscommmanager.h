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
#include <cflib/util/log.h>

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
	virtual bool handleBinaryMsg(quint64 tag,
		const QByteArray & data, qint32 tlvLen, int tagLen, int lengthSize,
		const C & connData, uint connId)
	{
		Q_UNUSED(tag);
		Q_UNUSED(data); Q_UNUSED(tlvLen); Q_UNUSED(tagLen); Q_UNUSED(lengthSize);
		Q_UNUSED(connData); Q_UNUSED(connId);
		return false;
	}
};

template <class C>
class WSCommManager : public WebSocketService
{
	USE_LOG_MEMBER(LogCat::Http)
public:
	typedef WSCommStateListener<C> StateListener;

public:
	WSCommManager(const QString & path) :
		WebSocketService(path),
		connDataId_(0)
	{}
	~WSCommManager() { stopVerifyThread(); }

protected:
	virtual void newConnection(uint connId)
	{
		QListIterator<StateListener *> it(stateListener_);
		while (it.hasNext()) it.next()->newClient(connId);
	}

	virtual void newMsg(uint connId, const QByteArray & data, bool isBinary)
	{
		const uint dataId = connId2dataId_.value(connId);

		// handle text msg
		if (!isBinary) {
			if (dataId == 0) {
				close(TCPConn::HardClosed);
				logInfo("request without clientId from %1", connId);
				return;
			}

			QListIterator<StateListener *> it(stateListener_);
			while (it.hasNext()) {
				if (it.next()->handleTextMsg(data, connData_.value(connId).first, connId)) return;
			}
			close(TCPConn::HardClosed);
			logInfo("unhandled text message from %1", connId);
			return;
		}

		// read outer BER
		quint64 tag;
		int tagLen;
		int lengthSize;
		const qint32 valueLen = serialize::getTLVLength(data, tag, tagLen, lengthSize);
		if (valueLen < 0) {
			close(TCPConn::HardClosed);
			logInfo("broken BER msg %1 (%2)", connId, valueLen);
			return;
		}
		logTrace("ws msg (connId: %1, tag: %2, valueLen: %3)", connId, tag, valueLen);

		// handle new connections
		if (dataId == 0) {
			switch (tag) {
				case 1:
					sendNewClientId(connId);
				case 2: {
					const QByteArray clId = serialize::fromByteArray<QByteArray>(data, tagLen, lengthSize, valueLen);
					const uint dId = clientIds_.value(clId);
					if (dId == 0) sendNewClientId(connId);
					else          connId2dataId_[connId] = dId;
				}
				default:
					close(TCPConn::HardClosed);
					logInfo("request without clientId from %1", connId);
			}
			return;
		}

		switch (tag) {
			case 1:
			case 2:
				close(TCPConn::HardClosed);
				logInfo("two client id requests from %1", connId);
				return;
		}


	}

	virtual void closed(uint connId, TCPConn::CloseType type)
	{
		if (!(type & TCPConn::ReadClosed) || !(type & TCPConn::WriteClosed)) return;
		connId2dataId_.remove(connId);
	}

private:
	void sendNewClientId(uint connId)
	{
		const QByteArray clId = crypt::random(20);
		uint dId = ++connDataId_;
		while (connData_.contains(dId)) dId = ++connDataId_;
		clientIds_[clId] = dId;
		connData_[dId].second = clId;
		send(connId, serialize::toByteArray(clId, 1), true);
	}

private:
	QList<StateListener *> stateListener_;

	uint connDataId_;
	typedef QPair<C, QByteArray> ConnData;	// data, clientId
	QHash<uint, ConnData> connData_;
	QHash<uint, uint> connId2dataId_;
	QMap<QByteArray, uint> clientIds_;
};

}}	// namespace
