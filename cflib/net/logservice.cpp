/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "logservice.h"

USE_LOG(LogCat::Http)

namespace cflib::net {

LogService::LogService() :
    RMIServiceBase(serializeTypeInfo().typeName)
{
}

LogService::~LogService()
{
    stopVerifyThread();
}

void LogService::log(const String & file, int line, cflib::util::LogCategory category, const String & str)
{
    // remove evil chars
    ByteArray fileBa = file.toUtf8();
    for (int i = 0 ; i < fileBa.length() ; ++i) {
        uint8 c = (uint8)fileBa[i];
        if (c < 0x20 || c > 0x7E) fileBa[i] = '_';
    }

    cflib::util::Log(cflib::util::LogFileInfo(fileBa.constData(), LogCat::JS),
        line, category)("%1", str.toUtf8());
}

} // namespace
