/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/base.h>
#include <cflib/util/test.h>

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace cflib::base;

TEST_SUITE("Container") {

// ------------------------------------------------------------
// List<T> Tests
// ------------------------------------------------------------

TEST_CASE("List: constructors")
{
    // Default constructor
    List<int> list1;
    REQUIRE(list1.isEmpty());
    REQUIRE(list1.size() == 0);

    // From size_t n
    List<int> list2(5);
    REQUIRE(list2.size() == 5);
    REQUIRE(list2[0] == 0);
    REQUIRE(list2[4] == 0);

    // From size_t n, const T& val
    List<int> list3(3, 42);
    REQUIRE(list3.size() == 3);
    REQUIRE_EQ(list3[0], 42);
    REQUIRE_EQ(list3[1], 42);
    REQUIRE_EQ(list3[2], 42);

    // From initializer_list
    List<int> list4({1, 2, 3, 4});
    REQUIRE(list4.size() == 4);
    REQUIRE_EQ(list4[0], 1);
    REQUIRE_EQ(list4[1], 2);
    REQUIRE_EQ(list4[2], 3);
    REQUIRE_EQ(list4[3], 4);

    // From std::vector
    std::vector<int> vec = {10, 20, 30};
    List<int> list5(vec);
    REQUIRE(list5.size() == 3);
    REQUIRE_EQ(list5[0], 10);
    REQUIRE_EQ(list5[1], 20);
    REQUIRE_EQ(list5[2], 30);

    // From iterator range
    std::vector<int> vec2 = {100, 200, 300};
    List<int> list6(vec2.begin(), vec2.end());
    REQUIRE(list6.size() == 3);
    REQUIRE_EQ(list6[0], 100);
    REQUIRE_EQ(list6[1], 200);
    REQUIRE_EQ(list6[2], 300);

    // String list
    List<String> list7({"a", "b", "c"});
    REQUIRE(list7.size() == 3);
    REQUIRE_EQ(list7[0], String("a"));

    // Double list
    List<double> list8({1.1, 2.2, 3.3});
    REQUIRE(list8.size() == 3);
    REQUIRE_EQ(list8[0], 1.1);
}

TEST_CASE("List: elementAccess")
{
    List<int> list({10, 20, 30, 40, 50});

    // front() and back()
    REQUIRE_EQ(list.front(), 10);
    REQUIRE_EQ(list.back(), 50);

    // first() and last() (aliases)
    REQUIRE_EQ(list.first(), 10);
    REQUIRE_EQ(list.last(), 50);

    // at()
    REQUIRE_EQ(list.at(0), 10);
    REQUIRE_EQ(list.at(2), 30);
    REQUIRE_EQ(list.at(4), 50);

    // operator[]
    REQUIRE_EQ(list[0], 10);
    REQUIRE_EQ(list[2], 30);
    REQUIRE_EQ(list[4], 50);

    // Modify via operator[]
    list[1] = 99;
    REQUIRE_EQ(list[1], 99);

    // Const access
    const List<int> constList({1, 2, 3});
    REQUIRE_EQ(constList.front(), 1);
    REQUIRE_EQ(constList.back(), 3);
    REQUIRE_EQ(constList[1], 2);
}

TEST_CASE("List: modification")
{
    List<int> list;

    // push_back
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    REQUIRE(list.size() == 3);
    REQUIRE_EQ(list[0], 1);
    REQUIRE_EQ(list[1], 2);
    REQUIRE_EQ(list[2], 3);

    // push_back with rvalue
    list.push_back(99);
    REQUIRE_EQ(list.back(), 99);

    // emplace_back
    struct Point { int x, y; };
    List<Point> pointList;
    pointList.emplace_back().x = 1;
    pointList.emplace_back().y = 2;
    REQUIRE(pointList.size() == 2);

    // pop_back
    list.pop_back();
    REQUIRE(list.size() == 3);
    REQUIRE_EQ(list.back(), 3);

    // insert
    List<int> list2({1, 2, 3, 4});
    list2.insert(list2.begin() + 2, 99);
    REQUIRE(list2.size() == 5);
    REQUIRE_EQ(list2[2], 99);
    REQUIRE_EQ(list2[3], 3);

    // insert range
    List<int> list3({1, 2});
    std::vector<int> vec = {10, 20};
    list3.insert(list3.end(), vec.begin(), vec.end());
    REQUIRE(list3.size() == 4);
    REQUIRE_EQ(list3[2], 10);
    REQUIRE_EQ(list3[3], 20);

    // erase single
    List<int> list4({1, 2, 3, 4, 5});
    list4.erase(list4.begin() + 2);
    REQUIRE(list4.size() == 4);
    REQUIRE_EQ(list4[2], 4);

    // erase range
    List<int> list5({1, 2, 3, 4, 5, 6});
    list5.erase(list5.begin() + 1, list5.begin() + 4);
    REQUIRE(list5.size() == 3);
    REQUIRE_EQ(list5[0], 1);
    REQUIRE_EQ(list5[1], 5);
    REQUIRE_EQ(list5[2], 6);

    // clear
    List<int> list6({1, 2, 3});
    list6.clear();
    REQUIRE(list6.isEmpty());
    REQUIRE(list6.size() == 0);

    // resize
    List<int> list7({1, 2, 3});
    list7.resize(5);
    REQUIRE(list7.size() == 5);
    REQUIRE_EQ(list7[3], 0);
    REQUIRE(list7[4] == 0);

    // resize with value
    List<int> list8({1, 2, 3});
    list8.resize(5, 99);
    REQUIRE(list8.size() == 5);
    REQUIRE_EQ(list8[3], 99);
    REQUIRE_EQ(list8[4], 99);

    // reserve
    List<int> list9;
    list9.reserve(100);
}

TEST_CASE("List: iterators")
{
    List<int> list({1, 2, 3, 4, 5});

    // begin/end
    int sum = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        sum += *it;
    }
    REQUIRE_EQ(sum, 15);

    // Modified iteration
    for (auto& val : list) {
        val *= 2;
    }
    REQUIRE_EQ(list[0], 2);
    REQUIRE_EQ(list[4], 10);

    // cbegin/cend
    sum = 0;
    for (auto it = list.cbegin(); it != list.cend(); ++it) {
        sum += *it;
    }
    REQUIRE_EQ(sum, 30);
}

