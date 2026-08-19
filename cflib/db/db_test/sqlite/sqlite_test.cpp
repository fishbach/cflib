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

// scratch buffer for reading rows back
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

}

TEST_SUITE("SQLite") {

TEST_CASE("SQLite: initTestCase")
{
    // Use a temp file so multiple connections can share the same database.
    // Set env var SQLITE_TEST_DB to override the path.
    REQUIRE(SQLite::setParameter("/tmp/cflib_sqlite_test.db", "SQLITE_TEST_DB"));
    SQLiteConn;

    // drop any old existing tables
    sql.exec("DROP TABLE IF EXISTS cflib_db_test");
    sql.exec("DROP TABLE IF EXISTS cflib_db_test_2");

    REQUIRE(sql.exec(
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
    REQUIRE(sql.exec(
        "CREATE TABLE cflib_db_test_2 ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "x32 INTEGER"
        ")"
    ));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: result")
{
    SQLiteConn;

    REQUIRE( sql.exec("SELECT 42"));
    REQUIRE( sql.next());
    REQUIRE(!sql.next());

    REQUIRE( sql.exec("SELECT 23"));
    REQUIRE( sql.next());

    REQUIRE(!sql.exec("SULICT 42"));
    REQUIRE(!sql.next());
}

// -----------------------------------------------------------

TEST_CASE("SQLite: check_datatype_boolean")
{
    SQLiteConn;

    bool result = false;
    REQUIRE(sql.exec("SELECT 1=1"));
    REQUIRE(sql.next());
    sql >> result;
    REQUIRE(result);

    REQUIRE(sql.exec("SELECT 0=1"));
    REQUIRE(sql.next());
    sql >> result;
    REQUIRE(!result);

    sql.prepare("SELECT ?1");
    sql << false;
    REQUIRE(sql.exec());
    REQUIRE(sql.next());
    REQUIRE(!sql.get<bool>(0));
    sql << true;
    REQUIRE(sql.exec());
    REQUIRE(sql.next());
    REQUIRE(sql.get<bool>(0));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: insert_test_01")
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
    REQUIRE(sql.exec());
}

TEST_CASE("SQLite: select_test_01")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

    REQUIRE(sql.next());
    sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
    REQUIRE_EQ(tt.id,  (uint32)1);
    REQUIRE_EQ(tt.x16, (uint16)2);
    REQUIRE_EQ(tt.x32, (uint32)0xFFFFFFFF);
    REQUIRE_EQ(tt.x64, (uint64)4);
    REQUIRE_EQ(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
    REQUIRE_EQ(tt.a, ByteArray("A0"));
    REQUIRE_EQ(tt.s, String::fromUtf8("ABC\xC3\xB6\xC3\x9F"));
    REQUIRE(fuzzyCompare(tt.f, 1.23f));
    REQUIRE(fuzzyCompare(tt.d, 3.45));
    REQUIRE(tt.b);
    // no further lines
    REQUIRE(!sql.next());
}

TEST_CASE("SQLite: clean_test_01")
{
    // leave table empty
    SQLiteConn;
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id=1"));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: insert_test_02")
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
    REQUIRE(sql.exec());
}

TEST_CASE("SQLite: select_test_02")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

    REQUIRE(sql.next());
    sql >> tt.id >> tt.s16 >> tt.s32 >> tt.s64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
    REQUIRE_EQ(tt.id,  1u);
    REQUIRE_EQ(tt.s16, -32768);
    REQUIRE_EQ(tt.s32, -2147483648l);
    REQUIRE_EQ(tt.s64, -9223372036854775807LL);
    REQUIRE_EQ(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(1970, 1, 1, 0, 0, 0, 0).toMSecsSinceEpoch());
    REQUIRE_EQ(tt.a, ByteArray("A0"));
    REQUIRE_EQ(tt.s, String::fromUtf8("ABC\xC3\xB6\xC3\x9F"));
    REQUIRE(fuzzyCompare(tt.f, -1.23f));
    REQUIRE(fuzzyCompare(tt.d, -3.45));
    REQUIRE(!tt.b);
    // no further lines
    REQUIRE(!sql.next());
}

TEST_CASE("SQLite: clean_test_02")
{
    // leave table empty
    SQLiteConn;
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id=1"));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: insert_test_03")
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
    REQUIRE(sql.exec());
}

TEST_CASE("SQLite: select_test_03")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

    REQUIRE(sql.next());
    sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
    REQUIRE_EQ(tt.id,  (uint32)3);
    REQUIRE_EQ(tt.x16, (uint16)5);
    REQUIRE_EQ(tt.x64, (uint64)7);
    DateTime now = DateTime::nowUTC();
    REQUIRE(tt.t.toMSecsSinceEpoch() > now.toMSecsSinceEpoch() - 30000);
    REQUIRE(tt.t.toMSecsSinceEpoch() < now.toMSecsSinceEpoch() + 30000);
    REQUIRE_EQ(tt.a, ByteArray(""));
    REQUIRE_EQ(tt.s, String(""));
    REQUIRE_EQ(tt.f, 0.0f);
    REQUIRE(fuzzyCompare(tt.d, -999.0));
    REQUIRE(!tt.b);
    // no further lines
    REQUIRE(!sql.next());
}

