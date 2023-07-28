/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
