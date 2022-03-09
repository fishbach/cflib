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

#include <QMainWindow>
#include <QWebSocket>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit ChatWindow(QWidget *parent = 0);
	~ChatWindow();

signals:
	void connectionEstablished();

private slots:
	void onConnected();
	void onBinaryMessageReceived(const QByteArray & data);
	void onSendPressed();
	void onConnectionEstablished();

private:
	QWebSocket ws_;
	QByteArray clId_;

private:
	Ui::ChatWindow *ui;
};