TEST_CASE("List: utility")
{
    List<int> list({3, 1, 4, 1, 5, 2});

    // size and isEmpty
    REQUIRE(list.size() == 6);
    REQUIRE(!list.isEmpty());

    List<int> emptyList;
    REQUIRE(emptyList.isEmpty());
    REQUIRE(emptyList.size() == 0);

    // contains
    REQUIRE(list.contains(1));
    REQUIRE(list.contains(5));
    REQUIRE(!list.contains(99));

    // sort
    list.sort();
    REQUIRE_EQ(list[0], 1);
    REQUIRE_EQ(list[1], 1);
    REQUIRE_EQ(list[2], 2);
    REQUIRE_EQ(list[3], 3);
    REQUIRE_EQ(list[4], 4);
    REQUIRE_EQ(list[5], 5);

    // sorted (returns new list)
    List<int> list2({5, 3, 1});
    List<int> list3 = list2.sorted();
    REQUIRE(list2.size() == 3);
    REQUIRE(list3.size() == 3);
    REQUIRE_EQ(list3[0], 1);
    REQUIRE_EQ(list3[1], 3);
    REQUIRE_EQ(list3[2], 5);
    // Original should be unchanged
    REQUIRE_EQ(list2[0], 5);
}

TEST_CASE("List: takeOperations")
{
    List<int> list({1, 2, 3});

    // takeFirst
    int val1 = list.takeFirst();
    REQUIRE_EQ(val1, 1);
    REQUIRE(list.size() == 2);
    REQUIRE_EQ(list[0], 2);

    // takeLast
    int val2 = list.takeLast();
    REQUIRE_EQ(val2, 3);
    REQUIRE(list.size() == 1);
    REQUIRE_EQ(list[0], 2);
}

