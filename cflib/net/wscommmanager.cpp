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

#include "wscommmanager.h"

namespace cflib { namespace net {

void WSCommManagerBase::send(uint connId, const QByteArray & data, bool isBinary)
{
	if (!verifyThreadCall(&WSCommManagerBase::send, connId, data, isBinary)) return;
	WebSocketService::send(connId, data, isBinary);
}

void WSCommManagerBase::close(uint connId, TCPConn::CloseType type)
{
	if (!verifyThreadCall(&WSCommManagerBase::close, connId, type)) return;
	WebSocketService::close(connId, type);
}

QByteArray WSCommManagerBase::getRemoteIP(uint connId)
{
	SyncedThreadCall<QByteArray> stc(this);
	if (!stc.verify(&WSCommManagerBase::getRemoteIP, connId)) return stc.retval();
	return getRemoteIP(connId);
}

WSCommManagerBase::WSCommManagerBase(const QString & path, const QRegularExpression & allowedOrigin,
	uint connectionTimeoutSec)
:
	WebSocketService(path, allowedOrigin, connectionTimeoutSec)
{
}

}}	// namespace
