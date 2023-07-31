# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

CFLIB_DIR = $$PWD

# read configs if exists
exists($${CFLIB_DIR}/config.pri):include($${CFLIB_DIR}/config.pri)
exists(../../config.pri):include(../../config.pri)
exists(../config.pri):include(../config.pri)
exists(config.pri):include(config.pri)

# clean release / debug
CONFIG(debug, debug|release) {
	CONFIG -= release
	DEBUG_RELEASE = debug
} else {
	CONFIG -= debug
	DEBUG_RELEASE = release
}

# isInCFLib()
defineTest(isInCFLib) {
	IS_IN_CFLIB = $$relative_path($$CFLIB_DIR/cflib, $$PWD)
	IS_IN_CFLIB = $$replace(IS_IN_CFLIB, "../", "")
	equals(IS_IN_CFLIB, "") {
		return(true)
	}
	return(false)
}

# setBuildPaths()
defineTest(setBuildPaths) {
	INCLUDEPATH += . $$CFLIB_DIR
	export(INCLUDEPATH)

	OBJECTS_DIR  = build/$$DEBUG_RELEASE
	export(OBJECTS_DIR)
	MOC_DIR      = build/gen/moc
	export(MOC_DIR)
	UI_DIR       = build/gen/ui
	export(UI_DIR)
	RCC_DIR      = build/gen/rcc
	export(RCC_DIR)

	!no_pch {
		# find fitting pch
		modules = ""
		for(mod, QT) {
			modules = $${modules}_$${mod}
		}
		modules = $$replace(modules, ^_, )
		PRECOMPILED_HEADER = $${CFLIB_DIR}/pch/$${modules}.h
		export(PRECOMPILED_HEADER)
		HEADERS += $$PRECOMPILED_HEADER
		export(HEADERS)
		CONFIG *= precompile_header
	}

	CONFIG *= c++17 stl_off exceptions_off
	export(CONFIG)

	# Qt 5.15 does not have a config for 'c++20' yet.
	QMAKE_CXXFLAGS_CXX1Z ~= s/-std=.+/-std=c++20
	export(QMAKE_CXXFLAGS_CXX1Z)
	QMAKE_CXXFLAGS_GNUCXX1Z ~= s/-std=.+/-std=c++20
	export(QMAKE_CXXFLAGS_GNUCXX1Z)

#	QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-copy
#	equals(QMAKE_CXX, "clang++") {
#		QMAKE_CXXFLAGS_WARN_ON += -Wno-shift-count-overflow
#	}
#	equals(QMAKE_CXX, "g++") {
#		QMAKE_CXXFLAGS_WARN_ON += -Wno-unknown-pragmas
#	}
#	export(QMAKE_CXXFLAGS_WARN_ON)

	win32 {
		DEFINES += _WIN32_WINNT=0x0600
		export(DEFINES)
	}

	optimize {
		QMAKE_CXXFLAGS += -march=native
		export(QMAKE_CXXFLAGS)
	}

	LIBS += -lz
	export(LIBS)
}

# addSubdir(dir, dependandept1 dependant2 ...)
defineTest(addSubdir) {
	TEMPLATE = subdirs
	export(TEMPLATE)
	dir = $$replace(1, /, _)
	eval  ($${dir}.subdir = $$1)
	export($${dir}.subdir)
	depends = $$replace(2, /, _)
	eval  ($${dir}.depends = $$depends)
	export($${dir}.depends)
	SUBDIRS += $$dir
	export(SUBDIRS)
}

# lib()
# lib(path)
defineTest(lib) {
	defined(1, var) {
		DESTDIR = $$1
	} else {
		isInCFLib() {
			DESTDIR = $${CFLIB_DIR}/lib
		} else {
			DESTDIR = lib
		}
	}
	TEMPLATE = lib
	export(TEMPLATE)
	CONFIG *= staticlib
	export(CONFIG)
	export(DESTDIR)
	for(header, HEADERS) {
		source = $$replace(header, \\.h, .cpp)
		exists($$source):SOURCES *= $$source
	}
	export(SOURCES)

	setBuildPaths()
}

# useLibs(lib1 lib2 ...)
# useLibs(path, lib1 lib2 ...)
defineTest(useLibs) {
	defined(2, var) {
		path = $$1
		libs = $$2
	} else {
		path = $${CFLIB_DIR}/lib
		libs = $$1
	}
	LIBS += -L$$path
	for(lib, libs) {
		LIBS += -l$$lib
		*-msvc* {
			POST_TARGETDEPS += $${path}/$${lib}.lib
		} else {
			POST_TARGETDEPS += $${path}/lib$${lib}.a
		}
	}
	export(LIBS)
	export(POST_TARGETDEPS)
}

