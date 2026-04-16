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
    public RequestHandler,
    public WSCommMsgHandler<C>,
    public WSCommStateListener<C>,
    private impl::RMIServerBase
{
public:
    RMIServer(WSCommManager<C> & commMgr) : RMIServerBase(commMgr) {
        commMgr.registerMsgHandler(2, *this);
        commMgr.registerStateListener(*this);
    }

    using RMIServerBase::getServiceTypeInfos;

    void registerService(RMIServiceBase & serviceBase)
    {
        RMIServerBase::registerService(serviceBase);
        {
            WSCommConnDataChecker<C> * service = dynamic_cast<WSCommConnDataChecker<C> *>(&serviceBase);
            if (service) this->commMgr().setConnDataChecker(*service);
        }
        {
            WSCommStateListener<C> * service = dynamic_cast<WSCommStateListener<C> *>(&serviceBase);
            if (service) this->commMgr().registerStateListener(*service);
        }
    }

    void handleMsg(uint64,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        const C & connData, uint connDataId, uint connId) override
    {
        handleCall(data, (const uint8 *)data.constData() + tagLen + lengthSize, valueLen, connData, connDataId, connId);
    }

    void connDataChange(const C &, const C & newConnData, uint connDataId, const Set<uint> & connIds) override
    {
        RMIServerBase::connDataChange(newConnData, connDataId, connIds);
    }

    void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast) override
    {
        RMIServerBase::connectionClosed(connData, connDataId, connId, isLast);
    }

protected:
    void handleRequest(const Request & request) override { RMIServerBase::handleRequest(request); }
};

} // namespace
