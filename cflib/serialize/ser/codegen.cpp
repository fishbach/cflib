/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cflib/serialize/ser/headerparser.h>

namespace {

void writeMethods(QTextStream & out, const HeaderParser::Class & cl)
{
	if (cl.members.isEmpty() && !cl.doBaseSerialize) {
		out <<
			"template<typename T> void " << cl.name << "::serialize(T &) const {}\n"
			"template<typename T> void " << cl.name << "::deserialize(T &) {}\n";
	} else {
		out <<
			"template<typename T> void " << cl.name << "::serialize(T & ser) const {\n"
			"\tser";
		if (cl.doBaseSerialize) {
			out << " << (const " << cl.base << " &)*this";
		}
		foreach (const HeaderParser::Variable & m, cl.members) {
			out << " << " << (m.name.isEmpty() ? "cflib::serialize::Placeholder()" : m.name);
		}
		out <<
			";\n"
			"}\n"
			"template<typename T> void " << cl.name << "::deserialize(T & ser) {\n"
			"\tser";
		if (cl.doBaseSerialize) {
			out << " >> (" << cl.base << " &)*this";
		}
		foreach (const HeaderParser::Variable & m, cl.members) {
			out << " >> " << (m.name.isEmpty() ? "cflib::serialize::Placeholder()" : m.name);
		}
		out <<
			";\n"
			"}\n";
	}
	out <<
		"template void " << cl.name << "::serialize(cflib::serialize::impl::BERSerializerBase &) const;\n"
		"template void " << cl.name << "::deserialize(cflib::serialize::impl::BERDeserializerBase &);\n";
}

void writeFunctionSwitch(const HeaderParser::Functions & list, bool withReturnValues, QTextStream & out)
{
	out << "\tswitch (__callNo) {\n";
	int i = 0;
	foreach (const HeaderParser::Function & f, list) {
		++i;
		if (f.hasReturnValues() != withReturnValues) continue;

		out << "\t\tcase " << QByteArray::number(i) << ": {\n";
		if (!f.parameters.isEmpty()) {
			int id = 0;
			foreach (const HeaderParser::Variable & p, f.parameters) {
				out << "\t\t\t" << p.type << ' ';
				if (p.name.isEmpty()) out << "__param_" << QString::number(++id);
				else out << "__" << p.name;
				out << ";\n";
			}
			out << "\t\t\t__deser";
			id = 0;
			foreach (const HeaderParser::Variable & p, f.parameters) {
				out << " >> ";
				if (p.name.isEmpty()) out << "__param_" << QString::number(++id);
				else out << "__" << p.name;
			}
			out << ";\n";
		}
		out << "\t\t\t";
		if (f.returnType != "void") out << "__ser << ";
		out << f.name << '(';
		int id = 0;
		bool isFirst = true;
		foreach (const HeaderParser::Variable & p, f.parameters) {
			if (isFirst) isFirst = false;
			else out << ", ";
			if (p.name.isEmpty()) out << "__param_" << QString::number(++id);
			else out << "__" << p.name;
		}
		out << ")";
		if (f.hasReturnValues()) {
			if (f.returnType == "void") out << ";\n\t\t\t__ser";
			id = 0;
			foreach (const HeaderParser::Variable & p, f.parameters) {
				if (!p.isRef) continue;
				out << " << ";
				if (p.name.isEmpty()) out << "__param_" << QString::number(++id);
				else out << "__" << p.name;
			}
		}
		out << ";\n"
			"\t\t\treturn;\n"
			"\t\t}\n";
	}
	out << "\t}\n";
}

void writeFunction(const HeaderParser::Function & f, QTextStream & out)
{
	out <<
		"\t{\n"
		"\t\tcflib::serialize::SerializeFunctionTypeInfo func;\n"
		"\t\tfunc.name = \"" << f.name << "\";\n";
	if (f.returnType != "void") {
		out << "\t\tfunc.returnType = cflib::serialize::impl::fromType<" << f.returnType << " >();\n";
	}
	if (!f.parameters.isEmpty()) {
		out << "\t\tfunc.parameters";
		foreach (const HeaderParser::Variable & m, f.parameters) {
			out <<
				"\n\t\t\t<< cflib::serialize::SerializeVariableTypeInfo(\""
				<< m.name << "\", cflib::serialize::impl::fromType<" << m.type << " >(), "
				<< (m.isRef ? "true" : "false") << ")";
		}
		out << ";\n";
	}
}

}

