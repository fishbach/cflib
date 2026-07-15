/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/headerparser.h>
#include <cflib/util/util.h>

#include <format>
#include <ostream>
#include <string>
#include <vector>

using namespace cflib::serialize;
using namespace cflib::util;

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

std::string calcClassHash(const SerializeTypeInfo & cl)
{
    std::string fullName = cl.ns.str() + "::" + cl.typeName.str();
    ByteArray ba((const char *)fullName.c_str(), (size_t)fullName.size());
    return std::to_string(calcCRC32(ba));
}

void writeMethods(std::ostream & out, const SerializeTypeInfo & cl)
{
    if (cl.hasBase()) {
        out << std::format(
            "const cflib::serialize::impl::RegisterClass<{}> {}::cflib_serialize_impl_registerClass;\n"
            "\n",
            cl.typeName.c_str(), cl.typeName.c_str());
    }

    if (cl.members.isEmpty() && !cl.hasBase()) {
        out << std::format(
            "template<typename T> void {}::serialize(T &) const {{}}\n"
            "template<typename T> void {}::deserialize(T &) {{}}\n",
            cl.typeName.c_str(), cl.typeName.c_str());
    } else {
        out << std::format(
            "template<typename T> void {}::serialize(T & ser) const {{\n"
            "    ser << ",
            cl.typeName.c_str());
        if (cl.hasBase()) {
            out << std::format("(uint32){} << (const {} &)*this",
                calcClassHash(cl), cl.getBase().typeName.c_str());
        } else {
            out << "cflib::serialize::Placeholder()";
        }
        for (const SerializeVariableTypeInfo & m : cl.members) {
            if (m.name.isEmpty()) out << " << cflib::serialize::Placeholder()";
            else out << std::format(" << {}", m.name.c_str());
        }
        out << std::format(
            ";\n"
            "}}\n"
            "template<typename T> void {}::deserialize(T & ser) {{\n"
            "    ser >> cflib::serialize::Placeholder()",
            cl.typeName.c_str());
        if (cl.hasBase()) {
            out << std::format(" >> ({} &)*this", cl.getBase().typeName.c_str());
        }
        for (const SerializeVariableTypeInfo & m : cl.members) {
            if (m.name.isEmpty()) out << " >> cflib::serialize::Placeholder()";
            else out << std::format(" >> {}", m.name.c_str());
        }
        out << ";\n"
               "}\n";
    }
    out << std::format(
        "template void {}::serialize(cflib::serialize::impl::BERSerializerBase &) const;\n"
        "template void {}::deserialize(cflib::serialize::impl::BERDeserializerBase &);\n",
        cl.typeName.c_str(), cl.typeName.c_str());
}

void writeFunctionSwitch(const SerializeFunctionTypeInfos & list, bool withReturnValues, std::ostream & out)
{
    out << "    switch (__callNo) {\n";
    int i = 0;
    for (const SerializeFunctionTypeInfo & f : list) {
        ++i;
        if (f.hasReturnValues() != withReturnValues) continue;

        out << std::format("        case {}: {{\n", i);
        if (!f.parameters.isEmpty()) {
            int id = 0;
            for (const SerializeVariableTypeInfo & p : f.parameters) {
                out << std::format("            {} ", p.type.typeName.c_str());
                if (p.name.isEmpty()) out << std::format("__param_{}", ++id);
                else out << std::format("__{}", p.name.c_str());
                out << ";\n";
            }
            out << "            __deser";
            id = 0;
            for (const SerializeVariableTypeInfo & p : f.parameters) {
                out << " >> ";
                if (p.name.isEmpty()) out << std::format("__param_{}", ++id);
                else out << std::format("__{}", p.name.c_str());
            }
            out << ";\n";
        }
        out << "            ";
        if (!f.returnType.isNull()) out << "__ser << ";
        out << std::format("{}(", f.name.c_str());
        int id = 0;
        bool isFirst = true;
        for (const SerializeVariableTypeInfo & p : f.parameters) {
            if (isFirst) isFirst = false;
            else out << ", ";
            if (p.name.isEmpty()) out << std::format("__param_{}", ++id);
            else out << std::format("__{}", p.name.c_str());
        }
        out << ")";
        if (f.hasReturnValues()) {
            if (f.returnType.isNull()) {
                out << ";\n            __ser";
            }
            id = 0;
            for (const SerializeVariableTypeInfo & p : f.parameters) {
                if (!p.isRef) continue;
                out << " << ";
                if (p.name.isEmpty()) out << std::format("__param_{}", ++id);
                else out << std::format("__{}", p.name.c_str());
            }
        }
        out << ";\n"
               "            return;\n"
               "        }\n";
    }
    out << "    }\n";
}

