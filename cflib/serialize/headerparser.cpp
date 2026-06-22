/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "headerparser.h"

namespace cflib::serialize {

namespace {

int lineNr(const String & in, int pos)
{
    int count = 1;
    for (int i = 0; i < pos && i < (int)in.size(); ++i) {
        if (in[i] == '\n') ++count;
    }
    return count;
}

int findClosingBrace(const String & in, int startPos, char brOpen, char brClose)
{
    int level = 0;
    for (int i = startPos; i < (int)in.size(); ++i) {
        const char c = in[i];
        if (c == brOpen) ++level;
        else if (c == brClose) {
            if (level == 0) return i;
            --level;
        }
    }
    return -1;
}

SerializeTypeInfo rawTypeInfo(const String & ns, const String & typeName)
{
    SerializeTypeInfo ti;
    if (!typeName.isEmpty()) {
        ti.type     = SerializeTypeInfo::Placeholder;
        ti.ns       = ns;
        ti.typeName = typeName;
    }
    ti.typeName.replace(" ", "").replace("\t", "");
    return ti;
}

}

bool HeaderParser::getVariables(const String & in, int start, int end, SerializeTypeInfo & cl)
{
    static const Regex varRE(R"((?:^|\n)\s*([:\w]+(?:\s*<[^>]+>)?)\s+(\w+)\s*(?:=[^;]+)?(?:;|$)|SERIALIZE_SKIP)");

    int pos = start;
    for (;;) {
        Regex::MatchResult m = varRE.matchResult(in.mid(pos, end - pos));
        if (!m.hasMatch()) break;
        SerializeVariableTypeInfo var;
        var.type = rawTypeInfo(cl.getName(), m.captured(1));
        var.name = m.captured(2);
        cl.members.push_back(var);
        pos += m.capturedStart(0) + m.capturedLength(0);
    }
    return true;
}

bool HeaderParser::getParameters(const String & in, int start, int end, SerializeVariableTypeInfos & vars, const String & ns)
{
    static const Regex varRE(R"((?:^|,)\s*(const\s+)?([:\w]+(?:\s*<[^>]+>)?)(\s*&)?(?:\s+(\w+))?)");

    int pos = start;
    for (;;) {
        Regex::MatchResult m = varRE.matchResult(in.mid(pos, end - pos));
        if (!m.hasMatch()) break;
        SerializeVariableTypeInfo var;
        var.type = rawTypeInfo(ns, m.captured(2));
        var.name = m.captured(4);
        var.isRef = m.captured(1).isEmpty() && !m.captured(3).isEmpty();
        vars.push_back(var);
        pos += m.capturedStart(0) + m.capturedLength(0);
    }
    return true;
}

bool HeaderParser::getFunctions(const String & in, int start, int end, SerializeTypeInfo & cl)
{
    static const Regex funcRE(R"((?:^|;)\s*([:\w]+(?:\s*<[^>]+>)?)\s+(\w+)\s*\()");

    int searchPos = start;
    for (;;) {
        Regex::MatchResult m = funcRE.matchResult(in.mid(searchPos, end - searchPos));
        if (!m.hasMatch()) break;
        SerializeFunctionTypeInfo func;
        if (m.captured(1) != "void") func.returnType = rawTypeInfo(cl.getName(), m.captured(1));
        func.name = m.captured(2);
        const int pos = searchPos + m.capturedStart(0) + m.capturedLength(0);
        const int paramEnd = findClosingBrace(in, pos, '(', ')');
        if (paramEnd == -1 || paramEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + String::number(lineNr(in, pos - 1));
            return false;
        }

        if (!getParameters(in, pos, paramEnd, func.parameters, cl.getName())) return false;

        cl.functions.push_back(func);
        searchPos = paramEnd + 1;
    }
    return true;
}

bool HeaderParser::getCfSignals(const String & in, int start, int end, SerializeTypeInfo & cl)
{
    static const Regex sigRE(R"((?:^|;)\s*rsig\s*<\s*([:\w]+(?:\s*<[^>]+>)?)\s*\()");
    static const Regex sigRegisterRE(R"(\)\s*,\s*([:\w]+(?:\s*<[^>]+>)?)\s*\()");
    static const Regex sigNameRE(R"(\)\s*>\s*(\w+)\s*;)");

    int searchPos = start;
    for (;;) {
        Regex::MatchResult m = sigRE.matchResult(in.mid(searchPos, end - searchPos));
        if (!m.hasMatch()) break;
        SerializeFunctionTypeInfo func;
        if (m.captured(1) != "void") func.returnType = rawTypeInfo(cl.getName(), m.captured(1));
        int pos = searchPos + m.capturedStart(0) + m.capturedLength(0);
        int paramEnd = findClosingBrace(in, pos, '(', ')');
        if (paramEnd == -1 || paramEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + String::number(lineNr(in, pos - 1));
            return false;
        }

        if (!getParameters(in, pos, paramEnd, func.parameters, cl.getName())) return false;

        Regex::MatchResult m2 = sigRegisterRE.matchResult(in.mid(paramEnd, end - paramEnd));
        if (!m2.hasMatch()) return false;
        pos = paramEnd + m2.capturedStart(0) + m2.capturedLength(0);
        paramEnd = findClosingBrace(in, pos, '(', ')');
        if (paramEnd == -1 || paramEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + String::number(lineNr(in, pos - 1));
            return false;
        }

        if (!getParameters(in, pos, paramEnd, func.registerParameters, cl.getName())) return false;

        Regex::MatchResult m3 = sigNameRE.matchResult(in.mid(paramEnd, end - paramEnd));
        if (!m3.hasMatch()) return false;
        func.name = m3.captured(1);

        cl.cfSignals.push_back(func);
        searchPos = paramEnd + 1;
    }
    return true;
}

bool HeaderParser::getMembers(const String & in, int start, int end, SerializeTypeInfo & cl, int & state)
{
    static const Regex sectionRE(R"(\s(rmi|serialized|cfsignals)\s*:)");
    static const Regex endRE(R"([^:]:\s*\n)");

    bool openEnd = false;

    // continue with old block
    if (state > 1) {
        Regex::MatchResult m = endRE.matchResult(in.mid(start, end - start));
        const int secEnd = m.hasMatch() ? start + m.capturedStart(0) + m.capturedLength(0) : end;
        openEnd = !m.hasMatch();
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
    int searchPos = start;
    for (;;) {
        Regex::MatchResult m = sectionRE.matchResult(in.mid(searchPos, end - searchPos));
        if (!m.hasMatch()) break;
        const int pos = searchPos + m.capturedStart(0) + m.capturedLength(0);
        Regex::MatchResult m2 = endRE.matchResult(in.mid(pos, end - pos));
        const int secEnd = m2.hasMatch() ? pos + m2.capturedStart(0) + m2.capturedLength(0) : end;
        openEnd = !m2.hasMatch();

        const String section = m.captured(1);
        if (section == "rmi") {
            state = 2;
            if (!getFunctions(in, pos, secEnd, cl)) return false;
        } else if (section == "serialized") {
            state = 3;
            if (!getVariables(in, pos, secEnd, cl)) return false;
        } else { // cfsignals
            state = 4;
            if (!getCfSignals(in, pos, secEnd, cl)) return false;
        }

        searchPos = pos;
    }

    if (!openEnd) state = 1;
    return true;
}

bool HeaderParser::getMemberBlocks(const String & in, int start, int end, SerializeTypeInfo & cl, int & state)
{
    // Do we care about this class?
    if (state == 0) {
        const ssize_t serializeMembersPos = in.indexOf("SERIALIZE_CLASS", start);
        if (serializeMembersPos != -1 && serializeMembersPos < end) state = 1;
        else return true;

        // serialize base?
        if (cl.hasBase()) {
            static const Regex serBaseRE(R"(SERIALIZE_(STD)?BASE)");
            if (!serBaseRE.matchResult(in.mid(start, end - start)).hasMatch()) cl.bases.clear();
        }
    }

    // skip blocks
    ssize_t bracePos = in.indexOf('{', start);
    int pos = (bracePos == -1) ? -1 : (int)bracePos;
    while (pos != -1 && pos < end) {
        if (pos > start) {
            if (!getMembers(in, start, pos, cl, state)) return false;
        }

        ++pos;
        const int blockEnd = findClosingBrace(in, pos, '{', '}');
        if (blockEnd == -1 || blockEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + String::number(lineNr(in, pos - 1));
            return false;
        }

        start = blockEnd + 1;
        bracePos = in.indexOf('{', start);
        pos = (bracePos == -1) ? -1 : (int)bracePos;
    }
    if (start < end) {
        if (!getMembers(in, start, end, cl, state)) return false;
    }

    return true;
}

bool HeaderParser::getClasses(const String & in, int start, int end, SerializeTypeInfo cl)
{
    static const Regex classOrNamespaceRE(
        R"((class|struct|namespace)\s+(\w+(?:\s*::\s*\w+)*)\s*(?::\s*\w+\s+([\w:]+(?:\s*<[^{]+>)?)\s*)?(?:\s*,[^{]+)?\{)");

    int state = 0;
    for (;;) {
        Regex::MatchResult m = classOrNamespaceRE.matchResult(in.mid(start, end - start));
        if (!m.hasMatch()) break;
        const int pos = start + m.capturedStart(0);
        const String type = m.captured(1);
        const String name = m.captured(2);
        const String base = m.captured(3);

        // handle rest of previous
        if (pos > start && !cl.typeName.isEmpty()) {
            if (!getMemberBlocks(in, start, pos, cl, state)) return false;
        }

        // find end of block { }
        const int afterMatch = pos + m.capturedLength(0);
        const int blockEnd = findClosingBrace(in, afterMatch, '{', '}');
        if (blockEnd == -1 || blockEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + String::number(lineNr(in, afterMatch - 1));
            return false;
        }

        SerializeTypeInfo innerCl;
        innerCl.type = SerializeTypeInfo::Class;
        if (type != "namespace") {
            innerCl.ns = cl.ns;
            innerCl.typeName = cl.typeName.isEmpty() ? name : cl.typeName + "::" + name;
            if (!base.isEmpty()) innerCl.bases << rawTypeInfo(cl.getName(), base);
        } else {
            innerCl.ns = cl.ns.isEmpty() ? name : cl.ns + "::" + name;
        }
        if (!getClasses(in, afterMatch, blockEnd, innerCl)) return false;

        start = blockEnd + 1;
    }

    // handle rest of previous
    if (start < end && !cl.typeName.isEmpty()) {
        if (!getMemberBlocks(in, start, end, cl, state)) return false;
    }

    if (state > 0) classes_.push_back(cl);
    return true;
}

bool HeaderParser::removeCommentsAndStringContents(String & header)
{
    enum class State { None, InSingleQuote, InDoubleQuote, InLineComment, InBlockComment };

    size_t readPos = 0;
    size_t writePos = 0;
    size_t currentLine = 1;
    size_t openLine = 1;
    State state = State::None;

    while (readPos < header.size()) {
        const char c     = header[readPos++];
        const char nextC = readPos < header.size() ? header[readPos] : '\0';
        if (c == '\n') ++currentLine;

        switch (state) {
            case State::None:
                if (c == '/' && nextC == '/') {
                    state = State::InLineComment;
                    ++readPos;
                    continue;
                }
                if (c == '/' && nextC == '*') {
                    state = State::InBlockComment;
                    openLine = currentLine;
                    ++readPos;
                    continue;
                }

                if (c == '\'') {
                    state = State::InSingleQuote;
                    openLine = currentLine;
                } else if (c == '"') {
                    state = State::InDoubleQuote;
                    openLine = currentLine;
                }
                header[writePos++] = c;
                break;

            case State::InSingleQuote:
                if (c == '\\' && nextC == '\'') {
                    ++readPos;
                    continue;
                }
                if (c == '\'') {
                    state = State::None;
                    header[writePos++] = c;
                }
                break;

            case State::InDoubleQuote:
                if (c == '\\' && nextC == '"') {
                    ++readPos;
                    continue;
                }
                if (c == '"') {
                    state = State::None;
                    header[writePos++] = c;
                }
                break;

            case State::InLineComment:
                if (c == '\n') {
                    state = State::None;
                    header[writePos++] = c;
                }
                break;

            case State::InBlockComment:
                if (c == '*' && nextC == '/') {
                    state = State::None;
                    ++readPos;
                }
                break;
        }
    }

    if (state == State::InSingleQuote || state == State::InDoubleQuote) {
        lastError_ = "cannot find closing quotes at line: " + String::number((int)openLine);
        return false;
    }
    if (state == State::InBlockComment) {
        lastError_ = "cannot find closing comment at line: " + String::number((int)openLine);
        return false;
    }

    header.resize(writePos);
    return true;
}

bool HeaderParser::parse(const String & headerRef)
{
    if (headerRef.indexOf("SERIALIZE_CLASS") == -1) return true;

    String header = headerRef;

    // line endings
    header.replace("\r\n", "\n");   // Windows
    header.replace("\r",   "\n");   // old Mac OS

    if (!removeCommentsAndStringContents(header)) return false;

    return getClasses(header, 0, (int)header.length(), SerializeTypeInfo());
}

} // namespace
