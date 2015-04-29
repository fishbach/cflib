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

#include <cflib/util/impl/threadverifyimpl.h>
#include <cflib/util/timer.h>

namespace cflib { namespace util {

class ThreadVerify
{
public:
	enum LoopType {
		Qt     = 1,
		Net    = 2,
		Worker = 3
	};

public:
	ThreadVerify(const QString & threadName, LoopType loopType, uint threadCount = 1);
	ThreadVerify(ThreadVerify * other);
	virtual ~ThreadVerify();

	QObject * threadObject() const;
	void stopVerifyThread();
	ev_loop * libEVLoop() const;
	void execCall(const Functor * func) const;

protected:
	virtual void deleteThreadData() {}

	// ------------------------------------------------------------------------

	template<typename C>
	bool verifyThreadCall(void (C::*f)())
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor0<C>((C *)this, f));
		return false;
	}

	template<typename C>
	bool verifyThreadCall(void (C::*f)() const) const
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor0C<C>((const C *)this, f));
		return false;
	}

	template<typename C>
	bool verifySyncedThreadCall(void (C::*f)())
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor0<C>((C *)this, f, &sem));
		sem.acquire();
		return false;
	}

	template<typename C>
	bool verifySyncedThreadCall(void (C::*f)() const) const
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor0C<C>((const C *)this, f, &sem));
		sem.acquire();
		return false;
	}

	// ------------------------------------------------------------------------

	template<typename C, typename P1, typename A1>
	bool verifyThreadCall(void (C::*f)(P1), const A1 & a1)
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor1<C, P1>((C *)this, f, a1));
		return false;
	}

	template<typename C, typename P1, typename A1>
	bool verifyThreadCall(void (C::*f)(P1) const, const A1 & a1) const
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor1C<C, P1>((const C *)this, f, a1));
		return false;
	}

	template<typename C, typename P1, typename A1>
	bool verifySyncedThreadCall(void (C::*f)(P1), A1 & a1)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor1<C, P1>((C *)this, f, a1, &sem));
		sem.acquire();
		return false;
	}

	template<typename C, typename P1, typename A1>
	bool verifySyncedThreadCall(void (C::*f)(P1) const, A1 & a1) const
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor1C<C, P1>((const C *)this, f, a1, &sem));
		sem.acquire();
		return false;
	}

	// ------------------------------------------------------------------------

	template<typename C, typename P1, typename P2, typename A1, typename A2>
	bool verifyThreadCall(void (C::*f)(P1, P2), const A1 & a1, const A2 & a2)
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor2<C, P1, P2>((C *)this, f, a1, a2));
		return false;
	}

	template<typename C, typename P1, typename P2, typename A1, typename A2>
	bool verifyThreadCall(void (C::*f)(P1, P2) const, const A1 & a1, const A2 & a2) const
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor2C<C, P1, P2>((const C *)this, f, a1, a2));
		return false;
	}

	template<typename C, typename P1, typename P2, typename A1, typename A2>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2), A1 & a1, A2 & a2)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor2<C, P1, P2>((C *)this, f, a1, a2, &sem));
		sem.acquire();
		return false;
	}

	template<typename C, typename P1, typename P2, typename A1, typename A2>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2) const, A1 & a1, A2 & a2) const
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor2C<C, P1, P2>((const C *)this, f, a1, a2, &sem));
		sem.acquire();
		return false;
	}

	// ------------------------------------------------------------------------

	template<typename C, typename P1, typename P2, typename P3, typename A1, typename A2, typename A3>
	bool verifyThreadCall(void (C::*f)(P1, P2, P3), const A1 & a1, const A2 & a2, const A3 & a3)
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor3<C, P1, P2, P3>((C *)this, f, a1, a2, a3));
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename A1, typename A2, typename A3>
	bool verifyThreadCall(void (C::*f)(P1, P2, P3) const, const A1 & a1, const A2 & a2, const A3 & a3) const
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor3C<C, P1, P2, P3>((const C *)this, f, a1, a2, a3));
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename A1, typename A2, typename A3>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3), A1 & a1, A2 & a2, A3 & a3)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor3<C, P1, P2, P3>((C *)this, f, a1, a2, a3, &sem));
		sem.acquire();
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename A1, typename A2, typename A3>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3) const, A1 & a1, A2 & a2, A3 & a3) const
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor3C<C, P1, P2, P3>((const C *)this, f, a1, a2, a3, &sem));
		sem.acquire();
		return false;
	}

	// ------------------------------------------------------------------------

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename A1, typename A2, typename A3, typename A4>
	bool verifyThreadCall(void (C::*f)(P1, P2, P3, P4), const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4)
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor4<C, P1, P2, P3, P4>((C *)this, f, a1, a2, a3, a4));
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename A1, typename A2, typename A3, typename A4>
	bool verifyThreadCall(void (C::*f)(P1, P2, P3, P4) const, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4) const
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor4C<C, P1, P2, P3, P4>((const C *)this, f, a1, a2, a3, a4));
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename A1, typename A2, typename A3, typename A4>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3, P4), A1 & a1, A2 & a2, A3 & a3, A4 & a4)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor4<C, P1, P2, P3, P4>((C *)this, f, a1, a2, a3, a4, &sem));
		sem.acquire();
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename A1, typename A2, typename A3, typename A4>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3, P4) const, A1 & a1, A2 & a2, A3 & a3, A4 & a4) const
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor4C<C, P1, P2, P3, P4>((const C *)this, f, a1, a2, a3, a4, &sem));
		sem.acquire();
		return false;
	}

	// ------------------------------------------------------------------------

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename A1, typename A2, typename A3, typename A4, typename A5>
	bool verifyThreadCall(void (C::*f)(P1, P2, P3, P4, P5), const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4, const A5 & a5)
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor5<C, P1, P2, P3, P4, P5>((C *)this, f, a1, a2, a3, a4, a5));
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename A1, typename A2, typename A3, typename A4, typename A5>
	bool verifyThreadCall(void (C::*f)(P1, P2, P3, P4, P5) const, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4, const A5 & a5) const
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor5C<C, P1, P2, P3, P4, P5>((const C *)this, f, a1, a2, a3, a4, a5));
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename A1, typename A2, typename A3, typename A4, typename A5>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3, P4, P5), A1 & a1, A2 & a2, A3 & a3, A4 & a4, A5 & a5)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor5<C, P1, P2, P3, P4, P5>((C *)this, f, a1, a2, a3, a4, a5, &sem));
		sem.acquire();
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename A1, typename A2, typename A3, typename A4, typename A5>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3, P4, P5) const, A1 & a1, A2 & a2, A3 & a3, A4 & a4, A5 & a5) const
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor5C<C, P1, P2, P3, P4, P5>((const C *)this, f, a1, a2, a3, a4, a5, &sem));
		sem.acquire();
		return false;
	}

	// ------------------------------------------------------------------------

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	bool verifyThreadCall(void (C::*f)(P1, P2, P3, P4, P5, P6), const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4, const A5 & a5, const A6 & a6)
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor6<C, P1, P2, P3, P4, P5, P6>((C *)this, f, a1, a2, a3, a4, a5, a6));
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	bool verifyThreadCall(void (C::*f)(P1, P2, P3, P4, P5, P6) const, const A1 & a1, const A2 & a2, const A3 & a3, const A4 & a4, const A5 & a5, const A6 & a6) const
	{
		if (verifyThread_->isOwnThread()) return true;
		execCall(new Functor6C<C, P1, P2, P3, P4, P5, P6>((const C *)this, f, a1, a2, a3, a4, a5, a6));
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3, P4, P5, P6), A1 & a1, A2 & a2, A3 & a3, A4 & a4, A5 & a5, A6 & a6)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor6<C, P1, P2, P3, P4, P5, P6>((C *)this, f, a1, a2, a3, a4, a5, a6, &sem));
		sem.acquire();
		return false;
	}

	template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3, P4, P5, P6) const, A1 & a1, A2 & a2, A3 & a3, A4 & a4, A5 & a5, A6 & a6) const
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor6C<C, P1, P2, P3, P4, P5, P6>((const C *)this, f, a1, a2, a3, a4, a5, a6, &sem));
		sem.acquire();
		return false;
	}

	// ------------------------------------------------------------------------

	template<typename R>
	class SyncedThreadCall
	{
	public:
		SyncedThreadCall(const ThreadVerify * tv) : tv_(tv) {}
		inline R retval() const { return retval_; }

		template<typename C>
		bool verify(R (C::*f)())
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor0<C, R>((C *)tv_, f, retval_, &sem));
			sem.acquire();
			return false;
		}

		template<typename C>
		bool verify(R (C::*f)() const)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor0C<C, R>((const C *)tv_, f, retval_, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename A1>
		bool verify(R (C::*f)(P1), A1 & a1)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor1<C, R, P1>((C *)tv_, f, retval_, a1, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename A1>
		bool verify(R (C::*f)(P1) const, A1 & a1)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor1C<C, R, P1>((const C *)tv_, f, retval_, a1, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename A1, typename A2>
		bool verify(R (C::*f)(P1, P2), A1 & a1, A2 & a2)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor2<C, R, P1, P2>((C *)tv_, f, retval_, a1, a2, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename A1, typename A2>
		bool verify(R (C::*f)(P1, P2) const, A1 & a1, A2 & a2)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor2C<C, R, P1, P2>((const C *)tv_, f, retval_, a1, a2, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename P3, typename A1, typename A2, typename A3>
		bool verify(R (C::*f)(P1, P2, P3), A1 & a1, A2 & a2, A3 & a3)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor3<C, R, P1, P2, P3>((C *)tv_, f, retval_, a1, a2, a3, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename P3, typename A1, typename A2, typename A3>
		bool verify(R (C::*f)(P1, P2, P3) const, A1 & a1, A2 & a2, A3 & a3)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor3C<C, R, P1, P2, P3>((const C *)tv_, f, retval_, a1, a2, a3, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename P3, typename P4, typename A1, typename A2, typename A3, typename A4>
		bool verify(R (C::*f)(P1, P2, P3, P4), A1 & a1, A2 & a2, A3 & a3, A4 & a4)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor4<C, R, P1, P2, P3, P4>((C *)tv_, f, retval_, a1, a2, a3, a4, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename P3, typename P4, typename A1, typename A2, typename A3, typename A4>
		bool verify(R (C::*f)(P1, P2, P3, P4) const, A1 & a1, A2 & a2, A3 & a3, A4 & a4)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor4C<C, R, P1, P2, P3, P4>((const C *)tv_, f, retval_, a1, a2, a3, a4, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename A1, typename A2, typename A3, typename A4, typename A5>
		bool verify(R (C::*f)(P1, P2, P3, P4, P5), A1 & a1, A2 & a2, A3 & a3, A4 & a4, A5 & a5)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor5<C, R, P1, P2, P3, P4, P5>((C *)tv_, f, retval_, a1, a2, a3, a4, a5, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename A1, typename A2, typename A3, typename A4, typename A5>
		bool verify(R (C::*f)(P1, P2, P3, P4, P5) const, A1 & a1, A2 & a2, A3 & a3, A4 & a4, A5 & a5)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor5C<C, R, P1, P2, P3, P4, P5>((const C *)tv_, f, retval_, a1, a2, a3, a4, a5, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
		bool verify(R (C::*f)(P1, P2, P3, P4, P5, P6), A1 & a1, A2 & a2, A3 & a3, A4 & a4, A5 & a5, A6 & a6)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor6<C, R, P1, P2, P3, P4, P5, P6>((C *)tv_, f, retval_, a1, a2, a3, a4, a5, a6, &sem));
			sem.acquire();
			return false;
		}

		template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
		bool verify(R (C::*f)(P1, P2, P3, P4, P5, P6) const, A1 & a1, A2 & a2, A3 & a3, A4 & a4, A5 & a5, A6 & a6)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor6C<C, R, P1, P2, P3, P4, P5, P6>((const C *)tv_, f, retval_, a1, a2, a3, a4, a5, a6, &sem));
			sem.acquire();
			return false;
		}

	private:
		const ThreadVerify * tv_;
		R retval_;
	};

	// ------------------------------------------------------------------------

private:
	void shutdownThread();

private:
	impl::ThreadHolder * verifyThread_;
	const bool ownerOfVerifyThread_;
};

inline ev_loop * libEVLoopOfThread()
{
	const impl::ThreadHolderLibEV * thread = dynamic_cast<const impl::ThreadHolderLibEV *>(QThread::currentThread());
	if (!thread) return 0;
	return thread->loop();
}

}}	// namespace