TEST_CASE("List: operators")
{
    // operator<<
    List<int> list1;
    list1 << 1 << 2 << 3;
    REQUIRE(list1.size() == 3);
    REQUIRE_EQ(list1[0], 1);

    // operator<< with rvalue
    List<String> list2;
    list2 << String("a") << String("b");
    REQUIRE(list2.size() == 2);

    // operator+=
    List<int> list3({1, 2});
    List<int> list4({3, 4});
    list3 += list4;
    REQUIRE(list3.size() == 4);
    REQUIRE_EQ(list3[0], 1);
    REQUIRE_EQ(list3[3], 4);

    // operator+= with rvalue
    List<int> list5({1});
    List<int> list6({2});
    list5 += std::move(list6);
    REQUIRE(list5.size() == 2);
    // Note: std::vector insert with move iterators leaves source in valid but unspecified state

    // operator+
    List<int> list7({1, 2});
    List<int> list8({3, 4});
    List<int> list9 = list7 + list8;
    REQUIRE(list7.size() == 2);
    REQUIRE(list8.size() == 2);
    REQUIRE(list9.size() == 4);
    REQUIRE_EQ(list9[0], 1);
    REQUIRE_EQ(list9[3], 4);

    // operator+ with rvalue
    List<int> list10({1});
    List<int> list11({2});
    List<int> list12 = list10 + std::move(list11);
    // operator+ doesn't modify operands, list10 should still have 1 element
    REQUIRE(list10.size() == 1);
    // The result should have both elements
    REQUIRE(list12.size() == 2);
    REQUIRE_EQ(list12[0], 1);
    REQUIRE_EQ(list12[1], 2);
    // Note: move leaves source in valid but unspecified state

    // Equality operators
    List<int> list13({1, 2, 3});
    List<int> list14({1, 2, 3});
    List<int> list15({1, 2, 4});
    REQUIRE(list13 == list14);
    REQUIRE(!(list13 == list15));
    REQUIRE(list13 != list15);
}

TEST_CASE("List: join")
{
    // String join
    List<String> list1({"a", "b", "c"});
    String joined1 = list1.join(",");
    REQUIRE_EQ(joined1, String("a,b,c"));

    List<String> list2({"hello"});
    String joined2 = list2.join(", ");
    REQUIRE_EQ(joined2, String("hello"));

    // Numeric types that support operator+
    List<int> list3({1, 2, 3});
    // Note: join with int doesn't make much sense but tests the template
    // We'll test with strings instead which is the practical use case
}

TEST_CASE("List: boolSpecialization")
{
    // List<bool> uses std::vector<uint8> internally to avoid std::vector<bool> proxy issues
    List<bool> boolList;
    boolList.push_back(true);
    boolList.push_back(false);
    boolList.push_back(true);

    REQUIRE(boolList.size() == 3);
    REQUIRE(boolList[0] == true);
    REQUIRE(boolList[1] == false);
    REQUIRE(boolList[2] == true);

    // Use push_back to fill since initializer_list<bool> won't work with internal vector<uint8>
    List<bool> boolList2;
    boolList2.push_back(true);
    boolList2.push_back(false);
    boolList2.push_back(true);
    boolList2.push_back(false);
    REQUIRE(boolList2.size() == 4);

    for (size_t i = 0; i < boolList2.size(); ++i) {
        if (i % 2 == 0) {
            REQUIRE(boolList2[i] == true);
        } else {
            REQUIRE(boolList2[i] == false);
        }
    }

    // Test with size constructor
    List<bool> boolList3(3);
    REQUIRE(boolList3.size() == 3);
    REQUIRE(boolList3[0] == false);  // default initialized to 0
}

// ------------------------------------------------------------
// Hash<K,V> Tests
// ------------------------------------------------------------

TEST_CASE("Hash: constructors")
{
    Hash<int, String> hash1;
    REQUIRE(hash1.isEmpty());
    REQUIRE(hash1.size() == 0);

    Hash<String, int> hash2;
    REQUIRE(hash2.isEmpty());
}