void writeFunction(const SerializeFunctionTypeInfo & f, std::ostream & out)
{
    out << std::format(
        "    {{\n"
        "        cflib::serialize::SerializeFunctionTypeInfo func;\n"
        "        func.name = \"{}\";\n",
        f.name.c_str());
    if (!f.returnType.isNull()) {
        out << std::format("        func.returnType = cflib::serialize::impl::fromType<{}>();\n",
            f.returnType.typeName.c_str());
    }
    if (!f.parameters.isEmpty()) {
        out << "        func.parameters";
        for (const SerializeVariableTypeInfo & m : f.parameters) {
            out << std::format(
                "\n            << cflib::serialize::SerializeVariableTypeInfo(\"{}\", cflib::serialize::impl::fromType<{}>(), {})",
                m.name.c_str(), m.type.typeName.c_str(), m.isRef ? "true" : "false");
        }
        out << ";\n";
    }
    if (!f.registerParameters.isEmpty()) {
        out << "        func.registerParameters";
        for (const SerializeVariableTypeInfo & m : f.registerParameters) {
            out << std::format(
                "\n            << cflib::serialize::SerializeVariableTypeInfo(\"{}\", cflib::serialize::impl::fromType<{}>(), {})",
                m.name.c_str(), m.type.typeName.c_str(), m.isRef ? "true" : "false");
        }
        out << ";\n";
    }
}

}