TEST_CASE("SQLite: clean_test_03")
{
    // leave table empty
    SQLiteConn;
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id=3"));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: autoincrement")
{
    SQLiteConn;

    sql.prepare(
        "INSERT INTO cflib_db_test_2 (x32) VALUES (?1) RETURNING id"
    );

    sql << 42;
    REQUIRE(sql.exec());
    REQUIRE(sql.next());
    sql >> tt.id;
    REQUIRE_EQ(tt.id, (uint32)1);

    sql << 23;
    REQUIRE(sql.exec());
    REQUIRE(sql.next());
    sql >> tt.id;
    REQUIRE_EQ(tt.id, (uint32)2);

    REQUIRE(sql.exec("SELECT id, x32 FROM cflib_db_test_2 ORDER BY id"));
    REQUIRE(sql.next());
    sql >> tt.id >> tt.x32;
    REQUIRE_EQ(tt.id,  (uint32)1);
    REQUIRE_EQ(tt.x32, (uint32)42);
    REQUIRE(sql.next());
    sql >> tt.id >> tt.x32;
    REQUIRE_EQ(tt.id,  (uint32)2);
    REQUIRE_EQ(tt.x32, (uint32)23);
    REQUIRE(!sql.next());
}

// -----------------------------------------------------------

TEST_CASE("SQLite: insert_prepared")
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
    REQUIRE(sql.exec());
    sql << 4
        << (int8)-45 << sql.null << (int32)-89
        << sql.null
        << sql.null << sql.null
        << sql.null << sql.null
        << false;
    REQUIRE(sql.exec());
}

TEST_CASE("SQLite: select_prepared_insert")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test ORDER BY id"));

    REQUIRE(sql.next());
    REQUIRE(!sql.isNull());
    sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64;
    REQUIRE_EQ(tt.id,  (uint32)3);
    REQUIRE_EQ(tt.x16, (uint16)0xFFFA);
    REQUIRE_EQ(tt.x32, (uint32)67);
    REQUIRE_EQ(tt.x64, (uint64)89);
    REQUIRE(!sql.isNull());
    sql >> tt.t;
    REQUIRE_EQ(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 10, 47, 34, 123).toMSecsSinceEpoch());
    REQUIRE(!sql.lastFieldIsNull());
    sql >> tt.a >> tt.s >> tt.f >> tt.d;
    REQUIRE_EQ(tt.f, 123.456f);
    REQUIRE_EQ(tt.d, 789.123);
    sql >> tt.b;
    REQUIRE(tt.b);
}

// -----------------------------------------------------------

TEST_CASE("SQLite: select_NULLs")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test ORDER BY id"));
    REQUIRE(sql.next());

    sql >> sql.null;    // id
    sql >> sql.null;    // x16
    sql >> sql.null;    // x32
    sql >> sql.null;    // x64
    sql >> sql.null;    // t

    REQUIRE(sql.isNull());  // a
    sql >> sql.null;
    REQUIRE(sql.lastFieldIsNull());
    REQUIRE(!sql.isNull()); // s
    sql >> tt.s;
    REQUIRE(!sql.lastFieldIsNull());
    REQUIRE_EQ(tt.s, String::fromUtf8("d\xC3\xB6""d\xC3\xAF""d\xC3\xBC\xC3\x9F"));
    REQUIRE(!sql.isNull()); // r
    sql >> sql.null;
    REQUIRE(!sql.lastFieldIsNull());

    int16 sx16;
    int32 sx32;
    int64 sx64;

    REQUIRE(sql.next());

    sql >> tt.id >> sx16;   // id, x16
    REQUIRE_EQ(tt.id,  (uint32)4);
    REQUIRE_EQ(sx16, (int16)-45);
    REQUIRE(!sql.lastFieldIsNull());
    REQUIRE(sql.isNull());  // x32
    sql >> sx32;
    REQUIRE(sql.lastFieldIsNull());
    sql >> sx64;
    REQUIRE_EQ(sx64, (int64)-89);
    REQUIRE(!sql.lastFieldIsNull());
    REQUIRE(!sql.isNull(1));    // x16
    REQUIRE(!sql.isNull(3));    // x64
    REQUIRE( sql.isNull(4));    // t

    sql >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
    REQUIRE(tt.t.isNull());
    REQUIRE(tt.a.isNull());
    REQUIRE(tt.s.isNull());
    REQUIRE_EQ(tt.f, 0.0f);
    REQUIRE_EQ(tt.d, 0.0);
    REQUIRE(!tt.b);
    REQUIRE(!sql.lastFieldIsNull());

    REQUIRE(!sql.next());

    // leave table empty
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id=3"));
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id=4"));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: insert_special_symbols")
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
    REQUIRE(sql.exec());
}

