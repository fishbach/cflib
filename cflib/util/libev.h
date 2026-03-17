/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#define EV_COMPAT3 0
#define EV_MULTIPLICITY 1
#define EV_STANDALONE 1
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__APPLE__)
    #define EV_USE_KQUEUE 1
#endif

#include "libev/ev.h"
