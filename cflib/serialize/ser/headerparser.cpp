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

#include "headerparser.h"

namespace {

int lineNr(const QString & in, int pos)
{
	return in.left(pos).split('\n').count();
}

int count(const QString & in, const QString & sub)
{
	int retval = 0;
	int pos = in.indexOf(sub);
	while (pos != -1) {
		++retval;
		pos = in.indexOf(sub, pos + sub.length());
	}
	return retval;
}

int findClosingBrace(const QString & in, int startPos, char brOpen, char brClose)
{
	QString::ConstIterator it = in.constBegin();
	it += startPos;
	int level = 0;
	while (it < in.constEnd()) {
		const char c = it->toLatin1();
		if (c == brOpen) ++level;
		else if (c == brClose) {
			if (level == 0) return it - in.constBegin();
			--level;
		}
		++it;
	}
	return -1;
}

}

bool HeaderParser::getVariables(const QString & in, int start, int end, Class & cl)
{
	static const QRegExp varRE("(?:^|\n)\\s*([:\\w]+(?:\\s*<[^>]+>)?)\\s+(\\w+)\\s*;|SERIALIZE_SKIP");

	int pos = varRE.indexIn(in, start, QRegExp::CaretAtOffset);
	while (pos != -1 && pos < end) {
		HeaderParser::Variable var;
		var.type = varRE.cap(1);
		var.name = varRE.cap(2);
		cl.members << var;
		pos = varRE.indexIn(in, pos + varRE.matchedLength(), QRegExp::CaretAtOffset);
	}
	return true;
}

bool HeaderParser::getParameters(const QString & in, int start, int end, Variables & vars)
{
	static const QRegExp varRE("(?:^|,)\\s*(const\\s+)?([:\\w]+(?:\\s*<[^>]+>)?)(\\s*&)?(?:\\s+(\\w+))?");

	int pos = varRE.indexIn(in, start, QRegExp::CaretAtOffset);
	while (pos != -1 && pos < end) {
		HeaderParser::Variable var;
		var.type = varRE.cap(2);
		var.name = varRE.cap(4);
		var.isRef = varRE.cap(1).isEmpty() && !varRE.cap(3).isEmpty();
		vars << var;
		pos = varRE.indexIn(in, pos + varRE.matchedLength(), QRegExp::CaretAtOffset);
	}
	return true;
}

bool HeaderParser::getFunctions(const QString & in, int start, int end, Class & cl)
{
	static const QRegExp funcRE("(?:^|;)\\s*([:\\w]+(?:\\s*<[^>]+>)?)\\s+(\\w+)\\s*\\(");

	int pos = funcRE.indexIn(in, start, QRegExp::CaretAtOffset);
	while (pos != -1 && pos < end) {
		HeaderParser::Function func;
		func.returnType = funcRE.cap(1);
		func.name       = funcRE.cap(2);
		pos += funcRE.matchedLength();
		const int paramEnd = findClosingBrace(in, pos, '(', ')');
		if (paramEnd == -1 || paramEnd >= end) {
			lastError_ = QString("cannot find closing brace at line: %1").arg(lineNr(in, pos - 1));
			return false;
		}

		if (!getParameters(in, pos, paramEnd, func.parameters)) return false;

		cl.functions << func;
		pos = funcRE.indexIn(in, paramEnd + 1, QRegExp::CaretAtOffset);
	}
	return true;
}

bool HeaderParser::getCfSignals(const QString & in, int start, int end, Class & cl)
{
	static const QRegExp sigRE("(?:^|;)\\s*rsig\\s*<\\s*([:\\w]+(?:\\s*<[^>]+>)?)\\s*\\(");
	static const QRegExp sigNameRE("\\)\\s*>\\s*(\\w+)\\s*;");

	int pos = sigRE.indexIn(in, start, QRegExp::CaretAtOffset);
	while (pos != -1 && pos < end) {
		HeaderParser::Function func;
		func.returnType = sigRE.cap(1);
		pos += sigRE.matchedLength();
		const int paramEnd = findClosingBrace(in, pos, '(', ')');
		if (paramEnd == -1 || paramEnd >= end) {
			lastError_ = QString("cannot find closing brace at line: %1").arg(lineNr(in, pos - 1));
			return false;
		}

		if (!getParameters(in, pos, paramEnd, func.parameters)) return false;

		if (sigNameRE.indexIn(in, paramEnd) == -1) return false;
		func.name = sigNameRE.cap(1);

		cl.cfSignals << func;
		pos = sigRE.indexIn(in, paramEnd + 1, QRegExp::CaretAtOffset);
	}
	return true;
}

bool HeaderParser::getMembers(const QString & in, int start, int end, Class & cl, int & state)
{
	static const QRegExp sectionRE("\\s(rmi|serialized|cfsignals)\\s*:");
	static const QRegExp endRE("[^:]:\\s*\n");

	bool openEnd = false;

	// continue with old block
	if (state > 1) {
		int secEnd = endRE.indexIn(in, start);
		openEnd = secEnd == -1 || secEnd > end;
		if (openEnd) secEnd = end;
		if (state == 2) {
			if (!getFunctions(in, start, secEnd, cl)) return false;
		} else if (state == 3) {
			if (!getVariables(in, start, secEnd, cl)) return false;
		} else {
			if (!getCfSignals(in, start, secEnd, cl)) return false;
		}
		if (openEnd) return true;
	}

	// search for block
	int pos = sectionRE.indexIn(in, start);
	while (pos != -1 && pos < end) {
		pos += sectionRE.matchedLength();
		int secEnd = endRE.indexIn(in, pos);
		openEnd = secEnd == -1 || secEnd > end;
		if (openEnd) secEnd = end;

		if (sectionRE.cap(1) == "rmi") {
			state = 2;
			if (!getFunctions(in, pos, secEnd, cl)) return false;
		} else if (sectionRE.cap(1) == "serialized") {
			state = 3;
			if (!getVariables(in, pos, secEnd, cl)) return false;
		} else { // cfsignals
			state = 4;
			if (!getCfSignals(in, pos, secEnd, cl)) return false;
		}

		pos = sectionRE.indexIn(in, pos);
	}

	if (!openEnd) state = 1;
	return true;
}

