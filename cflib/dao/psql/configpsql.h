/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serialize.h>

namespace cflib::dao {

class ConfigPSql
{
    SERIALIZE_CLASS
    SERIALIZE_IS_BASE(ConfigPSql)
public:
    ConfigPSql();

    void loadFromDB();
    static const ConfigPSql & instance() { return *instance_; }

public serialized:
    bool isProduction;
    bool emailsEnabled;
    String baseURL;

protected:
    virtual void init(const Map<String, String> &) {}

private:
    static ConfigPSql * instance_;
};

} // namespace
