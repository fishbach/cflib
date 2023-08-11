/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <QtCore>

namespace cflib { namespace db { namespace impl {

// avoid warning:
// file: cflib/db/libcflib_db.a(mocs_compilation.cpp.o) has no symbols
class Dummy : public QObject
{
	Q_OBJECT
};

}}}	// namespace
