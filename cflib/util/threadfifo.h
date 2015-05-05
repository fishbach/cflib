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
	ThreadFifo(int size) : buffer_(new T[size]), max_(size), count_(0), reader_(0), writer_(0) {}
	~ThreadFifo() { delete[] buffer_; }

	inline bool put(T data) {
		QMutexLocker lock(&mutex_);
		if (count_ == max_) return false;
		++count_;
		buffer_[writer_] = data;
		writer_ = (writer_ + 1) % max_;
		return true;
	}

	inline T take() {
		QMutexLocker lock(&mutex_);
		if (count_ == 0) return T();
		--count_;
		T rv = buffer_[reader_];
		reader_ = (reader_ + 1) % max_;
		return rv;
	}

private:
	T * buffer_;
	QMutex mutex_;
	const int max_;
	int count_;
	int reader_;
	int writer_;
};

}}	// namespace
