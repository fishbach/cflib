/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/macros.h>
#include <cflib/base/types.h>

#include <thread>

// Forward declare ThreadHolder for the thread-local pointer
namespace cflib { namespace util { namespace impl { class ThreadHolder; }}}

// Thread-local pointer to the current cflib-managed thread holder.
// Set at the start of each managed thread's run() body.
extern thread_local cflib::util::impl::ThreadHolder * cf_current_thread;

template<typename T>
class CFThreadLocal
{
    CF_DISABLE_COPY(CFThreadLocal)
public:
    CFThreadLocal() : ptr_(nullptr) {}
    ~CFThreadLocal() { delete ptr_; }

    T * localData() { return ptr_; }
    void setLocalData(T * data) {
        delete ptr_;
        ptr_ = data;
    }
    bool hasLocalData() const { return ptr_ != nullptr; }

private:
    T * ptr_;
};
