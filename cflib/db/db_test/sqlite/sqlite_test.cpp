/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/db/sqlite/sqlite.h>
#include <cflib/util/test.h>
#include <cflib/util/util.h>

#include <cflib/base.h>

#include <cmath>
#include <thread>
#include <unistd.h>

using namespace cflib::db;

USE_LOG(LogCat::Db)

namespace {

inline bool fuzzyCompare(float a, float b)
{
    return fabsf(a - b) <= 0.00001f * fmaxf(1.0f, fmaxf(fabsf(a), fabsf(b)));
}

inline bool fuzzyCompare(double a, double b)
{
    return fabs(a - b) <= 0.000000000001 * fmax(1.0, fmax(fabs(a), fabs(b)));
}

DateTime makeUTCDateTime(int year, int month, int day, int hour, int min, int sec, int msec)
{
    struct tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = min;
    t.tm_sec  = sec;
    time_t epoch = timegm(&t);
    return DateTime::fromMSecsSinceEpoch((int64)epoch * 1000 + msec);
}

}

class SQLite_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<SQLite_test *>(this);
        return {
            {"initTestCase",                       [self]() { self->initTestCase(); }},
            {"result_test",                        [self]() { self->result_test(); }},
            {"check_datatype_boolean",             [self]() { self->check_datatype_boolean(); }},
            {"insert_test_01",                     [self]() { self->insert_test_01(); }},
            {"select_test_01",                     [self]() { self->select_test_01(); }},
            {"clean_test_01",                      [self]() { self->clean_test_01(); }},
            {"insert_test_02",                     [self]() { self->insert_test_02(); }},
            {"select_test_02",                     [self]() { self->select_test_02(); }},
            {"clean_test_02",                      [self]() { self->clean_test_02(); }},
            {"insert_test_03",                     [self]() { self->insert_test_03(); }},
            {"select_test_03",                     [self]() { self->select_test_03(); }},
            {"clean_test_03",                      [self]() { self->clean_test_03(); }},
            {"autoincrement_test",                 [self]() { self->autoincrement_test(); }},
            {"insert_prepared_test",               [self]() { self->insert_prepared_test(); }},
            {"select_prepared_insert_test",        [self]() { self->select_prepared_insert_test(); }},
            {"select_NULLs_test",                  [self]() { self->select_NULLs_test(); }},
            {"insert_special_symbols_test",        [self]() { self->insert_special_symbols_test(); }},
            {"select_special_symbols_test",        [self]() { self->select_special_symbols_test(); }},
            {"alter_table_test",                   [self]() { self->alter_table_test(); }},
            {"insert_datetime_test",               [self]() { self->insert_datetime_test(); }},
            {"select_datetime_test",               [self]() { self->select_datetime_test(); }},
            {"update_table_test",                  [self]() { self->update_table_test(); }},
            {"select_updated_table_test",          [self]() { self->select_updated_table_test(); }},
            {"cascading_transaction_test",         [self]() { self->cascading_transaction_test(); }},
            {"keepFields_test",                    [self]() { self->keepFields_test(); }},
            {"multi_prepare_test",                 [self]() { self->multi_prepare_test(); }},
            {"multi_prepare_transaction_test",     [self]() { self->multi_prepare_transaction_test(); }},
            {"cascading_multi_prepare_test",       [self]() { self->cascading_multi_prepare_test(); }},
            {"two_connections_test",               [self]() { self->two_connections_test(); }},
            {"transaction_isolation_test",         [self]() { self->transaction_isolation_test(); }},
            {"transaction_blocking_commit_test",   [self]() { self->transaction_blocking_commit_test(); }},
            {"transaction_blocking_rollback_test", [self]() { self->transaction_blocking_rollback_test(); }},
            {"transaction_failing_exec_test",      [self]() { self->transaction_failing_exec_test(); }},
            {"cleanupTestCase",                    [self]() { self->cleanupTestCase(); }},
        };
    }

