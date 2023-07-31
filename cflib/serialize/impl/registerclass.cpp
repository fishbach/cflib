/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "registerclass.h"

namespace cflib { namespace serialize { namespace impl {

namespace {

typedef QHash<quint32, const RegisterClassBase *> Registry;
Q_GLOBAL_STATIC(Registry, getRegistry)

}

QHash<quint32, const RegisterClassBase *> & RegisterClassBase::registry()
{
	return *getRegistry();
}

void RegisterClassBase::duplicateId(quint32 classId)
{
	QTextStream(stderr) << "duplicate type id: %1" << classId << Qt::endl;
	::abort();
}

}}}	// namespace
