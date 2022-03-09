/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace util {

class ArgBase
{
public:
	bool isSet() const { return count_ > 0; }
	QByteArray value(const QByteArray & defaultValue = QByteArray()) const { return values_.isEmpty() ? defaultValue : values_.first(); }
	QList<QByteArray> values() const { return values_; }
	uint count() const { return count_; }

protected:
	ArgBase(char optionChar, const QByteArray & optionName, bool hasValue, bool isOptional, bool isRepeatable) :
		optionChar_(optionChar), optionName_(optionName), hasValue_(hasValue), isOptional_(isOptional), isRepeatable_(isRepeatable),
		count_(0) {}

	char optionChar_;
	QByteArray optionName_;
	bool hasValue_;
	bool isOptional_;
	bool isRepeatable_;
	uint count_;
	QList<QByteArray> values_;

	friend class CmdLine;
};

class Arg : public ArgBase
{
public:
	Arg(bool isOptional = true, bool isRepeatable = false) :
		ArgBase(0, QByteArray(), true, isOptional, isRepeatable) {}
};

class Option : public ArgBase
{
public:
	Option(const QByteArray & optionName, bool hasValue = false,
		bool isOptional = true, bool isRepeatable = false) :
		ArgBase(0, optionName, hasValue, isOptional, isRepeatable) {}
	Option(char optionChar, const QByteArray & optionName = QByteArray(), bool hasValue = false,
		bool isOptional = true, bool isRepeatable = false) :
		ArgBase(optionChar, optionName, hasValue, isOptional, isRepeatable) {}
};

class CmdLine
{
public:
	CmdLine(int argc, char *argv[]);
	bool parse();
	CmdLine & operator<<(Arg & arg);
	CmdLine & operator<<(Option & arg);

	QByteArray executable() const { return executable_; }

private:
	QList<QByteArray> rawArgs_;
	QList<Arg *> args_;
	QHash<char, Option *> shortOptions_;
	QHash<QByteArray, Option *> options_;
	QList<ArgBase *> nonOptionals_;
	QByteArray executable_;
};

}}	// namespace
