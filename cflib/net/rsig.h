/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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
	typedef QPair<uint, uint> ConnIdRegId;
	typedef QVector<ConnIdRegId> Listeners;

public:
	RSigBase() : server_(0) {}

	virtual void regClient(uint connId, serialize::BERDeserializer & deser) = 0;
	virtual void unregClient(uint connId, serialize::BERDeserializer & deser) = 0;

	Listeners defaultListeners;

protected:
	void send(uint connId, const QByteArray & data);

private:
	impl::RMIServerBase * server_;
	friend class impl::RMIServerBase;
};

template<typename F, typename R> class RSig;

template<typename... P, typename... R>
class RSig<void (P...), void (R...)> : public RSigBase, public util::Sig<void (P...)>
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
		uint regId; deser >> regId;
		if (!registerFunc_) {
			defaultListeners << qMakePair(connId, regId);
			return;
		}
		serialize::readAndCall<R...>(deser, [this, connId, regId](R... r) {
			registerFunc_(connId, regId, std::forward<R>(r)...);
		});
	}

	virtual void unregClient(uint connId, serialize::BERDeserializer & deser)
	{
		uint regId; deser >> regId;
		if (!unregisterFunc_) {
			defaultListeners.removeAll(qMakePair(connId, regId));
			return;
		}
		unregisterFunc_(connId, regId);
	}

	template<typename F>
	void setRegisterFunction(F func)
	{
		registerFunc_ = func;
	}

	template<typename C>
	void setRegisterFunction(C * obj, void (C::*func)(uint connId, uint regId, R...))
	{
		registerFunc_ = util::Delegate<C *, void, uint, uint, R...>(obj, func);
	}

	template<typename C>
	void setRegisterFunction(const C * obj, void (C::*func)(uint connId, uint regId, R...) const)
	{
		registerFunc_ = util::Delegate<const C *, void, uint, uint, R...>(obj, func);
	}

	template<typename F>
	void setUnregisterFunction(F func)
	{
		unregisterFunc_ = func;
	}

	template<typename C>
	void setUnregisterFunction(C * obj, void (C::*func)(uint connId, uint regId))
	{
		unregisterFunc_ = util::Delegate<C *, void, uint, uint>(obj, func);
	}

	template<typename C>
	void setUnregisterFunction(const C * obj, bool (C::*func)(uint connId, uint regId) const)
	{
		unregisterFunc_ = util::Delegate<const C *, void, uint, uint>(obj, func);
	}

	template<typename F>
	void setFilterFunction(F func)
	{
		filterFunc_ = func;
	}

	template<typename C>
	void setFilterFunction(C * obj, Listeners (C::*func)(P...))
	{
		filterFunc_ = util::Delegate<C *, Listeners, P...>(obj, func);
	}

	template<typename C>
	void setFilterFunction(const C * obj, Listeners (C::*func)(P...) const)
	{
		filterFunc_ = util::Delegate<const C *, Listeners, P...>(obj, func);
	}

private:
	void handleRemote(P... p)
	{
		QByteArray encodedParams;
		{
			serialize::BERSerializer ser;
			serialize::toByteArray(ser, std::forward<P>(p)...);
			encodedParams = ser.data();
		}
		for (const ConnIdRegId & dest : (filterFunc_ ? filterFunc_(std::forward<P>(p)...) : defaultListeners)) {
			serialize::BERSerializer ser(3);
			ser << dest.second << encodedParams;
			send(dest.first, ser.data());
		}
	}

private:
	std::function<void (uint connId, uint regId, R... r)> registerFunc_;
	std::function<void (uint connId, uint regId)> unregisterFunc_;
	std::function<Listeners (P... p)> filterFunc_;
};

}}	// namespace
