# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.16)

project(
	cflib
	VERSION 1.0.0
	LANGUAGES CXX
)

include(cmake/ProjectConfig.cmake)

cf_add_other(project_files
	.gitignore
	COPYING
	MODULES
	README.md
	js/*
)

add_subdirectory(cflib)
#add_subdirectory(examples)
