/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/util/cmdline.h>
#include <cflib/util/util.h>

#include <algorithm>
#include <format>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

using namespace cflib::util;

const std::regex callRE(
    R"((?:^|\W)(require|define)\s*\((\s*|\s*\[[\s\S]*?\]\s*,\s*)(?:function\s*)?\()",
    std::regex::ECMAScript);
const std::regex closingRE(R"(\(|\)|"|'|//|/\*)");

std::string output;
std::string basePath;

struct Space {
    std::map<std::string, Space> sub;
};
Space spaces;

std::set<std::string> available;
std::vector<std::string> order;
std::set<std::string> defined;
std::set<std::string> excludes;

int usage(const char * progName)
{
    std::cerr << std::format("usage: {} <main.js>\n", progName);
    return 1;
}

static std::vector<std::string> splitStr(const std::string & s, char delim)
{
    std::vector<std::string> result;
    size_t start = 0, pos;
    while ((pos = s.find(delim, start)) != std::string::npos) {
        result.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    result.push_back(s.substr(start));
    return result;
}

int findClosing(const std::string & src, int start)
{
    const int len = (int)src.size();
    int openCount = 0;
    while (true) {
        std::smatch m;
        auto searchFrom = src.cbegin() + start;
        if (!std::regex_search(searchFrom, src.cend(), m, closingRE)) break;
        start += (int)m.position() + (int)m.length();

        std::string found = m[0].str();
        if (found == "(") {
            ++openCount;
        } else if (found == ")") {
            if (openCount == 0) return start;
            --openCount;
        } else if (found == "'") {
            int pos = start;
            while (pos < len) {
                char c = src[pos++];
                if (c == '\\') ++pos;
                else if (c == '\'') { start = pos; break; }
            }
        } else if (found == "\"") {
            int pos = start;
            while (pos < len) {
                char c = src[pos++];
                if (c == '\\') ++pos;
                else if (c == '"') { start = pos; break; }
            }
        } else if (found == "//") {
            size_t pos = src.find_first_of("\r\n", start);
            if (pos == std::string::npos) break;
            start = (int)pos + 1;
        } else if (found == "/*") {
            size_t pos = src.find("*/", start);
            if (pos == std::string::npos) break;
            start = (int)pos + 1;
        }
    }

    std::cerr << "cannot find closing bracket\n";
    return len - 1;
}

std::vector<std::string> parseDepends(const std::string & depends)
{
    std::vector<std::string> rv;
    int pos = 0;
    int start = -1;
    while (pos < (int)depends.size()) {
        char c = depends[pos++];
        if (c == '\'' || c == '"') {
            if (start == -1) start = pos;
            else {
                rv.push_back(depends.substr(start, pos - start - 1));
                start = -1;
            }
        }
    }
    return rv;
}

void getDependencies(const std::string & name)
{
    std::string file(File::readUtf8(String(name.c_str())).str());

    int pos = 0;
    while (true) {
        std::smatch m;
        auto searchFrom = file.cbegin() + pos;
        std::regex_constants::match_flag_type flags = std::regex_constants::match_default;
        if (pos > 0) flags |= std::regex_constants::match_prev_avail;
        if (!std::regex_search(searchFrom, file.cend(), m, callRE, flags)) break;

        int prevPos = pos;
        pos = prevPos + (int)m.position() + (int)m.length();

        if (m[1].str() == "define") {
            std::string mod = name.substr(basePath.size(), name.size() - basePath.size() - 3);
            defined.insert(mod);
            std::vector<std::string> parts = splitStr(mod, '/');
            parts.pop_back();
            Space * sp = &spaces;
            for (const auto & p : parts) sp = &(sp->sub[p]);
        }

        for (const auto & dep : parseDepends(m[2].str())) {
            if (excludes.count(dep)) continue;
            std::string depFile = basePath + dep + ".js";
            if (!available.count(depFile)) getDependencies(depFile);
        }
    }

    available.insert(name);
    order.push_back(name);
}

void convertFile(const std::string & name)
{
    std::string file(File::readUtf8(String(name.c_str())).str());

    int pos = 0;
    while (true) {
        std::smatch m;
        auto searchFrom = file.cbegin() + pos;
        std::regex_constants::match_flag_type flags = std::regex_constants::match_default;
        if (pos > 0) flags |= std::regex_constants::match_prev_avail;
        if (!std::regex_search(searchFrom, file.cend(), m, callRE, flags)) break;

        int prevPos = pos;
        pos = prevPos + (int)m.position() + (int)m.length();

        int start   = prevPos + (int)m.position(1);
        int end     = prevPos + (int)m.position(2) + (int)m.length(2);
        int closing = findClosing(file, end);

        std::string params = "(";
        bool first = true;
        for (std::string dep : parseDepends(m[2].str())) {
            if (first) first = false; else params += ", ";
            if (defined.count(dep)) {
                std::replace(dep.begin(), dep.end(), '/', '.');
                params += "mod." + dep;
            } else {
                params += "null";
            }
        }
        params += ")";
        file.insert(closing, params);

        std::string head;
        if (m[1].str() == "define") {
            std::string mod = name.substr(basePath.size(), name.size() - basePath.size() - 3);
            std::replace(mod.begin(), mod.end(), '/', '.');
            head = "mod." + mod + " = ";
        }
        head += "(";
        file.replace(start, end - start, head);
        pos += (int)head.size() - end + start;
    }
    output += file;
}

void printSpaces(const Space & space)
{
    if (space.sub.empty()) {
        output += "{}";
        return;
    }
    output += "{ ";
    bool first = true;
    for (const auto & [key, val] : space.sub) {
        if (first) first = false; else output += ", ";
        output += key + ": ";
        printSpaces(val);
    }
    output += " }";
}

int main(int argc, char * argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help    ('h', "help"                     ); cmdLine << help;
    Option exclude ('e', "exclude", true, true, true); cmdLine << exclude;
    Arg    fileName(false                           ); cmdLine << fileName;
    if (!cmdLine.parse() || help.isSet()) return usage(argv[0]);

    std::string mainFile(fileName.value().constData(), fileName.value().size());
    if (access(mainFile.c_str(), R_OK) != 0) return usage(argv[0]);

    for (const auto & a : exclude.values())
        excludes.insert(std::string(a.constData(), a.size()));

    size_t slashPos = mainFile.rfind('/');
    if (slashPos != std::string::npos) basePath = mainFile.substr(0, slashPos + 1);

    getDependencies(mainFile);

    output += "var mod = ";
    printSpaces(spaces);
    output += ";\n";

    for (const auto & name : order) {
        output +=
            "\n// ============================================================================\n"
            "// " + name + "\n"
            "// ============================================================================\n\n";
        convertFile(name);
        output += "\n";
    }

    std::cout << output;
    return 0;
}