TEST_CASE("Hash: elementAccess")
{
    Hash<int, String> hash;

    // operator[]
    hash[1] = "one";
    hash[2] = "two";
    hash[3] = "three";

    REQUIRE(hash.size() == 3);
    REQUIRE_EQ(hash[1], "one");
    REQUIRE_EQ(hash[2], "two");
    REQUIRE_EQ(hash[3], "three");

    // Modify via operator[]
    hash[2] = "TWO";
    REQUIRE_EQ(hash[2], "TWO");

    // at()
    REQUIRE_EQ(hash.at(1), "one");
    REQUIRE_EQ(hash.at(3), "three");

    // Const at
    const Hash<int, String> constHash = hash;
    REQUIRE_EQ(constHash.at(1), "one");
}

TEST_CASE("Hash: modification")
{
    Hash<int, String> hash;

    // insert
    auto result = hash.insert(std::make_pair(1, "one"));
    REQUIRE(result.second);
    REQUIRE(hash.size() == 1);

    // insert duplicate (returns false)
    result = hash.insert(std::make_pair(1, "ONE"));
    REQUIRE(!result.second);
    REQUIRE(hash.size() == 1);
    REQUIRE_EQ(hash[1], "one");  // Original value unchanged

    // insert with multiple values
    hash.insert(std::make_pair(2, "two"));
    hash.insert(std::make_pair(3, "three"));
    REQUIRE(hash.size() == 3);

    // erase by key
    size_t erased = hash.erase(2);
    REQUIRE(erased == 1);
    REQUIRE(hash.size() == 2);
    REQUIRE(!hash.contains(2));

    // erase iterator
    auto it = hash.find(3);
    auto it2 = hash.erase(it);
    REQUIRE(hash.size() == 1);
    // Note: unordered_map::erase returns iterator to element after erased one
    // After erasing 3, it2 should be end() (or potentially valid if container reordered)
    // We just verify it's a valid iterator by checking it's either end or points to remaining element
    if (it2 != hash.end()) {
        REQUIRE(it2->first == 1);
    }

    // clear
    hash.clear();
    REQUIRE(hash.isEmpty());
    REQUIRE(hash.size() == 0);
}

TEST_CASE("Hash: lookup")
{
    Hash<int, String> hash;
    hash[1] = "one";
    hash[2] = "two";

    // find
    auto it = hash.find(1);
    REQUIRE(it != hash.end());
    REQUIRE_EQ(it->second, "one");

    auto it2 = hash.find(99);
    REQUIRE(it2 == hash.end());

    // count
    REQUIRE(hash.count(1) == 1);
    REQUIRE(hash.count(99) == 0);

    // contains
    REQUIRE(hash.contains(1));
    REQUIRE(hash.contains(2));
    REQUIRE(!hash.contains(99));
}

TEST_CASE("Hash: iterators")
{
    Hash<int, String> hash;
    hash[1] = "one";
    hash[2] = "two";
    hash[3] = "three";

    // iterate
    int count = 0;
    for (auto it = hash.begin(); it != hash.end(); ++it) {
        count++;
    }
    REQUIRE(count == 3);

    // Iteration with const iterator
    count = 0;
    const Hash<int, String> constHash = hash;
    for (auto it = constHash.begin(); it != constHash.end(); ++it) {
        count++;
    }
    REQUIRE(count == 3);
}

TEST_CASE("Hash: utility")
{
    Hash<int, String> hash;
    hash[1] = "one";
    hash[2] = "two";

    // size and isEmpty
    REQUIRE(hash.size() == 2);
    REQUIRE(!hash.isEmpty());

    // value with default
    REQUIRE_EQ(hash.value(1), "one");
    REQUIRE_EQ(hash.value(99), String(""));
    REQUIRE_EQ(hash.value(99, String("default")), "default");

    // keys()
    List<int> keys = hash.keys();
    REQUIRE(keys.size() == 2);
    REQUIRE(keys.contains(1));
    REQUIRE(keys.contains(2));

    // values()
    List<String> values = hash.values();
    REQUIRE(values.size() == 2);
    REQUIRE(values.contains("one"));
    REQUIRE(values.contains("two"));
}

