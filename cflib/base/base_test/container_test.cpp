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

class Container_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Container_test *>(this);
        return {
            {"testList_constructors",                [self]() { self->testList_constructors(); }},
            {"testList_elementAccess",               [self]() { self->testList_elementAccess(); }},
            {"testList_modification",                [self]() { self->testList_modification(); }},
            {"testList_iterators",                   [self]() { self->testList_iterators(); }},
            {"testList_utility",                     [self]() { self->testList_utility(); }},
            {"testList_takeOperations",              [self]() { self->testList_takeOperations(); }},
            {"testList_operators",                   [self]() { self->testList_operators(); }},
            {"testList_join",                        [self]() { self->testList_join(); }},
            {"testList_boolSpecialization",          [self]() { self->testList_boolSpecialization(); }},
            {"testHash_constructors",                [self]() { self->testHash_constructors(); }},
            {"testHash_elementAccess",               [self]() { self->testHash_elementAccess(); }},
            {"testHash_modification",                [self]() { self->testHash_modification(); }},
            {"testHash_lookup",                      [self]() { self->testHash_lookup(); }},
            {"testHash_iterators",                   [self]() { self->testHash_iterators(); }},
            {"testHash_utility",                     [self]() { self->testHash_utility(); }},
            {"testMap_constructors",                 [self]() { self->testMap_constructors(); }},
            {"testMap_modification",                 [self]() { self->testMap_modification(); }},
            {"testMap_bounds",                       [self]() { self->testMap_bounds(); }},
            {"testSet_constructors",                 [self]() { self->testSet_constructors(); }},
            {"testSet_modification",                 [self]() { self->testSet_modification(); }},
            {"testSet_operators",                    [self]() { self->testSet_operators(); }},
            {"testMultiMap_constructors",            [self]() { self->testMultiMap_constructors(); }},
            {"testMultiHash_constructors",           [self]() { self->testMultiHash_constructors(); }},
            {"testPair_constructors",                [self]() { self->testPair_constructors(); }},
            {"testPair_operators",                   [self]() { self->testPair_operators(); }},
            {"testSetOperators",                     [self]() { self->testSetOperators(); }},
        };
    }

