/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/rmiservice.h>
#include <cflib/util/log.h>

namespace cflib { namespace net {

class LogService : public RMIServiceBase
{
	SERIALIZE_CLASS
public:
	LogService();
	~LogService();

rmi:
	void log(const QString & file, int line, cflib::util::LogCategory category, const QString & str);
};

}}	// namespace