// ------------------------------------------------------------
// Map<K,V> Tests
// ------------------------------------------------------------

TEST_CASE("Map: constructors")
{
    Map<int, String> map1;
    REQUIRE(map1.isEmpty());

    Map<String, int> map2;
    REQUIRE(map2.isEmpty());

    // operator== and !=
    Map<int, String> map3;
    map3[1] = "one";
    map3[2] = "two";

    Map<int, String> map4;
    map4[1] = "one";
    map4[2] = "two";

    Map<int, String> map5;
    map5[1] = "ONE";
    map5[2] = "two";

    REQUIRE(map3 == map4);
    REQUIRE(!(map3 == map5));
    REQUIRE(map3 != map5);
}

TEST_CASE("Map: modification")
{
    Map<int, String> map;

    // insert
    auto result = map.insert(std::make_pair(1, "one"));
    REQUIRE(result.second);

    // erase
    map.erase(1);
    REQUIRE(map.isEmpty());

    // clear
    map[1] = "one";
    map[2] = "two";
    map.clear();
    REQUIRE(map.isEmpty());
}

TEST_CASE("Map: bounds")
{
    Map<int, String> map;
    map[1] = "one";
    map[3] = "three";
    map[5] = "five";

    // lower_bound
    auto it1 = map.lower_bound(3);
    REQUIRE(it1 != map.end());
    REQUIRE(it1->first == 3);

    auto it2 = map.lower_bound(2);
    REQUIRE(it2 != map.end());
    REQUIRE(it2->first == 3);  // First >= 2 is 3

    // upper_bound
    auto it3 = map.upper_bound(3);
    REQUIRE(it3 != map.end());
    REQUIRE(it3->first == 5);  // First > 3 is 5

    auto it4 = map.upper_bound(5);
    REQUIRE(it4 == map.end());  // No > 5
}

// ------------------------------------------------------------
// Set<T> Tests
// ------------------------------------------------------------

TEST_CASE("Set: constructors")
{
    Set<int> set1;
    REQUIRE(set1.isEmpty());

    Set<String> set2;
    REQUIRE(set2.isEmpty());

    // Insert elements - Set doesn't have initializer_list constructor
    Set<int> set3;
    set3.insert(1);
    set3.insert(2);
    set3.insert(3);
    REQUIRE(set3.size() == 3);
}

TEST_CASE("Set: modification")
{
    Set<int> set;

    // insert
    auto result = set.insert(1);
    REQUIRE(result.second);
    REQUIRE(set.size() == 1);

    // insert duplicate (first is iterator to existing)
    auto result2 = set.insert(1);
    REQUIRE(!result2.second);
    REQUIRE(set.size() == 1);

    // insert multiple
    set.insert(2);
    set.insert(3);
    REQUIRE(set.size() == 3);

    // erase by value
    size_t erased = set.erase(2);
    REQUIRE(erased == 1);
    REQUIRE(set.size() == 2);
    REQUIRE(!set.contains(2));

    // clear
    set.clear();
    REQUIRE(set.isEmpty());

    // find
    set.insert(1);
    auto it = set.find(1);
    REQUIRE(it != set.end());
    REQUIRE(*it == 1);

    auto it2 = set.find(99);
    REQUIRE(it2 == set.end());

    // count
    REQUIRE(set.count(1) == 1);
    REQUIRE(set.count(99) == 0);

    // contains
    REQUIRE(set.contains(1));
    REQUIRE(!set.contains(99));

    // remove (alias for erase)
    set.insert(2);
    set.remove(2);
    REQUIRE(!set.contains(2));
}

TEST_CASE("Set: operators")
{
    Set<int> set1;
    set1 << 1 << 2 << 3;

    // operator<<
    REQUIRE(set1.size() == 3);

    // operator+=
    Set<int> set2;
    set2 << 4 << 5;
    set1 += set2;
    REQUIRE(set1.size() == 5);
    REQUIRE(set1.contains(4));
    REQUIRE(set1.contains(5));

    // toList
    List<int> list = set1.toList();
    REQUIRE(list.size() == 5);
    REQUIRE(list.contains(1));
    REQUIRE(list.contains(5));
}

