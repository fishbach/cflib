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
				if (it.next()->handleTextMsg(data, connData_.value(connId).first, connId)) break;
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
			case 1: {
				sendNewClientId(connId);
				return;
			}
			case 2: {
				QByteArray clId; serialize::fromByteArray<QByteArray>(data, tlvLen, tagLen, lengthSize);
				ConnData & cd = connData_[connId];

				// any change?
				if (cd.second == clId) return;

				// does the id exist?
				if (!clientIds_.contains(clId)) {
					sendNewClientId(connId);
					return;
				}

				clientIds_[clId] << connId;
				if (!cd.second.isNull()) {
					clientIds_[cd.second].remove(connId);
				}
				return;
			}
		}

		// only with clientId from here on
		ConnData & cd = connData_[connId];
		if (cd.second.isNull()) return;


	}

	virtual void closed(uint connId, TCPConn::CloseType type)
	{
		if (!(type & TCPConn::ReadClosed) || !(type & TCPConn::WriteClosed)) return;
		ConnData & cd = connData_[connId];
		if (!cd.second.isEmpty()) clientIds_[cd.second].remove(connId);
		connData_.remove(connId);
	}

private:
	void sendNewClientId(uint connId)
	{
		QByteArray clId = crypt::random(20);
		clientIds_[clId] << connId;
		ConnData & cd = connData_[connId];
		cd.second = clId;
		send(connId, serialize::toByteArray(clId, 1), true);
	}

private:
	QList<StateListener *> stateListener_;
	typedef QPair<C, QByteArray> ConnData;	// data, clientId
	QHash<uint, ConnData> connData_;
	QHash<QByteArray, QSet<uint> > clientIds_;
};

}}	// namespace
