/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/impl/rmiserverbase.h>
#include <cflib/net/requesthandler.h>
#include <cflib/net/rmiservice.h>
#include <cflib/net/wscommmanager.h>

namespace cflib::net {

template<typename C>
class RMIServer :
    public WSCommMsgHandler<C>,
    public WSCommStateListener<C>,
    private impl::RMIServerBase
{
public:
    RMIServer(WSCommManager<C> & commMgr) : RMIServerBase(commMgr) {
        commMgr.registerMsgHandler(2, *this);
        commMgr.registerStateListener(*this);
    }

    void registerService(RMIService<C> & service)
    {
        RMIServerBase::registerService(service);
        {
            WSCommConnDataChecker<C> * checker = dynamic_cast<WSCommConnDataChecker<C> *>(&service);
            if (checker) this->commMgr().setConnDataChecker(*checker);
        }
        {
            WSCommStateListener<C> * listener = dynamic_cast<WSCommStateListener<C> *>(&service);
            if (listener) this->commMgr().registerStateListener(*listener);
        }
    }

    void registerService(RMIService<void> & service)
    {
        RMIServerBase::registerService(service);
    }

    void handleMsg(uint64,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        const C & connData, uint connDataId, uint connId) override
    {
        handleCall(data, (const uint8 *)data.constCharPtr() + tagLen + lengthSize, valueLen, connData, connDataId, connId);
    }

    void connDataChange(const C &, const C & newConnData, uint connDataId, const Set<uint> & connIds) override
    {
        RMIServerBase::connDataChange(newConnData, connDataId, connIds);
    }

    void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast) override
    {
        RMIServerBase::connectionClosed(connData, connDataId, connId, isLast);
    }
};

// ----------------------------------------------------------------------------

template<>
class RMIServer<void> :
    public WSCommMsgHandler<void>,
    public WSCommStateListener<void>,
    private impl::RMIServerBase
{
public:
    RMIServer(WSCommManager<void> & commMgr) : RMIServerBase(commMgr) {
        commMgr.registerMsgHandler(2, *this);
        commMgr.registerStateListener(*this);
    }

    void registerService(RMIService<void> & service)
    {
        RMIServerBase::registerService(service);
        {
            WSCommConnDataChecker<void> * checker = dynamic_cast<WSCommConnDataChecker<void> *>(&service);
            if (checker) this->commMgr().setConnDataChecker(*checker);
        }
        {
            WSCommStateListener<void> * listener = dynamic_cast<WSCommStateListener<void> *>(&service);
            if (listener) this->commMgr().registerStateListener(*listener);
        }
    }

    void handleMsg(uint64,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        uint connDataId, uint connId) override
    {
        handleCall(data, (const uint8 *)data.constCharPtr() + tagLen + lengthSize, valueLen, connDataId, connId);
    }

    void connectionClosed(uint connDataId, uint connId, bool isLast) override
    {
        RMIServerBase::connectionClosed(connDataId, connId, isLast);
    }
};

} // namespace
