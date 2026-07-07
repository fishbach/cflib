/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/util.h>
#include <cflib/util/sig.h>
#include <cflib/util/tuplecompare.h>

#include <cflib/base.h>

namespace cflib::net {

namespace impl { class RMIServerBase; }

class RSigBase
{
public:
    typedef Pair<uint64, uint64>  ConnIdRegId;
    typedef List<ConnIdRegId>     Listeners;
    typedef Hash<uint, Listeners> ConnDataIdRegs;

public:
    RSigBase() : server_(0) {}

    virtual void   regClient(uint connDataId, uint connId, serialize::BERDeserializer & deser) = 0;
    virtual void unregClient(uint connDataId, uint connId, serialize::BERDeserializer & deser) = 0;

    ConnDataIdRegs defaultListeners;

protected:
    void send(uint connId, const ByteArray & data);

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

    void regClient(uint connDataId, uint connId, serialize::BERDeserializer & deser) override
    {
        uint64 regId; deser >> regId;
        if (!registerFunc_) {
            defaultListeners[connDataId] << ConnIdRegId(connId, regId);
            return;
        }
        serialize::readAndCall<R...>(deser, [this, connDataId, connId, regId](R... r) {
            registerFunc_(connDataId, connId, regId, std::forward<R>(r)...);
        });
    }

    void unregClient(uint connDataId, uint connId, serialize::BERDeserializer & deser) override
    {
        uint64 regId; deser >> regId;
        if (!unregisterFunc_) {
            Listeners & regs = defaultListeners[connDataId];
            for (auto it = regs.begin() ; it != regs.end() ; ) {
                if (it->first == connId && (regId == 0 || it->second == regId)) {
                    it = regs.erase(it);
                } else {
                    ++it;
                }
            }
            if (regs.isEmpty()) defaultListeners.erase(connDataId);
            return;
        }
        unregisterFunc_(connDataId, connId, regId);
    }

    template<typename F>
    void setRegisterFunction(F func)
    {
        registerFunc_ = func;
    }

    template<typename C>
    void setRegisterFunction(C * obj, void (C::*func)(uint connDataId, uint connId, uint regId, R...))
    {
        registerFunc_ = util::Delegate<C *, void, uint, uint, uint, R...>(obj, func);
    }

    template<typename C>
    void setRegisterFunction(const C * obj, void (C::*func)(uint connDataId, uint connId, uint regId, R...) const)
    {
        registerFunc_ = util::Delegate<const C *, void, uint, uint, uint, R...>(obj, func);
    }

    template<typename F>
    void setUnregisterFunction(F func)
    {
        unregisterFunc_ = func;
    }

    template<typename C>
    void setUnregisterFunction(C * obj, void (C::*func)(uint connDataId, uint connId, uint regId))
    {
        unregisterFunc_ = util::Delegate<C *, void, uint, uint, uint>(obj, func);
    }

    template<typename C>
    void setUnregisterFunction(const C * obj, bool (C::*func)(uint connDataId, uint connId, uint regId) const)
    {
        unregisterFunc_ = util::Delegate<const C *, void, uint, uint, uint>(obj, func);
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

    RSig & to(uint connDataId)
    {
        filterFunc_ = [listeners = defaultListeners.value(connDataId)](P...) {
            return listeners;
        };
        return *this;
    }

private:
    void handleRemote(P... p)
    {
        ByteArray encodedParams;
        {
            serialize::BERSerializer ser;
            serialize::toByteArray(ser, std::forward<P>(p)...);
            encodedParams = ser.data();
        }
        for (const ConnIdRegId & dest : (filterFunc_ ?
                filterFunc_(std::forward<P>(p)...) :
                defaultListeners.values().concatenated()))
        {
            serialize::BERSerializer ser(3);
            ser << dest.second << encodedParams;
            send(dest.first, ser.data());
        }
    }

private:
    std::function<void (uint connDataId, uint connId, uint regId, R... r)> registerFunc_;
    std::function<void (uint connDataId, uint connId, uint regId)>         unregisterFunc_;
    std::function<Listeners (P... p)> filterFunc_;
};

} // namespace
