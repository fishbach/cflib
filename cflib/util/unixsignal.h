/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace util {

// catches signals 1, 2 and 15
class UnixSignal : public QObject
{
	Q_OBJECT
public:
	UnixSignal(bool quitQCoreApplication = false);
	~UnixSignal();

signals:
	void catchedSignal(int sig);

private slots:
	void activated();

private:
	QSocketNotifier * sn_;
};

}}	// namespace
