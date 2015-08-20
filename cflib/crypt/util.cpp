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

namespace cflib { namespace crypt {

QByteArray random(uint size)
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

quint64 randomUInt64()
{
	TRY {
		AutoSeeded_RNG rng;
		quint64 retval = 0;
		rng.randomize((byte *)&retval, 8);
		return retval;
	} CATCH
	return 0;
}

QByteArray memorableRandom()
{
	const char * vowels     = "aeiou";
	const char * consonants = "bcdfghjklmnpqrstvwxyz";
	const QByteArray rnd = random(8);
	if (rnd.size() != 8) return QByteArray();
	QByteArray rv(8, '\0');
	for (int i = 0 ; i < 6 ; ++i) rv[i] = (i % 2 == 0) ? consonants[(uchar)rnd[i] * 21 / 256] : vowels[(uchar)rnd[i] * 5 / 256];
	for (int i = 6 ; i < 8 ; ++i) rv[i] = '0' + ((uchar)rnd[i] * 10 / 256);
	return rv;
}

QByteArray hashPassword(const QString & password)
{
	TRY {
		AutoSeeded_RNG rng;
		std::string hash = generate_bcrypt(password.toStdString(), rng);
		return QByteArray(hash.c_str(), (int)hash.length());
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

QByteArray sha1(const QByteArray & data)
{
	TRY {
		Pipe pipe(new Hash_Filter("SHA-1"));
		pipe.process_msg((const byte *)data.constData(), data.size());
		std::string hash = pipe.read_all_as_string();
		return QByteArray(hash.c_str(), (int)hash.length());
	} CATCH
	return QByteArray();
}

}}	// namespace
