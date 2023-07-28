/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/log.h>

#ifdef Q_OS_WIN32
	#pragma warning( push )
	#pragma warning( disable : 4267 )
#endif

#include <botan/auto_rng.h>
#include <botan/bcrypt.h>
#include <botan/der_enc.h>
#include <botan/filters.h>
#include <botan/pem.h>
#include <botan/pkcs10.h>
#include <botan/pkcs8.h>
#include <botan/pubkey.h>
#include <botan/rsa.h>
#include <botan/tls.h>
#include <botan/x509_ca.h>
#include <botan/x509_ext.h>
#include <botan/x509_key.h>

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

template<typename T>
std::shared_ptr<T> detachedShared(T & obj)
{
	return std::shared_ptr<T>(std::shared_ptr<T>{}, &obj);
}
