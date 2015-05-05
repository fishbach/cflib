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
	ThreadFifo(int size) : buffer_(new El[size]), max_(size), count_(1) {
		buffer_[0].filled = 1;
	}
	~ThreadFifo() { delete[] buffer_; }

	inline bool put(T data) {
		// test for full
		forever {
			int c = count_.load();
			if (c == max_) { QTextStream(stdout) << "full" << endl; return false; }
			if (count_.testAndSetAcquire(c, c + 1)) break;
		}

		// search free block
		int w = writer_.load();
		int i = w;
		El * el;
		do {
			i = (i + 1) % max_;
			el = buffer_ + i;
		} while (!el->filled.testAndSetAcquire(0, 1));
		el->data = data;

		// insert new block
		forever {
			el = buffer_ + w;
			if (el->next.testAndSetOrdered(-1, i)) break;
//			int n = el->next.load();
//			w = writer_.testAndSetRelease(w, n) ? n : writer_.load();
			w = writer_.load();
		}
		writer_.testAndSetRelease(w, i);

		return true;
	}

	inline T take() {
		T rv;
		El * el;
		int r, n;
		forever {
			r = reader_.load();
			el = buffer_ + r;
			n = el->next.load();
			if (n == -1) { QTextStream(stdout) << "empty " << (int)count_ << endl; return T(); }
			rv = buffer_[n].data;
			if (reader_.testAndSetOrdered(r, n)) break;
		}

//		writer_.testAndSetAcquire(r, n);
		el->next.fetchAndStoreOrdered(-1);
		el->filled.fetchAndStoreRelease(0);
		count_.fetchAndSubRelease(1);

		return rv;
	}

private:
	struct El {
		QAtomicInt filled;
		QAtomicInt next;
		T data;
		El() : next(-1) {}
	};
	El * buffer_;
	const int max_;
	QAtomicInt count_;
	QAtomicInt reader_;
	QAtomicInt writer_;
};

}}	// namespace
