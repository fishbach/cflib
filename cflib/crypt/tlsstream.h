/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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

#include <QtCore>

namespace cflib { namespace crypt {

class TLSStream
{
	Q_DISABLE_COPY(TLSStream)
public:
	TLSStream() {}
	virtual ~TLSStream() {}
	virtual QByteArray initialSend() = 0;
	virtual bool received(const QByteArray & encrypted, QByteArray & plain, QByteArray & sendBack) = 0;
	virtual bool send(const QByteArray & plain, QByteArray & encrypted) = 0;
};

}}	// namespace
