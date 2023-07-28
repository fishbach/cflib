/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace util {

template<typename T> struct RemoveConstRef            { typedef T Type; };
template<typename T> struct RemoveConstRef<const T &> { typedef T Type; };

// ============================================================================

class Functor
{
public:
	virtual ~Functor() {}
	virtual void operator()() const = 0;
};

template<typename C>
class Deleter : public Functor
{
public:
	Deleter(const C * obj) : obj_(obj) {}
	virtual void operator()() const { delete obj_; }
private:
	const C * obj_;
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

template<typename C>
class Functor0C : public Functor
{
	typedef void (C::*F)() const;
public:
	Functor0C(const C * o, F f, QSemaphore * sem = 0) :
		o_(o), f_(f), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)();
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	QSemaphore * sem_;
};

template<typename C, typename R>
class RFunctor0C : public Functor
{
	typedef R (C::*F)() const;
public:
	RFunctor0C(const C * o, F f, R & r, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)();
		if (sem_) sem_->release();
	}

private:
	const C * o_;
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

template<typename C, typename P1>
class Functor1C : public Functor
{
	typedef void (C::*F)(P1) const;
	typedef typename RemoveConstRef<P1>::Type A1;
public:
	Functor1C(const C * o, F f, P1 a1, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	A1 a1_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1>
class RFunctor1C : public Functor
{
	typedef R (C::*F)(P1) const;
	typedef typename RemoveConstRef<P1>::Type A1;
public:
	RFunctor1C(const C * o, F f, R & r, P1 a1, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
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

template<typename C, typename P1, typename P2>
class Functor2C : public Functor
{
	typedef void (C::*F)(P1, P2) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
public:
	Functor2C(const C * o, F f, P1 a1, P2 a2, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2>
class RFunctor2C : public Functor
{
	typedef R (C::*F)(P1, P2) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
public:
	RFunctor2C(const C * o, F f, R & r, P1 a1, P2 a2, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
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

template<typename C, typename R, typename P1, typename P2, typename P3>
class RFunctor3 : public Functor
{
	typedef R (C::*F)(P1, P2, P3);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
public:
	RFunctor3(C * o, F f, R & r, P1 a1, P2 a2, P3 a3, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	QSemaphore * sem_;
};

template<typename C, typename P1, typename P2, typename P3>
class Functor3C : public Functor
{
	typedef void (C::*F)(P1, P2, P3) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
public:
	Functor3C(const C * o, F f, P1 a1, P2 a2, P3 a3, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3>
class RFunctor3C : public Functor
{
	typedef R (C::*F)(P1, P2, P3) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
public:
	RFunctor3C(const C * o, F f, R & r, P1 a1, P2 a2, P3 a3, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	R & r_;
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

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4>
class RFunctor4 : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
public:
	RFunctor4(C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	QSemaphore * sem_;
};

template<typename C, typename P1, typename P2, typename P3, typename P4>
class Functor4C : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
public:
	Functor4C(const C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4>
class RFunctor4C : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
public:
	RFunctor4C(const C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	QSemaphore * sem_;
};

// ============================================================================

template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5>
class Functor5 : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4, P5);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
public:
	Functor5(C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_, a5_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class RFunctor5 : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4, P5);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
public:
	RFunctor5(C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_, a5_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	QSemaphore * sem_;
};

template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5>
class Functor5C : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4, P5) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
public:
	Functor5C(const C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_, a5_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class RFunctor5C : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4, P5) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
public:
	RFunctor5C(const C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_, a5_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	QSemaphore * sem_;
};

// ============================================================================

template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class Functor6 : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4, P5, P6);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
public:
	Functor6(C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class RFunctor6 : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4, P5, P6);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
public:
	RFunctor6(C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	QSemaphore * sem_;
};

template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class Functor6C : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4, P5, P6) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
public:
	Functor6C(const C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class RFunctor6C : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4, P5, P6) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
public:
	RFunctor6C(const C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	QSemaphore * sem_;
};

// ============================================================================

template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
class Functor7 : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4, P5, P6, P7);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
	typedef typename RemoveConstRef<P7>::Type A7;
public:
	Functor7(C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), a7_(a7), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_, a7_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	A7 a7_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
class RFunctor7 : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4, P5, P6, P7);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
	typedef typename RemoveConstRef<P7>::Type A7;
public:
	RFunctor7(C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), a7_(a7), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_, a7_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	A7 a7_;
	QSemaphore * sem_;
};

template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
class Functor7C : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4, P5, P6, P7) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
	typedef typename RemoveConstRef<P7>::Type A7;
public:
	Functor7C(const C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), a7_(a7), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_, a7_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	A7 a7_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
class RFunctor7C : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4, P5, P6, P7) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
	typedef typename RemoveConstRef<P7>::Type A7;
public:
	RFunctor7C(const C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), a7_(a7), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_, a7_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	A7 a7_;
	QSemaphore * sem_;
};

// ============================================================================

template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
class Functor8 : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4, P5, P6, P7, P8);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
	typedef typename RemoveConstRef<P7>::Type A7;
	typedef typename RemoveConstRef<P8>::Type A8;
public:
	Functor8(C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, P8 a8, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), a7_(a7), a8_(a8), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_, a7_, a8_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	A7 a7_;
	A8 a8_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
class RFunctor8 : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4, P5, P6, P7, P8);
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
	typedef typename RemoveConstRef<P7>::Type A7;
	typedef typename RemoveConstRef<P8>::Type A8;
public:
	RFunctor8(C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, P8 a8, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), a7_(a7), a8_(a8), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_, a7_, a8_);
		if (sem_) sem_->release();
	}

private:
	C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	A7 a7_;
	A8 a8_;
	QSemaphore * sem_;
};

