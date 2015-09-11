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
	virtual void newConnection(C & connData, uint connDataId, uint connId);
	virtual void connDataChange(const C & connData, uint connDataId);
	virtual void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast);
};

// ----------------------------------------------------------------------------

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
		const C & connData, uint connDataId, uint connId) = 0;
};

// ----------------------------------------------------------------------------

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

public:
	WSCommManager(const QString & path, uint connectionTimeoutSec = 30, uint sessionTimeoutSec = 86400);
	~WSCommManager();

	void registerStateListener (StateListener & listener)      { stateListener_ << &listener; }
	void registerTextMsgHandler(TextMsgHandler & hdl)          { textMsgHandler_ << &hdl; }
	void registerMsgHandler    (quint64 tag, MsgHandler & hdl) { msgHandler_[tag] = &hdl; }
	void updateConnData(uint connDataId, const C & connData);
	void updateChannel(quint64 tag, uint srcConnId, const ConnIds & dstConnIds);

protected:
	virtual void newMsg(uint connId, const QByteArray & data, bool isBinary);
	virtual void closed(uint connId, TCPConn::CloseType type);

private:
	void init();
	uint sendNewClientId(uint connId);
	void checkTimeout();

private:
	class ConnInfo {
	public:
		C connData;
		ConnIds connIds;
		QDateTime lastClosed;
	};

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
void WSCommStateListener<C>::newConnection(C & connData, uint connDataId, uint connId)
{
	Q_UNUSED(connData) Q_UNUSED(connDataId) Q_UNUSED(connId)
}

template<typename C>
void WSCommStateListener<C>::connDataChange(const C & connData, uint connDataId)
{
	Q_UNUSED(connData) Q_UNUSED(connDataId)
}

template<typename C>
void WSCommStateListener<C>::connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast)
{
	Q_UNUSED(connData) Q_UNUSED(connDataId) Q_UNUSED(connId) Q_UNUSED(isLast)
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
void WSCommManager<C>::updateConnData(uint connDataId, const C & connData)
{
	if (!verifyThreadCall(&WSCommManager<C>::updateConnData, connDataId, connData)) return;

	if (!connData_.contains(connDataId)) return;
	connData_[connDataId].connData = connData;

	// inform state listener
	QListIterator<StateListener *> it(stateListener_);
	while (it.hasNext()) it.next()->connDataChange(connData, connDataId);
}

template<typename C>
void WSCommManager<C>::updateChannel(quint64 tag, uint srcConnId, const ConnIds & dstConnIds)
{
	if (!verifyThreadCall(&WSCommManager<C>::updateChannel, tag, srcConnId, dstConnIds)) return;

	if (dstConnIds.isEmpty()) channels_.remove(TagConnId(tag, srcConnId));
	else channels_[TagConnId(tag, srcConnId)] = dstConnIds;
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
	quint64 tag = 0;
	int tagLen = 0;
	int lengthSize = 0;
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
			uint dId;
			if (valueLen != 20) {
				dId = sendNewClientId(connId);
			} else {
				const QByteArray clId = serialize::fromByteArray<QByteArray>(data, tagLen, lengthSize, valueLen);
				dId = clientIds_.value(clId);
				if (dId == 0) {
					dId = sendNewClientId(connId);
				} else {
					connId2dataId_[connId] = dId;
					connData_[dId].connIds << connId;
				}
			}

			// inform state listener
			QListIterator<StateListener *> it(stateListener_);
			while (it.hasNext()) it.next()->newConnection(connData_[dId].connData, dId, connId);
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
		hdl->handleMsg(tag, data, tagLen, lengthSize, valueLen, connData_[dataId].connData, dataId, connId);
		return;
	}

	close(connId, TCPConn::HardClosed);
	logInfo("unhandled message from %1 (tag: %2)", connId, tag);
}

template<typename C>
void WSCommManager<C>::closed(uint connId, TCPConn::CloseType)
{
	// no partial close on websockets
	close(connId, TCPConn::ReadWriteClosed);

	// Do we know anything?
	const uint dataId = connId2dataId_.value(connId);
	if (dataId == 0) return;

	connId2dataId_.remove(connId);
	ConnInfo & info = connData_[dataId];
	info.connIds.remove(connId);
	const bool isLast = info.connIds.isEmpty();
	if (isLast) info.lastClosed = QDateTime::currentDateTime();

	// inform state listener
	QListIterator<StateListener *> it(stateListener_);
	while (it.hasNext()) it.next()->connectionClosed(info.connData, dataId, connId, isLast);
}

template<typename C>
void WSCommManager<C>::init()
{
	if (!verifyThreadCall(&WSCommManager::init)) return;
	timer_.start(sessionTimeoutSec_ / 10.0);
}

template<typename C>
uint WSCommManager<C>::sendNewClientId(uint connId)
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
	return dataId;
}

template<typename C>
void WSCommManager<C>::checkTimeout()
{
	logFunctionTrace

	QSet<uint> removedIds;
	{
		const QDateTime now = QDateTime::currentDateTime();
		QMutableHashIterator<uint, ConnInfo> it(connData_);
		while (it.hasNext()) {
			ConnInfo & info = it.next().value();
			if (info.connIds.isEmpty() && info.lastClosed.secsTo(now) > sessionTimeoutSec_) {
				removedIds << it.key();
				it.remove();
			}
		}
	}

	if (!removedIds.isEmpty()) {
		QMutableMapIterator<QByteArray, uint> it(clientIds_);
		while (it.hasNext()) if (removedIds.contains(it.next().value())) it.remove();
		logDebug("timeout of %1 sessions", removedIds.size());
	}
}

}}	// namespace
