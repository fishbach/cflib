/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/string.h>

#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace cflib::base {

// ============================================================
// List<T>
// ============================================================

template<typename T>
class List {
    std::vector<T> d;
public:
    using iterator       = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    List() = default;
    explicit List(size_t n) : d(n) {}
    List(size_t n, const T& val) : d(n, val) {}
    List(std::initializer_list<T> il) : d(il) {}
    List(std::vector<T> v) : d(std::move(v)) {}
    template<typename InputIt>
    List(InputIt first, InputIt last) : d(first, last) {}

    bool operator==(const List& o) const { return d == o.d; }
    bool operator!=(const List& o) const { return d != o.d; }

    void push_back(const T& v)  { d.push_back(v); }
    void push_back(T&& v)       { d.push_back(std::move(v)); }
    template<typename... Args>
    T& emplace_back(Args&&... args) { return d.emplace_back(std::forward<Args>(args)...); }
    void pop_back() { d.pop_back(); }

    iterator insert(iterator pos, const T& v)       { return d.insert(pos, v); }
    iterator insert(iterator pos, T&& v)             { return d.insert(pos, std::move(v)); }
    template<typename InputIt>
    iterator insert(iterator pos, InputIt first, InputIt last) { return d.insert(pos, first, last); }
    iterator erase(iterator pos)                     { return d.erase(pos); }
    iterator erase(iterator first, iterator last)    { return d.erase(first, last); }

    void clear()                        { d.clear(); }
    void resize(size_t n)               { d.resize(n); }
    void resize(size_t n, const T& val) { d.resize(n, val); }
    void reserve(size_t n)              { d.reserve(n); }

    size_t size()  const noexcept { return d.size(); }
    bool   empty() const noexcept { return d.empty(); }

    T&       front()         { return d.front(); }
    const T& front() const   { return d.front(); }
    T&       back()          { return d.back(); }
    const T& back()  const   { return d.back(); }
    T&            at(size_t i)        { return d.at(i); }
    const T&      at(size_t i) const  { return d.at(i); }
    decltype(auto) operator[](size_t i)       { return d[i]; }
    decltype(auto) operator[](size_t i) const { return d[i]; }

    iterator       begin()  noexcept { return d.begin(); }
    iterator       end()    noexcept { return d.end(); }
    const_iterator begin()  const noexcept { return d.begin(); }
    const_iterator end()    const noexcept { return d.end(); }
    const_iterator cbegin() const noexcept { return d.cbegin(); }
    const_iterator cend()   const noexcept { return d.cend(); }

    List& operator<<(const T& v) { d.push_back(v); return *this; }
    List& operator<<(T&& v)      { d.push_back(std::move(v)); return *this; }

    T takeFirst() {
        T val = std::move(d.front());
        d.erase(d.begin());
        return val;
    }
    T takeLast() {
        T val = std::move(d.back());
        d.pop_back();
        return val;
    }
    T&       last()       { return d.back(); }
    const T& last() const { return d.back(); }

    void sort() { std::sort(d.begin(), d.end()); }

    template<typename U>
    bool contains(const U& val) const {
        for (const auto& item : d) if (item == val) return true;
        return false;
    }

    // join — works when T supports operator+= with T and Sep
    template<typename Sep>
    T join(Sep sep) const {
        T r;
        for (size_t i = 0; i < d.size(); ++i) {
            if (i > 0) r += sep;
            r += d[i];
        }
        return r;
    }
};

// ============================================================
// Hash<K,V>
// ============================================================

template<typename K, typename V>
class Hash {
    std::unordered_map<K, V> d;
public:
    using iterator       = typename std::unordered_map<K, V>::iterator;
    using const_iterator = typename std::unordered_map<K, V>::const_iterator;
    using value_type     = typename std::unordered_map<K, V>::value_type;

    Hash() = default;

    V&       operator[](const K& k)       { return d[k]; }
    V&       at(const K& k)               { return d.at(k); }
    const V& at(const K& k) const         { return d.at(k); }

    template<typename... Args>
    auto insert(Args&&... args) { return d.insert(std::forward<Args>(args)...); }
    size_t   erase(const K& k) { return d.erase(k); }
    iterator erase(iterator it) { return d.erase(it); }
    void     clear() { d.clear(); }

