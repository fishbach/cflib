# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_base)
if(ENABLE_PCH)
    target_precompile_headers(cflib_base PUBLIC ../base.h)
endif()
