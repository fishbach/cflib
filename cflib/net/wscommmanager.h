/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

template<typename C> class WSCommManager;

template<typename C>
class WSCommConnMgrAccess
{
public:
	WSCommConnMgrAccess() : mgr_(0) {}

protected:
	inline WSCommManager<C> & commMgr() { return *mgr_; }

private:
	WSCommManager<C> * mgr_;
	friend class WSCommManager<C>;
};

template<typename C>
class WSCommConnDataChecker : public virtual WSCommConnMgrAccess<C>
{
public:
	virtual void checkConnData(const C & connData, uint connDataId) = 0;
};

template<typename C>
class WSCommStateListener : public virtual WSCommConnMgrAccess<C>
{
public:
	virtual void newConnection(const C & connData, uint connDataId, uint connId);
	virtual void connDataChange(const C & oldConnData, const C & newConnData, uint connDataId, const QSet<uint> & connIds);
	virtual void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast);
};

// ----------------------------------------------------------------------------

template<typename C>
class WSCommTextMsgHandler : public virtual WSCommConnMgrAccess<C>
{
public:
	virtual bool handleTextMsg(const QByteArray & data, const C & connData, uint connId) = 0;
};

template<typename C>
class WSCommMsgHandler : public virtual WSCommConnMgrAccess<C>
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
	WSCommManagerBase(const QString & path, const QRegularExpression & allowedOrigin, uint connectionTimeoutSec);
};

/*
 * Tags:
 * 1 : client id (s <-> c)
 * 2 : rmi       (s <-> c)
 * 3 : rsig      (s  -> c)
 */

template<typename C>
class WSCommManager : public WSCommManagerBase
{
	USE_LOG_MEMBER(LogCat::Http)
public:
	typedef WSCommConnDataChecker<C> ConnDataChecker;
	typedef WSCommStateListener  <C> StateListener;
	typedef WSCommTextMsgHandler <C> TextMsgHandler;
	typedef WSCommMsgHandler     <C> MsgHandler;
	typedef QSet<uint> ConnIds;

public:
	WSCommManager(const QString & path, const QRegularExpression & allowedOrigin = QRegularExpression(),
		uint connectionTimeoutSec = 30, uint sessionTimeoutSec = 86400);
	~WSCommManager();

	void setConnDataChecker(ConnDataChecker & checker)     { connDataChecker_ = &checker; checker.mgr_ = this; }
	void registerStateListener (StateListener & listener)  { stateListener_ << &listener; listener.mgr_ = this; }
	void registerTextMsgHandler(TextMsgHandler & hdl)      { textMsgHandler_ << &hdl; hdl.mgr_ = this; }
	void registerMsgHandler(quint64 tag, MsgHandler & hdl) { msgHandler_[tag] = &hdl; hdl.mgr_ = this; }
	void updateConnData(uint connDataId, const C & connData);
	void connDataOk(uint connDataId);
	void getConnData(const QByteArray & clientId, C & connData, uint & connDataId);

protected:
	virtual void newMsg(uint connId, const QByteArray & data, bool isBinary, bool & stopRead);
	virtual void closed(uint connId, TCPConn::CloseType type);

private:
	class ConnInfo {
	public:
		ConnInfo() : waitingForCheck(true), connDataVerified(true) {}
		C connData;
		ConnIds connIds;
		QDateTime lastClosed;
		bool waitingForCheck;
		bool connDataVerified;
	};

private:
	void init();
	uint sendNewClientId(uint connId);
	void checkTimeout();
	bool connDataOk(ConnInfo & info, uint connDataId);

private:
	ConnDataChecker * connDataChecker_;
	QList<StateListener *> stateListener_;
	QList<TextMsgHandler *> textMsgHandler_;
	QHash<quint64, MsgHandler *> msgHandler_;

	QHash<uint, uint> connId2dataId_;
	QHash<uint, ConnInfo> connData_;
	QMap<QByteArray, uint> clientIds_;

	util::EVTimer timer_;
	uint sessionTimeoutSec_;
};

// ============================================================================

template<typename C>
void WSCommStateListener<C>::newConnection(const C & connData, uint connDataId, uint connId)
{
	Q_UNUSED(connData) Q_UNUSED(connDataId) Q_UNUSED(connId)
}

template<typename C>
void WSCommStateListener<C>::connDataChange(const C & oldConnData, const C & newConnData,
	uint connDataId, const QSet<uint> & connIds)
{
	Q_UNUSED(oldConnData) Q_UNUSED(newConnData) Q_UNUSED(connDataId) Q_UNUSED(connIds)
}

template<typename C>
void WSCommStateListener<C>::connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast)
{
	Q_UNUSED(connData) Q_UNUSED(connDataId) Q_UNUSED(connId) Q_UNUSED(isLast)
}

// ----------------------------------------------------------------------------

template<typename C>
WSCommManager<C>::WSCommManager(const QString & path, const QRegularExpression & allowedOrigin,
	uint connectionTimeoutSec, uint sessionTimeoutSec)
:
	WSCommManagerBase(path, allowedOrigin, connectionTimeoutSec),
	connDataChecker_(0),
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
	ConnInfo & info = connData_[connDataId];
	C oldConnData = info.connData;
	info.connData = connData;
	if (connDataOk(info, connDataId)) {
		// inform state listener
		QListIterator<StateListener *> it(stateListener_);
		while (it.hasNext()) it.next()->connDataChange(oldConnData, connData, connDataId, info.connIds);
	}
}

template<typename C>
void WSCommManager<C>::connDataOk(uint connDataId)
{
	if (!verifyThreadCall(&WSCommManager<C>::connDataOk, connDataId)) return;

	connDataOk(connData_[connDataId], connDataId);
}

template<typename C>
void WSCommManager<C>::getConnData(const QByteArray & clientId, C & connData, uint & connDataId)
{
	if (!verifySyncedThreadCall(&WSCommManager<C>::getConnData, clientId, connData, connDataId)) return;

	connDataId = clientIds_.value(clientId);
	connData = connDataId == 0 ? C() : connData_.value(connDataId).connData;
}

template<typename C>
bool WSCommManager<C>::connDataOk(WSCommManager::ConnInfo & info, uint connDataId)
{
	if (info.connDataVerified) return true;
	info.connDataVerified = true;
	foreach (uint connId, info.connIds) {
		// inform state listener
		QListIterator<StateListener *> it(stateListener_);
		while (it.hasNext()) it.next()->newConnection(info.connData, connDataId, connId);

		continueRead(connId);
	}
	return false;
}

template<typename C>
void WSCommManager<C>::newMsg(uint connId, const QByteArray & data, bool isBinary, bool & stopRead)
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
					ConnInfo & info = connData_[dId];
					info.connIds << connId;
					if (!info.connDataVerified) {
						if (!connDataChecker_) {
							info.connDataVerified = true;
						} else {
							stopRead = true;
							connDataChecker_->checkConnData(info.connData, dId);
							return;
						}
					}
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

	// handler
	MsgHandler * hdl = msgHandler_.value(tag);
	if (hdl) {
		hdl->handleMsg(tag, data, tagLen, lengthSize, valueLen, connData_[dataId].connData, dataId, connId);
		return;
	}

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
	if (isLast) {
		info.connDataVerified = false;
		info.lastClosed = QDateTime::currentDateTime();
	}

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
	uint dataId;
	do {
		dataId = crypt::randomUInt32();
	} while (dataId == 0 || connData_.contains(dataId));

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
