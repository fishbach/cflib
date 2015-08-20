/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cflib/util/log.h>

#ifdef Q_OS_WIN32
	#pragma warning( push )
	#pragma warning( disable : 4267 )
#endif

#include <botan/auto_rng.h>
#include <botan/bcrypt.h>
#include <botan/filters.h>
#include <botan/init.h>
#include <botan/pkcs8.h>
#include <botan/tls_client.h>
#include <botan/tls_server.h>

#ifdef Q_OS_WIN32
	#pragma warning( pop )
#endif

#define TRY \
	try
#define CATCH \
	catch (std::exception & e) { \
		logWarn("Botan exception: %1", e.what()); \
	} catch (...) { \
		logWarn("unknown Botan exception"); \
	}

using namespace Botan;
