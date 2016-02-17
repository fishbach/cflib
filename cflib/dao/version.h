/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

#pragma once

#include <cflib/serialize/serialize.h>

#ifdef major
	#undef major
#endif
#ifdef minor
	#undef minor
#endif

namespace cflib { namespace dao {

class Version
{
	SERIALIZE_CLASS
public:
	Version(uint major = 0, uint minor = 0, uint revision = 0, const QString & patchLevel = QString()) :
		major(major), minor(minor), revision(revision), patchLevel(patchLevel) {}

	bool isNull() const { return major == 0 && minor == 0 && revision == 0 && patchLevel.isNull(); }

	bool operator==(const Version & rhs) const { return
		major      == rhs.major &&
		minor      == rhs.minor &&
		revision   == rhs.revision &&
		patchLevel == rhs.patchLevel;
	}
	bool operator!=(const Version & rhs) const { return !operator==(rhs); }
	bool operator>=(const Version & rhs) const {
		if (major >= rhs.major) return true;
		if (minor >= rhs.minor) return true;
		if (revision >= rhs.revision) return true;
		return patchLevel >= rhs.patchLevel;
	}
	bool operator<(const Version & rhs) const { return !operator>=(rhs); }
	bool operator>(const Version & rhs) const { return !rhs.operator>=(*this); }
	bool operator<=(const Version & rhs) const { return rhs.operator>=(*this); }

	QString toString() const;

	static Version current() { return current_; }
	static void setCurrent(const Version & version);
	static void setCurrent(uint major = 0, uint minor = 0, uint revision = 0, const QString & patchLevel = QString());

public serialized:
	uint major;
	uint minor;
	uint revision;
	QString patchLevel;

private:
	static Version current_;
};

}}	// namespace