bool HeaderParser::getMemberBlocks(const QString & in, int start, int end, Class & cl, int & state)
{
	// Do we care about this class?
	if (state == 0) {
		const int serializeMembersPos = in.indexOf("SERIALIZE_CLASS", start);
		if (serializeMembersPos != -1 && serializeMembersPos < end) state = 1;
		else return true;
	}

	// serialize base?
	const int serializeBasePos = in.indexOf("SERIALIZE_BASE", start);
	if (serializeBasePos != -1 && serializeBasePos < end) cl.doBaseSerialize = true;

	// skip blocks
	int pos = in.indexOf('{', start);
	while (pos != -1 && pos < end) {
		if (pos > start) {
			if (!getMembers(in, start, pos, cl, state)) return false;
		}

		++pos;
		int blockEnd = findClosingBrace(in, pos, '{', '}');
		if (blockEnd == -1 || blockEnd >= end) {
			lastError_ = QString("cannot find closing brace at line: %1").arg(lineNr(in, pos - 1));
			return false;
		}

		start = blockEnd + 1;
		pos = in.indexOf('{', start);
	}
	if (start < end) {
		if (!getMembers(in, start, end, cl, state)) return false;
	}

	return true;
}

bool HeaderParser::getClasses(const QString & in, int start, int end, Class cl)
{
	static const QRegExp classOrNamespaceRE(
		"(class|struct|namespace)\\s+(\\w+)\\s*(?::\\s*\\w+\\s+([\\w:]+(?:\\s*<[^{]+>)?)\\s*)?(?:\\s*,[^{]+)?\\{");

	int state = 0;
	int pos = classOrNamespaceRE.indexIn(in, start);
	while (pos != -1 && pos < end) {
		Class innerCl;
		QString type = classOrNamespaceRE.cap(1);
		QString name = classOrNamespaceRE.cap(2);
		innerCl.base = classOrNamespaceRE.cap(3);

		if (pos > start && !cl.name.isEmpty()) {
			if (!getMemberBlocks(in, start, pos, cl, state)) return false;
		}

		pos += classOrNamespaceRE.matchedLength();
		int blockEnd = findClosingBrace(in, pos, '{', '}');
		if (blockEnd == -1 || blockEnd >= end) {
			lastError_ = QString("cannot find closing brace at line: %1").arg(lineNr(in, pos - 1));
			return false;
		}

		if (type != "namespace") {
			innerCl.ns = cl.ns;
			innerCl.name = cl.name.isEmpty() ? name : cl.name + "::" + name;
		} else {
			innerCl.ns = cl.ns.isEmpty() ? name : cl.ns + "::" + name;
		}
		if (!getClasses(in, pos, blockEnd, innerCl)) return false;

		start = blockEnd + 1;
		pos = classOrNamespaceRE.indexIn(in, start);
	}
	if (start < end && !cl.name.isEmpty()) {
		if (!getMemberBlocks(in, start, end, cl, state)) return false;
	}

	if (state > 0) classes_ << cl;
	return true;
}

bool HeaderParser::removeCommentsAndStringContents(QString & header)
{
	static const QRegExp stringOrCommentRE("\"|//|/\\*");

	int pos = stringOrCommentRE.indexIn(header);
	while (pos != -1) {
		const QString m = stringOrCommentRE.cap(0);
		int next = 0;
		if (m == "\"") {
			next = header.indexOf('"', pos + 1);
			while (next != -1 && header[next - 1] == '\\') {
				next = header.indexOf('"', next + 1);
			}
			if (next == -1) {
				lastError_ = QString("cannot find closing quotes at line: %1").arg(lineNr(header, pos));
				return false;
			}
			++pos;
		} else if (m == "//") {
			next = header.indexOf('\n', pos + 2);
			if (next == -1) next = header.length();
		} else if (m == "/*") {
			next = header.indexOf("*/", pos + 2);
			if (next == -1) {
				lastError_ = QString("cannot find closing comment at line: %1").arg(lineNr(header, pos));
				return false;
			}
			next += 2;
		}

		header = header.left(pos) + header.mid(next);
		if (m == "\"") ++pos;

		pos = stringOrCommentRE.indexIn(header, pos);
	}
	return true;
}

bool HeaderParser::parse(const QString & headerRef)
{
	if (headerRef.indexOf("SERIALIZE_CLASS") == -1) return true;

	QString header = headerRef;

	// normalize line endings
	if (count(header, "\r\n") > count(header, "\n\r")) {
		header.replace("\r\n", "\n");
	} else {
		header.replace("\n\r", "\n");
	}
	header.replace("\r", "\n");

	if (!removeCommentsAndStringContents(header)) return false;

	return getClasses(header, 0, header.length(), Class());
}

bool HeaderParser::Function::hasReturnValues() const
{
	if (returnType != "void") return true;
	foreach (const HeaderParser::Variable & p, parameters) {
		if (p.isRef) return true;
	}
	return false;
}