// ------------------------------------------------------------
// MultiMap<K,V> Tests
// ------------------------------------------------------------

TEST_CASE("MultiMap: constructors")
{
    MultiMap<int, String> mmap;
    REQUIRE(mmap.isEmpty());
    REQUIRE(mmap.size() == 0);
}

// ------------------------------------------------------------
// MultiHash<K,V> Tests
// ------------------------------------------------------------

TEST_CASE("MultiHash: constructors")
{
    MultiHash<int, String> mhash;
    REQUIRE(mhash.isEmpty());
    REQUIRE(mhash.size() == 0);
}

// ------------------------------------------------------------
// Pair<A,B> Tests
// ------------------------------------------------------------

TEST_CASE("Pair: constructors")
{
    // Default constructor
    Pair<int, String> p1;
    // Note: int is uninitialized with default constructor, so we just verify it compiles
    REQUIRE(p1.second == "");

    // From two values
    Pair<int, String> p2(42, "answer");
    REQUIRE(p2.first == 42);
    REQUIRE(p2.second == "answer");

    // From rvalues
    Pair<int, String> p3(99, String("fast"));
    REQUIRE(p3.first == 99);
    REQUIRE(p3.second == "fast");

    // From different Pair type
    Pair<int, int> p4(1, 2);
    Pair<double, double> p5(p4);
    REQUIRE(p5.first == 1.0);
    REQUIRE(p5.second == 2.0);

    // Assignment from different Pair type
    Pair<int, int> p6(3, 4);
    Pair<double, double> p7;
    p7 = p6;
    REQUIRE(p7.first == 3.0);
    REQUIRE(p7.second == 4.0);

    // Deduction guide
    auto p8 = Pair(123, "test");
    REQUIRE(p8.first == 123);
    REQUIRE(p8.second == String("test"));
}

TEST_CASE("Pair: operators")
{
    // operator==
    Pair<int, String> p1(1, "a");
    Pair<int, String> p2(1, "a");
    Pair<int, String> p3(2, "a");
    Pair<int, String> p4(1, "b");

    REQUIRE(p1 == p2);
    REQUIRE(!(p1 == p3));
    REQUIRE(!(p1 == p4));

    // operator!=
    REQUIRE(p1 != p3);
    REQUIRE(p1 != p4);
    REQUIRE(!(p1 != p2));

    // operator<
    Pair<int, String> p5(1, "b");
    Pair<int, String> p6(1, "a");
    Pair<int, String> p7(2, "a");

    REQUIRE(p6 < p5);  // Same first, second "a" < "b"
    REQUIRE(p5 < p7);  // First 1 < 2
    REQUIRE(!(p7 < p5));
    REQUIRE(!(p5 < p6));
}

// ------------------------------------------------------------
// std::set Operators Tests
// ------------------------------------------------------------

TEST_CASE("std::set: operators")
{
    // operator<<
    std::set<int> set1;
    set1 << 1 << 2 << 3;
    REQUIRE(set1.size() == 3);
    REQUIRE(set1.count(1) == 1);
    REQUIRE(set1.count(2) == 1);
    REQUIRE(set1.count(3) == 1);

    // operator+=
    std::set<int> set2;
    set2 << 4 << 5;
    set1 += set2;
    REQUIRE(set1.size() == 5);
    REQUIRE(set1.count(4) == 1);
    REQUIRE(set1.count(5) == 1);

    // operator+
    std::set<int> set3;
    set3 << 1 << 2;
    std::set<int> set4;
    set4 << 3 << 4;
    std::set<int> set5 = set3 + set4;
    REQUIRE(set3.size() == 2);
    REQUIRE(set4.size() == 2);
    REQUIRE(set5.size() == 4);
    REQUIRE(set5.count(1) == 1);
    REQUIRE(set5.count(4) == 1);
}

}
