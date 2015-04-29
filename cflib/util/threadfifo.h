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
	ThreadFifo(int size) : buffer_(new T[size]), max_(size) {}
	~ThreadFifo() { delete[] buffer_; }

	inline bool put(T data) {
		int c = writeCount_.fetchAndAddAcquire(1);
		if (c >= max_) {
			writeCount_.fetchAndSubRelaxed(1);
			return T();
		}
		int o = writer_.load();
		while (!writer_.testAndSetRelease(o, (o + 1) % max_)) o = writer_.load();
		buffer_[o] = data;
		readCount_.fetchAndAddRelease(1);
		return true;
	}

	inline T take() {
		int c = readCount_.fetchAndSubAcquire(1);
		if (c <= 0) {
			readCount_.fetchAndAddRelaxed(1);
			return T();
		}
		int o = reader_.load();
		while (!reader_.testAndSetRelease(o, (o + 1) % max_)) o = reader_.load();
		T rv = buffer_[o];
		writeCount_.fetchAndSubRelease(1);
		return rv;
	}

private:
	T * buffer_;
	const int max_;
	QAtomicInt readCount_;
	QAtomicInt writeCount_;
	QAtomicInt reader_;
	QAtomicInt writer_;
};

}}	// namespace
