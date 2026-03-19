/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "infoservice.h"

#include <cflib/util/log.h>

#include <format>
#include <iostream>

USE_LOG(LogCat::Compute)

InfoService::InfoService() :
    RMIService(serializeTypeInfo().typeName)
{
}

InfoService::~InfoService()
{
    stopVerifyThread();
}

String InfoService::test()
{
    return String("hello w\xc3\xb6rld");
}

String InfoService::test(const String & msg)
{
    logInfo("msg %1: %2", msg.charCount(), msg.c_str());
    std::cout << std::format("{}\n", msg.c_str());
    return msg;
}

void InfoService::async(cfint64 i)
{
    logInfo("async: %1", i);
}

Dao InfoService::update(const Dao & dao)
{
    Dao rv;
    rv.name = dao.name + "XX";
    rv.number = dao.number + 13;
    return rv;
}

void InfoService::update(Dao2 & dao)
{
    dao.numbers.push_back(3);
    dao.numbers.push_back(4);
    dao.numbers.push_back(5);
}

void InfoService::update(Dao3 & dao)
{
    dao.timestamp = CFDateTime::currentDateTimeUtc();
}

void InfoService::talk(const String & msg)
{
    std::cout << std::format("connId:{}    wrote: {}\n", connId(), msg.c_str());
    newMessage(connId(), msg);
}
