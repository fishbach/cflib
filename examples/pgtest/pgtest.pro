include(../../include.pri)

QT = core
CONFIG += console

TARGET = pgtest

HEADERS = \

SOURCES = \
	main.cpp \

useLibs(cflib_db cflib_util)
app()
