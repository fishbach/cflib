/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

#pragma once

#include <QtCore>

class HeaderParser
{
public:
	struct Variable
	{
		QString name;
		QString type;
		bool isRef;

		Variable() : isRef(false) {}
	};
	typedef QList<Variable> Variables;

	struct Function
	{
		QString returnType;
		QString name;
		Variables parameters;
		Variables registerParameters;

		bool hasReturnValues() const;
	};
	typedef QList<Function> Functions;

	struct Class
	{
		QString ns;
		QString name;
		QString base;
		bool doBaseSerialize;
		Variables members;
		Functions functions;
		Functions cfSignals;

		Class() : doBaseSerialize(false) {}
	};
	typedef QList<Class> Classes;

public:
	bool parse(const QString & header);
	QString lastError() const { return lastError_; }

	bool hasSerializeElements() const { return !classes_.isEmpty(); }
	QList<Class> classes() const { return classes_; }

private:
	bool getVariables(const QString & in, int start, int end, Class & cl);
	bool getParameters(const QString & in, int start, int end, Variables & vars);
	bool getFunctions(const QString & in, int start, int end, Class & cl);
	bool getCfSignals(const QString & in, int start, int end, Class & cl);
	bool getMembers(const QString & in, int start, int end, Class & cl, int & state);
	bool getMemberBlocks(const QString & in, int start, int end, Class & cl, int & state);
	bool getClasses(const QString & in, int start, int end, Class cl);
	bool removeCommentsAndStringContents(QString & header);

private:
	QString lastError_;
	Classes classes_;
};
