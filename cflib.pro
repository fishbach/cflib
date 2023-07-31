# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

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
addSubdir(cflib/db/db_test, cflib/db cflib/util)
addSubdir(cflib/net, cflib/serialize/ser)
addSubdir(cflib/net/net_test, cflib_net cflib_crypt cflib_util)
addSubdir(cflib/serialize)
addSubdir(cflib/serialize/asn1dump, cflib/serialize cflib/util)
addSubdir(cflib/serialize/ser, cflib/util)
addSubdir(cflib/serialize/serialize_test, cflib/serialize/ser cflib/serialize cflib/util)
addSubdir(cflib/util)
addSubdir(cflib/util/filefinder)
addSubdir(cflib/util/gitversion)
addSubdir(cflib/util/jscombiner, cflib_util)
addSubdir(cflib/util/util_test, cflib/util)
addSubdir(examples/getssl, cflib_net cflib_crypt cflib_util)
addSubdir(examples/request, cflib_net cflib_crypt cflib_util)
addSubdir(examples/webchat, cflib_net cflib_crypt cflib/serialize/ser cflib_serialize cflib_util)

postgresql {
	addSubdir(cflib/db/migrationmoped, cflib/db cflib/util)
	addSubdir(examples/pgtest, cflib_db cflib_util)
}

OTHER_FILES = \
	README \
	COPYING \

addOtherFiles(js)
