/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "headerparser.h"

#include <regex>

namespace cflib::serialize {

namespace {

int lineNr(const std::string & in, int pos)
{
    int count = 1;
    for (int i = 0; i < pos && i < (int)in.size(); ++i) {
        if (in[i] == '\n') ++count;
    }
    return count;
}

int findClosingBrace(const std::string & in, int startPos, char brOpen, char brClose)
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

// Simple string replace all
void replaceAll(std::string & str, const std::string & from, const std::string & to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

SerializeTypeInfo rawTypeInfo(const std::string & typeName)
{
    SerializeTypeInfo ti;
    if (!typeName.empty()) {
        ti.type     = SerializeTypeInfo::Class;
        ti.typeName = typeName;
    }
    return ti;
}

}

bool HeaderParser::getVariables(const std::string & in, int start, int end, SerializeTypeInfo & cl)
{
    static const std::regex varRE(R"((?:^|\n)\s*([:\w]+(?:\s*<[^>]+>)?)\s+(\w+)\s*(?:=[^;]+)?(?:;|$)|SERIALIZE_SKIP)");

    auto searchBegin = in.cbegin() + start;
    auto searchEnd = in.cbegin() + end;
    std::smatch m;
    while (std::regex_search(searchBegin, searchEnd, m, varRE)) {
        SerializeVariableTypeInfo var;
        var.type = rawTypeInfo(m[1].str());
        var.name = m[2].str();
        cl.members.push_back(var);
        searchBegin = m.suffix().first;
    }
    return true;
}

bool HeaderParser::getParameters(const std::string & in, int start, int end, SerializeVariableTypeInfos & vars)
{
    static const std::regex varRE(R"((?:^|,)\s*(const\s+)?([:\w]+(?:\s*<[^>]+>)?)(\s*&)?(?:\s+(\w+))?)");

    auto searchBegin = in.cbegin() + start;
    auto searchEnd = in.cbegin() + end;
    std::smatch m;
    while (std::regex_search(searchBegin, searchEnd, m, varRE)) {
        SerializeVariableTypeInfo var;
        var.type = rawTypeInfo(m[2].str());
        var.name = m[4].str();
        var.isRef = m[1].str().empty() && !m[3].str().empty();
        vars.push_back(var);
        searchBegin = m.suffix().first;
    }
    return true;
}

bool HeaderParser::getFunctions(const std::string & in, int start, int end, SerializeTypeInfo & cl)
{
    static const std::regex funcRE(R"((?:^|;)\s*([:\w]+(?:\s*<[^>]+>)?)\s+(\w+)\s*\()");

    auto searchBegin = in.cbegin() + start;
    auto searchEnd = in.cbegin() + end;
    std::smatch m;
    while (std::regex_search(searchBegin, searchEnd, m, funcRE)) {
        SerializeFunctionTypeInfo func;
        if (m[1].str() != "void") func.returnType = rawTypeInfo(m[1].str());
        func.name = m[2].str();
        int pos = (m.suffix().first - in.cbegin());
        const int paramEnd = findClosingBrace(in, pos, '(', ')');
        if (paramEnd == -1 || paramEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + std::to_string(lineNr(in, pos - 1));
            return false;
        }

        if (!getParameters(in, pos, paramEnd, func.parameters)) return false;

        cl.functions.push_back(func);
        searchBegin = in.cbegin() + paramEnd + 1;
    }
    return true;
}

bool HeaderParser::getCfSignals(const std::string & in, int start, int end, SerializeTypeInfo & cl)
{
    static const std::regex sigRE(R"((?:^|;)\s*rsig\s*<\s*([:\w]+(?:\s*<[^>]+>)?)\s*\()");
    static const std::regex sigRegisterRE(R"(\)\s*,\s*([:\w]+(?:\s*<[^>]+>)?)\s*\()");
    static const std::regex sigNameRE(R"(\)\s*>\s*(\w+)\s*;)");

    auto searchBegin = in.cbegin() + start;
    auto searchEnd = in.cbegin() + end;
    std::smatch m;
    while (std::regex_search(searchBegin, searchEnd, m, sigRE)) {
        SerializeFunctionTypeInfo func;
        if (m[1].str() != "void") func.returnType = rawTypeInfo(m[1].str());
        int pos = (m.suffix().first - in.cbegin());
        int paramEnd = findClosingBrace(in, pos, '(', ')');
        if (paramEnd == -1 || paramEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + std::to_string(lineNr(in, pos - 1));
            return false;
        }

        if (!getParameters(in, pos, paramEnd, func.parameters)) return false;

        std::smatch m2;
        std::string remainder = in.substr(paramEnd);
        if (!std::regex_search(remainder, m2, sigRegisterRE)) return false;
        pos = paramEnd + (m2.suffix().first - remainder.cbegin());
        paramEnd = findClosingBrace(in, pos, '(', ')');
        if (paramEnd == -1 || paramEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + std::to_string(lineNr(in, pos - 1));
            return false;
        }

        if (!getParameters(in, pos, paramEnd, func.registerParameters)) return false;

        std::string remainder2 = in.substr(paramEnd);
        std::smatch m3;
        if (!std::regex_search(remainder2, m3, sigNameRE)) return false;
        func.name = m3[1].str();

        cl.cfSignals.push_back(func);
        searchBegin = in.cbegin() + paramEnd + 1;
    }
    return true;
}

bool HeaderParser::getMembers(const std::string & in, int start, int end, SerializeTypeInfo & cl, int & state)
{
    static const std::regex sectionRE(R"(\s(rmi|serialized|cfsignals)\s*:)");
    static const std::regex endRE(R"([^:]:\s*\n)");

    bool openEnd = false;

    // continue with old block
    if (state > 1) {
        std::smatch m;
        std::string sub = in.substr(start);
        int secEnd;
        if (std::regex_search(sub, m, endRE)) {
            int matchPos = start + (m.prefix().length());
            int matchEnd = matchPos + m.length();
            if (matchEnd > end) {
                openEnd = true;
                secEnd = end;
            } else {
                secEnd = matchEnd;
            }
        } else {
            openEnd = true;
            secEnd = end;
        }
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
    auto searchBegin = in.cbegin() + start;
    auto searchEnd = in.cbegin() + end;
    std::smatch m;
    while (std::regex_search(searchBegin, searchEnd, m, sectionRE)) {
        int pos = (m.suffix().first - in.cbegin());
        std::string sub = in.substr(pos);
        std::smatch m2;
        int secEnd;
        if (std::regex_search(sub, m2, endRE)) {
            int matchEnd = pos + (int)m2.prefix().length() + (int)m2.length();
            if (matchEnd > end) {
                openEnd = true;
                secEnd = end;
            } else {
                secEnd = matchEnd;
            }
        } else {
            openEnd = true;
            secEnd = end;
        }

        std::string section = m[1].str();
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

        searchBegin = in.cbegin() + pos;
    }

    if (!openEnd) state = 1;
    return true;
}

bool HeaderParser::getMemberBlocks(const std::string & in, int start, int end, SerializeTypeInfo & cl, int & state)
{
    // Do we care about this class?
    if (state == 0) {
        size_t serializeMembersPos = in.find("SERIALIZE_CLASS", start);
        if (serializeMembersPos != std::string::npos && (int)serializeMembersPos < end) state = 1;
        else return true;

        // serialize base?
        if (cl.hasBase()) {
            static const std::regex serBaseRE(R"(SERIALIZE_(STD)?BASE)");
            std::string sub = in.substr(start, end - start);
            std::smatch m;
            if (!std::regex_search(sub, m, serBaseRE)) cl.bases.clear();
        }
    }

    // skip blocks
    size_t bracePos = in.find('{', start);
    int pos = (bracePos == std::string::npos) ? -1 : (int)bracePos;
    while (pos != -1 && pos < end) {
        if (pos > start) {
            if (!getMembers(in, start, pos, cl, state)) return false;
        }

        ++pos;
        int blockEnd = findClosingBrace(in, pos, '{', '}');
        if (blockEnd == -1 || blockEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + std::to_string(lineNr(in, pos - 1));
            return false;
        }

        start = blockEnd + 1;
        bracePos = in.find('{', start);
        pos = (bracePos == std::string::npos) ? -1 : (int)bracePos;
    }
    if (start < end) {
        if (!getMembers(in, start, end, cl, state)) return false;
    }

    return true;
}

bool HeaderParser::getClasses(const std::string & in, int start, int end, SerializeTypeInfo cl)
{
    static const std::regex classOrNamespaceRE(
        R"((class|struct|namespace)\s+(\w+(?:\s*::\s*\w+)*)\s*(?::\s*\w+\s+([\w:]+(?:\s*<[^{]+>)?)\s*)?(?:\s*,[^{]+)?\{)");

    int state = 0;
    auto searchBegin = in.cbegin() + start;
    const auto searchEnd = in.cbegin() + end;
    std::smatch m;
    while (std::regex_search(searchBegin, searchEnd, m, classOrNamespaceRE)) {
        const int pos = (int)(m[0].first - in.cbegin());
        const std::string type = m[1].str();
        const std::string name = m[2].str();
        const std::string base = m[3].str();

        // handle rest of previous
        if (pos > start && !cl.typeName.isEmpty()) {
            if (!getMemberBlocks(in, start, pos, cl, state)) return false;
        }

        // find end of block { }
        const int afterMatch = (m.suffix().first - in.cbegin());
        const int blockEnd = findClosingBrace(in, afterMatch, '{', '}');
        if (blockEnd == -1 || blockEnd >= end) {
            lastError_ = "cannot find closing brace at line: " + std::to_string(lineNr(in, afterMatch - 1));
            return false;
        }

        SerializeTypeInfo innerCl;
        innerCl.type = SerializeTypeInfo::Class;
        if (type != "namespace") {
            innerCl.ns = cl.ns;
            innerCl.typeName = cl.typeName.isEmpty() ? String(name) : cl.typeName + "::" + name;
            if (!base.empty()) innerCl.bases << rawTypeInfo(base);
        } else {
            innerCl.ns = cl.ns.isEmpty() ? String(name) : cl.ns + "::" + name;
        }
        if (!getClasses(in, afterMatch, blockEnd, innerCl)) return false;

        start = blockEnd + 1;
        searchBegin = in.cbegin() + start;
    }

    // handle rest of previous
    if (start < end && !cl.typeName.isEmpty()) {
        if (!getMemberBlocks(in, start, end, cl, state)) return false;
    }

    if (state > 0) classes_.push_back(cl);
    return true;
}

bool HeaderParser::removeCommentsAndStringContents(std::string & header)
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
        lastError_ = "cannot find closing quotes at line: " + std::to_string(openLine);
        return false;
    }
    if (state == State::InBlockComment) {
        lastError_ = "cannot find closing comment at line: " + std::to_string(openLine);
        return false;
    }

    header.resize(writePos);
    return true;
}

bool HeaderParser::parse(const std::string & headerRef)
{
    if (headerRef.find("SERIALIZE_CLASS") == std::string::npos) return true;

    std::string header = headerRef;

    // line endings
    replaceAll(header, "\r\n", "\n");   // Windows
    replaceAll(header, "\r",   "\n");   // old Mac OS

    if (!removeCommentsAndStringContents(header)) return false;

    return getClasses(header, 0, header.length(), SerializeTypeInfo());
}

} // namespace
