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

#include <QtCore>

// Threadsafe Fifo
// Many threads can put simultaneously
// Only one is allowed to take.

namespace cflib { namespace util {

template<typename T>
class ThreadFifo
{
public:
	ThreadFifo(int size) : buffer_(size), max_(size) {}

	inline bool put(T data) {
		int o, w;
		do {
			o = writer_;
			w = (o + 1) % max_;
			if (w == reader_) return false;
		} while (!writer_.testAndSetOrdered(o, w));
		El & el = (El &)buffer_[o];
		el.data = data;
		el.filled.storeRelease(1);
		return true;
	}

	inline T take() {
		const int r = reader_;
		if (r == writer_) return 0;
		El & el = (El &)buffer_[r];
		if (el.filled.loadAcquire() == 0) return 0;
		T rv = el.data;
		el.data = 0;
		el.filled.storeRelease(0);
		reader_.storeRelease((r + 1) % max_);
		return rv;
	}

private:
	struct El {
		QAtomicInt filled;
		T data;
		El() : filled(false), data(0) {}
	};
	const QVector<El> buffer_;	// const for performance (no Qt ref counting)
	const int max_;
	QAtomicInt reader_;
	QAtomicInt writer_;
};

}}	// namespace
