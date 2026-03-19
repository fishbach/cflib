/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <memory>

namespace cflib::base {

template<typename T>
using CFSharedPtr = std::shared_ptr<T>;

template<typename T>
using CFUniquePtr = std::unique_ptr<T>;

template<typename T, typename... Args>
CFSharedPtr<T> CFMakeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
CFUniquePtr<T> CFMakeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace
