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

namespace cflib { namespace util { namespace impl {

template<typename T> struct RemoveConstRef            { typedef T Type; };
template<typename T> struct RemoveConstRef<const T &> { typedef T Type; };

// ============================================================================

class Functor
{
public:
	virtual ~Functor() {}
	virtual void operator()() const = 0;
};

// ============================================================================

template<typename C>
class Functor0 : public Functor
{
	typedef void (C::*F)();
public:
	Functor0(C * o, F f, QSemaphore * sem = 0) :
		o_(o), f_(f), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)();
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	QSemaphore * sem_;
};

template<typename C, typename R>
class RFunctor0 : public Functor
{
	typedef R (C::*F)();
public:
	RFunctor0(C * o, F f, R & r, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)();
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	QSemaphore * sem_;
};

// ============================================================================

template<typename C, typename P1>
class Functor1 : public Functor
{
	typedef void (C::*F)(P1);
	typedef typename RemoveConstRef<P1>::Type A1;
public:
	Functor1(C * o, F f, P1 a1, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	A1 a1_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1>
class RFunctor1 : public Functor
{
	typedef R (C::*F)(P1);
	typedef typename RemoveConstRef<P1>::Type A1;
public:
	RFunctor1(C * o, F f, R & r, P1 a1, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	A1 a1_;
	QSemaphore * sem_;
};


// ============================================================================

template<typename C, typename P1, typename P2>
class Functor2 : public Functor
{
	typedef void (C::*F)(P1, P2);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
public:
	Functor2(C * o, F f, P1 a1, P2 a2, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2>
class RFunctor2 : public Functor
{
	typedef R (C::*F)(P1, P2);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
public:
	RFunctor2(C * o, F f, R & r, P1 a1, P2 a2, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	QSemaphore * sem_;
};

// ============================================================================

template<typename C, typename P1, typename P2, typename P3>
class Functor3 : public Functor
{
	typedef void (C::*F)(P1, P2, P3);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
public:
	Functor3(C * o, F f, P1 a1, P2 a2, P3 a3, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	QSemaphore * sem_;
};

// ============================================================================

template<typename C, typename P1, typename P2, typename P3, typename P4>
class Functor4 : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
public:
	Functor4(C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	QSemaphore * sem_;
};

// ============================================================================

class ThreadHolderEvent : public QEvent
{
public:
	ThreadHolderEvent(const impl::Functor * func) :
		QEvent(QEvent::User),
		func(func)
	{}

	const impl::Functor * func;
};

class ThreadObject : public QObject
{
public:
	virtual bool event(QEvent * event);
};

class ThreadHolder : public QThread
{
public:
	ThreadHolder(const QString & threadName);

	bool isActive() const { return isActive_; }
	const QString threadName;
	ThreadObject * threadObject;

protected:
    virtual void run();

private:
	bool isActive_;
};

}}}	// namespace
