/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#include "threadverifyimpl.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib { namespace util { namespace impl {

bool ThreadObject::event(QEvent * event)
{
	if (event->type() == QEvent::User) {
		const impl::Functor * func = ((ThreadHolderEvent *)event)->func;
		(*func)();
		delete func;
		return true;
	}
	return QObject::event(event);
}

ThreadHolder::ThreadHolder(const QString & threadName) :
	threadName(threadName), isActive_(true)
{
	threadObject = new ThreadObject();
	threadObject->moveToThread(this);
}

void ThreadHolder::run()
{
	logDebug("thread %1 started", threadName);
	exec();
	isActive_ = false;
	logDebug("thread %1 events stopped", threadName);
	delete threadObject;
	threadObject = 0;
	logDebug("thread %1 stopped", threadName);
}

}}}	// namespace
