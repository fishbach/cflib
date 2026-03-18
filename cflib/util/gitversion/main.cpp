/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>

static int usage(const char * prog)
{
    fprintf(stderr, "usage: %s <create> <git repo path> <path to gitversion.h>\n", prog);
    return 1;
}

static std::string runGit(const std::string & repoPath)
{
    std::string cmd = "git -C \"" + repoPath + "\" rev-parse HEAD 2>/dev/null";
    FILE * p = popen(cmd.c_str(), "r");
    if (!p) return {};
    char buf[256] = {};
    fgets(buf, sizeof(buf), p);
    pclose(p);
    std::string hash(buf);
    while (!hash.empty() && (hash.back() == '\n' || hash.back() == '\r' || hash.back() == ' '))
        hash.pop_back();
    return hash;
}

static int createHeader(const std::string & repoPath, const std::string & filename)
{
    std::string hash = runGit(repoPath);
    if (hash.empty()) {
        fprintf(stderr, "cannot determine git hash\n");
        return 5;
    }

    // Check if existing file already has the same hash
    {
        std::ifstream in(filename);
        if (in) {
            std::string content((std::istreambuf_iterator<char>(in)), {});
            const std::string marker = "GIT_VERSION \"";
            auto pos = content.find(marker);
            if (pos != std::string::npos && content.substr(pos + marker.size(), 40) == hash)
                return 0;
        }
    }

    std::ofstream out(filename, std::ios::trunc);
    if (!out) {
        fprintf(stderr, "cannot write: %s\n", filename.c_str());
        return 4;
    }
    out << "#pragma once\n\n#define GIT_VERSION \"" << hash << "\"\n";
    return 0;
}

int main(int argc, char * argv[])
{
    if (argc == 4 && strcmp(argv[1], "create") == 0) return createHeader(argv[2], argv[3]);
    return usage(argv[0]);
}
