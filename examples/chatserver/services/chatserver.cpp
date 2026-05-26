/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "chatserver.h"

namespace services {

ChatService::ChatService() :
    RMIServiceBase(serializeTypeInfo().typeName)
{
}

ChatService::~ChatService()
{
    stopVerifyThread();
}

void ChatService::sendMessage(const String & message)
{
    newMessage(message);
}

}
