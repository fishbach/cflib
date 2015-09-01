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
	virtual void newClient(uint connId) { Q_UNUSED(connId) }
	virtual void connDataChange(const C & connData, uint connId) { Q_UNUSED(connData) Q_UNUSED(connId) }
};

template <class C>
class WSCommTextMsgHandler
{
public:
	virtual bool handleTextMsg(const QByteArray & data, const C & connData, uint connId)
	{
		Q_UNUSED(data) Q_UNUSED(connData) Q_UNUSED(connId)
		return false;
	}
};

template <class C>
class WSCommMsgHandler
{
public:
	virtual void handleMsg(quint64 tag,
		const QByteArray & data, int tagLen, int lengthSize, qint32 valueLen,
		const C & connData, uint connId)
	{
		Q_UNUSED(tag)
		Q_UNUSED(data) Q_UNUSED(tagLen) Q_UNUSED(lengthSize) Q_UNUSED(valueLen)
		Q_UNUSED(connData) Q_UNUSED(connId)
	}
};

template <class C>
class WSCommManager : public WebSocketService
{
	USE_LOG_MEMBER(LogCat::Http)
public:
	typedef WSCommStateListener <C> StateListener;
	typedef WSCommTextMsgHandler<C> TextMsgHandler;
	typedef WSCommMsgHandler    <C> MsgHandler;

public:
	WSCommManager(const QString & path, uint connectionTimeoutSec = 30) :
		WebSocketService(path, connectionTimeoutSec),
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
				close(connId, TCPConn::HardClosed);
				logInfo("request without clientId from %1", connId);
				return;
			}

			QListIterator<TextMsgHandler *> it(textMsgHandler_);
			while (it.hasNext()) {
				if (it.next()->handleTextMsg(data, connData_[dataId].first, connId)) return;
			}
			close(connId, TCPConn::HardClosed);
			logInfo("unhandled text message from %1", connId);
			return;
		}

		// read outer BER
		quint64 tag;
		int tagLen;
		int lengthSize;
		const qint32 valueLen = serialize::getTLVLength(data, tag, tagLen, lengthSize);
		if (valueLen < 0) {
			close(connId, TCPConn::HardClosed);
			logInfo("broken BER msg %1 (%2)", connId, valueLen);
			return;
		}
		logTrace("ws msg (connId: %1, tag: %2, valueLen: %3)", connId, tag, valueLen);

		// handle new connections
		if (dataId == 0) {
			switch (tag) {
				case 1:
					sendNewClientId(connId);
					return;
				case 2: {
					const QByteArray clId = serialize::fromByteArray<QByteArray>(data, tagLen, lengthSize, valueLen);
					const uint dId = clientIds_.value(clId);
					if (dId == 0) sendNewClientId(connId);
					else          connId2dataId_[connId] = dId;
					return;
				}
				default:
					close(connId, TCPConn::HardClosed);
					logInfo("request without clientId from %1", connId);
					return;
			}
		}

		if (tag < 10) {
			close(connId, TCPConn::HardClosed);
			logInfo("invalid management request from %1", connId);
			return;
		}

		// forwarding
		if (channels_.contains(TagConnId(tag, connId))) {
			QSetIterator<uint> it(channels_[TagConnId(tag, connId)]);
			while (it.hasNext()) send(it.next(), data, true);
			return;
		}

		// handler
		MsgHandler * hdl = msgHandler_.value(tag);
		if (hdl) {
			hdl->handleMsg(tag, data, tagLen, lengthSize, valueLen, connData_[dataId].first, connId);
			return;
		}

		close(connId, TCPConn::HardClosed);
		logInfo("unhandled message from %1 (tag: %2)", connId, tag);
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
		connId2dataId_[connId] = dId;
		send(connId, serialize::toByteArray(clId, 1), true);
	}

private:
	QList<StateListener *> stateListener_;
	QList<TextMsgHandler *> textMsgHandler_;
	QHash<quint64, MsgHandler *> msgHandler_;

	typedef QPair<quint64, uint> TagConnId;
	typedef QSet<uint> ConnIds;
	QHash<TagConnId, ConnIds> channels_;

	uint connDataId_;
	typedef QPair<C, QByteArray> ConnData;	// data, clientId
	QHash<uint, ConnData> connData_;
	QHash<uint, uint> connId2dataId_;
	QMap<QByteArray, uint> clientIds_;
};

}}	// namespace
