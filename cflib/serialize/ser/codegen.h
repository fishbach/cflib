/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <ostream>
#include <string>

namespace cflib::serialize { class HeaderParser; }

int genSerialize(const std::string & headerName, const cflib::serialize::HeaderParser & hp, std::ostream & out);
