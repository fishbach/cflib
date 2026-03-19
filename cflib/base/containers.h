/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/string.h>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <vector>

namespace cflib::base {

template<typename T>
using List = std::vector<T>;

template<typename T>
using Vector = std::vector<T>;

template<typename K, typename V>
using Hash = std::unordered_map<K, V>;

template<typename K, typename V>
using Map = std::map<K, V>;

template<typename T>
using Set = std::unordered_set<T>;

template<typename K, typename V>
using MultiMap = std::multimap<K, V>;

template<typename K, typename V>
using MultiHash = std::unordered_multimap<K, V>;

template<typename A, typename B>
using Pair = std::pair<A, B>;

using StringList = List<String>;

template<typename T>
inline std::vector<T> & operator<<(std::vector<T> & vec, const T & val) {
    vec.push_back(val);
    return vec;
}
template<typename T>
inline std::vector<T> & operator<<(std::vector<T> & vec, T && val) {
    vec.push_back(std::move(val));
    return vec;
}

template<typename T>
inline std::unordered_set<T> & operator<<(std::unordered_set<T> & set, const T & val) {
    set.insert(val);
    return set;
}
template<typename T>
inline std::unordered_set<T> & operator<<(std::unordered_set<T> & set, T && val) {
    set.insert(std::move(val));
    return set;
}

template<typename T>
inline std::unordered_set<T> & operator+=(std::unordered_set<T> & lhs, const std::unordered_set<T> & rhs) {
    for (const auto & v : rhs) lhs.insert(v);
    return lhs;
}

// operator<< for insert semantics on ordered sets
template<typename T>
inline std::set<T> & operator<<(std::set<T> & set, const T & val) {
    set.insert(val);
    return set;
}

// operator+= for ordered set union
template<typename T>
inline std::set<T> & operator+=(std::set<T> & lhs, const std::set<T> & rhs) {
    for (const auto & v : rhs) lhs.insert(v);
    return lhs;
}

// operator+ for ordered set union (returns new set)
template<typename T>
inline std::set<T> operator+(const std::set<T> & lhs, const std::set<T> & rhs) {
    std::set<T> result = lhs;
    for (const auto & v : rhs) result.insert(v);
    return result;
}

template<typename T>
inline std::vector<T> setValues(const std::unordered_set<T> & set) {
    return std::vector<T>(set.begin(), set.end());
}

inline ByteArray join(const std::vector<ByteArray> & list, char sep) {
    ByteArray r;
    for (cfsize_t i = 0; i < (cfsize_t)list.size(); ++i) {
        if (i > 0) r += sep;
        r += list[i];
    }
    return r;
}

inline String join(const std::vector<String> & list, char sep) {
    String r;
    for (cfsize_t i = 0; i < (cfsize_t)list.size(); ++i) {
        if (i > 0) r += sep;
        r += list[i];
    }
    return r;
}

template<typename T>
inline void sort(std::vector<T> & vec) {
    std::sort(vec.begin(), vec.end());
}

template<typename K, typename V>
inline std::vector<K> keys(const std::map<K, V> & m) {
    std::vector<K> r;
    r.reserve(m.size());
    for (const auto & p : m) r.push_back(p.first);
    return r;
}

template<typename K, typename V>
inline V mapValue(const std::map<K, V> & m, const K & key, const V & def = V()) {
    auto it = m.find(key);
    return it != m.end() ? it->second : def;
}

template<typename K, typename V>
inline V hashValue(const std::unordered_map<K, V> & m, const K & key, const V & def = V()) {
    auto it = m.find(key);
    return it != m.end() ? it->second : def;
}

template<typename K, typename V>
inline bool contains(const std::map<K, V> & m, const K & key) {
    return m.find(key) != m.end();
}

template<typename T>
inline bool contains(const std::unordered_set<T> & s, const T & val) {
    return s.find(val) != s.end();
}

template<typename T>
inline void remove(std::unordered_set<T> & s, const T & val) {
    s.erase(val);
}

template<typename T>
inline T takeFirst(std::vector<T> & vec) {
    T val = std::move(vec.front());
    vec.erase(vec.begin());
    return val;
}

template<typename T>
inline T takeLast(std::vector<T> & vec) {
    T val = std::move(vec.back());
    vec.pop_back();
    return val;
}

template<typename T>
inline const T & last(const std::vector<T> & vec) {
    return vec.back();
}

template<typename T, typename U>
inline bool contains(const std::vector<T> & v, const U & val) {
    for (const auto & item : v) if (item == val) return true;
    return false;
}

} // namespace
