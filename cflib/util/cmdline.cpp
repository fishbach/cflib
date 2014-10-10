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

#include "cmdline.h"

CmdLine::CmdLine(int argc, char *argv[])
{
	for (int i = 0 ; i < argc ; ++i) rawArgs_ << argv[i];
}

bool CmdLine::parse()
{
	QListIterator<QByteArray> it(rawArgs_);

	if (!it.hasNext()) return false;
	executable_ = QFileInfo(it.next()).fileName().toLatin1();

	int argCount = 0;
	bool parseMoreOptions = true;
	while (it.hasNext()) {
		const QByteArray & raw = it.next();

		if (parseMoreOptions && raw.startsWith("--")) {
			if (raw.length() == 2) {
				parseMoreOptions = false;
				continue;
			}
			Option * opt = options_.value(raw.mid(2));
			if (!opt || (!opt->isRepeatable_ && opt->count_ > 0)) return false;
			opt->count_++;
			if (opt->hasValue_) {
				if (!it.hasNext()) return false;
				opt->values_ << it.next();
			}
		} else if (parseMoreOptions && raw.startsWith("-")) {
			if (raw.length() < 2) return false;
			int pos = 0;
			while (++pos < raw.length()) {
				Option * opt = shortOptions_.value(raw.at(pos));
				if (!opt || (!opt->isRepeatable_ && opt->count_ > 0)) return false;
				opt->count_++;
				if (opt->hasValue_) {
					if (pos < raw.length() - 1 || !it.hasNext()) return false;
					opt->values_ << it.next();
				}
			}
		} else {
			Arg * arg = args_.value(argCount);
			if (!arg) return false;
			arg->count_++;
			arg->values_ << raw;
			if (!arg->isRepeatable_) ++argCount;
		}
	}

	foreach (const ArgBase * arg, nonOptionals_) if (!arg->isOptional_ && arg->count_ == 0) return false;

	return true;
}

CmdLine & CmdLine::operator<<(Arg & arg)
{
	args_ << &arg;
	if (!arg.isOptional_) nonOptionals_ << &arg;
	return *this;
}

CmdLine & CmdLine::operator<<(Option & opt)
{
	if (opt.optionChar_ != 0) shortOptions_[opt.optionChar_] = &opt;
	if (!opt.optionName_.isEmpty()) options_[opt.optionName_] = &opt;
	if (!opt.isOptional_) nonOptionals_ << &opt;
	return *this;
}
