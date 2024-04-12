/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serialize.h>

namespace cflib { namespace dao {

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
    QString baseURL;

protected:
    virtual void init(const QMap<QString, QString> &) {}

private:
    static ConfigPSql * instance_;
};

}}    // namespace