template<typename C, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
class Functor8C : public Functor
{
	typedef void (C::*F)(P1, P2, P3, P4, P5, P6, P7, P8) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
	typedef typename RemoveConstRef<P7>::Type A7;
	typedef typename RemoveConstRef<P8>::Type A8;
public:
	Functor8C(const C * o, F f, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, P8 a8, QSemaphore * sem = 0) :
		o_(o), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), a7_(a7), a8_(a8), sem_(sem) {}

	virtual void operator()() const
	{
		(o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_, a7_, a8_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	A7 a7_;
	A8 a8_;
	QSemaphore * sem_;
};

template<typename C, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
class RFunctor8C : public Functor
{
	typedef R (C::*F)(P1, P2, P3, P4, P5, P6, P7, P8) const;
	typedef typename RemoveConstRef<P1>::Type A1;
	typedef typename RemoveConstRef<P2>::Type A2;
	typedef typename RemoveConstRef<P3>::Type A3;
	typedef typename RemoveConstRef<P4>::Type A4;
	typedef typename RemoveConstRef<P5>::Type A5;
	typedef typename RemoveConstRef<P6>::Type A6;
	typedef typename RemoveConstRef<P7>::Type A7;
	typedef typename RemoveConstRef<P8>::Type A8;
public:
	RFunctor8C(const C * o, F f, R & r, P1 a1, P2 a2, P3 a3, P4 a4, P5 a5, P6 a6, P7 a7, P8 a8, QSemaphore * sem = 0) :
		o_(o), f_(f), r_(r), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5), a6_(a6), a7_(a7), a8_(a8), sem_(sem) {}

	virtual void operator()() const
	{
		r_ = (o_->*f_)(a1_, a2_, a3_, a4_, a5_, a6_, a7_, a8_);
		if (sem_) sem_->release();
	}

private:
	const C * o_;
	F f_;
	R & r_;
	A1 a1_;
	A2 a2_;
	A3 a3_;
	A4 a4_;
	A5 a5_;
	A6 a6_;
	A7 a7_;
	A8 a8_;
	QSemaphore * sem_;
};

}}	// namespace
