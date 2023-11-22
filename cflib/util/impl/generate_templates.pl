#!/usr/bin/env perl

# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

use strict; use warnings; use utf8;

my $maxParams = 8;

sub verifyThreadCall
{
    my ($paramCount, $sync, $const) = @_;

    my $synced = $sync ? "Synced" : "";
    my $sem1   = $sync ? "\n        QSemaphore sem;" : "";
    my $sem2   = $sync ? ", &sem" : "";
    my $sem3   = $sync ? "\n        sem.acquire();" : "";
    my $const1 = $const ? " const" : "";
    my $const2 = $const ? "const " : "";
    my $const3 = $const ? "C" : "";
    my $typesP = "";
    my $typesA = "";
    my $tempP1 = "";
    my $paramD = "";
    my $paramU = "";
    for (my $i = 1 ; $i <= $paramCount ; ++$i) {
        $typesP .= ", typename P$i";
        $typesA .= ", typename A$i";
        $tempP1 .= ", P$i";
        $paramD .= ", " . ($sync ? "" : "const ") . "A$i & a$i";
        $paramU .= ", a$i";
    }
    my $tempP2 = $paramCount > 0 ? substr($tempP1, 2) : '';

    print
<<"EOF"
    template<typename C$typesP$typesA>
    bool verify${synced}ThreadCall(void (C::*f)($tempP2)$const1$paramD)$const1
    {
        if (verifyThread_->isOwnThread()) return true;$sem1
        execCall(new Functor$paramCount$const3<C$tempP1>((${const2}C *)this, f$paramU$sem2));$sem3
        return false;
    }

EOF
}

sub verify
{
    my ($paramCount, $const) = @_;

    my $const1 = $const ? " const" : "";
    my $const2 = $const ? "const " : "";
    my $const3 = $const ? "C" : "";
    my $typesP = "";
    my $typesA = "";
    my $tempP1 = "";
    my $paramD = "";
    my $paramU = "";
    for (my $i = 1 ; $i <= $paramCount ; ++$i) {
        $typesP .= ", typename P$i";
        $typesA .= ", typename A$i";
        $tempP1 .= ", P$i";
        $paramD .= ", A$i & a$i";
        $paramU .= ", a$i";
    }
    my $tempP2 = $paramCount > 0 ? substr($tempP1, 2) : '';

    print
<<"EOF"
        template<typename C$typesP$typesA>
        bool verify(R (C::*f)($tempP2)$const1$paramD)
        {
            if (tv_->verifyThread_->isOwnThread()) return true;
            QSemaphore sem;
            tv_->execCall(new RFunctor$paramCount$const3<C, R$tempP1>((${const2}C *)tv_, f, retval_$paramU, &sem));
            sem.acquire();
            return false;
        }

EOF
}

sub genVerify
{
    for (my $i = 0 ; $i <= $maxParams ; ++$i) {
        verifyThreadCall($i, 0, 0);
        verifyThreadCall($i, 0, 1);
        verifyThreadCall($i, 1, 0);
        verifyThreadCall($i, 1, 1);
        print "    // ------------------------------------------------------------------------\n\n";
    }
    print
<<"EOF"
    template<typename R>
    class SyncedThreadCall
    {
    public:
        SyncedThreadCall(const ThreadVerify * tv) : tv_(tv) {}
        inline R retval() const { return retval_; }

EOF
;
    for (my $i = 0 ; $i <= $maxParams ; ++$i) {
        verify($i, 0);
        verify($i, 1);
    }
    print
<<"EOF"
    private:
        const ThreadVerify * tv_;
        R retval_;
    };
EOF
}

sub functor
{
    my ($paramCount, $const) = @_;

    my $const1  = $const ? " const" : "";
    my $const2  = $const ? "const " : "";
    my $const3  = $const ? "C" : "";
    my $typesP  = "";
    my $typesA  = "";
    my $tempP   = "";
    my $memberD = "";
    my $memberU = "";
    my $paramD  = "";
    my $paramU  = "";
    for (my $i = 1 ; $i <= $paramCount ; ++$i) {
        $typesP  .= ", typename P$i";
        $typesA  .= "\n    typedef typename RemoveConstRef<P$i>::Type A$i;";
        $tempP   .= ($i > 1 ? ", " : "") . "P$i";
        $memberD .= "\n    A$i a${i}_;";
        $memberU .= ($i > 1 ? ", " : "") . "a${i}_";
        $paramD  .= ", P$i a$i";
        $paramU  .= ", a${i}_(a$i)";
    }

    print
<<"EOF"
template<typename C$typesP>
class Functor$paramCount$const3 : public Functor
{
    typedef void (C::*F)($tempP)$const1;$typesA
public:
    Functor$paramCount$const3(${const2}C * o, F f$paramD, QSemaphore * sem = 0) :
        o_(o), f_(f)$paramU, sem_(sem) {}

    virtual void operator()() const
    {
        (o_->*f_)($memberU);
        if (sem_) sem_->release();
    }

private:
    ${const2}C * o_;
    F f_;$memberD
    QSemaphore * sem_;
};

template<typename C, typename R$typesP>
class RFunctor$paramCount$const3 : public Functor
{
    typedef R (C::*F)($tempP)$const1;$typesA
public:
    RFunctor$paramCount$const3(${const2}C * o, F f, R & r$paramD, QSemaphore * sem = 0) :
        o_(o), f_(f), r_(r)$paramU, sem_(sem) {}

    virtual void operator()() const
    {
        r_ = (o_->*f_)($memberU);
        if (sem_) sem_->release();
    }

private:
    ${const2}C * o_;
    F f_;
    R & r_;$memberD
    QSemaphore * sem_;
};

EOF
}

sub genFunctor
{
    for (my $i = 0 ; $i <= $maxParams ; ++$i) {
        print "// ============================================================================\n\n";
        functor($i, 0);
        functor($i, 1);
    }
}

my $cmd = $ARGV[0] // '';
if    ($cmd eq 'verify')  { genVerify();  }
elsif ($cmd eq 'functor') { genFunctor(); }
else {
    print STDERR "usage: $0 (verify|functor)\n";
    exit 1;
}
