# Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# cflib is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# cflib is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cflib. If not, see <http://www.gnu.org/licenses/>.

win32 {
	system(cd cflib\util\filefinder & qmake & nmake)
} else {
	system(cd cflib/util/filefinder ; qmake ; make)
}

include(include.pri)

addSubdir(cflib/crypt)
addSubdir(cflib/crypt/crypt_test, cflib/crypt cflib/util)
addSubdir(cflib/crypt/pwhash, cflib/crypt cflib/util)
addSubdir(cflib/dao, cflib/serialize/ser)
addSubdir(cflib/db)
addSubdir(cflib/net, cflib/serialize/ser)
addSubdir(cflib/net/net_test, cflib_net cflib_crypt cflib_util)
addSubdir(cflib/serialize)
addSubdir(cflib/serialize/ser, cflib/util)
addSubdir(cflib/serialize/serialize_test, cflib/serialize/ser cflib/serialize cflib/util)
addSubdir(cflib/util)
addSubdir(cflib/util/util_test, cflib/util)
addSubdir(cflib/util/filefinder)
addSubdir(cflib/util/gitversion)
addSubdir(cflib/util/jscombiner, cflib_util)
addSubdir(examples/getssl, cflib_net cflib_crypt cflib_util)
addSubdir(examples/webchat, cflib_net cflib_crypt cflib/serialize/ser cflib_serialize cflib_util)

OTHER_FILES = \
	README \
	COPYING \

addOtherFiles(js)
