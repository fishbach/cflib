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

#include "util.h"

#include <cflib/crypt/impl/botanhelper.h>

USE_LOG(LogCat::Crypt)

namespace {

static const struct BotanInit
{
	BotanInit() : init_(0)
	{
		// Logging is not yet initialized. So we can only write to stderr.
		try {
			init_ = new LibraryInitializer("thread_safe=true");
		} catch(std::exception & e) {
			QTextStream(stderr) << "Botan exception: " << e.what() << endl;
		} catch (...) {
			QTextStream(stderr) << "unknown Botan exception" << endl;
		}
	}
	~BotanInit()
	{
		delete init_;
	}
private:
	LibraryInitializer * init_;
} botanInit;

}

namespace cflib { namespace crypt {

QByteArray random(int size)
{
	TRY {
		AutoSeeded_RNG rng;
		QByteArray retval(size, '\0');
		rng.randomize((byte *)retval.constData(), size);
		return retval;
	} CATCH
	return QByteArray();
}

quint32 randomUInt32()
{
	TRY {
		AutoSeeded_RNG rng;
		quint32 retval = 0;
		rng.randomize((byte *)&retval, 4);
		return retval;
	} CATCH
	return 0;
}

QByteArray hashPassword(const QString & password)
{
	TRY {
		AutoSeeded_RNG rng;
		std::string hash = generate_bcrypt(password.toStdString(), rng);
		return QByteArray(hash.c_str(), hash.length());
	} CATCH
	return QByteArray();
}

bool checkPassword(const QString & password, const QByteArray & hash)
{
	TRY {
		return check_bcrypt(password.toStdString(), std::string(hash.constData(), hash.length()));
	} CATCH
	return false;
}

}}	// namespace
