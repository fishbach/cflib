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
#include <cflib/util/evtimer.h>
#include <cflib/util/log.h>

namespace cflib { namespace net {

template<typename C>
class WSCommStateListener
{
public:
	virtual void newClient(uint connId);
	virtual void connDataChange(const C & connData, uint connId);
};

template<typename C>
class WSCommTextMsgHandler
{
public:
	virtual bool handleTextMsg(const QByteArray & data, const C & connData, uint connId) = 0;
};

template<typename C>
class WSCommMsgHandler
{
public:
	virtual void handleMsg(quint64 tag,
		const QByteArray & data, int tagLen, int lengthSize, qint32 valueLen,
		const C & connData, uint connId) = 0;
};

class WSCommManagerBase : public WebSocketService
{
public:
	void send(uint connId, const QByteArray & data, bool isBinary);
	void close(uint connId, TCPConn::CloseType type = TCPConn::ReadWriteClosed);
	QByteArray getRemoteIP(uint connId);

protected:
	WSCommManagerBase(const QString & path, uint connectionTimeoutSec);
};

template<typename C>
class WSCommManager : public WSCommManagerBase
{
	USE_LOG_MEMBER(LogCat::Http)
public:
	typedef WSCommStateListener <C> StateListener;
	typedef WSCommTextMsgHandler<C> TextMsgHandler;
	typedef WSCommMsgHandler    <C> MsgHandler;
	typedef QSet<uint> ConnIds;

	class ConnInfo {
	public:
		C connData;
		ConnIds connIds;
		QDateTime lastClosed;
	};

public:
	WSCommManager(const QString & path, uint connectionTimeoutSec = 30, uint sessionTimeoutSec = 86400);
	~WSCommManager();

	void registerMsgHandler(quint64 tag, MsgHandler & hdl) { msgHandler_[tag] = &hdl; }

protected:
	virtual void newConnection(uint connId);
	virtual void newMsg(uint connId, const QByteArray & data, bool isBinary);
	virtual void closed(uint connId, TCPConn::CloseType type);

private:
	void init();
	void sendNewClientId(uint connId);
	void checkTimeout();

private:
	QList<StateListener *> stateListener_;
	QList<TextMsgHandler *> textMsgHandler_;
	QHash<quint64, MsgHandler *> msgHandler_;

	typedef QPair<quint64, uint> TagConnId;
	QHash<TagConnId, ConnIds> channels_;

	uint connDataId_;
	QHash<uint, uint> connId2dataId_;
	QHash<uint, ConnInfo> connData_;
	QMap<QByteArray, uint> clientIds_;

	util::EVTimer timer_;
	uint sessionTimeoutSec_;
};

// ============================================================================

template<typename C>
void WSCommStateListener<C>::newClient(uint connId)
{
	Q_UNUSED(connId)
}

template<typename C>
void WSCommStateListener<C>::connDataChange(const C & connData, uint connId)
{
	Q_UNUSED(connData) Q_UNUSED(connId)
}

// ----------------------------------------------------------------------------

template<typename C>
WSCommManager<C>::WSCommManager(const QString & path, uint connectionTimeoutSec, uint sessionTimeoutSec) :
	WSCommManagerBase(path, connectionTimeoutSec),
	connDataId_(0),
	timer_(this, &WSCommManager::checkTimeout), sessionTimeoutSec_(sessionTimeoutSec)
{
	init();
}

template<typename C>
WSCommManager<C>::~WSCommManager()
{
	stopVerifyThread();
}

template<typename C>
void WSCommManager<C>::newConnection(uint connId)
{
	QListIterator<StateListener *> it(stateListener_);
	while (it.hasNext()) it.next()->newClient(connId);
}

template<typename C>
void WSCommManager<C>::newMsg(uint connId, const QByteArray & data, bool isBinary)
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
			if (it.next()->handleTextMsg(data, connData_[dataId].connData, connId)) return;
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
		if (tag == 1) {
			if (valueLen != 20) {
				sendNewClientId(connId);
			} else {
				const QByteArray clId = serialize::fromByteArray<QByteArray>(data, tagLen, lengthSize, valueLen);
				const uint dId = clientIds_.value(clId);
				if (dId == 0) {
					sendNewClientId(connId);
				} else {
					connId2dataId_[connId] = dId;
					connData_[dId].connIds << connId;
				}
			}
		} else {
			close(connId, TCPConn::HardClosed);
			logInfo("request without clientId from %1", connId);
		}
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
		hdl->handleMsg(tag, data, tagLen, lengthSize, valueLen, connData_[dataId].connData, connId);
		return;
	}

	close(connId, TCPConn::HardClosed);
	logInfo("unhandled message from %1 (tag: %2)", connId, tag);
}

template<typename C>
void WSCommManager<C>::closed(uint connId, TCPConn::CloseType)
{
	close(connId, TCPConn::ReadWriteClosed);
	const uint dataId = connId2dataId_.value(connId);
	if (dataId == 0) return;
	connId2dataId_.remove(connId);
	ConnInfo & info = connData_[dataId];
	info.connIds.remove(connId);
	if (info.connIds.isEmpty()) {
		info.lastClosed = QDateTime::currentDateTime();

		// msg
	}
}

template<typename C>
void WSCommManager<C>::init()
{
	if (!verifyThreadCall(&WSCommManager::init)) return;
	timer_.start(sessionTimeoutSec_ / 10.0);
}

template<typename C>
void WSCommManager<C>::sendNewClientId(uint connId)
{
	// create clientId
	const QByteArray clId = crypt::random(20);

	// get free id
	uint dataId = ++connDataId_;
	while (connData_.contains(dataId)) dataId = ++connDataId_;

	connId2dataId_[connId] = dataId;
	connData_[dataId].connIds << connId;
	clientIds_[clId] = dataId;
	send(connId, serialize::toByteArray(clId, 1), true);
}

template<typename C>
void WSCommManager<C>::checkTimeout()
{
	const QDateTime now = QDateTime::currentDateTime();
	QSet<uint> removedIds;

	QMutableHashIterator<uint, ConnInfo> it(connData_);
	while (it.hasNext()) {
		ConnInfo & info = it.next().value();
		if (info.connIds.isEmpty() && info.lastClosed.secsTo(now) > sessionTimeoutSec_) {
			removedIds << it.key();
			it.remove();
		}
	}

	if (!removedIds.isEmpty()) {
		QMutableMapIterator<QByteArray, uint> it2(clientIds_);
		while (it2.hasNext()) if (removedIds.contains(it2.next().value())) it2.remove();
		logDebug("timeout of %1 sessions", removedIds.size());
	}
}

}}	// namespace
