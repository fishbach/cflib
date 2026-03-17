/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/ser/headerparser.h>
#include <cflib/util/util.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

namespace {

// Simple helper to split a string
std::vector<std::string> splitStr(const std::string & s, const std::string & delim)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t pos;
    while ((pos = s.find(delim, start)) != std::string::npos) {
        std::string part = s.substr(start, pos - start);
        if (!part.empty()) result.push_back(part);
        start = pos + delim.size();
    }
    std::string last = s.substr(start);
    if (!last.empty()) result.push_back(last);
    return result;
}

std::string calcClassHash(const HeaderParser::Class & cl)
{
    std::string fullName = cl.ns + "::" + cl.name;
    CFByteArray ba((const char *)fullName.c_str(), (cfsize_t)fullName.size());
    return std::to_string(cflib::util::calcCRC32(ba));
}

void writeMethods(FILE * out, const HeaderParser::Class & cl)
{
    if (cl.doBaseSerialize) {
        fprintf(out,
            "const cflib::serialize::impl::RegisterClass<%s> %s::cflib_serialize_impl_registerClass;\n"
            "\n",
            cl.name.c_str(), cl.name.c_str());
    }

    if (cl.members.empty() && !cl.doBaseSerialize) {
        fprintf(out,
            "template<typename T> void %s::serialize(T &) const {}\n"
            "template<typename T> void %s::deserialize(T &) {}\n",
            cl.name.c_str(), cl.name.c_str());
    } else {
        fprintf(out,
            "template<typename T> void %s::serialize(T & ser) const {\n"
            "    ser << ",
            cl.name.c_str());
        if (cl.doBaseSerialize) {
            fprintf(out, "(cfuint32)%s << (const %s &)*this",
                calcClassHash(cl).c_str(), cl.base.c_str());
        } else {
            fprintf(out, "cflib::serialize::Placeholder()");
        }
        for (const HeaderParser::Variable & m : cl.members) {
            if (m.name.empty()) fprintf(out, " << cflib::serialize::Placeholder()");
            else fprintf(out, " << %s", m.name.c_str());
        }
        fprintf(out,
            ";\n"
            "}\n"
            "template<typename T> void %s::deserialize(T & ser) {\n"
            "    ser >> cflib::serialize::Placeholder()",
            cl.name.c_str());
        if (cl.doBaseSerialize) {
            fprintf(out, " >> (%s &)*this", cl.base.c_str());
        }
        for (const HeaderParser::Variable & m : cl.members) {
            if (m.name.empty()) fprintf(out, " >> cflib::serialize::Placeholder()");
            else fprintf(out, " >> %s", m.name.c_str());
        }
        fprintf(out,
            ";\n"
            "}\n");
    }
    fprintf(out,
        "template void %s::serialize(cflib::serialize::impl::BERSerializerBase &) const;\n"
        "template void %s::deserialize(cflib::serialize::impl::BERDeserializerBase &);\n",
        cl.name.c_str(), cl.name.c_str());
}