    iterator       find(const K& k)       { return d.find(k); }
    const_iterator find(const K& k) const { return d.find(k); }
    size_t count(const K& k) const        { return d.count(k); }
    size_t size()  const noexcept         { return d.size(); }
    bool   empty() const noexcept         { return d.empty(); }

    iterator       begin() noexcept       { return d.begin(); }
    iterator       end()   noexcept       { return d.end(); }
    const_iterator begin() const noexcept { return d.begin(); }
    const_iterator end()   const noexcept { return d.end(); }

    bool contains(const K& k) const { return d.find(k) != d.end(); }

    V value(const K& k, const V& def = V()) const {
        auto it = d.find(k);
        return it != d.end() ? it->second : def;
    }
};

// ============================================================
// Map<K,V>
// ============================================================

template<typename K, typename V>
class Map {
    std::map<K, V> d;
public:
    using iterator       = typename std::map<K, V>::iterator;
    using const_iterator = typename std::map<K, V>::const_iterator;
    using value_type     = typename std::map<K, V>::value_type;

    Map() = default;

    bool operator==(const Map& o) const { return d == o.d; }
    bool operator!=(const Map& o) const { return d != o.d; }

    V&       operator[](const K& k)       { return d[k]; }
    V&       at(const K& k)               { return d.at(k); }
    const V& at(const K& k) const         { return d.at(k); }

    template<typename... Args>
    auto insert(Args&&... args) { return d.insert(std::forward<Args>(args)...); }
    size_t   erase(const K& k) { return d.erase(k); }
    iterator erase(iterator it) { return d.erase(it); }
    void     clear() { d.clear(); }

    iterator       find(const K& k)       { return d.find(k); }
    const_iterator find(const K& k) const { return d.find(k); }
    size_t count(const K& k) const        { return d.count(k); }
    size_t size()  const noexcept         { return d.size(); }
    bool   empty() const noexcept         { return d.empty(); }

    iterator       begin() noexcept       { return d.begin(); }
    iterator       end()   noexcept       { return d.end(); }
    const_iterator begin() const noexcept { return d.begin(); }
    const_iterator end()   const noexcept { return d.end(); }

    iterator       lower_bound(const K& k)       { return d.lower_bound(k); }
    const_iterator lower_bound(const K& k) const { return d.lower_bound(k); }
    iterator       upper_bound(const K& k)       { return d.upper_bound(k); }
    const_iterator upper_bound(const K& k) const { return d.upper_bound(k); }

    bool contains(const K& k) const { return d.find(k) != d.end(); }

    V value(const K& k, const V& def = V()) const {
        auto it = d.find(k);
        return it != d.end() ? it->second : def;
    }

    List<K> keys() const {
        List<K> r;
        r.reserve(d.size());
        for (const auto& p : d) r.push_back(p.first);
        return r;
    }
};

// ============================================================
// Set<T>
// ============================================================

template<typename T>
class Set {
    std::unordered_set<T> d;
public:
    using iterator       = typename std::unordered_set<T>::iterator;
    using const_iterator = typename std::unordered_set<T>::const_iterator;

    Set() = default;

    auto insert(const T& v) { return d.insert(v); }
    auto insert(T&& v)      { return d.insert(std::move(v)); }
    size_t erase(const T& v) { return d.erase(v); }
    void   clear() { d.clear(); }

    iterator       find(const T& v)       { return d.find(v); }
    const_iterator find(const T& v) const { return d.find(v); }
    size_t count(const T& v) const        { return d.count(v); }
    size_t size()  const noexcept         { return d.size(); }
    bool   empty() const noexcept         { return d.empty(); }

    iterator       begin() noexcept       { return d.begin(); }
    iterator       end()   noexcept       { return d.end(); }
    const_iterator begin() const noexcept { return d.begin(); }
    const_iterator end()   const noexcept { return d.end(); }

    Set& operator<<(const T& v) { d.insert(v);            return *this; }
    Set& operator<<(T&& v)      { d.insert(std::move(v)); return *this; }
    Set& operator+=(const Set<T>& o) {
        for (const auto& v : o.d) d.insert(v);
        return *this;
    }

