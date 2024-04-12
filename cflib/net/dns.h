/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace net {

// Attention: this function may need some time (blocks)
QList<QByteArray> getIPFromDNS(const QByteArray & name, bool preferIPv6 = false);

}}    // namespace