int genSerialize(const std::string & headerName, const HeaderParser & hp, std::ostream & out)
{
    if (!hp.hasSerializeElements()) {
        out << "// empty";
        return 0;
    }

    out << std::format(
        "#include \"{}\"\n"
        "\n"
        "#include <cflib/serialize/impl/serializetypeinfoimpl.h>\n"
        "#include <cflib/serialize/serializeber.h>\n",
        headerName.c_str());

    if (!hp.permissions().isEmpty()) {
        out << "\n";
        for (const HeaderParser::NameDesc & nd : hp.permissions()) {
            String var = nd.name;
            var.replace(".", "::");
            out << "cflib::serialize::CFPermission " << var.str() << "(\"" << nd.name.str() << "\"";
            if (!nd.desc.isNull()) out << ", \"" << nd.desc.str() << "\"";
            out << ");\n";
        }
    }

    for (const SerializeTypeInfo & cl : hp.classes()) {

        out << "\n";
        std::vector<std::string> nsList = splitStr(cl.ns.str(), "::");
        bool isFirst = true;
        for (const std::string & ns : nsList) {
            if (isFirst) isFirst = false;
            else out << " ";
            out << std::format("namespace {} {{", ns.c_str());
        }
        if (!nsList.empty()) out << "\n";

        writeMethods(out, cl);

        out << std::format(
            "cflib::serialize::SerializeTypeInfo {}::serializeTypeInfo() {{\n"
            "    cflib::serialize::SerializeTypeInfo retval;\n"
            "    retval.type = cflib::serialize::SerializeTypeInfo::Class;\n",
            cl.typeName.c_str());
        if (cl.hasBase()) out << std::format("    retval.classId = {};\n", calcClassHash(cl).c_str());
        out << std::format(
            "    retval.ns = \"{}\";\n"
            "    retval.typeName = \"{}\";\n",
            cl.ns.c_str(), cl.typeName.c_str());
        if (cl.hasBase()) {
            out << std::format("    retval.bases.push_back(cflib::serialize::impl::fromType<{}>());\n",
                cl.getBase().typeName.c_str());
        }
        if (!cl.members.isEmpty()) {
            out << "    retval.members";
            for (const SerializeVariableTypeInfo & m : cl.members) {
                if (m.name.isEmpty()) {
                    out << "\n        << cflib::serialize::SerializeVariableTypeInfo()";
                } else {
                    out << std::format(
                        "\n        << cflib::serialize::SerializeVariableTypeInfo(\"{}\", cflib::serialize::impl::fromType<{}>())",
                        m.name.c_str(), m.type.typeName.c_str());
                }
            }
            out << ";\n";
        }
        for (const SerializeFunctionTypeInfo & f : cl.functions) {
            writeFunction(f, out);
            out << "        retval.functions.push_back(func);\n"
                   "    }\n";
        }
        for (const SerializeFunctionTypeInfo & f : cl.cfSignals) {
            writeFunction(f, out);
            out << "        retval.cfSignals.push_back(func);\n"
                   "    }\n";
        }
        out << "    return retval;\n"
               "}\n";

        if (cl.isRMIService()) {
            bool existsWithReturnValues                 = false;
            bool existsWithoutReturnValues              = false;
            bool existsWithReturnValuesAndParameters    = false;
            bool existsWithoutReturnValuesAndParameters = false;
            for (const SerializeFunctionTypeInfo & f : cl.functions) {
                if (f.hasReturnValues()) {
                    existsWithReturnValues = true;
                    if (!f.parameters.isEmpty()) existsWithReturnValuesAndParameters = true;
                } else {
                    existsWithoutReturnValues = true;
                    if (!f.parameters.isEmpty()) existsWithoutReturnValuesAndParameters = true;
                }
                if (existsWithReturnValues              && existsWithoutReturnValues &&
                    existsWithReturnValuesAndParameters && existsWithoutReturnValuesAndParameters) break;
            }
            if (existsWithoutReturnValues) {
                out << std::format("void {}::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &{}, uint __callNo) {{\n",
                    cl.typeName.c_str(), existsWithoutReturnValuesAndParameters ? " __deser" : "");
                writeFunctionSwitch(cl.functions, false, out);
                out << "}\n";
            } else {
                out << std::format("void {}::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &, uint) {{}}\n",
                    cl.typeName.c_str());
            }
            if (existsWithReturnValues) {
                out << std::format("void {}::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &{}, uint __callNo, cflib::serialize::BERSerializer & __ser) {{\n",
                    cl.typeName.c_str(), existsWithReturnValuesAndParameters ? " __deser" : "");
                writeFunctionSwitch(cl.functions, true, out);
                out << "}\n";
            } else {
                out << std::format("void {}::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &, uint, cflib::serialize::BERSerializer &) {{}}\n",
                    cl.typeName.c_str());
            }

            if (cl.cfSignals.isEmpty()) {
                out << std::format("cflib::net::RSigBase * {}::getCfSignal(uint) {{ return 0; }}\n", cl.typeName.c_str());
            } else {
                out << std::format(
                    "cflib::net::RSigBase * {}::getCfSignal(uint __sigNo) {{\n"
                    "    switch (__sigNo) {{\n",
                    cl.typeName.c_str());
                int i = 0;
                for (const SerializeFunctionTypeInfo & f : cl.cfSignals) {
                    ++i;
                    out << std::format("        case {}: return &{};\n", i, f.name.c_str());
                }
                out << "    }\n"
                       "    return 0;\n"
                       "}\n";
            }
        }

        for (size_t i = 0 ; i < nsList.size() ; ++i) out << "}";
        if (!nsList.empty()) out << "\n";
    }

    return 0;
}