void writeFunctionSwitch(const HeaderParser::Functions & list, bool withReturnValues, FILE * out)
{
    fprintf(out, "    switch (__callNo) {\n");
    int i = 0;
    for (const HeaderParser::Function & f : list) {
        ++i;
        if (f.hasReturnValues() != withReturnValues) continue;

        fprintf(out, "        case %d: {\n", i);
        if (!f.parameters.empty()) {
            int id = 0;
            for (const HeaderParser::Variable & p : f.parameters) {
                fprintf(out, "            %s ", p.type.c_str());
                if (p.name.empty()) fprintf(out, "__param_%d", ++id);
                else fprintf(out, "__%s", p.name.c_str());
                fprintf(out, ";\n");
            }
            fprintf(out, "            __deser");
            id = 0;
            for (const HeaderParser::Variable & p : f.parameters) {
                fprintf(out, " >> ");
                if (p.name.empty()) fprintf(out, "__param_%d", ++id);
                else fprintf(out, "__%s", p.name.c_str());
            }
            fprintf(out, ";\n");
        }
        fprintf(out, "            ");
        if (f.returnType != "void") fprintf(out, "__ser << ");
        fprintf(out, "%s(", f.name.c_str());
        int id = 0;
        bool isFirst = true;
        for (const HeaderParser::Variable & p : f.parameters) {
            if (isFirst) isFirst = false;
            else fprintf(out, ", ");
            if (p.name.empty()) fprintf(out, "__param_%d", ++id);
            else fprintf(out, "__%s", p.name.c_str());
        }
        fprintf(out, ")");
        if (f.hasReturnValues()) {
            if (f.returnType == "void") {
                fprintf(out, ";\n            __ser");
            }
            id = 0;
            for (const HeaderParser::Variable & p : f.parameters) {
                if (!p.isRef) continue;
                fprintf(out, " << ");
                if (p.name.empty()) fprintf(out, "__param_%d", ++id);
                else fprintf(out, "__%s", p.name.c_str());
            }
        }
        fprintf(out, ";\n"
            "            return;\n"
            "        }\n");
    }
    fprintf(out, "    }\n");
}

void writeFunction(const HeaderParser::Function & f, FILE * out)
{
    fprintf(out,
        "    {\n"
        "        cflib::serialize::SerializeFunctionTypeInfo func;\n"
        "        func.name = \"%s\";\n",
        f.name.c_str());
    if (f.returnType != "void") {
        fprintf(out, "        func.returnType = cflib::serialize::impl::fromType<%s>();\n",
            f.returnType.c_str());
    }
    if (!f.parameters.empty()) {
        fprintf(out, "        func.parameters");
        for (const HeaderParser::Variable & m : f.parameters) {
            fprintf(out,
                "\n            << cflib::serialize::SerializeVariableTypeInfo(\"%s\", cflib::serialize::impl::fromType<%s>(), %s)",
                m.name.c_str(), m.type.c_str(), m.isRef ? "true" : "false");
        }
        fprintf(out, ";\n");
    }
    if (!f.registerParameters.empty()) {
        fprintf(out, "        func.registerParameters");
        for (const HeaderParser::Variable & m : f.registerParameters) {
            fprintf(out,
                "\n            << cflib::serialize::SerializeVariableTypeInfo(\"%s\", cflib::serialize::impl::fromType<%s>(), %s)",
                m.name.c_str(), m.type.c_str(), m.isRef ? "true" : "false");
        }
        fprintf(out, ";\n");
    }
}

}

