/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <string>
#include <vector>

class HeaderParser
{
public:
    struct Variable
    {
        std::string name;
        std::string type;
        bool isRef;

        Variable() : isRef(false) {}
    };
    typedef std::vector<Variable> Variables;

    struct Function
    {
        std::string returnType;
        std::string name;
        Variables parameters;
        Variables registerParameters;

        bool hasReturnValues() const;
    };
    typedef std::vector<Function> Functions;

    struct Class
    {
        std::string ns;
        std::string name;
        std::string base;
        bool doBaseSerialize;
        Variables members;
        Functions functions;
        Functions cfSignals;

        Class() : doBaseSerialize(false) {}
    };
    typedef std::vector<Class> Classes;

public:
    bool parse(const std::string & header);
    std::string lastError() const { return lastError_; }

    bool hasSerializeElements() const { return !classes_.empty(); }
    std::vector<Class> classes() const { return classes_; }

private:
    bool getVariables(const std::string & in, int start, int end, Class & cl);
    bool getParameters(const std::string & in, int start, int end, Variables & vars);
    bool getFunctions(const std::string & in, int start, int end, Class & cl);
    bool getCfSignals(const std::string & in, int start, int end, Class & cl);
    bool getMembers(const std::string & in, int start, int end, Class & cl, int & state);
    bool getMemberBlocks(const std::string & in, int start, int end, Class & cl, int & state);
    bool getClasses(const std::string & in, int start, int end, Class cl);
    bool removeCommentsAndStringContents(std::string & header);

private:
    std::string lastError_;
    Classes classes_;
};
