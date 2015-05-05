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
		// test if full
		forever {
			int c = writeCount_.load();
			if (c == max_) return false;
			if (writeCount_.testAndSetAcquire(c, c + 1)) break;
		}

		// search free block
		int w = writer_.load();
		while (!writer_.testAndSetAcquire(w, (w + 1) % max_)) w = writer_.load();
		El * el;
		forever {
			el = buffer_ + w;
			if (el->state.testAndSetAcquire(0, 1)) break;
			w = (w + 1) % max_;
		}

		// store
		el->data = data;
		el->state.storeRelease(2);
		readCount_.fetchAndAddRelease(1);
		return true;
	}

	inline T take() {
		// test for empty
		forever {
			int c = readCount_.load();
			if (c == 0) return T();
			if (readCount_.testAndSetAcquire(c, c - 1)) break;
		}

		// search filled block
		int r = reader_.load();
		while (!reader_.testAndSetAcquire(r, (r + 1) % max_)) r = reader_.load();
		El * el;
		forever {
			el = buffer_ + r;
			if (el->state.testAndSetAcquire(2, 3)) break;
			r = (r + 1) % max_;
		}

		// load
		T rv = el->data;
		el->data = T();
		el->state.storeRelease(0);
		writeCount_.fetchAndSubRelease(1);
		return rv;
	}

private:
	struct El {
		QAtomicInt state;
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
