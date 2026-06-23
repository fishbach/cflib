/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "javascript.h"

#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <dirent.h>
#include <filesystem>

using namespace cflib::util;
using namespace std::filesystem;

USE_LOG(LogCat::JS)

namespace cflib::serialize::generate {

namespace {

String getParameters(const SerializeFunctionTypeInfo & func)
{
    String js;
    bool isFirst = true;
    int id = 0;
    for (const auto & p : func.parameters) {
        if (isFirst) isFirst = false;
        else js << ", ";
        if (p.name.isEmpty()) js << "__param_" << String::number(++id);
        else js << p.name;
    }
    return js;
}

String getSerializeCode(const SerializeTypeInfo & ti, const String & name)
{
    String js;
    if (ti.type == SerializeTypeInfo::Class) {
        js << ".o(" << name << ")";
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName.startsWith("Pair<")) {
            js    << ".p(" << name << ", function(__e, __S) { __S"
                << getSerializeCode(ti.bases[0], "__e[0]")
                << getSerializeCode(ti.bases[1], "__e[1]")
                << "; })";
        } else if (ti.typeName.startsWith("List<")) {
            js    << ".map(" << name << ", function(__e, __S) { __S"
                << getSerializeCode(ti.bases[0], "__e") << "; })";
        } else if (ti.typeName.startsWith("Map<")) {
            js    << ".map(" << name << ", function(__e, __S) { __S"
                << getSerializeCode(ti.bases[0], "__e[0]")
                << getSerializeCode(ti.bases[1], "__e[1]")
                << "; })";
        } else {
            logWarn("no code for Container type '%1'", ti.typeName);
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if (ti.typeName == "DateTime") {
            js << ".i(!" << name << " ? 0 : " << name << ".getTime())";
        } else if (ti.typeName == "String") {
            js << ".s(" << name << ")";
        } else if (ti.typeName == "ByteArray") {
            js << ".a(" << name << ")";
        } else if (ti.typeName.indexOf("int") != -1) {
            js << ".i(" << name << ")";
        } else if (ti.typeName == "float32") {
            js << ".f32(" << name << ")";
        } else if (ti.typeName == "float64") {
            js << ".f64(" << name << ")";
        } else if (ti.typeName == "tribool") {
            js << ".i(" << name << " === true || " << name << " === 1 ? 1 : " << name << " === false || " << name << " === 2 ? 2 : 0)";
        } else if (ti.typeName == "bool") {
            js << ".i(" << name << " ? 1 : 0)";
        } else {
            logWarn("no code for Basic type '%1'", ti.typeName);
        }
    } else {    // SerializeTypeInfo::Null
        logWarn("no code for type 'Null'");
    }
    return js;
}

String getSerializeParameters(const SerializeFunctionTypeInfo & func)
{
    String js;
    int id = 0;
    for (const auto & p : func.parameters) {
        String name = p.name;
        if (name.isEmpty()) name << "__param_" << String::number(++id);
        js << getSerializeCode(p.type, name);
    }
    return js;
}

Set<SerializeTypeInfo> getCustomTypes(const SerializeTypeInfo & ti)
{
    Set<SerializeTypeInfo> types;
    if (ti.type == SerializeTypeInfo::Class) {
        types << ti;
    } else if (ti.type == SerializeTypeInfo::Container) {
        for (const auto & base : ti.bases) {
            types += getCustomTypes(base);
        }
    }
    return types;
}

SerializeTypeInfos getMemberTypes(const SerializeTypeInfo & ti)
{
    Set<SerializeTypeInfo> types;
    for (const auto & base : ti.bases) {
        types += getCustomTypes(base);
    }
    for (const auto & member : ti.members) {
        types += getCustomTypes(member.type);
    }
    for (const auto & func : ti.functions) {
        types += getCustomTypes(func.returnType);
        for (const auto & param : func.parameters) {
            types += getCustomTypes(param.type);
        }
    }
    for (const auto & func : ti.cfSignals) {
        types += getCustomTypes(func.returnType);
        for (const auto & param : func.parameters) {
            types += getCustomTypes(param.type);
        }
    }

    SerializeTypeInfos retval = types.toList();
    retval.sort();
    return retval;
}

String getDeserializeCode(const SerializeTypeInfo & ti)
{
    String js;
    if (ti.type == SerializeTypeInfo::Class) {
        String cl = ti.getName();
        cl.replace("::", "__");
        js << "new " << cl << "(__D.a())";
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName.startsWith("Pair<")) {
            js    << "(function(__data) { var __D = __ber.D(__data); return ["
                << getDeserializeCode(ti.bases[0]) << ", "
                << getDeserializeCode(ti.bases[1]) << "]; })(__D.a())";
        } else if (ti.typeName.startsWith("List<")) {
            js    << "__D.map(function(__D) { return "
                << getDeserializeCode(ti.bases[0]) << "; })";
        } else if (ti.typeName.startsWith("Map<")) {
            js    << "__D.map(function(__D) { return ["
                << getDeserializeCode(ti.bases[0]) << ", "
                << getDeserializeCode(ti.bases[1]) << "]; })";
        } else {
            logWarn("no code for Container type '%1'", ti.typeName);
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if (ti.typeName == "DateTime") {
            js << "(function() { var ti = __D.i(); return !ti ? null : new Date(ti); })()";
        } else if (ti.typeName == "String") {
            js << "__D.s()";
        } else if (ti.typeName == "ByteArray") {
            js << "__D.a()";
        } else if (ti.typeName.indexOf("int") != -1) {
            js << "__D.i()";
        } else if (ti.typeName == "float32") {
            js << "__D.f32()";
        } else if (ti.typeName == "float64") {
            js << "__D.f64()";
        } else if (ti.typeName == "tribool") {
            js << "(function() { var tb = __D.i(); return tb == 1 ? true : tb == 2 ? false : undefined; })()";
        } else if (ti.typeName == "bool") {
            js << "__D.i() ? true : false";
        } else {
            logWarn("no code for Basic type '%1'", ti.typeName);
        }
    } else {    // SerializeTypeInfo::Null
        logWarn("no code for type 'Null'");
    }
    return js;
}

inline String formatMembername(const SerializeVariableTypeInfo & vti)
{
    if (vti.name.endsWith("_")) return vti.name.left(vti.name.length() - 1);
    return vti.name;
}

inline String formatClassname(const SerializeTypeInfo & ti)
{
    String retval = ti.getName();
    retval.replace("::", "__");
    return retval;
}

String formatTypeConstruction(const SerializeTypeInfo & ti, const String & raw)
{
    String js;
    if (ti.type == SerializeTypeInfo::Class) {
        js << "new " << formatClassname(ti) << "(" << (raw == "null" ? "" : raw) << ")";
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName.startsWith("Pair<")) {
            if (raw == "null") js << "["
                << formatTypeConstruction(ti.bases[0], "null") << ", "
                << formatTypeConstruction(ti.bases[1], "null") << "]";
            else js << "(!" << raw << " ? ["
                << formatTypeConstruction(ti.bases[0], "null") << ", "
                << formatTypeConstruction(ti.bases[1], "null") << "] : ["
                << formatTypeConstruction(ti.bases[0], raw + "[0]") << ", "
                << formatTypeConstruction(ti.bases[1], raw + "[1]") << "])";
        } else if (ti.typeName.startsWith("List<")) {
            if (raw == "null") js << "[]";
            else js << "(" << raw << " || []).map(function(__e) { return "
                << formatTypeConstruction(ti.bases[0], "__e") << "; })";
        } else if (ti.typeName.startsWith("Map<")) {
            if (raw == "null") js << "[]";
            else js << "(" << raw << " || []).map(function(__e) { return ["
                << formatTypeConstruction(ti.bases[0], "__e[0]")
                << ", "
                << formatTypeConstruction(ti.bases[1], "__e[1]") << "]; })";
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if (ti.typeName == "DateTime") {
            if (raw == "null") js << "null";
            else               js << "(!" << raw << " ? null : new Date(" << raw << "))";
        } else if (ti.typeName == "String") {
            if (raw == "null") js << "null";
            else               js << "(!" << raw << " && " << raw << " !== '' ? null : " << raw << ")";
        } else if (ti.typeName == "ByteArray") {
            if (raw == "null") js << "null";
            else               js << "(!" << raw << " ? null : " << raw << ")";
        } else if (ti.typeName.indexOf("int") != -1 || ti.typeName.indexOf("float") != -1) {
            if (raw == "null") js << "0";
            else               js << "(!" << raw << " ? 0 : " << raw << ")";
        } else if (ti.typeName == "tribool") {
            if (raw == "null") js << "undefined";
            else               js << "(" << raw << " === true || " << raw << " === 1 ? true : " << raw << " === false || " << raw << " === 2 ? false : undefined)";
        } else if (ti.typeName == "bool") {
            if (raw == "null") js << "false";
            else               js << "(" << raw << " ? true : false)";
        }
    }
    return js;
}

// JS namespace for debugging
String getNsTypeName(const SerializeTypeInfo & ti, String & typeName)
{
    String js;

    String nsPrefix;
    typeName = ti.typeName;
    if (!ti.ns.isEmpty() || typeName.indexOf("::") != -1) {
        js << "var ";
        int i = 0;
        bool isFirst = true;
        for (const auto & ns : ti.ns.split("::")) {
            if (isFirst) {
                isFirst = false;
                js << ns << " = {";
            } else js << ns <<  ": {";
            nsPrefix << ns << '.';
            ++i;
        }
        StringList classNs = typeName.split("::");
        typeName = classNs.takeLast();
        for (const auto & ns : classNs) {
            if (isFirst) {
                isFirst = false;
                js << ns << " = {";
            } else js << ns <<  ": {";
            nsPrefix << ns << '.';
            ++i;
        }
        while (--i >= 0) js << '}';
        js << ";\n"
            "\n";
    }

    typeName = nsPrefix + typeName;
    return js;
}

String generateForClass(const SerializeTypeInfo & ti)
{
    String base;
    if (!ti.bases.isEmpty()) {
        base = ti.bases[0].getName();
        base.replace("::", "__");
    }

    String typeName;
    String js = getNsTypeName(ti, typeName);

    if (typeName == ti.typeName) js << "var ";
    js <<
        typeName << " = function() { " <<
        typeName << ".prototype.__init.apply(this, arguments); };\n"
        "__inherit.setBase(" << typeName << ", ";
    if (base.isEmpty()) js << "__inherit.Base";
    else js << base;
    js << ");\n";
    if (ti.classId != 0) js << typeName << ".__classId = " << String::number(ti.classId) << ";\n";
    js << typeName << ".prototype.__init = function(param) {\n";
    if (base.isEmpty()) js << "    " << typeName << ".__super.apply(this, arguments);\n";
    js <<
        "    if (param instanceof Uint8Array) {\n"
        "        var __D = __ber.D(param);\n"
        "        __D.n();\n";
    if (!base.isEmpty()) js << "        " << typeName << ".__super.call(this, __D.a());\n";
    for (const auto & vti : ti.members) {
        js << "        this." << formatMembername(vti) << " = " << getDeserializeCode(vti.type) << ";\n";
    }
    js <<
        "    } else {\n";
    if (!base.isEmpty()) js <<"        " << typeName << ".__super.apply(this, arguments);\n";
    js <<
        "        if (!param || typeof param != 'object') param = {};\n";
    for (const auto & vti : ti.members) {
        const String name = formatMembername(vti);
        js << "        this." << name << " = " << formatTypeConstruction(vti.type, "param." + name) << ";\n";
    }
    js <<
        "    }\n"
        "};\n"
        << typeName << ".prototype.__serialize = function(__S) {\n"
        "    __S";
    if (ti.classId != 0) js << ".i(" << typeName << ".__classId)";
    else                 js << ".n()";
    if (!base.isEmpty()) js << ".o(this, " << base << ".prototype.__serialize)";
    for (const auto & vti : ti.members) {
        js << getSerializeCode(vti.type, "this." + formatMembername(vti));
    }
    js << ".data();\n"
        "};\n"
        "export default " << typeName << ";\n";
    return js;
}

String generateForService(const SerializeTypeInfo & ti)
{
    String objName = ti.typeName;
    { char c = ti.typeName[0]; if (c >= 'A' && c <= 'Z') c += 32; objName[0] = c; }

    String typeName;
    String js = getNsTypeName(ti, typeName);

    if (typeName == ti.typeName) js << "var ";
    js <<
        typeName << " = function() {};\n"
        "var " << objName << " = new " << typeName << "();\n"
        "\n";

    if (!ti.cfSignals.isEmpty()) {
        js << objName << ".rsig = {\n";
        bool isFirst = true;
        for (const SerializeFunctionTypeInfo & func : ti.cfSignals) {
            if (isFirst) isFirst = false;
            else         js << ",\n";
            js << "    " << func.name << ": new __RSig(" << objName << ", '" << func.name << "', '" <<
                ti.typeName.toLower() << "', '" << func.name << "', function(";
            if (func.parameters.isEmpty()) {
                js << ") { " << objName << ".rsig." << func.name << ".fire(); })";
            } else {
                js << "__D) { " << objName << ".rsig." << func.name << ".fire(";
                bool isFirst2 = true;
                for (const SerializeVariableTypeInfo & p : func.parameters) {
                    if (isFirst2) isFirst2 = false;
                    else          js << ", ";
                    js << getDeserializeCode(p.type);
                }
                js << "); })";
            }
        }
        js << "\n"
            "};\n"
            "\n";
    }

    for (const auto & func : ti.functions) {
        if (!func.hasReturnValues()) {
            js << objName << '.' << func.name << " = function(" << getParameters(func) << ") {\n"
                "    __rmi.sendAsync(__ber.S().s('"
                << ti.typeName.toLower() << "').s('" << func.signature() << "')" << getSerializeParameters(func) << ".box(2));\n"
                "};\n"
                "\n";
            continue;
        }

        js << objName << '.' << func.name << " = function(" << getParameters(func)
            << (func.parameters.isEmpty() ? "" : ", ") << "callback, context) {\n"
            "    let __call = __replyFunc => __rmi.sendRequest(__ber.S().s('"
            << ti.typeName.toLower() << "').s('" << func.signature() << "')" << getSerializeParameters(func) << ".box(2), __data => {\n"
            "        let __D = __ber.D(__data);\n"
            "        __replyFunc(";
        if (func.returnValueCount() > 1) js << '[';
        if (func.returnType.type != SerializeTypeInfo::Null) js << getDeserializeCode(func.returnType);
        for (const auto & p : func.parameters) {
            if (!p.isRef) continue;
            js << ", " << getDeserializeCode(p.type);
        }
        if (func.returnValueCount() > 1) js << ']';
        js << ");\n"
            "    });\n"
            "    if (callback) __call(__rv => { callback." << (func.returnValueCount() > 1 ? "apply" : "call") << "(context, __rv); });\n"
            "    else return new Promise(__call);\n"
            "};\n"
            "\n";
    }

    js << "export default " << objName << ";\n";
    return js;
}

String generate(const SerializeTypeInfo & ti)
{
    auto pathPrefix = [&](const String & toPath) -> String {
        StringList from = ti.getNSPath().split('/');
        StringList to   = toPath.split('/');

        // equal
        if (from == to) return "./";

        // remove common prefixe
        while (!from.isEmpty() && !to.isEmpty()) {
            if (from.first() == to.first()) {
                from.takeFirst();
                to.takeFirst();
            } else break;
        }

        // build path
        String rv;
        for (size_t i = 0 ; i < from.size() ; ++i) rv += "../";
        rv += to.join('/');
        rv += "/";
        return rv;
    };

    String js;
    js <<
        "// ============================================================================\n"
        "// Generated by CFLib\n"
        "// ============================================================================\n"
        "\n";

    js << "import __ber from '" << pathPrefix("cflib/net") << "ber.js';\n";
    if (ti.isRMIService()) js << "import __rmi from '" << pathPrefix("cflib/net") << "rmi.js';\n";
    else                   js << "import __inherit from '" << pathPrefix("cflib/util") << "inherit.js';\n";
    if (!ti.cfSignals.isEmpty()) js << "import __RSig from '" << pathPrefix("cflib/net") << "rsig.js';\n";
    for (const SerializeTypeInfo & mTi : getMemberTypes(ti)) {
        String name = mTi.getName();
        name.replace("::", "__");
        js << "import " << name;
        js << " from '" << pathPrefix(mTi.getNSPath()) << mTi.typeName.toLower() << ".js';\n";
    }
    js << "\n";

    if (ti.isRMIService()) js << generateForService(ti);
    else                   js << generateForClass(ti);

    return js;
}

}

void generateJavaScript(const StructuredTypeInfos & typeInfos, const String & dest)
{
    const String destJs = dest + "/js/";

    // write services
    Set<String> files;
    for (const SerializeTypeInfo & ti : typeInfos.services()) {
        String file = ti.getFilePath() + ".js";
        files << file;
        String js = generate(ti);
        mkPath(destJs + ti.getNSPath());
        File::write(destJs + file, js.toUtf8());
    }

    // write classes
    for (const SerializeTypeInfo & ti : typeInfos.types()) {
        mkPath(destJs + ti.getNSPath());
        String path = ti.getFilePath() + ".js";
        files << path;
        logInfo("added to files: %1", path);
        String js = generate(ti);
        File::write(destJs + path, js.toUtf8());
    }

    // remove old
    auto opt = directory_options::follow_directory_symlink;
    for (const directory_entry & entry : recursive_directory_iterator(destJs.str(), opt)) {
        if (!entry.is_regular_file()) continue;
        String rel = entry.path().lexically_relative(destJs.str()).generic_string();
        if ((rel.contains("/dao/") || rel.contains("/services/")) && !files.contains(rel)) remove(entry.path());
    }

    // remove empty dirs
    std::function<void (const path &)> removeEmptyDirs = [&](const path & p) {
        for (const directory_entry & entry : directory_iterator(p, opt)) {
            if (!entry.is_directory()) continue;
            removeEmptyDirs(entry.path());
            if (is_empty(entry.path())) remove(entry);
        }
    };
    removeEmptyDirs(destJs.str());
}

} // namespace
