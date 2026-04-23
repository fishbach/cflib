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

class RMIRemoteService;

class RSigClientBase
{
public:
    RSigClientBase(RMIRemoteService & service, const String & name) : service_(service), name_(name) {}

private:
    RMIRemoteService & service_;
    const String name_;
};

template<typename F, typename R> class RSigClient;

template<typename... P, typename... R>
class RSigClient<void (P...), void (R...)> : public RSigClientBase, public util::Sig<void (P...)>
{
public:
    typedef util::Sig<void (P...)> Base;

public:
    using RSigClientBase::RSigClientBase;
};

} // namespace