TEST_CASE("SQLite: select_special_symbols")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));

    REQUIRE(sql.next());
    sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d;
    REQUIRE_EQ(tt.id,  (uint32)1);
    REQUIRE_EQ(tt.x16, (uint16)2);
    REQUIRE_EQ(tt.x32, (uint32)3);
    REQUIRE_EQ(tt.x64, (uint64)4);
    REQUIRE_EQ(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
    REQUIRE_EQ(tt.a, ByteArray("\xC3\x96\xC3\x84\xC3\x9C"));
    REQUIRE_EQ(tt.s, String::fromUtf8("\xC3\x96\xC3\x84\xC3\x9C"));
    REQUIRE(fuzzyCompare(tt.f, 1.23f));
    REQUIRE(fuzzyCompare(tt.d, 3.45));
    // no further lines
    REQUIRE(!sql.next());
    // leave table empty
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id=1"));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: alter_table")
{
    SQLiteConn;

    REQUIRE(sql.exec("ALTER TABLE cflib_db_test DROP COLUMN t"));
    REQUIRE(sql.exec("ALTER TABLE cflib_db_test ADD COLUMN t INTEGER"));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: insert_datetime")
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
    REQUIRE(sql.exec());
}

TEST_CASE("SQLite: select_datetime")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));

    REQUIRE(sql.next());
    sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.a >> tt.s >> tt.f >> tt.d >> tt.t;
    REQUIRE_EQ(tt.id,  (uint32)1);
    REQUIRE_EQ(tt.x16, (uint16)2);
    REQUIRE_EQ(tt.x32, (uint32)3);
    REQUIRE_EQ(tt.x64, (uint64)4);
    REQUIRE_EQ(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
    REQUIRE_EQ(tt.a, ByteArray("\xe6\xbc\xa2\xe5\xad\x97"));
    REQUIRE_EQ(tt.s, String::fromUtf8("Hello World!"));
    REQUIRE(fuzzyCompare(tt.f, 1.23f));
    REQUIRE(fuzzyCompare(tt.d, 3.45));
    // no further lines
    REQUIRE(!sql.next());
}

// -----------------------------------------------------------

TEST_CASE("SQLite: update_table")
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
    REQUIRE(sql.exec());
}

TEST_CASE("SQLite: select_updated_table")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));

    REQUIRE(sql.next());
    sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.a >> tt.s >> tt.f >> tt.d >> tt.t;
    REQUIRE_EQ(tt.id,  (uint32)2);
    REQUIRE_EQ(tt.x16, (uint16)123);
    REQUIRE_EQ(tt.x32, (uint32)345);
    REQUIRE_EQ(tt.x64, (uint64)1234567);
    REQUIRE_EQ(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
    REQUIRE_EQ(tt.a, ByteArray("2017-02-27T14:47:34.123Z"));
    REQUIRE_EQ(tt.s, String::fromUtf8("Hello again"));
    REQUIRE(fuzzyCompare(tt.f, 123.456f));
    REQUIRE(fuzzyCompare(tt.d, 12345.6789));
    // no further lines
    REQUIRE(!sql.next());
}

// -----------------------------------------------------------

TEST_CASE("SQLite: cascading_transaction")
{
    SQLiteConn;

    REQUIRE(!sql.commit());

    sql.begin();
    REQUIRE(sql.commit());

    sql.begin();
    sql.rollback();
    REQUIRE(!sql.commit());

    sql.begin();
    {
        SQLiteConn;

        REQUIRE(!sql.commit());

        sql.begin();
        REQUIRE(sql.commit());
    }
    REQUIRE(sql.commit());

    sql.begin();
    {
        SQLiteConn;

        sql.begin();
        sql.rollback();
        REQUIRE(!sql.commit());
    }
    REQUIRE(!sql.commit());
}

