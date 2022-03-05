/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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

#include "chatwindow.h"

#include "ui_chatwindow.h"

#include <cflib/serialize/util.h>

ChatWindow::ChatWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChatWindow)
{
	ui->setupUi(this);
	ui->pbSend->setDisabled(true);

	connect(ui->pbSend, &QPushButton::clicked, this, &ChatWindow::onSendPressed);
	connect(&ws_, &QWebSocket::binaryMessageReceived, this, &ChatWindow::onBinaryMessageReceived);
	connect(&ws_, &QWebSocket::connected, this, &ChatWindow::onConnected);
	connect(&ws_, &QWebSocket::disconnected, [&]() {ui->pbSend->setDisabled(true);});

	connect(this, &ChatWindow::connectionEstablished, this, &ChatWindow::onConnectionEstablished);

	ws_.open(QUrl("ws://localhost:8080/ws"));

	qDebug() << "connecting ... ";
}

ChatWindow::~ChatWindow()
{
	delete ui;
}

void ChatWindow::onConnected()
{
	ui->pbSend->setEnabled(true);

	cflib::serialize::BERSerializer ser(1);
	if (!clId_.isNull()) ser << clId_;
	ws_.sendBinaryMessage(ser.data());
}

void ChatWindow::onBinaryMessageReceived(const QByteArray & data)
{
	qDebug() << "bin data: " << data.toHex();

	quint64 tag;
	int tagLen;
	int lengthSize;
	const qint32 valueLen = cflib::serialize::getTLVLength(data, tag, tagLen, lengthSize);
	if (valueLen < 0) return;

	if (tag == 1) {
		// normaly only used on web when reloading the webpage
		clId_ = cflib::serialize::fromByteArray<QByteArray>(data, tagLen, lengthSize, valueLen);
		qDebug() << "Session id:" << clId_.toHex();
		emit connectionEstablished();
	} else if (tag == 3) {
		cflib::serialize::BERDeserializer dser(data, (const quint8 *)data.constData() + tagLen + lengthSize, valueLen);
		QString service;
		QString method;
		dser >> service >> method;

		if (service == "infoservice" && method == "newMessage") {
			int connectionId;
			QString message;
			dser >> connectionId >> message;
			ui->chatTextArea->append(QString("%1:\t%2").arg(connectionId).arg(message));
		} else {
			qDebug() << "Unknown signal " << service << method;
		}
	} else {
		qDebug() << "Unhandled tag: " << tag;
	}
}

void ChatWindow::onSendPressed()
{
	cflib::serialize::BERSerializer ser(2);
	ser << "infoservice" << "void talk(String)" << ui->leMessage->text().toUtf8();
	ws_.sendBinaryMessage(ser.data());
}

void ChatWindow::onConnectionEstablished()
{
	qDebug() << "connected";

	// register for newMessage Signal
	cflib::serialize::BERSerializer ser(2);
	ser << "infoservice" << "newMessage" << true; // true for register, false for unregister
	ws_.sendBinaryMessage(ser.data());
}
