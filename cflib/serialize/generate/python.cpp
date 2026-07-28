/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "python.h"

#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <dirent.h>
#include <filesystem>

using namespace std::filesystem;

USE_LOG(LogCat::JS)

namespace cflib::serialize::generate {

namespace {

// ---------------------------------------------------------------------------
// name helpers
// ---------------------------------------------------------------------------

// fully qualified class alias, e.g. chatserver::dao::Message -> chatserver__dao__Message
inline String formatClassname(const SerializeTypeInfo & ti)
{
    String rv = ti.getName();
    rv.replace("::", "__");
    return rv;
}

// python module path, e.g. chatserver::dao::Message -> chatserver.dao.message
inline String modulePath(const String & fqName)
{
    String rv = fqName;
    return rv.replace("::", ".").toLower();
}

// short class name (last component), e.g. chatserver::dao::Message -> Message
inline String shortClassName(const SerializeTypeInfo & ti)
{
    StringList parts = ti.getName().split("::");
    return parts.takeLast();
}

// short class name from a fully qualified type name string
inline String shortClassName(const String & fqName)
{
    StringList parts = fqName.split("::");
    return parts.takeLast();
}

inline String formatMembername(const SerializeVariableTypeInfo & vti)
{
    if (vti.name.endsWith("_")) return vti.name.left(vti.name.length() - 1);
    return vti.name;
}

String getParameters(const SerializeFunctionTypeInfo & func)
{
    String py;
    int id = 0;
    for (const auto & p : func.parameters) {
        py << ", ";
        if (p.name.isEmpty()) py << "_param_" << String::number(++id);
        else py << p.name;
    }
    return py;
}

// ---------------------------------------------------------------------------
// import collection (mirrors javascript.cpp)
// ---------------------------------------------------------------------------

Set<String> getCustomTypes(const SerializeTypeInfo & ti)
{
    Set<String> types;
    if (ti.type == SerializeTypeInfo::Class) {
        types << ti.getName();
    } else if (ti.type == SerializeTypeInfo::Container) {
        for (const auto & base : ti.bases) types += getCustomTypes(base);
    }
    return types;
}

StringList getMemberTypes(const SerializeTypeInfo & ti)
{
    Set<String> types;
    for (const auto & base : ti.bases) types += getCustomTypes(base);
    for (const auto & member : ti.members) types += getCustomTypes(member.type);
    for (const auto & func : ti.functions) {
        types += getCustomTypes(func.returnType);
        for (const auto & param : func.parameters) types += getCustomTypes(param.type);
    }
    for (const auto & func : ti.cfSignals) {
        types += getCustomTypes(func.returnType);
        for (const auto & param : func.parameters) types += getCustomTypes(param.type);
    }
    StringList retval = types.toList();
    retval.sort();
    return retval;
}

// ---------------------------------------------------------------------------
// serialize / deserialize code
// ---------------------------------------------------------------------------

// returns a chain of Serializer calls appended after "_S", serializing `name`
String getSerializeCode(const SerializeTypeInfo & ti, const String & name)
{
    String py;
    if (ti.type == SerializeTypeInfo::Class) {
        py << ".o(" << name << ")";
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName == "Pair") {
            py << ".p(" << name << ", lambda _e, _S: _S"
               << getSerializeCode(ti.bases[0], "_e[0]")
               << getSerializeCode(ti.bases[1], "_e[1]") << ")";
        } else if (ti.typeName == "List") {
            py << ".map(" << name << ", lambda _e, _S: _S"
               << getSerializeCode(ti.bases[0], "_e") << ")";
        } else if (ti.typeName == "Map") {
            py << ".map((" << name << ".items() if " << name << " else None), lambda _e, _S: _S"
               << getSerializeCode(ti.bases[0], "_e[0]")
               << getSerializeCode(ti.bases[1], "_e[1]") << ")";
        } else {
            logWarn("no code for Container type '%1'", ti.typeName);
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if      (ti.typeName == "DateTime")     py << ".i(_cf.to_millis(" << name << "))";
        else if (ti.typeName == "String")       py << ".s(" << name << ")";
        else if (ti.typeName == "ByteArray")    py << ".a(" << name << ")";
        else if (ti.typeName.indexOf("int") != -1) py << ".i(" << name << ")";
        else if (ti.typeName == "float32")      py << ".f32(" << name << ")";
        else if (ti.typeName == "float64")      py << ".f64(" << name << ")";
        else if (ti.typeName == "tribool")      py << ".i(_cf.to_tribool(" << name << "))";
        else if (ti.typeName == "bool")         py << ".i(1 if " << name << " else 0)";
        else logWarn("no code for Basic type '%1'", ti.typeName);
    } else {
        logWarn("no code for type 'Null'");
    }
    return py;
}

// returns a python expression reading the value from a Deserializer named _D
String getDeserializeCode(const SerializeTypeInfo & ti)
{
    String py;
    if (ti.type == SerializeTypeInfo::Class) {
        py << formatClassname(ti) << "._deserialize(_D.a())";
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName == "Pair") {
            py << "(lambda _D: (" << getDeserializeCode(ti.bases[0]) << ", "
               << getDeserializeCode(ti.bases[1]) << "))(_ber.D(_D.a()))";
        } else if (ti.typeName == "List") {
            py << "_D.map(lambda _D: " << getDeserializeCode(ti.bases[0]) << ")";
        } else if (ti.typeName == "Map") {
            py << "dict(_D.map(lambda _D: (" << getDeserializeCode(ti.bases[0]) << ", "
               << getDeserializeCode(ti.bases[1]) << ")))";
        } else {
            logWarn("no code for Container type '%1'", ti.typeName);
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if      (ti.typeName == "DateTime")     py << "_cf.from_millis(_D.i())";
        else if (ti.typeName == "String")       py << "_D.s()";
        else if (ti.typeName == "ByteArray")    py << "_D.a()";
        else if (ti.typeName.indexOf("int") != -1) py << "_D.i()";
        else if (ti.typeName == "float32")      py << "_D.f32()";
        else if (ti.typeName == "float64")      py << "_D.f64()";
        else if (ti.typeName == "tribool")      py << "_cf.from_tribool(_D.i())";
        else if (ti.typeName == "bool")         py << "(_D.i() != 0)";
        else logWarn("no code for Basic type '%1'", ti.typeName);
    } else {
        logWarn("no code for type 'Null'");
    }
    return py;
}

// serialize call chain for the parameters of a remote call
String getSerializeParameters(const SerializeFunctionTypeInfo & func)
{
    String py;
    int id = 0;
    for (const auto & p : func.parameters) {
        String name = p.name;
        if (name.isEmpty()) name << "_param_" << String::number(++id);
        py << getSerializeCode(p.type, name);
    }
    return py;
}

String pyDefault(const SerializeTypeInfo & ti)
{
    if (ti.type == SerializeTypeInfo::Basic) {
        if (ti.typeName.indexOf("int") != -1) return "0";
        if (ti.typeName == "float32" || ti.typeName == "float64") return "0.0";
        if (ti.typeName == "bool") return "False";
    }
    return "None";
}

// ---------------------------------------------------------------------------
// data class generation
// ---------------------------------------------------------------------------

String generateForClass(const SerializeTypeInfo & ti)
{
    const String name = shortClassName(ti);
    String baseAlias;
    if (!ti.bases.isEmpty()) baseAlias = formatClassname(ti.bases[0]);

    String py;

    // class head
    py << "class " << name << "(" << (baseAlias.isEmpty() ? "object" : baseAlias) << "):\n";
    if (ti.classId != 0) py << "    _class_id = " << String::number(ti.classId) << "\n\n";

    // __init__
    py << "    def __init__(self";
    for (const auto & vti : ti.members) {
        py << ", " << formatMembername(vti) << "=" << pyDefault(vti.type);
    }
    py << "):\n";
    if (!baseAlias.isEmpty()) py << "        super().__init__()\n";
    if (ti.members.isEmpty() && baseAlias.isEmpty()) py << "        pass\n";
    for (const auto & vti : ti.members) {
        const String m = formatMembername(vti);
        py << "        self." << m << " = " << m << "\n";
    }
    py << "\n";

    // _deserialize
    py << "    @classmethod\n"
          "    def _deserialize(cls, data):\n"
          "        if data is None:\n"
          "            return None\n"
          "        self = cls.__new__(cls)\n"
          "        cls._cf_deser(self, _ber.D(data))\n"
          "        return self\n"
          "\n";

    // _cf_deser
    py << "    def _cf_deser(self, _D):\n"
          "        _D.n()\n";
    if (!baseAlias.isEmpty()) py << "        " << baseAlias << "._cf_deser(self, _ber.D(_D.a()))\n";
    for (const auto & vti : ti.members) {
        py << "        self." << formatMembername(vti) << " = " << getDeserializeCode(vti.type) << "\n";
    }
    if (ti.members.isEmpty() && baseAlias.isEmpty()) py << "        return\n";
    py << "\n";

    // _serialize
    py << "    def _serialize(self, _S):\n"
          "        return (_S\n            ";
    if (ti.classId != 0) py << ".i(" << String::number(ti.classId) << ")";
    else                 py << ".n()";
    if (!baseAlias.isEmpty()) py << ".o(self, " << baseAlias << "._serialize)";
    for (const auto & vti : ti.members) {
        py << getSerializeCode(vti.type, "self." + formatMembername(vti));
    }
    py << ")\n\n";

    // __repr__
    py << "    def __repr__(self):\n"
          "        return \"" << name << "(\" + \", \".join(";
    {
        py << "[";
        bool first = true;
        for (const auto & vti : ti.members) {
            const String m = formatMembername(vti);
            if (first) first = false; else py << ", ";
            py << "\"" << m << "=%r\" % (self." << m << ",)";
        }
        py << "]";
    }
    py << ") + \")\"\n\n";

    // __eq__
    py << "    def __eq__(self, other):\n"
          "        return isinstance(other, " << name << ")";
    for (const auto & vti : ti.members) {
        const String m = formatMembername(vti);
        py << " and self." << m << " == other." << m;
    }
    if (ti.members.isEmpty()) py << " ";
    py << "\n";

    return py;
}

// ---------------------------------------------------------------------------
// service generation
// ---------------------------------------------------------------------------

String generateForService(const SerializeTypeInfo & ti)
{
    const String wireName = String(ti.typeName).toLower();

    String py;
    py << "class " << ti.typeName << ":\n"
          "    def __init__(self, rmi):\n"
          "        self._rmi = rmi\n";

    // signals
    for (const SerializeFunctionTypeInfo & func : ti.cfSignals) {
        py << "        self." << func.name << " = _RSig(rmi, '" << wireName << "', '" << func.name
           << "', lambda _D: (";
        bool first = true;
        for (const SerializeVariableTypeInfo & p : func.parameters) {
            if (first) first = false; else py << ", ";
            py << getDeserializeCode(p.type);
        }
        if (func.parameters.size() == 1) py << ",";   // single-element tuple
        py << "))\n";
    }
    py << "\n";

    // methods
    for (const SerializeFunctionTypeInfo & func : ti.functions) {
        const String sendData = String("_ber.S().s('") << wireName << "').s('" << func.signature() << "')"
            << getSerializeParameters(func) << ".box(2)";

        if (!func.hasReturnValues()) {
            py << "    async def " << func.name << "(self" << getParameters(func) << "):\n"
                  "        await self._rmi.send_async(" << sendData << ")\n\n";
            continue;
        }

        py << "    async def " << func.name << "(self" << getParameters(func) << "):\n"
              "        _data = await self._rmi.send_request(" << sendData << ")\n"
              "        _D = _ber.D(_data)\n";

        // collect result expressions: return value (if any) + ref out-params
        StringList results;
        if (func.returnType.type != SerializeTypeInfo::Null) results << getDeserializeCode(func.returnType);
        for (const auto & p : func.parameters) {
            if (p.isRef) results << getDeserializeCode(p.type);
        }

        if (results.size() == 1) {
            py << "        return " << results[0] << "\n\n";
        } else {
            py << "        return (";
            bool first = true;
            for (const String & r : results) { if (first) first = false; else py << ", "; py << r; }
            py << ")\n\n";
        }
    }

    return py;
}

String generate(const SerializeTypeInfo & ti)
{
    String body = ti.isRMIService() ? generateForService(ti) : generateForClass(ti);

    // assemble imports based on what the body uses
    String py;
    py << "# ============================================================================\n"
          "# Generated by CFLib\n"
          "# ============================================================================\n"
          "\n";

    py << "from cflib.net import ber as _ber\n";
    if (!ti.cfSignals.isEmpty()) py << "from cflib.net.rsig import RSig as _RSig\n";
    if (body.indexOf("_cf.") != -1) py << "from cflib.net import codec as _cf\n";

    for (const String & type : getMemberTypes(ti)) {
        // skip self reference
        if (type == ti.getName()) continue;
        String alias = type; alias.replace("::", "__");
        py << "from " << modulePath(type) << " import " << shortClassName(type) << " as " << alias << "\n";
    }
    py << "\n\n" << body;

    return py;
}

void writeWhenChanged(const String & path, const String & data)
{
    const ByteArray newContent = data.toUtf8();
    if (newContent != File::read(path)) File::write(path, newContent);
}

void ensureInit(const String & dir, Set<String> & writtenInits)
{
    const String path = dir + "/__init__.py";
    if (writtenInits.contains(path)) return;
    writtenInits << path;
    if (File::read(path).isNull()) File::write(path, ByteArray());
}

}

void generatePython(const StructuredTypeInfos & typeInfos, const String & dest)
{
    const String destPy = dest + "/py/";
    Set<String> writtenInits;

    // services -----------------------------------------------------------
    const String destServices = destPy + "services/";
    util::mkPath(destServices);
    // py/ itself is a sources root added to sys.path, not a package -> no __init__ there
    ensureInit(destServices.left(destServices.length() - 1), writtenInits);
    Set<String> serviceFiles;
    for (const SerializeTypeInfo & ti : typeInfos.services()) {
        const String file = String(ti.typeName).toLower() + ".py";
        serviceFiles << file << "__init__.py";
        writeWhenChanged(destServices + file, generate(ti));
    }
    {
        DIR * d = opendir(destServices.c_str());
        if (d) {
            struct dirent * ent;
            while ((ent = readdir(d)) != nullptr) {
                String name(ent->d_name);
                if (name == "." || name == "..") continue;
                if (!serviceFiles.contains(name)) util::removeFile(destServices + name);
            }
            closedir(d);
        }
    }

    // classes (daos) ------------------------------------------------------
    Set<String> classFiles;
    for (const SerializeTypeInfo & ti : typeInfos.types()) {
        String relDir = ti.getNSPath();
        util::mkPath(destPy + relDir);

        // ensure __init__.py up the package chain
        String cur;
        for (const String & part : relDir.split("/")) {
            cur = cur.isEmpty() ? part : (cur + "/" + part);
            ensureInit(destPy + cur, writtenInits);
        }

        String rel = relDir + "/" + String(ti.typeName).toLower() + ".py";
        classFiles << rel;
        writeWhenChanged(destPy + rel, generate(ti));
    }

    // remove stale dao files
    auto opt = directory_options::follow_directory_symlink;
    if (exists(destPy.str())) {
        for (const directory_entry & entry : recursive_directory_iterator(destPy.str(), opt)) {
            if (!entry.is_regular_file()) continue;
            String rel = entry.path().lexically_relative(destPy.str()).generic_string();
            if (rel.contains("/dao/") && rel.endsWith(".py") && !rel.endsWith("__init__.py")
                && !classFiles.contains(rel)) {
                remove(entry.path());
            }
        }
    }
}

} // namespace
