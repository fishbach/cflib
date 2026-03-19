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
class SharedPtr {
    std::shared_ptr<T> d;

    explicit SharedPtr(std::shared_ptr<T> p) : d(std::move(p)) {}

    template<typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

    template<typename U, typename V>
    friend SharedPtr<U> dynamic_pointer_cast(const SharedPtr<V>&);

public:
    SharedPtr() = default;
    SharedPtr(std::nullptr_t) noexcept : d(nullptr) {}
    template<typename U>
    explicit SharedPtr(U* p) : d(p) {}
    SharedPtr(const SharedPtr&) = default;
    SharedPtr(SharedPtr&&) = default;
    SharedPtr& operator=(const SharedPtr&) = default;
    SharedPtr& operator=(SharedPtr&&) = default;
    SharedPtr& operator=(std::nullptr_t) noexcept { d = nullptr; return *this; }

    T& operator*() const noexcept { return *d; }
    T* operator->() const noexcept { return d.get(); }
    T* get() const noexcept { return d.get(); }
    void reset() noexcept { d.reset(); }
    template<typename U>
    void reset(U* p) { d.reset(p); }
    explicit operator bool() const noexcept { return (bool)d; }
    bool operator==(const SharedPtr& o) const noexcept { return d == o.d; }
    bool operator!=(const SharedPtr& o) const noexcept { return d != o.d; }
    bool operator==(std::nullptr_t) const noexcept { return d == nullptr; }
    bool operator!=(std::nullptr_t) const noexcept { return d != nullptr; }
};

template<typename U, typename V>
SharedPtr<U> dynamic_pointer_cast(const SharedPtr<V>& p)
{
    return SharedPtr<U>(std::dynamic_pointer_cast<U>(p.d));
}

template<typename T>
class UniquePtr {
    std::unique_ptr<T> d;

    explicit UniquePtr(std::unique_ptr<T> p) : d(std::move(p)) {}

    template<typename U, typename... Args>
    friend UniquePtr<U> MakeUnique(Args&&... args);

public:
    UniquePtr() = default;
    UniquePtr(std::nullptr_t) noexcept : d(nullptr) {}
    explicit UniquePtr(T* p) : d(p) {}
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr(UniquePtr&&) = default;
    UniquePtr& operator=(const UniquePtr&) = delete;
    UniquePtr& operator=(UniquePtr&&) = default;

    T& operator*() const { return *d; }
    T* operator->() const noexcept { return d.get(); }
    T* get() const noexcept { return d.get(); }
    void reset() noexcept { d.reset(); }
    template<typename U>
    void reset(U* p) { d.reset(p); }
    T* release() noexcept { return d.release(); }
    explicit operator bool() const noexcept { return (bool)d; }
};

template<typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args)
{
    return SharedPtr<T>(std::make_shared<T>(std::forward<Args>(args)...));
}

template<typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args)
{
    return UniquePtr<T>(std::make_unique<T>(std::forward<Args>(args)...));
}

} // namespace
