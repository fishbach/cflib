/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
