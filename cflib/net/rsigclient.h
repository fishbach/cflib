/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/util/sig.h>

#define rsigs \
    public: \
        template<typename F, typename R> class rsig : public cflib::net::RSigClient<F, R> { \
        public: \
            using cflib::net::RSigClient<F, R>::RSigClient; \
        }; \
    public

namespace cflib::net {

class RMIClient;

class RSigClientBase
{
public:
    RSigClientBase(RMIClient & client, const String & service, const String & name);

    void registerRSig();
    void unregisterRSig();

    RMIClient & rmiClient() { return client_; }
    const String & service() const { return service_; }
    const String & name() const { return name_; }
    uint id() const { return id_; }

protected:
    ~RSigClientBase();

private:
    RMIClient & client_;
    const String service_;
    const String name_;
    uint id_ = 0;
};

template<typename F, typename R> class RSigClient;

template<typename... P, typename... R>
class RSigClient<void (P...), void (R...)> : public RSigClientBase, public util::Sig<void (P...)>
{
public:
    typedef util::Sig<void (P...)> Base;

public:
    RSigClient(RMIClient & client, const String & service, const String & name);
    ~RSigClient();

    using RSigClientBase::RSigClientBase;
};

template<typename... P, typename... R>
RSigClient<void (P...), void (R...)>::RSigClient(RMIClient & client, const String & service, const String & name)
    : RSigClientBase(client, service, name)
{
    // registerRSig();
}

template<typename... P, typename... R>
RSigClient<void (P...), void (R...)>::~RSigClient()
{
    unregisterRSig();
}

} // namespace
