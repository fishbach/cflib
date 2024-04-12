/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
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

namespace cflib { namespace net {

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

    using RMIServerBase::exportTo;

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

    virtual void handleMsg(quint64,
        const QByteArray & data, int tagLen, int lengthSize, qint32 valueLen,
        const C & connData, uint connDataId, uint connId)
    {
        handleCall(data, (const quint8 *)data.constData() + tagLen + lengthSize, valueLen, connData, connDataId, connId);
    }

    virtual void connDataChange(const C &, const C & newConnData, uint connDataId, const QSet<uint> & connIds)
    {
        RMIServerBase::connDataChange(newConnData, connDataId, connIds);
    }

    virtual void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast)
    {
        RMIServerBase::connectionClosed(connData, connDataId, connId, isLast);
    }

protected:
    virtual void handleRequest(const Request & request) { RMIServerBase::handleRequest(request); }
};

}}    // namespace
