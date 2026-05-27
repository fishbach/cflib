/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serialize.h>

namespace chatserver::dao {

class Message
{
    SERIALIZE_CLASS

    Message() = default;
    Message(const String & text);

public serialized:
    DateTime timestamp;
    String text;
};

}