private:

    struct TestTypes
    {
        uint32    id;
        uint16    x16;
        uint32    x32;
        uint64    x64;
        int16     s16;
        int32     s32;
        int64     s64;
        DateTime  t;
        ByteArray a;
        String    s;
        float     f;
        double    d;
        bool      b;
    };
    TestTypes tt;

    void initTestCase()
    {
        // Use a temp file so multiple connections can share the same database.
        // Set env var SQLITE_TEST_DB to override the path.
        TVERIFY(SQLite::setParameter("/tmp/cflib_sqlite_test.db", "SQLITE_TEST_DB"));
        SQLiteConn;

        // drop any old existing tables
        sql.exec("DROP TABLE IF EXISTS cflib_db_test");
        sql.exec("DROP TABLE IF EXISTS cflib_db_test_2");

        TVERIFY(sql.exec(
            "CREATE TABLE cflib_db_test ("
                "id INTEGER NOT NULL PRIMARY KEY,"
                "x16 INTEGER,"
                "x32 INTEGER,"
                "x64 INTEGER,"
                "t INTEGER,"
                "a BLOB,"
                "s TEXT,"
                "r REAL,"
                "d REAL,"
                "b INTEGER"
            ")"
        ));
        TVERIFY(sql.exec(
            "CREATE TABLE cflib_db_test_2 ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "x32 INTEGER"
            ")"
        ));
    }

    void cleanupTestCase()
    {
        SQLiteConn;
        TVERIFY(sql.exec("DROP TABLE cflib_db_test"));
        TVERIFY(sql.exec("DROP TABLE cflib_db_test_2"));
    }

    // -----------------------------------------------------------

    void result_test()
    {
        SQLiteConn;

        TVERIFY( sql.exec("SELECT 42"));
        TVERIFY( sql.next());
        TVERIFY(!sql.next());

        TVERIFY( sql.exec("SELECT 23"));
        TVERIFY( sql.next());

        TVERIFY(!sql.exec("SULICT 42"));
        TVERIFY(!sql.next());
    }

    // -----------------------------------------------------------

    void check_datatype_boolean()
    {
        SQLiteConn;

        bool result = false;
        TVERIFY(sql.exec("SELECT 1=1"));
        TVERIFY(sql.next());
        sql >> result;
        TVERIFY(result);

        TVERIFY(sql.exec("SELECT 0=1"));
        TVERIFY(sql.next());
        sql >> result;
        TVERIFY(!result);

        sql.prepare("SELECT ?1");
        sql << false;
        TVERIFY(sql.exec());
        TVERIFY(sql.next());
        TVERIFY(!sql.get<bool>(0));
        sql << true;
        TVERIFY(sql.exec());
        TVERIFY(sql.next());
        TVERIFY(sql.get<bool>(0));
    }

    // -----------------------------------------------------------

    void insert_test_01()
    {
        SQLiteConn;

        sql.prepare(
            "INSERT INTO cflib_db_test (id, x16, x32, x64, t, a, s, r, d, b) "
            "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)"
        );
        sql << 1
            << (uint16)2 << (uint32)0xFFFFFFFF << (uint64)4
            << makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123)
            << ByteArray("A0") << String::fromUtf8("ABC\xC3\xB6\xC3\x9F")
            << 1.23f << 3.45
            << true;
        TVERIFY(sql.exec());
    }

    void select_test_01()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

        TVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
        TCOMPARE(tt.id,  (uint32)1);
        TCOMPARE(tt.x16, (uint16)2);
        TCOMPARE(tt.x32, (uint32)0xFFFFFFFF);
        TCOMPARE(tt.x64, (uint64)4);
        TCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
        TCOMPARE(tt.a, ByteArray("A0"));
        TCOMPARE(tt.s, String::fromUtf8("ABC\xC3\xB6\xC3\x9F"));
        TVERIFY(fuzzyCompare(tt.f, 1.23f));
        TVERIFY(fuzzyCompare(tt.d, 3.45));
        TVERIFY(tt.b);
        // no further lines
        TVERIFY(!sql.next());
    }

    void clean_test_01()
    {
        // leave table empty
        SQLiteConn;
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=1"));
    }

    // -----------------------------------------------------------

    void insert_test_02()
    {
        SQLiteConn;

        sql.prepare(
            "INSERT INTO cflib_db_test (id, x16, x32, x64, t, a, s, r, d, b) "
            "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)"
        );
        sql << 1
            << (int16)-32768 << (int32)-2147483648 << (int64)-9223372036854775807LL
            << makeUTCDateTime(1970, 1, 1, 0, 0, 0, 0)
            << ByteArray("A0") << String::fromUtf8("ABC\xC3\xB6\xC3\x9F")
            << -1.23f << -3.45
            << false;
        TVERIFY(sql.exec());
    }

    void select_test_02()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

        TVERIFY(sql.next());
        sql >> tt.id >> tt.s16 >> tt.s32 >> tt.s64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
        TCOMPARE(tt.id,  1u);
        TCOMPARE(tt.s16, -32768);
        TCOMPARE(tt.s32, -2147483648l);
        TCOMPARE(tt.s64, -9223372036854775807LL);
        TCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(1970, 1, 1, 0, 0, 0, 0).toMSecsSinceEpoch());
        TCOMPARE(tt.a, ByteArray("A0"));
        TCOMPARE(tt.s, String::fromUtf8("ABC\xC3\xB6\xC3\x9F"));
        TVERIFY(fuzzyCompare(tt.f, -1.23f));
        TVERIFY(fuzzyCompare(tt.d, -3.45));
        TVERIFY(!tt.b);
        // no further lines
        TVERIFY(!sql.next());
    }

    void clean_test_02()
    {
        // leave table empty
        SQLiteConn;
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=1"));
    }

    // -----------------------------------------------------------

    void insert_test_03()
    {
        SQLiteConn;

        sql.prepare(
            "INSERT INTO cflib_db_test (id, x16, x32, x64, t, a, s, r, d) "
            "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)"
        );
        sql << 3
            << (uint16)5 << (uint32)6 << (uint64)7
            << DateTime::nowUTC()
            << ByteArray("") << String("")
            << 0.0f << -999.0;
        TVERIFY(sql.exec());
    }

    void select_test_03()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

        TVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
        TCOMPARE(tt.id,  (uint32)3);
        TCOMPARE(tt.x16, (uint16)5);
        TCOMPARE(tt.x64, (uint64)7);
        DateTime now = DateTime::nowUTC();
        TVERIFY(tt.t.toMSecsSinceEpoch() > now.toMSecsSinceEpoch() - 30000);
        TVERIFY(tt.t.toMSecsSinceEpoch() < now.toMSecsSinceEpoch() + 30000);
        TCOMPARE(tt.a, ByteArray(""));
        TCOMPARE(tt.s, String(""));
        TCOMPARE(tt.f, 0.0f);
        TVERIFY(fuzzyCompare(tt.d, -999.0));
        TVERIFY(!tt.b);
        // no further lines
        TVERIFY(!sql.next());
    }

    void clean_test_03()
    {
        // leave table empty
        SQLiteConn;
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=3"));
    }

    // -----------------------------------------------------------

    void autoincrement_test()
    {
        SQLiteConn;

        sql.prepare(
            "INSERT INTO cflib_db_test_2 (x32) VALUES (?1) RETURNING id"
        );

        sql << 42;
        TVERIFY(sql.exec());
        TVERIFY(sql.next());
        sql >> tt.id;
        TCOMPARE(tt.id, (uint32)1);

        sql << 23;
        TVERIFY(sql.exec());
        TVERIFY(sql.next());
        sql >> tt.id;
        TCOMPARE(tt.id, (uint32)2);

        TVERIFY(sql.exec("SELECT id, x32 FROM cflib_db_test_2 ORDER BY id"));
        TVERIFY(sql.next());
        sql >> tt.id >> tt.x32;
        TCOMPARE(tt.id,  (uint32)1);
        TCOMPARE(tt.x32, (uint32)42);
        TVERIFY(sql.next());
        sql >> tt.id >> tt.x32;
        TCOMPARE(tt.id,  (uint32)2);
        TCOMPARE(tt.x32, (uint32)23);
        TVERIFY(!sql.next());
    }

    // -----------------------------------------------------------

    void insert_prepared_test()
    {
        SQLiteConn;

        sql.prepare(
            "INSERT INTO cflib_db_test (id, x16, x32, x64, t, a, s, r, d, b) "
            "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10)"
        );
        sql << 3
            << (uint16)0xFFFA << 67 << 89
            << makeUTCDateTime(2017, 2, 27, 10, 47, 34, 123)
            << sql.null << "d\xC3\xB6""d\xC3\xAF""d\xC3\xBC\xC3\x9F"
            << 123.456f << 789.123
            << true;
        TVERIFY(sql.exec());
        sql << 4
            << (int8)-45 << sql.null << (int32)-89
            << sql.null
            << sql.null << sql.null
            << sql.null << sql.null
            << false;
        TVERIFY(sql.exec());
    }

    void select_prepared_insert_test()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test ORDER BY id"));

        TVERIFY(sql.next());
        TVERIFY(!sql.isNull());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64;
        TCOMPARE(tt.id,  (uint32)3);
        TCOMPARE(tt.x16, (uint16)0xFFFA);
        TCOMPARE(tt.x32, (uint32)67);
        TCOMPARE(tt.x64, (uint64)89);
        TVERIFY(!sql.isNull());
        sql >> tt.t;
        TCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 10, 47, 34, 123).toMSecsSinceEpoch());
        TVERIFY(!sql.lastFieldIsNull());
        sql >> tt.a >> tt.s >> tt.f >> tt.d;
        TCOMPARE(tt.f, 123.456f);
        TCOMPARE(tt.d, 789.123);
        sql >> tt.b;
        TVERIFY(tt.b);
    }

    // -----------------------------------------------------------

    void select_NULLs_test()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test ORDER BY id"));
        TVERIFY(sql.next());

        sql >> sql.null;    // id
        sql >> sql.null;    // x16
        sql >> sql.null;    // x32
        sql >> sql.null;    // x64
        sql >> sql.null;    // t

        TVERIFY(sql.isNull());  // a
        sql >> sql.null;
        TVERIFY(sql.lastFieldIsNull());
        TVERIFY(!sql.isNull()); // s
        sql >> tt.s;
        TVERIFY(!sql.lastFieldIsNull());
        TCOMPARE(tt.s, String::fromUtf8("d\xC3\xB6""d\xC3\xAF""d\xC3\xBC\xC3\x9F"));
        TVERIFY(!sql.isNull()); // r
        sql >> sql.null;
        TVERIFY(!sql.lastFieldIsNull());

        int16 sx16;
        int32 sx32;
        int64 sx64;

        TVERIFY(sql.next());

        sql >> tt.id >> sx16;   // id, x16
        TCOMPARE(tt.id,  (uint32)4);
        TCOMPARE(sx16, (int16)-45);
        TVERIFY(!sql.lastFieldIsNull());
        TVERIFY(sql.isNull());  // x32
        sql >> sx32;
        TVERIFY(sql.lastFieldIsNull());
        sql >> sx64;
        TCOMPARE(sx64, (int64)-89);
        TVERIFY(!sql.lastFieldIsNull());
        TVERIFY(!sql.isNull(1));    // x16
        TVERIFY(!sql.isNull(3));    // x64
        TVERIFY( sql.isNull(4));    // t

        sql >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
        TVERIFY(tt.t.isNull());
        TVERIFY(tt.a.isNull());
        TVERIFY(tt.s.isNull());
        TCOMPARE(tt.f, 0.0f);
        TCOMPARE(tt.d, 0.0);
        TVERIFY(!tt.b);
        TVERIFY(!sql.lastFieldIsNull());

        TVERIFY(!sql.next());

        // leave table empty
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=3"));
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=4"));
    }

    // -----------------------------------------------------------

    void insert_special_symbols_test()
    {
        SQLiteConn;

        sql.prepare(
            "INSERT INTO cflib_db_test (id, x16, x32, x64, t, a, s, r, d) "
            "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)"
        );
        sql << 1
            << (uint16)2 << (uint32)3 << (uint64)4
            << makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123)
            << ByteArray("\xC3\x96\xC3\x84\xC3\x9C")
            << String::fromUtf8("\xC3\x96\xC3\x84\xC3\x9C")
            << 1.23f << 3.45;
        TVERIFY(sql.exec());
    }

    void select_special_symbols_test()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));

        TVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d;
        TCOMPARE(tt.id,  (uint32)1);
        TCOMPARE(tt.x16, (uint16)2);
        TCOMPARE(tt.x32, (uint32)3);
        TCOMPARE(tt.x64, (uint64)4);
        TCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
        TCOMPARE(tt.a, ByteArray("\xC3\x96\xC3\x84\xC3\x9C"));
        TCOMPARE(tt.s, String::fromUtf8("\xC3\x96\xC3\x84\xC3\x9C"));
        TVERIFY(fuzzyCompare(tt.f, 1.23f));
        TVERIFY(fuzzyCompare(tt.d, 3.45));
        // no further lines
        TVERIFY(!sql.next());
        // leave table empty
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=1"));
    }

    // -----------------------------------------------------------

    void alter_table_test()
    {
        SQLiteConn;

        TVERIFY(sql.exec("ALTER TABLE cflib_db_test DROP COLUMN t"));
        TVERIFY(sql.exec("ALTER TABLE cflib_db_test ADD COLUMN t INTEGER"));
    }

    // -----------------------------------------------------------

    void insert_datetime_test()
    {
        SQLiteConn;

        sql.prepare(
            "INSERT INTO cflib_db_test (id, x16, x32, x64, a, s, r, d, t) "
            "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)"
        );
        sql << 1
            << (uint16)2 << (uint32)3 << (uint64)4
            << ByteArray("\xe6\xbc\xa2\xe5\xad\x97")
            << String::fromUtf8("Hello World!")
            << 1.23f << 3.45
            << makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123);
        TVERIFY(sql.exec());
    }

    void select_datetime_test()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));

        TVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.a >> tt.s >> tt.f >> tt.d >> tt.t;
        TCOMPARE(tt.id,  (uint32)1);
        TCOMPARE(tt.x16, (uint16)2);
        TCOMPARE(tt.x32, (uint32)3);
        TCOMPARE(tt.x64, (uint64)4);
        TCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
        TCOMPARE(tt.a, ByteArray("\xe6\xbc\xa2\xe5\xad\x97"));
        TCOMPARE(tt.s, String::fromUtf8("Hello World!"));
        TVERIFY(fuzzyCompare(tt.f, 1.23f));
        TVERIFY(fuzzyCompare(tt.d, 3.45));
        // no further lines
        TVERIFY(!sql.next());
    }

    // -----------------------------------------------------------

    void update_table_test()
    {
        SQLiteConn;

        sql.prepare(
            "UPDATE cflib_db_test SET "
                "id=2, x16=123, x32=345, x64=1234567, "
                "a=?1, "
                "s='Hello again', r=123.456, d=12345.6789, "
                "t=?2 "
            "WHERE id=1"
        );
        sql << ByteArray("2017-02-27T14:47:34.123Z")
            << makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123);
        TVERIFY(sql.exec());
    }

    void select_updated_table_test()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));

        TVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.a >> tt.s >> tt.f >> tt.d >> tt.t;
        TCOMPARE(tt.id,  (uint32)2);
        TCOMPARE(tt.x16, (uint16)123);
        TCOMPARE(tt.x32, (uint32)345);
        TCOMPARE(tt.x64, (uint64)1234567);
        TCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
        TCOMPARE(tt.a, ByteArray("2017-02-27T14:47:34.123Z"));
        TCOMPARE(tt.s, String::fromUtf8("Hello again"));
        TVERIFY(fuzzyCompare(tt.f, 123.456f));
        TVERIFY(fuzzyCompare(tt.d, 12345.6789));
        // no further lines
        TVERIFY(!sql.next());
    }

    // -----------------------------------------------------------

    void cascading_transaction_test()
    {
        SQLiteConn;

        TVERIFY(!sql.commit());

        sql.begin();
        TVERIFY(sql.commit());

        sql.begin();
        sql.rollback();
        TVERIFY(!sql.commit());

        sql.begin();
        {
            SQLiteConn;

            TVERIFY(!sql.commit());

            sql.begin();
            TVERIFY(sql.commit());
        }
        TVERIFY(sql.commit());

        sql.begin();
        {
            SQLiteConn;

            sql.begin();
            sql.rollback();
            TVERIFY(!sql.commit());
        }
        TVERIFY(!sql.commit());
    }

    // -----------------------------------------------------------

    void keepFields_test()
    {
        SQLiteConn;

        sql.prepare(
            "INSERT INTO cflib_db_test (id, x32) VALUES (?2, ?1)"
        );
        sql << 123 << 5;
        TVERIFY(sql.exec(1));
        sql << 6;
        TVERIFY(sql.exec());

        sql.prepare("SELECT x32 FROM cflib_db_test WHERE id = ?1");

        sql << 5;
        TVERIFY(sql.exec());
        TVERIFY(sql.next());
        sql >> tt.x32;
        TCOMPARE(tt.x32, (uint32)123);

        sql << 6;
        TVERIFY(sql.exec());
        TVERIFY(sql.next());
        sql >> tt.x32;
        TCOMPARE(tt.x32, (uint32)123);
    }

    // -----------------------------------------------------------

    void multi_prepare_test()
    {
        SQLiteConn;

        sql.prepare("INSERT INTO cflib_db_test (id) VALUES (?1)");

        SQLiteConn2;

        sql2.prepare("SELECT id FROM cflib_db_test WHERE id = ?1");

        sql << 7;
        sql2 << 6;
        TVERIFY(sql.exec());
        TVERIFY(sql2.exec());
        TVERIFY(sql2.next());
        sql2 >> tt.id;
        TCOMPARE(tt.id, (uint32)6);
        TVERIFY(!sql2.next());

        sql << 8;
        sql2 << 7;
        TVERIFY(sql.exec());
        TVERIFY(sql2.exec());
        TVERIFY(sql2.next());
        sql2 >> tt.id;
        TCOMPARE(tt.id, (uint32)7);
        TVERIFY(!sql2.next());
    }

    void multi_prepare_transaction_test()
    {
        {
            SQLiteConn;
            sql.begin();
            {
                SQLiteConn;
                sql.begin();
                sql.prepare("INSERT INTO cflib_db_test (id) VALUES (?1)");
                sql << 7;
                TVERIFY(!sql.exec());  // fails: id=7 already exists
                TVERIFY(sql.commit()); // nested commit (no-op)
            }
            TVERIFY(sql.commit());
        }
        {
            SQLiteConn;
            sql.begin();
            {
                SQLiteConn;
                sql.begin();
                sql.prepare("SELECT id FROM cflib_db_test");
                TVERIFY(sql.exec());
                TVERIFY(sql.commit());
            }
            TVERIFY(sql.commit());
        }
    }

    void cascading_multi_prepare_test()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT COUNT(*) FROM cflib_db_test"));
        TVERIFY(sql.next());
        sql >> tt.x64;
        TVERIFY(tt.x64 > (uint64)1);

        sql.prepare("SELECT id FROM cflib_db_test ORDER BY id");
        TVERIFY(sql.exec());

        SQLiteConn2;
        sql2.prepare("SELECT x32 FROM cflib_db_test WHERE id = ?1");

        TVERIFY(sql.next());
        sql >> tt.id;
        TCOMPARE(tt.id, (uint32)2);

        sql2 << tt.id;
        TVERIFY(sql2.exec());
        TVERIFY(sql2.next());
        sql2 >> tt.x32;
        TCOMPARE(tt.x32, (uint32)345);
        TVERIFY(!sql2.next());

        // Unlike PSql, SQLite allows simultaneous active statements on the same connection.
        TVERIFY(sql.next());
        sql >> tt.id;
        TCOMPARE(tt.id, (uint32)5);
    }

    void two_connections_test()
    {
        SQLiteConn;

        TVERIFY(sql.exec("SELECT COUNT(*) FROM cflib_db_test"));
        TVERIFY(sql.next());
        sql >> tt.x64;
        TVERIFY(tt.x64 > (uint64)2);

        sql.prepare("SELECT id FROM cflib_db_test ORDER BY id");
        TVERIFY(sql.exec());

        SQLite sql2;  // dedicated connection to the same file
        sql2.prepare("SELECT x32 FROM cflib_db_test WHERE id = ?1");

        TVERIFY(sql.next());
        sql >> tt.id;
        TCOMPARE(tt.id, (uint32)2);

        sql2 << tt.id;
        TVERIFY(sql2.exec());
        TVERIFY(sql2.next());
        sql2 >> tt.x32;
        TCOMPARE(tt.x32, (uint32)345);
        TVERIFY(!sql2.next());

        TVERIFY(sql.next());
        sql >> tt.id;
        TCOMPARE(tt.id, (uint32)5);

        sql2 << tt.id;
        TVERIFY(sql2.exec());
        TVERIFY(sql2.next());
        sql2 >> tt.x32;
        TCOMPARE(tt.x32, (uint32)123);
        TVERIFY(!sql2.next());

        TVERIFY(sql.next());
    }

    // -----------------------------------------------------------

    void transaction_isolation_test()
    {
        SQLiteConn;
        sql.begin();
        sql.prepare("INSERT INTO cflib_db_test (id) VALUES (?1)");
        sql << 9;
        TVERIFY(sql.exec());

        SQLite sql2;  // dedicated connection
        sql2.prepare("SELECT id FROM cflib_db_test WHERE id = ?1");
        sql2 << 9;
        TVERIFY(sql2.exec(1));
        TVERIFY(!sql2.next());  // cannot see uncommitted data from sql

        sql.commit();

        TVERIFY(sql2.exec(1));
        TVERIFY(sql2.next());   // now sees committed data
    }

    // -----------------------------------------------------------

    void transaction_blocking_commit_test()
    {
        // SQLite uses database-level write locking (not row-level like PostgreSQL).
        // With busy_timeout=5000ms set in ThreadData, the thread retries for up to 5 seconds.
        // Main holds a write lock for ~1 second, then commits (id=10 persists).
        // Thread's INSERT id=10 is then unblocked and fails with unique constraint.
        Semaphore sem;
        bool threadResult = false;

        SQLiteConn;
        sql.begin();
        TVERIFY(sql.exec("INSERT INTO cflib_db_test (id) VALUES (10)"));

        std::thread thread([&sem, &threadResult]() {
            SQLiteConn;
            // Blocks on write lock while main is in transaction; after commit, unique violation.
            threadResult = sql.exec("INSERT INTO cflib_db_test (id) VALUES (10)");
            sem.release();
        });

        usleep(1000000); // 1 second: give thread time to start and block on write lock
        TVERIFY(sql.commit());

        sem.acquire();
        TVERIFY(!threadResult);  // unique constraint violation after main committed id=10

        thread.join();

        // cleanup: id=10 was committed by main
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=10"));
    }

    void transaction_blocking_rollback_test()
    {
        // Main holds a write lock for ~1 second, then rolls back (id=12 is gone).
        // Thread's INSERT id=12 is then unblocked and succeeds.
        Semaphore sem;
        bool threadResult = false;

        SQLiteConn;
        sql.begin();
        TVERIFY(sql.exec("INSERT INTO cflib_db_test (id) VALUES (12)"));

        std::thread thread([&sem, &threadResult]() {
            SQLiteConn;
            // Blocks on write lock while main is in transaction; after rollback, succeeds.
            threadResult = sql.exec("INSERT INTO cflib_db_test (id) VALUES (12)");
            sem.release();
        });

        usleep(1000000); // 1 second
        sql.rollback();

        sem.acquire();
        TVERIFY(threadResult);  // succeeds: id=12 not present after rollback

        thread.join();

        // cleanup: id=12 was inserted by thread
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=12"));
    }

    // -----------------------------------------------------------

    void transaction_failing_exec_test()
    {
        // In SQLite, a constraint violation does NOT invalidate the transaction.
        // Subsequent execs in the same transaction continue to work normally.
        SQLiteConn;
        sql.begin();
        sql.prepare("INSERT INTO cflib_db_test (id) VALUES (?1)");
        sql << 13;
        TVERIFY(sql.exec());     // succeeds
        sql << 13;
        TVERIFY(!sql.exec());    // fails: duplicate primary key
        sql << 14;
        TVERIFY(sql.exec());     // SQLite: transaction still usable, succeeds
        TVERIFY(sql.commit());

        // cleanup
        TVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id IN (13, 14)"));
    }

};
ADD_TEST(SQLite_test)
