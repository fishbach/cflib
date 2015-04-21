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
	bool verifySyncedThreadCall(void (C::*f)())
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor0<C>((C *)this, f, &sem));
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
	bool verifySyncedThreadCall(void (C::*f)(P1), A1 & a1)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor1<C, P1>((C *)this, f, a1, &sem));
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
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3), A1 & a1, A2 & a2, A3 & a3)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor3<C, P1, P2, P3>((C *)this, f, a1, a2, a3, &sem));
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
	bool verifySyncedThreadCall(void (C::*f)(P1, P2, P3, P4), A1 & a1, A2 & a2, A3 & a3, A4 & a4)
	{
		if (verifyThread_->isOwnThread()) return true;
		QSemaphore sem;
		execCall(new Functor4<C, P1, P2, P3, P4>((C *)this, f, a1, a2, a3, a4, &sem));
		sem.acquire();
		return false;
	}

	// ------------------------------------------------------------------------

	template<class R>
	class SyncedThreadCall
	{
	public:
		SyncedThreadCall(ThreadVerify * tv) : tv_(tv) {}
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

		template<typename C, typename P1, typename A1>
		bool verify(R (C::*f)(P1), A1 & a1)
		{
			if (tv_->verifyThread_->isOwnThread()) return true;
			QSemaphore sem;
			tv_->execCall(new RFunctor1<C, R, P1>((C *)tv_, f, retval_, a1, &sem));
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

	private:
		ThreadVerify * tv_;
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
	const util::impl::ThreadHolderLibEV * thread = dynamic_cast<const impl::ThreadHolderLibEV *>(QThread::currentThread());
	if (!thread) return 0;
	return thread->loop();
}

}}	// namespace
