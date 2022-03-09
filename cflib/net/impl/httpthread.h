/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

#pragma once

#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class TCPConnData;
class RequestHandler;

namespace impl {

class HttpThread : public util::ThreadVerify
{
public:
	HttpThread(uint no, uint count);
	~HttpThread();

	void newRequest(TCPConnData * data, const QList<RequestHandler *> & handlers);
	void requestFinished();

private:
	void waitForRequestsToFinish();

private:
	uint activeRequests_;
	bool shutdown_;
	QSemaphore sem_;
};

}}}	// namespace