int genSerialize(const std::string & headerName, const HeaderParser & hp, FILE * out)
{
    if (!hp.hasSerializeElements()) {
        fprintf(out, "// empty");
        return 0;
    }

    fprintf(out,
        "#include \"%s\"\n"
        "\n"
        "#include <cflib/serialize/impl/serializetypeinfoimpl.h>\n"
        "#include <cflib/serialize/serializeber.h>\n",
        headerName.c_str());

    for (const HeaderParser::Class & cl : hp.classes()) {
        fprintf(out, "\n");
        std::vector<std::string> nsList = splitStr(cl.ns, "::");
        bool isFirst = true;
        for (const std::string & ns : nsList) {
            if (isFirst) isFirst = false;
            else fprintf(out, " ");
            fprintf(out, "namespace %s {", ns.c_str());
        }
        if (!nsList.empty()) fprintf(out, "\n");

        writeMethods(out, cl);

        fprintf(out,
            "cflib::serialize::SerializeTypeInfo %s::serializeTypeInfo() {\n"
            "    cflib::serialize::SerializeTypeInfo retval;\n"
            "    retval.type = cflib::serialize::SerializeTypeInfo::Class;\n",
            cl.name.c_str());
        if (cl.doBaseSerialize) fprintf(out, "    retval.classId = %s;\n", calcClassHash(cl).c_str());
        fprintf(out,
            "    retval.ns = \"%s\";\n"
            "    retval.typeName = \"%s\";\n",
            cl.ns.c_str(), cl.name.c_str());
        if (cl.doBaseSerialize) {
            fprintf(out, "    retval.bases.push_back(cflib::serialize::impl::fromType<%s>());\n", cl.base.c_str());
        }
        if (!cl.members.empty()) {
            fprintf(out, "    retval.members");
            for (const HeaderParser::Variable & m : cl.members) {
                if (m.name.empty()) {
                    fprintf(out, "\n        << cflib::serialize::SerializeVariableTypeInfo()");
                } else {
                    fprintf(out, "\n        << cflib::serialize::SerializeVariableTypeInfo(\"%s\", cflib::serialize::impl::fromType<%s>())",
                        m.name.c_str(), m.type.c_str());
                }
            }
            fprintf(out, ";\n");
        }
        for (const HeaderParser::Function & f : cl.functions) {
            writeFunction(f, out);
            fprintf(out,
                "        retval.functions.push_back(func);\n"
                "    }\n");
        }
        for (const HeaderParser::Function & f : cl.cfSignals) {
            writeFunction(f, out);
            fprintf(out,
                "        retval.cfSignals.push_back(func);\n"
                "    }\n");
        }
        fprintf(out,
            "    return retval;\n"
            "}\n");

        if (!cl.functions.empty() || !cl.cfSignals.empty()) {
            bool existsWithReturnValues                 = false;
            bool existsWithoutReturnValues              = false;
            bool existsWithReturnValuesAndParameters    = false;
            bool existsWithoutReturnValuesAndParameters = false;
            for (const HeaderParser::Function & f : cl.functions) {
                if (f.hasReturnValues()) {
                    existsWithReturnValues = true;
                    if (!f.parameters.empty()) existsWithReturnValuesAndParameters = true;
                } else {
                    existsWithoutReturnValues = true;
                    if (!f.parameters.empty()) existsWithoutReturnValuesAndParameters = true;
                }
                if (existsWithReturnValues              && existsWithoutReturnValues &&
                    existsWithReturnValuesAndParameters && existsWithoutReturnValuesAndParameters) break;
            }
            if (existsWithoutReturnValues) {
                fprintf(out, "void %s::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &%s, uint __callNo) {\n",
                    cl.name.c_str(), existsWithoutReturnValuesAndParameters ? " __deser" : "");
                writeFunctionSwitch(cl.functions, false, out);
                fprintf(out, "}\n");
            } else {
                fprintf(out, "void %s::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &, uint) {}\n",
                    cl.name.c_str());
            }
            if (existsWithReturnValues) {
                fprintf(out, "void %s::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &%s, uint __callNo, cflib::serialize::BERSerializer & __ser) {\n",
                    cl.name.c_str(), existsWithReturnValuesAndParameters ? " __deser" : "");
                writeFunctionSwitch(cl.functions, true, out);
                fprintf(out, "}\n");
            } else {
                fprintf(out, "void %s::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &, uint, cflib::serialize::BERSerializer &) {}\n",
                    cl.name.c_str());
            }

            if (cl.cfSignals.empty()) {
                fprintf(out, "cflib::net::RSigBase * %s::getCfSignal(uint) { return 0; }\n", cl.name.c_str());
            } else {
                fprintf(out,
                    "cflib::net::RSigBase * %s::getCfSignal(uint __sigNo) {\n"
                    "    switch (__sigNo) {\n",
                    cl.name.c_str());
                int i = 0;
                for (const HeaderParser::Function & f : cl.cfSignals) {
                    ++i;
                    fprintf(out, "        case %d: return &%s;\n", i, f.name.c_str());
                }
                fprintf(out,
                    "    }\n"
                    "    return 0;\n"
                    "}\n");
            }
        }

        for (size_t i = 0 ; i < nsList.size() ; ++i) fprintf(out, "}");
        if (!nsList.empty()) fprintf(out, "\n");
    }

    return 0;
}
