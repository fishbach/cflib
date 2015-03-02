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

#include "uploadservice.h"

#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

UploadService::UploadService(const QString & threadName, uint threadCount) :
	ThreadVerify(threadName, util::ThreadVerify::Worker, threadCount)
{
}

UploadService::~UploadService()
{
	stopVerifyThread();
}

UploadRequestHandler::UploadRequestHandler(UploadService * uploadHandler, const Request & request) :
	ThreadVerify(uploadHandler),
	request_(request),
	state_(1)
{
	connect(
		this,          &UploadRequestHandler::getClientId,
		uploadHandler,        &UploadService::getClientId,
		Qt::DirectConnection
	);
	init();
}

void UploadRequestHandler::morePassThroughData()
{
	if (!verifyThreadCall(&UploadRequestHandler::morePassThroughData)) return;

	parseMoreData();
}

void UploadRequestHandler::handleClientId(uint)
{
}

void UploadRequestHandler::requestEnd()
{
	logFunctionTrace
	request_.sendText(
		"<html><body onload=\"location.href='" + request_.getHeaderFields().value("referer") + "'\"></body></html>"
	);
}

void UploadRequestHandler::init()
{
	if (!verifyThreadCall(&UploadRequestHandler::init)) return;

	const Request::KeyVal header = request_.getHeaderFields();

	// get boundary from content type
	boundary_ = header["content-type"];
	const QByteArray cTypeLc = boundary_.toLower();
	int pos = cTypeLc.indexOf("boundary=");
	if (pos == -1 || !cTypeLc.startsWith("multipart/form-data")) {
		logInfo("Wrong content type: %1", boundary_);
		request_.sendNotFound();
		deleteNext(this);
		return;
	}
	boundary_ = "--" + boundary_.mid(pos + 9);
	buffer_ = request_.getBody();

	if (request_.isPassThrough()) request_.setPassThroughHandler(this);

	parseMoreData();
}

void UploadRequestHandler::parseMoreData()
{
	logFunctionTrace

	bool isLast = true;
	if (request_.isPassThrough()) buffer_ += request_.readPassThrough(isLast);

	forever {

		if (state_ == 1) {
			// search for first boundary
			if (buffer_.indexOf(boundary_) != 0) break;

			buffer_.remove(0, boundary_.size() + 2);	// \r\n
			state_ = 2;
		}

		if (state_ == 2) {
			// search for header end
			int bodyPos = buffer_.indexOf("\r\n\r\n");
			if (bodyPos == -1) break;

			foreach (const QByteArray & line, buffer_.left(bodyPos).split('\n')) {
				const int pos = line.indexOf(':');
				if (pos == -1) {
					logWarn("funny line in header: %1", line);
					continue;
				}
				QByteArray key   = line.left(pos).trimmed().toLower();
				QByteArray value = line.mid(pos + 1).trimmed();
				if (key == "content-type") contentType_ = value;
				else if (key == "content-disposition") {
					int pos = value.indexOf("name=\"");
					if (pos != -1) {
						int pos2 = value.indexOf('"', pos + 6);
						if (pos2 != -1) name_ = value.mid(pos + 6, pos2 - pos - 6);
					}
					pos = value.indexOf("filename=\"");
					if (pos != -1) {
						int pos2 = value.indexOf('"', pos + 10);
						if (pos2 != -1) filename_ = value.mid(pos + 10, pos2 - pos - 10);
					}
				}
			}

			buffer_.remove(0, bodyPos + 4);
			state_ = 3;
		}

		if (state_ == 3) {
			// search for end boundary
			int pos = buffer_.indexOf(boundary_);
			if (pos != -1) {
				QByteArray data = buffer_.left(pos - 2);
				if (name_ == "clientId") {
					uint clId = 0;
					emit getClientId(data, clId);
					handleClientId(clId);
				} else handleData(data, true);	// finish
				buffer_.remove(0, pos + boundary_.size() + 2);	// \r\n
				state_ = 2;
			} else if (isLast) {
				logWarn("broken request: last boundary is missing (%1/%2)",
					request_.getId().first, request_.getId().second);
				break;
			} else if (name_ == "clientId") {
				break;	// we wait for more data
			} else {
				// pass intermediate data on
				handleData(buffer_, false);
				buffer_.clear();
				break;
			}
		}
	}

	if (isLast) {
		requestEnd();
		deleteNext(this);
	} else {
		request_.startWatcher();
	}
}

}}	// namespace
