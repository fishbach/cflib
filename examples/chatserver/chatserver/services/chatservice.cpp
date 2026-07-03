/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "chatservice.h"

namespace chatserver::services {

ChatService::ChatService() :
    RMIService(serializeTypeInfo().typeName)
{
}

ChatService::~ChatService()
{
    stopVerifyThread();
}

void ChatService::sendMessage(const dao::Message & message)
{
    newMessage(message);
}

}
