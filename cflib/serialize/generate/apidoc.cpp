/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "apidoc.h"

#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <dirent.h>

using namespace cflib::util;

USE_LOG(LogCat::JS)

namespace cflib::serialize::generate {

namespace {

StringList nsToList(const String & ns)
{
    if (ns.isEmpty()) return {};
    return ns.split("::");
};

String getPath(const SerializeTypeInfo & ti)
{
    return ti.getName().replace("::", "/").toLower();
}

class HMTLGen
{
public:
    HMTLGen(const StructuredTypeInfos & typeInfos, const String & dest, const String & prefix, const String & name) :
        typeInfos_(typeInfos),
        rootPath_(dest + "/" + prefix),
        servicesPath_(rootPath_ + "/services"),
        classesPath_(rootPath_ + "/classes"),
        prefix_(prefix),
        name_(name)
    {}

    void generate()
    {
        mkPath(rootPath_);
        writeFile(rootPath_ + "/index.html", mainIndex().toUtf8());
        mkPath(servicesPath_);
        writeFile(servicesPath_ + "/index.html", services().toUtf8());
        for (const SerializeTypeInfo & ti : typeInfos_.services()) {
            String path = servicesPath_ + "/" + ti.typeName.toLower();
            mkPath(path);
            writeFile(path + "/index.html", service(ti).toUtf8());
        }

        mkPath(classesPath_);
        writeFile(classesPath_ + "/index.html", classes().toUtf8());
        for (const SerializeTypeInfo & ti : typeInfos_.types()) {
            String path = classesPath_ + "/" + getPath(ti);
            mkPath(path);
            writeFile(path + "/index.html", classDesc(ti).toUtf8());
        }
    }

private:
    String header() { return
        "<html><head>\n"
        "<title>" + name_ + "</title>\n"
        "<style type=\"text/css\">\n"
        "body { font-family: \"Verdana\"; }\n"
        "h2, h3, h4 { font-weight: normal; }\n"
        "</style>\n"
        "</head><body>\n"
        "<h2>" + name_ + "</h2>\n";
    }

    const String Footer =
        "</body></html>\n";

    String mainIndex() { return
        header() +
        "<ul>\n"
        "<li><a href=\"/" + prefix_ + "/services\">services</a> - API Services Description</li>\n"
        "<li><a href=\"/" + prefix_ + "/classes\">classes</a> - API Classes Description</li>\n"
        "</ul>\n" +
        Footer;
    }

    String services()
    {
        String html = header();
        html <<
            "<h3>Services:</h3>\n"
            "<ul>\n";
        for (const SerializeTypeInfo & ti : typeInfos_.services()) {
            html
                << "<li><a href=\"/" + prefix_ + "/services/" << ti.typeName.toLower() << "\">" << ti.typeName << "</a></li>\n";
        }
        html <<
            "</ul>\n";
        html << Footer;
        return html;
    }

    String service(const SerializeTypeInfo & ti)
    {
        String html = header();
        html <<
            "<h3>Service: <b>" << ti.typeName << "</b></h3>\n"
            "JavaScript File: <a href=\"/js/services/" << ti.typeName.toLower() << ".mjs\">/js/services/" << ti.typeName.toLower() << ".mjs</a><br>\n"
            "<h4>Methods:</h4>\n"
            "<ul>\n";

        for (const SerializeFunctionTypeInfo & func : ti.functions) {
            html << "<li>" << func.signature(true).replace("<", "&lt;").replace(">", "&gt;") << "</li>\n";
        }
        html << "</ul>\n";

        if (!ti.cfSignals.isEmpty()) {
            html <<
            "<h4>Signals:</h4>\n"
            "<ul>\n";
            for (const auto & func : ti.cfSignals) {
                html << "<li>" << func.signature(true).replace("<", "&lt;").replace(">", "&gt;") << "</li>\n";
            }
            html << "</ul>\n";
        }

        html << Footer;
        return html;
    }

    String classes()
    {
        String html = header();
        html <<
            "<h3>Classes:</h3>\n"
            "<ul>\n";

        String lastNs;
        for (const SerializeTypeInfo & ti : typeInfos_.types()) {
            if (ti.ns != lastNs) {
                StringList last    = nsToList(lastNs);
                StringList current = nsToList(ti.ns);
                lastNs = ti.ns;
                while (!last.isEmpty() && !current.isEmpty() && last.first() == current.first()) {
                    last.takeFirst();
                    current.takeFirst();
                }
                for (size_t i = 0 ; i < last.size() ; ++i) html << "</ul>\n";
                while (!current.isEmpty()) {
                    html <<
                        "<li>" << current.takeFirst() << ":</li>\n"
                        "<ul>\n";
                }
            }
            html << "<li><a href=\"classes/" << getPath(ti) << "\">" << ti.typeName.split("::").back() << "</a></li>\n";
        }
        for (int i = nsToList(lastNs).size() ; i > 0 ; --i) html << "</ul>\n";

        html << "</ul>\n";
        html << Footer;
        return html;
    }

    String classDesc(const SerializeTypeInfo & ti)
    {
        String html = header();
        html <<
            "<h3>Class: <b>" << ti.getName() << "</b></h3>\n"
            "JavaScript File: <a href=\"/js/" << getPath(ti) << ".mjs\">/js/" << getPath(ti) << ".mjs</a><br>\n"
            "<br>\n"
            "Base: ";

        if (ti.bases.isEmpty()) {
            html << "API.Base";
        } else {
            html << ti.bases[0].getName().replace("<", "&lt;").replace(">", "&gt;");
        }

        html <<
            "\n"
            "<h4>Members:</h4>\n"
            "<ul>\n";

        for (const auto & member : ti.members) {
            html
                << "<li>" << member.type.getName().replace("<", "&lt;").replace(">", "&gt;")
                << ' ' << member.name << "</li>\n";
        }

        html << "</ul>\n";
        html << Footer;
        return html;
    }

private:
    const StructuredTypeInfos & typeInfos_;
    const String rootPath_;
    const String servicesPath_;
    const String classesPath_;
    const String & prefix_;
    const String & name_;
};

}

void generateAPIDoc(const StructuredTypeInfos & typeInfos, const String & dest, const String & prefix, const String & name)
{
    HMTLGen(typeInfos, dest, prefix, name).generate();
}

} // namespace