int genSerialize(const QString & headerName, const HeaderParser & hp, QIODevice & outDev)
{
	QTextStream out(&outDev);

	if (!hp.hasSerializeElements()) {
		out << "// empty";
		return 0;
	}

	out <<
		"#include \"" << headerName << "\"\n"
		"\n"
		"#include <cflib/serialize/impl/serializetypeinfoimpl.h>\n"
		"#include <cflib/serialize/serializeber.h>\n";

	foreach (const HeaderParser::Class & cl, hp.classes()) {
		out << "\n";
		QStringList nsList = cl.ns.split("::", QString::SkipEmptyParts);
		bool isFirst = true;
		foreach (const QString & ns, nsList) {
			if (isFirst) isFirst = false;
			else out << " ";
			out << "namespace " << ns << " {";
		}
		if (!nsList.isEmpty()) out << "\n";

		writeMethods(out, cl);

		out <<
			"cflib::serialize::SerializeTypeInfo " << cl.name << "::serializeTypeInfo() {\n"
			"\tcflib::serialize::SerializeTypeInfo retval;\n"
			"\tretval.type = cflib::serialize::SerializeTypeInfo::Class;\n"
			"\tretval.ns = \"" << cl.ns << "\";\n"
			"\tretval.typeName = \"" << cl.name << "\";\n";
		if (cl.doBaseSerialize) {
			out << "\tretval.bases << cflib::serialize::impl::fromType<" << cl.base << " >();\n";
		}
		if (!cl.members.isEmpty()) {
			out << "\tretval.members";
			foreach (const HeaderParser::Variable & m, cl.members) {
				if (m.name.isEmpty()) {
					out << "\n\t\t<< cflib::serialize::SerializeVariableTypeInfo()";
				} else {
					out << "\n\t\t<< cflib::serialize::SerializeVariableTypeInfo(\"" << m.name
						<< "\", cflib::serialize::impl::fromType<" << m.type << " >())";
				}
			}
			out << ";\n";
		}
		foreach (const HeaderParser::Function & f, cl.functions) {
			writeFunction(f, out);
			out <<
				"\t\tretval.functions << func;\n"
				"\t}\n";
		}
		foreach (const HeaderParser::Function & f, cl.cfSignals) {
			writeFunction(f, out);
			out <<
				"\t\tretval.cfSignals << func;\n"
				"\t}\n";
		}
		out <<
			"\treturn retval;\n"
			"}\n";

		if (!cl.functions.isEmpty()) {
			bool existsWithReturnValues                 = false;
			bool existsWithoutReturnValues              = false;
			bool existsWithReturnValuesAndParameters    = false;
			bool existsWithoutReturnValuesAndParameters = false;
			foreach (const HeaderParser::Function & f, cl.functions) {
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
				out << "void " << cl.name << "::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &"
					<< (existsWithoutReturnValuesAndParameters ? " __deser" : "") << ", uint __callNo) {\n";
				writeFunctionSwitch(cl.functions, false, out);
				out << "}\n";
			} else {
				out << "void " << cl.name << "::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &, uint) {}\n";
			}
			if (existsWithReturnValues) {
				out << "void " << cl.name << "::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &"
					<< (existsWithReturnValuesAndParameters ? " __deser" :"")
					<< ", uint __callNo, cflib::serialize::BERSerializer & __ser) {\n";
				writeFunctionSwitch(cl.functions, true, out);
				out << "}\n";
			} else {
				out << "void " << cl.name << "::processRMIServiceCallImpl(cflib::serialize::BERDeserializer &, uint, cflib::serialize::BERSerializer &) {}\n";
			}

			if (cl.cfSignals.isEmpty()) {
				out << "cflib::net::RSigBase * " << cl.name << "::getCfSignal(uint) { return 0; }\n";
			} else {
				out <<
					"cflib::net::RSigBase * " << cl.name << "::getCfSignal(uint __sigNo) {\n"
					"\tswitch (__sigNo) {\n";
				int i = 0;
				foreach (const HeaderParser::Function & f, cl.cfSignals) {
					++i;
					out << "\t\tcase " << QByteArray::number(i) << ": return &" << f.name << ";\n";
				}
				out <<
					"\t}\n"
					"\t return 0;\n"
					"}\n";
			}
		}

		for (int i = 0 ; i < nsList.size() ; ++i) out << "}";
		if (!nsList.isEmpty()) out << "\n";
	}

	return 0;
}
