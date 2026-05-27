#pragma once

#include <cflib/serialize/serialize.h>

namespace chatserver::dao {

class Message
{
    SERIALIZE_CLASS
public serialized:
    DateTime timestamp;
    String text;
};

}
