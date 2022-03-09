/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/util/test.h>
#include <cflib/util/threadfifo.h>

#include <functional>
#include <random>

using namespace cflib::util;

namespace {

class Worker : public QThread
{
public:
	Worker(ThreadFifo<int> & fifo, uint seed) : fifo_(fifo), rnd_(seed), ok_(true) { start(); }

	bool ok() {
		wait();
		return ok_;
	}

protected:
	virtual void run()
	{
		std::uniform_int_distribution<int> dist1(0, 3);
		std::uniform_int_distribution<int> dist2(0, 127);
		auto rnd1 = std::bind(dist1, rnd_);
		auto rnd2 = std::bind(dist2, rnd_);
		int count = 0;
		for (int i = 0 ; i < 10000 && ok_; ++i) {
			int max = rnd1() == 0 ? 127 : rnd2();
			if (rnd1() < 2) {
				max -= count;
				if (max > 0) {
					for (int j = 0 ; j < max ; ++j) ok_ = ok_ && fifo_.put(1);
					count += max;
				}
			} else if (count > 0) {
				if (max > count) max = count;
				for (int j = 0 ; j < max ; ++j) ok_ = ok_ && fifo_.take() == 1;
				count -= max;
			}
		}
	}

private:
	ThreadFifo<int> & fifo_;
	std::minstd_rand rnd_;
	bool ok_;
};

}

class ThreadFifo_Test: public QObject
{
	Q_OBJECT
private slots:

	void basic_test()
	{
		ThreadFifo<int> fifo(1023);
		for (int i =    1 ; i <= 1023 ; ++i) QVERIFY(fifo.put(i));
		QVERIFY(!fifo.put(1024));
		for (int i =    1 ; i <= 1023 ; ++i) QCOMPARE(fifo.take(), i);
		QCOMPARE(fifo.take(), 0);
		for (int i =    1 ; i <=  500 ; ++i) QVERIFY(fifo.put(i));
		for (int i =    1 ; i <=  200 ; ++i) QCOMPARE(fifo.take(), i);
		for (int i =  501 ; i <= 1223 ; ++i) QVERIFY(fifo.put(i));
		QVERIFY(!fifo.put(1224));
		for (int i =  201 ; i <=  923 ; ++i) QCOMPARE(fifo.take(), i);
		for (int i = 1224 ; i <= 1723 ; ++i) QVERIFY(fifo.put(i));
		for (int i =  924 ; i <= 1723 ; ++i) QCOMPARE(fifo.take(), i);
		QCOMPARE(fifo.take(), 0);
	}

	void thread_test()
	{
		ThreadFifo<int> fifo(1024);
		Worker w1(fifo, 1);
		Worker w2(fifo, 2);
		Worker w3(fifo, 3);
		Worker w4(fifo, 4);
		Worker w5(fifo, 5);
		Worker w6(fifo, 6);
		Worker w7(fifo, 7);
		Worker w8(fifo, 8);
		int i = 0;
		if (w1.ok()) ++i;
		if (w2.ok()) ++i;
		if (w3.ok()) ++i;
		if (w4.ok()) ++i;
		if (w5.ok()) ++i;
		if (w6.ok()) ++i;
		if (w7.ok()) ++i;
		if (w8.ok()) ++i;
		QCOMPARE(i, 8);
	}

};
#include "threadfifo_test.moc"
ADD_TEST(ThreadFifo_Test)
