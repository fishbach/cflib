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

#include <cflib/util/log.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace http {

UploadHandler::UploadHandler(const QString & path, const QString & threadName) :
	util::ThreadVerify(threadName),
	path_(path),
	apiServer_(0)
{
}

void UploadHandler::processUploadRequest(const Request & request)
{
	if (!verifyThreadCall(&UploadHandler::processUploadRequest, request)) return;

	const Request::KeyVal header = request.getHeaderFields();

	// get boundary from content type
	QByteArray boundary = header["content-type"];
	const QByteArray cTypeLc = boundary.toLower();
	int pos = cTypeLc.indexOf("boundary=");
	if (pos == -1 || !cTypeLc.startsWith("multipart/form-data")) {
		logInfo("Wrong content type: %1", boundary);
		request.sendNotFound();
		return;
	}

	RequestData & rd = requests_[request.getId()];
	rd.request  = request;
	rd.boundary = "--" + boundary.mid(pos + 9);
	rd.buffer = request.getBody();
	rd.state = 1;
	if (request.isPassThrough()) request.setPassThroughHandler(this);

	QTextStream(stdout)
		<< "boundary: X>" << rd.boundary << "<X" << endl
		<< "body: X>" << rd.buffer << "<X" << endl;

	parseMoreData(rd);
}

void UploadHandler::morePassThroughData(const Request::Id & id)
{
	if (!verifyThreadCall(&UploadHandler::morePassThroughData, id)) return;

	if (requests_.contains(id)) parseMoreData(requests_[id]);
}

void UploadHandler::parseMoreData(UploadHandler::RequestData & rd)
{
	bool isLast = true;
	if (rd.request.isPassThrough()) {
		rd.buffer += rd.request.readPassThrough(isLast);
	}

	forever {
		if (rd.state == 1) {
			// search for first boundary
			if (rd.buffer.indexOf(rd.boundary) == 0) {
				rd.buffer.remove(0, rd.boundary.size() + 2);	// \r\n
				rd.state = 2;
			} else break;
		}
		if (rd.state == 2) {
			// search for header end
			int bodyPos = rd.buffer.indexOf("\r\n\r\n");
			if (bodyPos != -1) {
					foreach (const QByteArray & line, rd.buffer.left(bodyPos).split('\n')) {
						const int pos = line.indexOf(':');
						if (pos == -1) {
							logWarn("funny line in header: %1", line);
							continue;
						}
						QByteArray key   = line.left(pos).trimmed().toLower();
						QByteArray value = line.mid(pos + 1).trimmed();
						QTextStream(stderr) << key << " => " << value << endl;
					}
				rd.buffer.remove(0, bodyPos + 4);
				rd.state = 3;
			} else break;
		}
		if (rd.state == 3) {
			// search for end boundary
			int pos = rd.buffer.indexOf(rd.boundary);
			if (pos != -1) {
				QByteArray b = rd.buffer.left(pos - 2);
				handleData(rd.request, b, isLast);
				rd.buffer.remove(0, pos + rd.boundary.size() + 2);	// \r\n
				rd.state = 2;
			} else {
				handleData(rd.request, rd.buffer, isLast);
				break;
			}
		}
	}

	if (isLast) {
		rd.request.sendRedirect(rd.request.getHeaderFields().value("referer"));
		requests_.remove(rd.request.getId());
	}
}

}}	// namespace