// -----------------------------------------------------------

TEST_CASE("SQLite: keepFields")
{
    SQLiteConn;

    sql.prepare(
        "INSERT INTO cflib_db_test (id, x32) VALUES (?2, ?1)"
    );
    sql << 123 << 5;
    REQUIRE(sql.exec(1));
    sql << 6;
    REQUIRE(sql.exec());

    sql.prepare("SELECT x32 FROM cflib_db_test WHERE id = ?1");

    sql << 5;
    REQUIRE(sql.exec());
    REQUIRE(sql.next());
    sql >> tt.x32;
    REQUIRE_EQ(tt.x32, (uint32)123);

    sql << 6;
    REQUIRE(sql.exec());
    REQUIRE(sql.next());
    sql >> tt.x32;
    REQUIRE_EQ(tt.x32, (uint32)123);
}

// -----------------------------------------------------------

TEST_CASE("SQLite: multi_prepare")
{
    SQLiteConn;

    sql.prepare("INSERT INTO cflib_db_test (id) VALUES (?1)");

    SQLiteConn2;

    sql2.prepare("SELECT id FROM cflib_db_test WHERE id = ?1");

    sql << 7;
    sql2 << 6;
    REQUIRE(sql.exec());
    REQUIRE(sql2.exec());
    REQUIRE(sql2.next());
    sql2 >> tt.id;
    REQUIRE_EQ(tt.id, (uint32)6);
    REQUIRE(!sql2.next());

    sql << 8;
    sql2 << 7;
    REQUIRE(sql.exec());
    REQUIRE(sql2.exec());
    REQUIRE(sql2.next());
    sql2 >> tt.id;
    REQUIRE_EQ(tt.id, (uint32)7);
    REQUIRE(!sql2.next());
}

TEST_CASE("SQLite: multi_prepare_transaction")
{
    {
        SQLiteConn;
        sql.begin();
        {
            SQLiteConn;
            sql.begin();
            sql.prepare("INSERT INTO cflib_db_test (id) VALUES (?1)");
            sql << 7;
            REQUIRE(!sql.exec());  // fails: id=7 already exists
            REQUIRE(sql.commit()); // nested commit (no-op)
        }
        REQUIRE(sql.commit());
    }
    {
        SQLiteConn;
        sql.begin();
        {
            SQLiteConn;
            sql.begin();
            sql.prepare("SELECT id FROM cflib_db_test");
            REQUIRE(sql.exec());
            REQUIRE(sql.commit());
        }
        REQUIRE(sql.commit());
    }
}

TEST_CASE("SQLite: cascading_multi_prepare")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT COUNT(*) FROM cflib_db_test"));
    REQUIRE(sql.next());
    sql >> tt.x64;
    REQUIRE(tt.x64 > (uint64)1);

    sql.prepare("SELECT id FROM cflib_db_test ORDER BY id");
    REQUIRE(sql.exec());

    SQLiteConn2;
    sql2.prepare("SELECT x32 FROM cflib_db_test WHERE id = ?1");

    REQUIRE(sql.next());
    sql >> tt.id;
    REQUIRE_EQ(tt.id, (uint32)2);

    sql2 << tt.id;
    REQUIRE(sql2.exec());
    REQUIRE(sql2.next());
    sql2 >> tt.x32;
    REQUIRE_EQ(tt.x32, (uint32)345);
    REQUIRE(!sql2.next());

    // Unlike PSql, SQLite allows simultaneous active statements on the same connection.
    REQUIRE(sql.next());
    sql >> tt.id;
    REQUIRE_EQ(tt.id, (uint32)5);
}

TEST_CASE("SQLite: two_connections")
{
    SQLiteConn;

    REQUIRE(sql.exec("SELECT COUNT(*) FROM cflib_db_test"));
    REQUIRE(sql.next());
    sql >> tt.x64;
    REQUIRE(tt.x64 > (uint64)2);

    sql.prepare("SELECT id FROM cflib_db_test ORDER BY id");
    REQUIRE(sql.exec());

    SQLite sql2;  // dedicated connection to the same file
    sql2.prepare("SELECT x32 FROM cflib_db_test WHERE id = ?1");

    REQUIRE(sql.next());
    sql >> tt.id;
    REQUIRE_EQ(tt.id, (uint32)2);

    sql2 << tt.id;
    REQUIRE(sql2.exec());
    REQUIRE(sql2.next());
    sql2 >> tt.x32;
    REQUIRE_EQ(tt.x32, (uint32)345);
    REQUIRE(!sql2.next());

    REQUIRE(sql.next());
    sql >> tt.id;
    REQUIRE_EQ(tt.id, (uint32)5);

    sql2 << tt.id;
    REQUIRE(sql2.exec());
    REQUIRE(sql2.next());
    sql2 >> tt.x32;
    REQUIRE_EQ(tt.x32, (uint32)123);
    REQUIRE(!sql2.next());

    REQUIRE(sql.next());
}

