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

#include <cflib/serialize/util.h>
#include <cflib/util/sig.h>
#include <cflib/util/tuplecompare.h>

#include <QtCore>

namespace cflib { namespace net {

namespace impl { class RMIServerBase; }

class RSigBase
{
public:
	RSigBase() : server_(0) {}

	virtual void regClient(uint connId, serialize::BERDeserializer & deser) = 0;
	virtual void unregClient(uint connId, serialize::BERDeserializer & deser) = 0;
	virtual void unregClient(uint connId) = 0;
	virtual void checkClient(uint connId) = 0;

protected:
	QString serviceName_;
	QString sigName_;

	void send(uint connId, const QByteArray & data);

private:
	impl::RMIServerBase * server_;
	friend class impl::RMIServerBase;
};

template<typename> class RSig;

template<typename... P>
class RSig<void (P...)> : public RSigBase, public util::Sig<void (P...)>
{
public:
	typedef util::Sig<void (P...)> Base;
public:
	RSig()
	{
		Base::bind(this, &RSig::handleRemote);
	}

	void unbindAll()
	{
		Base::unbindAll();
		Base::bind(this, &RSig::handleRemote);
	}

	virtual void regClient(uint connId, serialize::BERDeserializer & deser)
	{
		RSig<void (P...)> * that = this;	// workaround for g++ 4.7.2 bug
		serialize::readAndCall<uint, P...>(deser, [that, connId](uint checkCount, P... p) {
			that->regClient(connId, checkCount, std::forward<P>(p)...);
		});
	}

	void regClient(uint connId, uint checkCount, P... p)
	{
		uint retStart = 0;
		uint retCount = sizeof...(P);
		if (checkRegister_ && !checkRegister_(retStart, retCount, checkCount, p...)) return;
		clients_ << ClData(connId, retStart, retCount, checkCount, std::forward<P>(p)...);
	}

	virtual void unregClient(uint connId, serialize::BERDeserializer & deser)
	{
		RSig<void (P...)> * that = this;	// workaround for g++ 4.7.2 bug
		serialize::readAndCall<uint, P...>(deser, [that, connId](uint checkCount, P... p) {
			that->unregClient(connId, checkCount, std::forward<P>(p)...);
		});
	}

	void unregClient(uint connId, uint checkCount, P... p)
	{
		QMutableVectorIterator<ClData> it(clients_);
		while (it.hasNext()) {
			const ClData & d = it.next();
			if (d.connId == connId && d.checkCount == checkCount &&
				util::partialEqual(d.params, d.checkCount, p...)) it.remove();
		}
	}

	virtual void unregClient(uint connId)
	{
		QMutableVectorIterator<ClData> it(clients_);
		while (it.hasNext()) {
			if (it.next().connId == connId) it.remove();
		}
	}

	virtual void checkClient(uint connId)
	{
		if (!checkRegister_) return;
		QMutableVectorIterator<ClData> it(clients_);
		while (it.hasNext()) {
			ClData & d = it.next();
			if (d.connId != connId) continue;
			if (!util::callWithTupleParams<bool>(checkRegister_, d.params, d.retStart, d.retCount, d.checkCount)) it.remove();
		}
	}

	template<typename F>
	void setRegisterCheck(F func)
	{
		checkRegister_ = func;
	}

	template<typename... A, typename C>
	void setRegisterCheck(C * obj, bool (C::*func)(A...))
	{
		checkRegister_ = util::Delegate<C *, bool, A...>(obj, func);
	}

	template<typename... A, typename C>
	void setRegisterCheck(const C * obj, bool (C::*func)(A...) const)
	{
		checkRegister_ = util::Delegate<const C *, bool, A...>(obj, func);
	}

private:
	void handleRemote(P... p)
	{
		QMap<QPair<uint, uint>, QByteArray> partials;
		QVectorIterator<ClData> it(clients_);
		while (it.hasNext()) {
			const ClData & d = it.next();
			if (!util::partialEqual(d.params, d.checkCount, p...)) continue;

			QByteArray & ba = partials[qMakePair(d.retStart, d.retCount)];
			if (ba.isNull()) {
				serialize::BERSerializer ser(3);
				ser << serviceName_ << sigName_;
				serialize::someToByteArray(ser, d.retStart, d.retCount, p...);
				ba = ser.data();
			}
			send(d.connId, ba);
		}
	}

private:
	struct ClData {
		uint connId;
		uint retStart;
		uint retCount;
		uint checkCount;
		std::tuple<typename std::decay<P>::type ...> params;

		ClData() : connId(0), retStart(0), retCount(0), checkCount(0) {}
		ClData(uint connId, uint retStart, uint retCount, uint checkCount, P... p) :
			connId(connId), retStart(retStart), retCount(retCount), checkCount(checkCount),
			params(std::forward<P>(p)...) {}
	};
	QVector<ClData> clients_;
	std::function<bool (uint & retStart, uint & retCount, uint & checkCount, P&... p)> checkRegister_;
};

}}	// namespace
