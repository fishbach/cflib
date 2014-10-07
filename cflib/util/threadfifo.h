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
// put and take from multiple threads possible

namespace cflib { namespace util {

template<typename T>
class ThreadFifo
{
	Q_DISABLE_COPY(ThreadFifo)
public:
	ThreadFifo(int size) : buffer_(size), max_(size) {}

	inline bool put(T * data) {
		int o, n;
		El * el;
		do {
			o = writer_;
			n = (o + 1) % max_;
			el = (El *)&buffer_[o];
			if (n == reader_ || el->filled.loadAcquire() == 1) return false;
		} while (!writer_.testAndSetOrdered(o, n));
		el->data = data;
		el->filled.storeRelease(1);
		return true;
	}

	inline T * take() {
		int o, n;
		El * el;
		do {
			o = reader_;
			n = (o + 1) % max_;
			el = (El *)&buffer_[o];
			if (o == writer_ || el->filled.loadAcquire() == 0) return 0;
		} while (!reader_.testAndSetOrdered(o, n));
		T * rv = el->data;
		el->data = 0;
		el->filled.storeRelease(0);
		return rv;
	}

private:
	struct El {
		QAtomicInt filled;
		T * data;
		El() : filled(0), data(0) {}
	};
	const QVector<El> buffer_;	// const for performance (no Qt ref counting)
	const int max_;
	QAtomicInt reader_;
	QAtomicInt writer_;
};

}}	// namespace