// -----------------------------------------------------------

TEST_CASE("SQLite: transaction_isolation")
{
    SQLiteConn;
    sql.begin();
    sql.prepare("INSERT INTO cflib_db_test (id) VALUES (?1)");
    sql << 9;
    REQUIRE(sql.exec());

    SQLite sql2;  // dedicated connection
    sql2.prepare("SELECT id FROM cflib_db_test WHERE id = ?1");
    sql2 << 9;
    REQUIRE(sql2.exec(1));
    REQUIRE(!sql2.next());  // cannot see uncommitted data from sql

    sql.commit();

    REQUIRE(sql2.exec(1));
    REQUIRE(sql2.next());   // now sees committed data
}

// -----------------------------------------------------------

TEST_CASE("SQLite: transaction_blocking_commit")
{
    // SQLite uses database-level write locking (not row-level like PostgreSQL).
    // With busy_timeout=5000ms set in ThreadData, the thread retries for up to 5 seconds.
    // Main holds a write lock for ~1 second, then commits (id=10 persists).
    // Thread's INSERT id=10 is then unblocked and fails with unique constraint.
    Semaphore sem;
    bool threadResult = false;

    SQLiteConn;
    sql.begin();
    REQUIRE(sql.exec("INSERT INTO cflib_db_test (id) VALUES (10)"));

    std::thread thread([&sem, &threadResult]() {
        {
            SQLiteConn;
            // Blocks on write lock while main is in transaction; after commit, unique violation.
            threadResult = sql.exec("INSERT INTO cflib_db_test (id) VALUES (10)");
            sem.release();
        }
        SQLite::closeThreadConnection();
    });

    usleep(1000000); // 1 second: give thread time to start and block on write lock
    REQUIRE(sql.commit());

    sem.acquire();
    REQUIRE(!threadResult);  // unique constraint violation after main committed id=10

    thread.join();

    // cleanup: id=10 was committed by main
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id=10"));
}

TEST_CASE("SQLite: transaction_blocking_rollback")
{
    // Main holds a write lock for ~1 second, then rolls back (id=12 is gone).
    // Thread's INSERT id=12 is then unblocked and succeeds.
    Semaphore sem;
    bool threadResult = false;

    SQLiteConn;
    sql.begin();
    REQUIRE(sql.exec("INSERT INTO cflib_db_test (id) VALUES (12)"));

    std::thread thread([&sem, &threadResult]() {
        {
            SQLiteConn;
            // Blocks on write lock while main is in transaction; after rollback, succeeds.
            threadResult = sql.exec("INSERT INTO cflib_db_test (id) VALUES (12)");
            sem.release();
        }
        SQLite::closeThreadConnection();
    });

    usleep(1000000); // 1 second
    sql.rollback();

    sem.acquire();
    REQUIRE(threadResult);  // succeeds: id=12 not present after rollback

    thread.join();

    // cleanup: id=12 was inserted by thread
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id=12"));
}

// -----------------------------------------------------------

TEST_CASE("SQLite: transaction_failing_exec")
{
    // In SQLite, a constraint violation does NOT invalidate the transaction.
    // Subsequent execs in the same transaction continue to work normally.
    SQLiteConn;
    sql.begin();
    sql.prepare("INSERT INTO cflib_db_test (id) VALUES (?1)");
    sql << 13;
    REQUIRE(sql.exec());     // succeeds
    sql << 13;
    REQUIRE(!sql.exec());    // fails: duplicate primary key
    sql << 14;
    REQUIRE(sql.exec());     // SQLite: transaction still usable, succeeds
    REQUIRE(sql.commit());

    // cleanup
    REQUIRE(sql.exec("DELETE FROM cflib_db_test WHERE id IN (13, 14)"));
}

TEST_CASE("SQLite: cleanupTestCase")
{
    SQLiteConn;
    REQUIRE(sql.exec("DROP TABLE cflib_db_test"));
    REQUIRE(sql.exec("DROP TABLE cflib_db_test_2"));
}

}
