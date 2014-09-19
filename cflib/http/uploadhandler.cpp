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

#include "uploadhandler.h"

namespace cflib { namespace http {

UploadHandler::UploadHandler(const QString & path, const QString & threadName) :
	util::ThreadVerify(threadName),
	path_(path)
{
}

void UploadHandler::processUploadRequest(const Request & request)
{
	if (!verifyThreadCall(&UploadHandler::processUploadRequest, request)) return;

	foreach (const QByteArray & key, request.getHeaderFields().keys()) {
		QTextStream(stderr) << key << ": " << request.getHeaderFields().value(key) << endl;
	}

	if (request.isPassThrough()) {
		bool isLast;
		request.readPassThrough(isLast);
		if (!isLast) {
			requests_[request.getId()] = request;
			request.setPassThroughHandler(this);
			return;
		}
	}
	request.sendRedirect(request.getHeaderFields().value("referer"));
}

void UploadHandler::morePassThroughData(const Request::Id & id)
{
	if (!verifyThreadCall(&UploadHandler::morePassThroughData, id)) return;

	if (!requests_.contains(id)) return;

	Request & request = requests_[id];
	bool isLast;
	request.readPassThrough(isLast);
	if (isLast) {
		request.sendRedirect(request.getHeaderFields().value("referer"));
		requests_.remove(id);
	}
}

}}	// namespace