# app()
defineTest(app) {
	TEMPLATE = app
	export(TEMPLATE)
	isInCFLib() {
		DESTDIR = $${CFLIB_DIR}/bin
	} else {
		DESTDIR = bin
	}
	export(DESTDIR)

	release {
		QMAKE_LFLAGS_RPATH =
		export(QMAKE_LFLAGS_RPATH)

		!win32:!no_strip:console {
			QMAKE_POST_LINK = strip $${DESTDIR}/$${TARGET}
			export(QMAKE_POST_LINK)
		}
	}

	macx {
		console {
			CONFIG -= app_bundle
			export(CONFIG)
		}
		LIBS += -framework Security
		export(LIBS)
	}

	win32 {
		LIBS += -lAdvapi32 -lUser32
		export(LIBS)
	}

	postgresql {
		macx: LIBS += -L/usr/local/lib
		LIBS += -lpq
		export(LIBS)
	}

	setBuildPaths()
}

# test()
defineTest(test) {
	QT += testlib
	export(QT)
	CONFIG += testcase console
	export(CONFIG)
	app()
}

# serializeGen()
defineTest(serializeGen) {
	for(header, HEADERS) {
		file = $$cat($$header)
		contains(file, SERIALIZE_CLASS) {
			SERIALIZE_HEADERS += $$header
		}
	}
	export(SERIALIZE_HEADERS)

	serializeGen.input        = SERIALIZE_HEADERS
	win32 {
		serializeGen.commands = $$shell_path($${CFLIB_DIR}/bin/ser.exe) serialize ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
		serializeGen.depends  = $$shell_path($${CFLIB_DIR}/bin/ser.exe)
	} else {
		serializeGen.commands = $${CFLIB_DIR}/bin/ser serialize ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
		serializeGen.depends  = $${CFLIB_DIR}/bin/ser
	}
	serializeGen.output       = build/gen/ser/${QMAKE_FILE_BASE}_ser.cpp
	serializeGen.variable_out = SOURCES
	QMAKE_EXTRA_COMPILERS += serializeGen

	export(serializeGen.input)
	export(serializeGen.commands)
	export(serializeGen.depends)
	export(serializeGen.output)
	export(serializeGen.variable_out)
	export(QMAKE_EXTRA_COMPILERS)
}

# createGitVersionHeader(header.h)
defineTest(createGitVersionHeader) {
	win32 {
		GIT_DIR = $$system(\"$${CFLIB_DIR}/bin/gitversion.exe\" depend \"$$_PRO_FILE_PWD_\")
	} else {
		GIT_DIR = $$system(\"$${CFLIB_DIR}/bin/gitversion\" depend \"$$_PRO_FILE_PWD_\")
	}
	export(GIT_DIR)

	gitVersion.input = GIT_DIR
	win32 {
		gitVersion.commands = $$shell_path($${CFLIB_DIR}/bin/gitversion.exe) create $$_PRO_FILE_PWD_ $$1
		gitVersion.depends  = $$shell_path($${CFLIB_DIR}/bin/gitversion.exe)
	} else {
		gitVersion.commands = $${CFLIB_DIR}/bin/gitversion create $$_PRO_FILE_PWD_ $$1
		gitVersion.depends  = $${CFLIB_DIR}/bin/gitversion
	}
	gitVersion.output = $$1
	gitVersion.variable_out = HEADERS
	gitVersion.CONFIG = target_predeps no_link
	QMAKE_EXTRA_COMPILERS += gitVersion

	export(gitVersion.input)
	export(gitVersion.commands)
	export(gitVersion.depends)
	export(gitVersion.output)
	export(gitVersion.variable_out)
	export(gitVersion.CONFIG)
	export(QMAKE_EXTRA_COMPILERS)
}

# getFiles(path, [!,] regex)
defineReplace(getFiles) {
	defined(3, var) {
		win32 {
			FILES = $$system(\"$${CFLIB_DIR}/bin/filefinder.exe\" \"$$1\" \"$$2\" \"$$3\")
		} else {
			FILES = $$system(\"$${CFLIB_DIR}/bin/filefinder\" \"$$1\" \"$$2\" \"$$3\")
		}
	} else {
		win32 {
			FILES = $$system(\"$${CFLIB_DIR}/bin/filefinder.exe\" \"$$1\" \"$$2\")
		} else {
			FILES = $$system(\"$${CFLIB_DIR}/bin/filefinder\" \"$$1\" \"$$2\")
		}
	}
	return($$FILES)
}

# addOtherFiles(path, [!,] regex)
defineTest(addOtherFiles) {
	defined(3, var) {
		OTHER_FILES += $$getFiles($$1, $$2, $$3)
	} else {
		OTHER_FILES += $$getFiles($$1, $$2)
	}
	export(OTHER_FILES)
}
