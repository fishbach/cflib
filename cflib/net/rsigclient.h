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
public:
    RSigClientBase(RMIRemoteService & service, const String & name);

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

    void reg(R... p);
};

template<typename... P, typename... R>
RSigClient<void (P...), void (R...)>::RSigClient(RMIRemoteService & service, const String & name) :
    RSigClientBase(service, name)
{
}

template<typename... P, typename... R>
void RSigClient<void (P...), void (R...)>::reg(R... p)
{
    id_ = service_.nextRSigId();
    serialize::BERSerializer ser = service_.getSer();
    ser << name_ << true << id_;
    serialize::toByteArray(ser, std::forward<P>(p)...);
    service_.sendRSigReg(ser.data());
}

} // namespace