private:
    // ------------------------------------------------------------
    // List<T> Tests
    // ------------------------------------------------------------

    void testList_constructors()
    {
        // Default constructor
        List<int> list1;
        TVERIFY(list1.isEmpty());
        TVERIFY(list1.size() == 0);

        // From size_t n
        List<int> list2(5);
        TVERIFY(list2.size() == 5);
        TVERIFY(list2[0] == 0);
        TVERIFY(list2[4] == 0);

        // From size_t n, const T& val
        List<int> list3(3, 42);
        TVERIFY(list3.size() == 3);
        TCOMPARE(list3[0], 42);
        TCOMPARE(list3[1], 42);
        TCOMPARE(list3[2], 42);

        // From initializer_list
        List<int> list4({1, 2, 3, 4});
        TVERIFY(list4.size() == 4);
        TCOMPARE(list4[0], 1);
        TCOMPARE(list4[1], 2);
        TCOMPARE(list4[2], 3);
        TCOMPARE(list4[3], 4);

        // From std::vector
        std::vector<int> vec = {10, 20, 30};
        List<int> list5(vec);
        TVERIFY(list5.size() == 3);
        TCOMPARE(list5[0], 10);
        TCOMPARE(list5[1], 20);
        TCOMPARE(list5[2], 30);

        // From iterator range
        std::vector<int> vec2 = {100, 200, 300};
        List<int> list6(vec2.begin(), vec2.end());
        TVERIFY(list6.size() == 3);
        TCOMPARE(list6[0], 100);
        TCOMPARE(list6[1], 200);
        TCOMPARE(list6[2], 300);

        // String list
        List<String> list7({"a", "b", "c"});
        TVERIFY(list7.size() == 3);
        TCOMPARE(list7[0], String("a"));

        // Double list
        List<double> list8({1.1, 2.2, 3.3});
        TVERIFY(list8.size() == 3);
        TCOMPARE(list8[0], 1.1);
    }

    void testList_elementAccess()
    {
        List<int> list({10, 20, 30, 40, 50});

        // front() and back()
        TCOMPARE(list.front(), 10);
        TCOMPARE(list.back(), 50);

        // first() and last() (aliases)
        TCOMPARE(list.first(), 10);
        TCOMPARE(list.last(), 50);

        // at()
        TCOMPARE(list.at(0), 10);
        TCOMPARE(list.at(2), 30);
        TCOMPARE(list.at(4), 50);

        // operator[]
        TCOMPARE(list[0], 10);
        TCOMPARE(list[2], 30);
        TCOMPARE(list[4], 50);

        // Modify via operator[]
        list[1] = 99;
        TCOMPARE(list[1], 99);

        // Const access
        const List<int> constList({1, 2, 3});
        TCOMPARE(constList.front(), 1);
        TCOMPARE(constList.back(), 3);
        TCOMPARE(constList[1], 2);
    }

    void testList_modification()
    {
        List<int> list;

        // push_back
        list.push_back(1);
        list.push_back(2);
        list.push_back(3);
        TVERIFY(list.size() == 3);
        TCOMPARE(list[0], 1);
        TCOMPARE(list[1], 2);
        TCOMPARE(list[2], 3);

        // push_back with rvalue
        list.push_back(99);
        TCOMPARE(list.back(), 99);

        // emplace_back
        struct Point { int x, y; };
        List<Point> pointList;
        pointList.emplace_back().x = 1;
        pointList.emplace_back().y = 2;
        TVERIFY(pointList.size() == 2);

        // pop_back
        list.pop_back();
        TVERIFY(list.size() == 3);
        TCOMPARE(list.back(), 3);

        // insert
        List<int> list2({1, 2, 3, 4});
        list2.insert(list2.begin() + 2, 99);
        TVERIFY(list2.size() == 5);
        TCOMPARE(list2[2], 99);
        TCOMPARE(list2[3], 3);

        // insert range
        List<int> list3({1, 2});
        std::vector<int> vec = {10, 20};
        list3.insert(list3.end(), vec.begin(), vec.end());
        TVERIFY(list3.size() == 4);
        TCOMPARE(list3[2], 10);
        TCOMPARE(list3[3], 20);

        // erase single
        List<int> list4({1, 2, 3, 4, 5});
        list4.erase(list4.begin() + 2);
        TVERIFY(list4.size() == 4);
        TCOMPARE(list4[2], 4);

        // erase range
        List<int> list5({1, 2, 3, 4, 5, 6});
        list5.erase(list5.begin() + 1, list5.begin() + 4);
        TVERIFY(list5.size() == 3);
        TCOMPARE(list5[0], 1);
        TCOMPARE(list5[1], 5);
        TCOMPARE(list5[2], 6);

        // clear
        List<int> list6({1, 2, 3});
        list6.clear();
        TVERIFY(list6.isEmpty());
        TVERIFY(list6.size() == 0);

        // resize
        List<int> list7({1, 2, 3});
        list7.resize(5);
        TVERIFY(list7.size() == 5);
        TCOMPARE(list7[3], 0);
        TVERIFY(list7[4] == 0);

        // resize with value
        List<int> list8({1, 2, 3});
        list8.resize(5, 99);
        TVERIFY(list8.size() == 5);
        TCOMPARE(list8[3], 99);
        TCOMPARE(list8[4], 99);

        // reserve
        List<int> list9;
        list9.reserve(100);
    }

    void testList_iterators()
    {
        List<int> list({1, 2, 3, 4, 5});

        // begin/end
        int sum = 0;
        for (auto it = list.begin(); it != list.end(); ++it) {
            sum += *it;
        }
        TCOMPARE(sum, 15);

        // Modified iteration
        for (auto& val : list) {
            val *= 2;
        }
        TCOMPARE(list[0], 2);
        TCOMPARE(list[4], 10);

        // cbegin/cend
        sum = 0;
        for (auto it = list.cbegin(); it != list.cend(); ++it) {
            sum += *it;
        }
        TCOMPARE(sum, 30);
    }

    void testList_utility()
    {
        List<int> list({3, 1, 4, 1, 5, 2});

        // size and isEmpty
        TVERIFY(list.size() == 6);
        TVERIFY(!list.isEmpty());

        List<int> emptyList;
        TVERIFY(emptyList.isEmpty());
        TVERIFY(emptyList.size() == 0);

        // contains
        TVERIFY(list.contains(1));
        TVERIFY(list.contains(5));
        TVERIFY(!list.contains(99));

        // sort
        list.sort();
        TCOMPARE(list[0], 1);
        TCOMPARE(list[1], 1);
        TCOMPARE(list[2], 2);
        TCOMPARE(list[3], 3);
        TCOMPARE(list[4], 4);
        TCOMPARE(list[5], 5);

        // sorted (returns new list)
        List<int> list2({5, 3, 1});
        List<int> list3 = list2.sorted();
        TVERIFY(list2.size() == 3);
        TVERIFY(list3.size() == 3);
        TCOMPARE(list3[0], 1);
        TCOMPARE(list3[1], 3);
        TCOMPARE(list3[2], 5);
        // Original should be unchanged
        TCOMPARE(list2[0], 5);
    }

    void testList_takeOperations()
    {
        List<int> list({1, 2, 3});

        // takeFirst
        int val1 = list.takeFirst();
        TCOMPARE(val1, 1);
        TVERIFY(list.size() == 2);
        TCOMPARE(list[0], 2);

        // takeLast
        int val2 = list.takeLast();
        TCOMPARE(val2, 3);
        TVERIFY(list.size() == 1);
        TCOMPARE(list[0], 2);
    }

    void testList_operators()
    {
        // operator<<
        List<int> list1;
        list1 << 1 << 2 << 3;
        TVERIFY(list1.size() == 3);
        TCOMPARE(list1[0], 1);

        // operator<< with rvalue
        List<String> list2;
        list2 << String("a") << String("b");
        TVERIFY(list2.size() == 2);

        // operator+=
        List<int> list3({1, 2});
        List<int> list4({3, 4});
        list3 += list4;
        TVERIFY(list3.size() == 4);
        TCOMPARE(list3[0], 1);
        TCOMPARE(list3[3], 4);

        // operator+= with rvalue
        List<int> list5({1});
        List<int> list6({2});
        list5 += std::move(list6);
        TVERIFY(list5.size() == 2);
        // Note: std::vector insert with move iterators leaves source in valid but unspecified state

        // operator+
        List<int> list7({1, 2});
        List<int> list8({3, 4});
        List<int> list9 = list7 + list8;
        TVERIFY(list7.size() == 2);
        TVERIFY(list8.size() == 2);
        TVERIFY(list9.size() == 4);
        TCOMPARE(list9[0], 1);
        TCOMPARE(list9[3], 4);

        // operator+ with rvalue
        List<int> list10({1});
        List<int> list11({2});
        List<int> list12 = list10 + std::move(list11);
        // operator+ doesn't modify operands, list10 should still have 1 element
        TVERIFY(list10.size() == 1);
        // The result should have both elements
        TVERIFY(list12.size() == 2);
        TCOMPARE(list12[0], 1);
        TCOMPARE(list12[1], 2);
        // Note: move leaves source in valid but unspecified state

        // Equality operators
        List<int> list13({1, 2, 3});
        List<int> list14({1, 2, 3});
        List<int> list15({1, 2, 4});
        TVERIFY(list13 == list14);
        TVERIFY(!(list13 == list15));
        TVERIFY(list13 != list15);
    }

    void testList_join()
    {
        // String join
        List<String> list1({"a", "b", "c"});
        String joined1 = list1.join(",");
        TCOMPARE(joined1, String("a,b,c"));

        List<String> list2({"hello"});
        String joined2 = list2.join(", ");
        TCOMPARE(joined2, String("hello"));

        // Numeric types that support operator+
        List<int> list3({1, 2, 3});
        // Note: join with int doesn't make much sense but tests the template
        // We'll test with strings instead which is the practical use case
    }

    void testList_boolSpecialization()
    {
        // List<bool> uses std::vector<uint8> internally to avoid std::vector<bool> proxy issues
        List<bool> boolList;
        boolList.push_back(true);
        boolList.push_back(false);
        boolList.push_back(true);

        TVERIFY(boolList.size() == 3);
        TVERIFY(boolList[0] == true);
        TVERIFY(boolList[1] == false);
        TVERIFY(boolList[2] == true);

        // Use push_back to fill since initializer_list<bool> won't work with internal vector<uint8>
        List<bool> boolList2;
        boolList2.push_back(true);
        boolList2.push_back(false);
        boolList2.push_back(true);
        boolList2.push_back(false);
        TVERIFY(boolList2.size() == 4);

        for (size_t i = 0; i < boolList2.size(); ++i) {
            if (i % 2 == 0) {
                TVERIFY(boolList2[i] == true);
            } else {
                TVERIFY(boolList2[i] == false);
            }
        }

        // Test with size constructor
        List<bool> boolList3(3);
        TVERIFY(boolList3.size() == 3);
        TVERIFY(boolList3[0] == false);  // default initialized to 0
    }

    // ------------------------------------------------------------
    // Hash<K,V> Tests
    // ------------------------------------------------------------

    void testHash_constructors()
    {
        Hash<int, String> hash1;
        TVERIFY(hash1.isEmpty());
        TVERIFY(hash1.size() == 0);

        Hash<String, int> hash2;
        TVERIFY(hash2.isEmpty());
    }

    void testHash_elementAccess()
    {
        Hash<int, String> hash;

        // operator[]
        hash[1] = "one";
        hash[2] = "two";
        hash[3] = "three";

        TVERIFY(hash.size() == 3);
        TCOMPARE(hash[1], "one");
        TCOMPARE(hash[2], "two");
        TCOMPARE(hash[3], "three");

        // Modify via operator[]
        hash[2] = "TWO";
        TCOMPARE(hash[2], "TWO");

        // at()
        TCOMPARE(hash.at(1), "one");
        TCOMPARE(hash.at(3), "three");

        // Const at
        const Hash<int, String> constHash = hash;
        TCOMPARE(constHash.at(1), "one");
    }

    void testHash_modification()
    {
        Hash<int, String> hash;

        // insert
        auto result = hash.insert(std::make_pair(1, "one"));
        TVERIFY(result.second);
        TVERIFY(hash.size() == 1);

        // insert duplicate (returns false)
        result = hash.insert(std::make_pair(1, "ONE"));
        TVERIFY(!result.second);
        TVERIFY(hash.size() == 1);
        TCOMPARE(hash[1], "one");  // Original value unchanged

        // insert with multiple values
        hash.insert(std::make_pair(2, "two"));
        hash.insert(std::make_pair(3, "three"));
        TVERIFY(hash.size() == 3);

        // erase by key
        size_t erased = hash.erase(2);
        TVERIFY(erased == 1);
        TVERIFY(hash.size() == 2);
        TVERIFY(!hash.contains(2));

        // erase iterator
        auto it = hash.find(3);
        auto it2 = hash.erase(it);
        TVERIFY(hash.size() == 1);
        // Note: unordered_map::erase returns iterator to element after erased one
        // After erasing 3, it2 should be end() (or potentially valid if container reordered)
        // We just verify it's a valid iterator by checking it's either end or points to remaining element
        if (it2 != hash.end()) {
            TVERIFY(it2->first == 1);
        }

        // clear
        hash.clear();
        TVERIFY(hash.isEmpty());
        TVERIFY(hash.size() == 0);
    }

    void testHash_lookup()
    {
        Hash<int, String> hash;
        hash[1] = "one";
        hash[2] = "two";

        // find
        auto it = hash.find(1);
        TVERIFY(it != hash.end());
        TCOMPARE(it->second, "one");

        auto it2 = hash.find(99);
        TVERIFY(it2 == hash.end());

        // count
        TVERIFY(hash.count(1) == 1);
        TVERIFY(hash.count(99) == 0);

        // contains
        TVERIFY(hash.contains(1));
        TVERIFY(hash.contains(2));
        TVERIFY(!hash.contains(99));
    }

    void testHash_iterators()
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
        TVERIFY(count == 3);

        // Iteration with const iterator
        count = 0;
        const Hash<int, String> constHash = hash;
        for (auto it = constHash.begin(); it != constHash.end(); ++it) {
            count++;
        }
        TVERIFY(count == 3);
    }

    void testHash_utility()
    {
        Hash<int, String> hash;
        hash[1] = "one";
        hash[2] = "two";

        // size and isEmpty
        TVERIFY(hash.size() == 2);
        TVERIFY(!hash.isEmpty());

        // value with default
        TCOMPARE(hash.value(1), "one");
        TCOMPARE(hash.value(99), String(""));
        TCOMPARE(hash.value(99, String("default")), "default");

        // keys()
        List<int> keys = hash.keys();
        TVERIFY(keys.size() == 2);
        TVERIFY(keys.contains(1));
        TVERIFY(keys.contains(2));

        // values()
        List<String> values = hash.values();
        TVERIFY(values.size() == 2);
        TVERIFY(values.contains("one"));
        TVERIFY(values.contains("two"));
    }

    // ------------------------------------------------------------
    // Map<K,V> Tests
    // ------------------------------------------------------------

    void testMap_constructors()
    {
        Map<int, String> map1;
        TVERIFY(map1.isEmpty());

        Map<String, int> map2;
        TVERIFY(map2.isEmpty());

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

        TVERIFY(map3 == map4);
        TVERIFY(!(map3 == map5));
        TVERIFY(map3 != map5);
    }

    void testMap_modification()
    {
        Map<int, String> map;

        // insert
        auto result = map.insert(std::make_pair(1, "one"));
        TVERIFY(result.second);

        // erase
        map.erase(1);
        TVERIFY(map.isEmpty());

        // clear
        map[1] = "one";
        map[2] = "two";
        map.clear();
        TVERIFY(map.isEmpty());
    }

    void testMap_bounds()
    {
        Map<int, String> map;
        map[1] = "one";
        map[3] = "three";
        map[5] = "five";

        // lower_bound
        auto it1 = map.lower_bound(3);
        TVERIFY(it1 != map.end());
        TVERIFY(it1->first == 3);

        auto it2 = map.lower_bound(2);
        TVERIFY(it2 != map.end());
        TVERIFY(it2->first == 3);  // First >= 2 is 3

        // upper_bound
        auto it3 = map.upper_bound(3);
        TVERIFY(it3 != map.end());
        TVERIFY(it3->first == 5);  // First > 3 is 5

        auto it4 = map.upper_bound(5);
        TVERIFY(it4 == map.end());  // No > 5
    }

    // ------------------------------------------------------------
    // Set<T> Tests
    // ------------------------------------------------------------

    void testSet_constructors()
    {
        Set<int> set1;
        TVERIFY(set1.isEmpty());

        Set<String> set2;
        TVERIFY(set2.isEmpty());

        // Insert elements - Set doesn't have initializer_list constructor
        Set<int> set3;
        set3.insert(1);
        set3.insert(2);
        set3.insert(3);
        TVERIFY(set3.size() == 3);
    }

    void testSet_modification()
    {
        Set<int> set;

        // insert
        auto result = set.insert(1);
        TVERIFY(result.second);
        TVERIFY(set.size() == 1);

        // insert duplicate (first is iterator to existing)
        auto result2 = set.insert(1);
        TVERIFY(!result2.second);
        TVERIFY(set.size() == 1);

        // insert multiple
        set.insert(2);
        set.insert(3);
        TVERIFY(set.size() == 3);

        // erase by value
        size_t erased = set.erase(2);
        TVERIFY(erased == 1);
        TVERIFY(set.size() == 2);
        TVERIFY(!set.contains(2));

        // clear
        set.clear();
        TVERIFY(set.isEmpty());

        // find
        set.insert(1);
        auto it = set.find(1);
        TVERIFY(it != set.end());
        TVERIFY(*it == 1);

        auto it2 = set.find(99);
        TVERIFY(it2 == set.end());

        // count
        TVERIFY(set.count(1) == 1);
        TVERIFY(set.count(99) == 0);

        // contains
        TVERIFY(set.contains(1));
        TVERIFY(!set.contains(99));

        // remove (alias for erase)
        set.insert(2);
        set.remove(2);
        TVERIFY(!set.contains(2));
    }

    void testSet_operators()
    {
        Set<int> set1;
        set1 << 1 << 2 << 3;

        // operator<<
        TVERIFY(set1.size() == 3);

        // operator+=
        Set<int> set2;
        set2 << 4 << 5;
        set1 += set2;
        TVERIFY(set1.size() == 5);
        TVERIFY(set1.contains(4));
        TVERIFY(set1.contains(5));

        // toList
        List<int> list = set1.toList();
        TVERIFY(list.size() == 5);
        TVERIFY(list.contains(1));
        TVERIFY(list.contains(5));
    }

    // ------------------------------------------------------------
    // MultiMap<K,V> Tests
    // ------------------------------------------------------------

    void testMultiMap_constructors()
    {
        MultiMap<int, String> mmap;
        TVERIFY(mmap.isEmpty());
        TVERIFY(mmap.size() == 0);
    }

    // ------------------------------------------------------------
    // MultiHash<K,V> Tests
    // ------------------------------------------------------------

    void testMultiHash_constructors()
    {
        MultiHash<int, String> mhash;
        TVERIFY(mhash.isEmpty());
        TVERIFY(mhash.size() == 0);
    }

    // ------------------------------------------------------------
    // Pair<A,B> Tests
    // ------------------------------------------------------------

    void testPair_constructors()
    {
        // Default constructor
        Pair<int, String> p1;
        // Note: int is uninitialized with default constructor, so we just verify it compiles
        TVERIFY(p1.second == "");

        // From two values
        Pair<int, String> p2(42, "answer");
        TVERIFY(p2.first == 42);
        TVERIFY(p2.second == "answer");

        // From rvalues
        Pair<int, String> p3(99, String("fast"));
        TVERIFY(p3.first == 99);
        TVERIFY(p3.second == "fast");

        // From different Pair type
        Pair<int, int> p4(1, 2);
        Pair<double, double> p5(p4);
        TVERIFY(p5.first == 1.0);
        TVERIFY(p5.second == 2.0);

        // Assignment from different Pair type
        Pair<int, int> p6(3, 4);
        Pair<double, double> p7;
        p7 = p6;
        TVERIFY(p7.first == 3.0);
        TVERIFY(p7.second == 4.0);

        // Deduction guide
        auto p8 = Pair(123, "test");
        TVERIFY(p8.first == 123);
        TVERIFY(p8.second == String("test"));
    }

    void testPair_operators()
    {
        // operator==
        Pair<int, String> p1(1, "a");
        Pair<int, String> p2(1, "a");
        Pair<int, String> p3(2, "a");
        Pair<int, String> p4(1, "b");

        TVERIFY(p1 == p2);
        TVERIFY(!(p1 == p3));
        TVERIFY(!(p1 == p4));

        // operator!=
        TVERIFY(p1 != p3);
        TVERIFY(p1 != p4);
        TVERIFY(!(p1 != p2));

        // operator<
        Pair<int, String> p5(1, "b");
        Pair<int, String> p6(1, "a");
        Pair<int, String> p7(2, "a");

        TVERIFY(p6 < p5);  // Same first, second "a" < "b"
        TVERIFY(p5 < p7);  // First 1 < 2
        TVERIFY(!(p7 < p5));
        TVERIFY(!(p5 < p6));
    }

    // ------------------------------------------------------------
    // std::set Operators Tests
    // ------------------------------------------------------------

    void testSetOperators()
    {
        // operator<<
        std::set<int> set1;
        set1 << 1 << 2 << 3;
        TVERIFY(set1.size() == 3);
        TVERIFY(set1.count(1) == 1);
        TVERIFY(set1.count(2) == 1);
        TVERIFY(set1.count(3) == 1);

        // operator+=
        std::set<int> set2;
        set2 << 4 << 5;
        set1 += set2;
        TVERIFY(set1.size() == 5);
        TVERIFY(set1.count(4) == 1);
        TVERIFY(set1.count(5) == 1);

        // operator+
        std::set<int> set3;
        set3 << 1 << 2;
        std::set<int> set4;
        set4 << 3 << 4;
        std::set<int> set5 = set3 + set4;
        TVERIFY(set3.size() == 2);
        TVERIFY(set4.size() == 2);
        TVERIFY(set5.size() == 4);
        TVERIFY(set5.count(1) == 1);
        TVERIFY(set5.count(4) == 1);
    }
};

ADD_TEST(Container_test)
