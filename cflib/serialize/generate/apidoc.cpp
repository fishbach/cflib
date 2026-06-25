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
#include <filesystem>

using namespace std::filesystem;

USE_LOG(LogCat::JS)

namespace cflib::serialize::generate {

namespace {

StringList nsToList(const String & ns)
{
    if (ns.isEmpty()) return {};
    return ns.split("::");
};

class HMTLGen
{
public:
    HMTLGen(const StructuredTypeInfos & typeInfos, const String & dest, const String & name) :
        typeInfos_(typeInfos),
        rootPath_(dest + "/"),
        name_(name)
    {}

    void generate()
    {
        remove_all(rootPath_.str());
        util::mkPath(rootPath_);

        File::write(rootPath_ + "/index.html", mainIndex().toUtf8());
        for (const SerializeTypeInfo & ti : typeInfos_.services()) {
            util::mkPath(rootPath_ + ti.getNSPath());
            File::write(rootPath_ + ti.getFilePath() + ".html", service(ti).toUtf8());
        }
        for (const SerializeTypeInfo & ti : typeInfos_.types()) {
            util::mkPath(rootPath_ + ti.getNSPath());
            File::write(rootPath_ + ti.getFilePath() + ".html", classDesc(ti).toUtf8());
        }
    }

private:
    uint tiDepth(const SerializeTypeInfo & ti)
    {
        if (ti.ns.isEmpty()) return 0;
        return 1 + ti.ns.count("::");
    }

    String upPath(uint depth)
    {
        String upPath;
        while (depth--) upPath += "../";
        return upPath;
    }

    String header(uint depth)
    {
        return
            "<html><head>\n"
            "<title>" + name_ + "</title>\n"
            "<style type=\"text/css\">\n"
            "body { font-family: \"Verdana\"; }\n"
            "h2, h3, h4 { font-weight: normal; }\n"
            "</style>\n"
            "</head><body>\n"
            "<h2><a href=\"" + upPath(depth) + "index.html\">" + name_ + "</a></h2>\n";
    }

    const String Footer =
        "</body></html>\n";

    String mainIndex()
    {
        String html = header(0);
        html <<
            "<h3>Index:</h3>\n"
            "<ul>\n";

        SerializeTypeInfos allInfos = typeInfos_.types() + typeInfos_.services();
        allInfos.sort();
        String lastNs;
        for (const SerializeTypeInfo & ti : allInfos) {
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
            html << "<li><a href=\"" << ti.getFilePath() << ".html\">" << ti.typeName << "</a></li>\n";
        }
        for (int i = nsToList(lastNs).size() ; i > 0 ; --i) html << "</ul>\n";

        html << "</ul>\n";
        html << Footer;
        return html;
    }

    String service(const SerializeTypeInfo & ti)
    {
        uint depth = tiDepth(ti);
        String html = header(depth);
        html <<
            "<h3>Service: <b>" << ti.getName() << "</b></h3>\n"
            "<h4>Methods:</h4>\n"
            "<ul>\n";

        for (const SerializeFunctionTypeInfo & func : ti.functions) {
            html << "<li>" << signatureHTML(func, depth) << "</li>\n";
        }
        html << "</ul>\n";

        if (!ti.cfSignals.isEmpty()) {
            html <<
            "<h4>Signals:</h4>\n"
            "<ul>\n";
            for (const auto & func : ti.cfSignals) {
                html << "<li>" << signatureHTML(func, depth) << "</li>\n";
            }
            html << "</ul>\n";
        }

        html << Footer;
        return html;
    }

    String classDesc(const SerializeTypeInfo & ti)
    {
        uint depth = tiDepth(ti);
        String html = header(depth);
        html <<
            "<h3>Class: <b>" << ti.getName() << "</b></h3>\n"
            "Base: ";

        if (ti.bases.isEmpty()) {
            html << "API.Base";
        } else {
            html << typeHTML(ti.bases[0], depth);
        }

        html <<
            "\n"
            "<h4>Members:</h4>\n"
            "<ul>\n";

        for (const auto & member : ti.members) {
            html
                << "<li>" << typeHTML(member.type, depth)
                << ' ' << member.name << "</li>\n";
        }

        html << "</ul>\n";
        html << Footer;
        return html;
    }

    String signatureHTML(const SerializeFunctionTypeInfo & fti, uint depth)
    {
        String retval = typeHTML(fti.returnType, depth);
        if (retval.isEmpty()) retval += "void";
        retval += ' ';
        retval += fti.name;
        retval += '(';
        bool isFirst = true;
        for (const SerializeVariableTypeInfo & inf : fti.parameters) {
            if (isFirst) isFirst = false;
            else retval += ", ";
            retval += typeHTML(inf.type, depth);
            if (inf.isRef) retval += " &amp;";
            if (!inf.name.isEmpty()) retval += " " + inf.name;
        }
        retval += ')';
        return retval;
    }

    String typeHTML(const SerializeTypeInfo & ti, uint depth)
    {
        String retval;
        if (ti.type == SerializeTypeInfo::Class) {
            retval = String("<a href=\"" + upPath(depth) + "") << ti.getFilePath() << ".html\">" << ti.getName() << "</a>";
        } else if (ti.type == SerializeTypeInfo::Container) {
            retval = ti.typeName.left(ti.typeName.indexOf('<'));
            retval += "&lt;";
            bool first = true;
            for (const SerializeTypeInfo & bti : ti.bases) {
                if (first) first = false;
                else retval += ", ";
                retval += typeHTML(bti, depth);
            }
            retval += "&gt;";
        } else {
            retval = ti.getName();
        }
        return retval;
    }

private:
    const StructuredTypeInfos & typeInfos_;
    const String rootPath_;
    const String servicesPath_;
    const String classesPath_;
    const String & name_;
};

}

void generateAPIDoc(const StructuredTypeInfos & typeInfos, const String & dest, const String & name)
{
    HMTLGen(typeInfos, dest, name).generate();
}

} // namespace
