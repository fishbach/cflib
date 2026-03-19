/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#define CF_DISABLE_COPY(Class) \
    Class(const Class &) = delete; \
    Class & operator=(const Class &) = delete;

#define CF_UNUSED(x) (void)(x)

#define CF_FUNC_INFO __PRETTY_FUNCTION__

// Meyers singleton: call name() to get the single instance
#define CF_GLOBAL_STATIC(Type, name) \
    static Type & name() { static Type instance; return instance; }

// Trigger construction before main()
#define CF_CONSTRUCTOR_FUNCTION(func) \
    namespace { struct _cf_ctor_##func { _cf_ctor_##func() { func(); } } _cf_ctor_inst_##func; }

#define forever for (;;)
