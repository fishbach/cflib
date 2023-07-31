/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "util.h"

#import <Cocoa/Cocoa.h>

namespace cflib { namespace util {

namespace {

id<NSObject> activityId;

}

void preventApplicationSuspend()
{
	if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
		activityId = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityUserInitiated reason:@"preventApplicationSuspend"];
		[activityId retain];
	}
}

}}	// namespace
