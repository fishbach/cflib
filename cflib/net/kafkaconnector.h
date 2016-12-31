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

#include <QtCore>

namespace cflib { namespace util  { class ThreadVerify; }}

namespace cflib { namespace net {

class KafkaConnector
{
	Q_DISABLE_COPY(KafkaConnector)
public:
	enum State
	{
		Error = -1,
		Idle = 1,
		Connecting,
		Ready
	};

	typedef QPair<QByteArray /* ip */, quint16 /* port */> Address;

public:
	KafkaConnector(util::ThreadVerify * other = 0);
	virtual ~KafkaConnector();

	void connect(const QByteArray & destAddress, quint16 destPort);
	void connect(const QList<Address> & cluster);

	void produce();

protected:
	virtual void stateChanged(State state) { Q_UNUSED(state) }

	virtual void produceReply() {}

private:
	class MetadataConnection;
	class Impl;
	Impl * impl_;
};

}}	// namespace
