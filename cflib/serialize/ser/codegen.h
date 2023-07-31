/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

class HeaderParser;
class QIODevice;
class QString;

int genSerialize(const QString & headerName, const HeaderParser & hp, QIODevice & out);
