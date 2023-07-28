/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtGlobal>

#define EV_COMPAT3 0
#define EV_MULTIPLICITY 1
#define EV_STANDALONE 1
#ifdef Q_OS_BSD4
	#define EV_USE_KQUEUE 1
#endif

#include "libev/ev.h"
