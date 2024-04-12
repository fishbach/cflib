/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "logservice.h"

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

LogService::LogService() :
    RMIServiceBase(serializeTypeInfo().typeName)
{
}

LogService::~LogService()
{
    stopVerifyThread();
}

void LogService::log(const QString & file, int line, cflib::util::LogCategory category, const QString & str)
{
    // remove evil chars
    QByteArray fileBa = file.toUtf8();
    for (int i = 0 ; i < fileBa.length() ; ++i) {
        uchar c = (uchar)fileBa[i];
        if (c < 0x20 || c > 0x7E) fileBa[i] = '_';
    }

    cflib::util::Log(cflib::util::LogFileInfo(fileBa.constData(), LogCat::JS),
        line, category)("%1", str.toUtf8());
}

}}    // namespace
