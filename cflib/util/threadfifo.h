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
	ThreadFifo(int size) : buffer_(new El[size]), max_(size) {}
	~ThreadFifo() { delete[] buffer_; }

	inline bool put(T data) {
		forever {
			int c = writeCount_.load();
			if (c >= max_) return false;
			if (writeCount_.testAndSetAcquire(c, c + 1)) break;
		}
		int o = writer_.load();
		while (!writer_.testAndSetOrdered(o, (o + 1) % max_)) o = writer_.load();
		El * el = buffer_ + o;
		while (el->filled.loadAcquire() != 0);
		el->data = data;
		el->filled.fetchAndStoreRelease(1);
		readCount_.fetchAndAddRelease(1);
		return true;
	}

	inline T take() {
		forever {
			int c = readCount_.load();
			if (c <= 0) return T();
			if (readCount_.testAndSetAcquire(c, c - 1)) break;
		}
		int o = reader_.load();
		while (!reader_.testAndSetOrdered(o, (o + 1) % max_)) o = reader_.load();
		El * el = buffer_ + o;
		while (el->filled.loadAcquire() != 1);
		T rv = el->data;
		el->data = T();
		el->filled.fetchAndStoreRelease(0);
		writeCount_.fetchAndSubRelease(1);
		return rv;
	}

private:
	struct El {
		QAtomicInt filled;
		T data;
	};
	El * buffer_;
	const int max_;
	QAtomicInt readCount_;
	QAtomicInt writeCount_;
	QAtomicInt reader_;
	QAtomicInt writer_;
};

}}	// namespace
