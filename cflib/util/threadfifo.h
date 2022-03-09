/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
	ThreadFifo(int size) : max_(size), buffer_(new T[size]), count_(0), reader_(0), writer_(0) {}
	~ThreadFifo() { delete[] buffer_; }

	inline bool put(T data) {
		while (!sl_.testAndSetAcquire(0, 1));
		if (count_ == max_) { sl_.storeRelease(0); return false; }
		++count_;
		buffer_[writer_] = data;
		writer_ = (writer_ + 1) % max_;
		sl_.storeRelease(0);
		return true;
	}

	inline T take() {
		while (!sl_.testAndSetAcquire(0, 1));
		if (count_ == 0) { sl_.storeRelease(0); return T(); }
		--count_;
		T rv = buffer_[reader_];
		reader_ = (reader_ + 1) % max_;
		sl_.storeRelease(0);
		return rv;
	}

private:
	const int max_;
	volatile T * buffer_;
	volatile int count_;
	volatile int reader_;
	volatile int writer_;
	QAtomicInt sl_;
};

}}	// namespace
