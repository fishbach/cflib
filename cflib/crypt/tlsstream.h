/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace crypt {

class TLSStream
{
    Q_DISABLE_COPY(TLSStream)
public:
    TLSStream() {}
    virtual ~TLSStream() {}
    virtual QByteArray initialSend() = 0;
    virtual bool received(const QByteArray & encrypted, QByteArray & plain, QByteArray & sendBack) = 0;
    virtual bool send(const QByteArray & plain, QByteArray & encrypted) = 0;
};

}}    // namespace
