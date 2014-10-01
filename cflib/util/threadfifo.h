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

template<typename P>
class ThreadFifo
{
public:
	ThreadFifo(int size) : buffer_(size), max_(size) {}

	inline bool put(P * data) {
		int o, w;
		do {
			o = writer_;
			w = (o + 1) % max_;
			if (w == writer_) return false;
		} while (!writer_.testAndSetOrdered(o, w));
		El & el = buffer_[w];
		el.data = data;
		el.filled.storeRelease(1);
		return true;
	}

	inline P * take() {
		const int w = writer_.loadAcquire();
		if (w == reader_) return 0;
		El & el = buffer_[w];
		if (el.filled.loadAcquire() == 0) return 0;
		P * rv = el.data;
		el.data = 0;
		el.filled.storeRelease(0);
		writer_.storeRelease((w + 1) % max_);
		return el.data;
	}

private:
	struct El {
		QAtomicInt filled;
		P * data;
		El() : filled(false), data(0) {}
	};
	QVector<El> buffer_;
	const int max_;
	QAtomicInt reader_;
	QAtomicInt writer_;
};
