/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/rmiremoteservice.h>
#include <cflib/serialize/util.h>
#include <cflib/util/sig.h>

#define rsigs \
    public: \
        template<typename F, typename R> class rsig : public cflib::net::RSigClient<F, R> { \
        public: \
            using cflib::net::RSigClient<F, R>::RSigClient; \
        }; \
    public

namespace cflib::net {

class RMIRemoteService;

class RSigClientBase
{
    CF_DISABLE_COPY(RSigClientBase)
public:
    RSigClientBase(RMIRemoteService & service, const String & name);
    ~RSigClientBase();

    void unreg();
    virtual void call(const ByteArray & paramsData) = 0;

protected:
    RMIRemoteService & service_;
    const String name_;
    uint64 id_ = 0;
};

template<typename F, typename R> class RSigClient;

template<typename... P, typename... R>
class RSigClient<void (P...), void (R...)> : public RSigClientBase, public util::Sig<void (P...)>
{
public:
    typedef util::Sig<void (P...)> Base;

public:
    RSigClient(RMIRemoteService & service, const String & name);

    RSigClient & reg(R... p);

    void call(const ByteArray & paramsData) override;
};

template<typename... P, typename... R>
RSigClient<void (P...), void (R...)>::RSigClient(RMIRemoteService & service, const String & name) :
    RSigClientBase(service, name)
{
}

template<typename... P, typename... R>
RSigClient<void (P...), void (R...)> & RSigClient<void (P...), void (R...)>::reg(R... p)
{
    if (id_) return *this;
    id_ = service_.nextRSigId();
    serialize::BERSerializer ser = service_.getSer();
    ser << name_ << true << id_;
    serialize::toByteArray(ser, std::forward<R>(p)...);
    service_.registerRSig(this, id_, ser.data());
    return *this;
}

template<typename... P, typename... R>
void RSigClient<void (P...), void (R...)>::call(const ByteArray & paramsData)
{
    serialize::readAndCall<P...>(paramsData, (Base &)*this);
}

} // namespace
