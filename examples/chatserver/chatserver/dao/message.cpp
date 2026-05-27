/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

 #include "message.h"

namespace chatserver::dao {

Message::Message(const String & text) :
    timestamp(DateTime::currentDateTimeUtc()),
    text(text)
{
}

}
