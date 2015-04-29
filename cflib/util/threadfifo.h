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
		int c = count_.fetchAndAddAcquire(1);
		if (c >= max_) {
			count_.fetchAndSubRelease(1);
			QTextStream(stdout) << "full" << endl;
			return false;
		}
		int o = writer_.load();
		while (!writer_.testAndSetRelease(o, (o + 1) % max_)) o = writer_.load();
		El * el = buffer_ + o;
		while (!el->filled.testAndSetAcquire(0, 1));
		el->data = data;
		el->filled.storeRelease(2);
		return true;
	}

	inline T take() {
		int c = count_.fetchAndSubAcquire(1);
		if (c <= 0) {
			count_.fetchAndAddRelease(1);
			QTextStream(stdout) << "empty" << endl;
			return T();
		}
		int o = reader_.load();
		while (!reader_.testAndSetRelease(o, (o + 1) % max_)) o = reader_.load();
		El * el = buffer_ + o;
		while (!el->filled.testAndSetAcquire(2, 3));
		T rv = el->data;
		el->data = T();
		el->filled.storeRelease(0);
		return rv;
	}

private:
	struct El {
		QAtomicInt filled;
		T data;
		El() : data() {}
	};
	El * buffer_;
	const int max_;
	QAtomicInt count_;
	QAtomicInt reader_;
	QAtomicInt writer_;
};

}}	// namespace