    bool contains(const T& v) const { return d.find(v) != d.end(); }
    void remove(const T& v)         { d.erase(v); }

    List<T> values() const { return List<T>(d.begin(), d.end()); }
};

// ============================================================
// MultiMap<K,V>
// ============================================================

template<typename K, typename V>
class MultiMap {
    std::multimap<K, V> d;
public:
    using iterator       = typename std::multimap<K, V>::iterator;
    using const_iterator = typename std::multimap<K, V>::const_iterator;
    using value_type     = typename std::multimap<K, V>::value_type;

    MultiMap() = default;

    template<typename... Args>
    auto insert(Args&&... args) { return d.insert(std::forward<Args>(args)...); }
    iterator erase(iterator it) { return d.erase(it); }
    void     clear() { d.clear(); }

    iterator       find(const K& k)       { return d.find(k); }
    const_iterator find(const K& k) const { return d.find(k); }
    size_t size()  const noexcept         { return d.size(); }
    bool   empty() const noexcept         { return d.empty(); }

    iterator       begin() noexcept       { return d.begin(); }
    iterator       end()   noexcept       { return d.end(); }
    const_iterator begin() const noexcept { return d.begin(); }
    const_iterator end()   const noexcept { return d.end(); }
};

// ============================================================
// MultiHash<K,V>
// ============================================================

template<typename K, typename V>
class MultiHash {
    std::unordered_multimap<K, V> d;
public:
    using iterator       = typename std::unordered_multimap<K, V>::iterator;
    using const_iterator = typename std::unordered_multimap<K, V>::const_iterator;
    using value_type     = typename std::unordered_multimap<K, V>::value_type;

    MultiHash() = default;

    template<typename... Args>
    auto insert(Args&&... args) { return d.insert(std::forward<Args>(args)...); }
    iterator erase(iterator it) { return d.erase(it); }
    void     clear() { d.clear(); }

    iterator       find(const K& k)       { return d.find(k); }
    const_iterator find(const K& k) const { return d.find(k); }
    size_t size()  const noexcept         { return d.size(); }
    bool   empty() const noexcept         { return d.empty(); }

    iterator       begin() noexcept       { return d.begin(); }
    iterator       end()   noexcept       { return d.end(); }
    const_iterator begin() const noexcept { return d.begin(); }
    const_iterator end()   const noexcept { return d.end(); }
};

// ============================================================
// Pair<A,B>
// ============================================================

template<typename A, typename B>
struct Pair {
    A first;
    B second;

    Pair() = default;
    Pair(const A& a, const B& b) : first(a), second(b) {}
    Pair(A&& a, B&& b) : first(std::move(a)), second(std::move(b)) {}
    template<typename U, typename V>
    Pair(const Pair<U, V>& o) : first(o.first), second(o.second) {}
    template<typename U, typename V>
    Pair& operator=(const Pair<U, V>& o) { first = o.first; second = o.second; return *this; }

    bool operator==(const Pair& o) const { return first == o.first && second == o.second; }
    bool operator!=(const Pair& o) const { return !(*this == o); }
    bool operator<(const Pair& o) const {
        return first < o.first || (first == o.first && second < o.second);
    }
};

template<typename A, typename B>
Pair(A, B) -> Pair<A, B>;

// ============================================================
// std::set operators (for code using raw std::set)
// ============================================================

template<typename T>
inline std::set<T>& operator<<(std::set<T>& s, const T& val) {
    s.insert(val);
    return s;
}

template<typename T>
inline std::set<T>& operator+=(std::set<T>& lhs, const std::set<T>& rhs) {
    for (const auto& v : rhs) lhs.insert(v);
    return lhs;
}

template<typename T>
inline std::set<T> operator+(const std::set<T>& lhs, const std::set<T>& rhs) {
    std::set<T> result = lhs;
    for (const auto& v : rhs) result.insert(v);
    return result;
}

// ============================================================
// Aliases
// ============================================================

template<typename T>
using Vector = List<T>;

using StringList = List<String>;

} // namespace
