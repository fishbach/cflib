/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/db/psql/psql.h>
#include <cflib/util/test.h>
#include <cflib/util/util.h>

#include <cflib/base/cfconcurrent.h>
#include <cflib/base/cfthread.h>

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

CFDateTime makeUTCDateTime(int year, int month, int day, int hour, int min, int sec, int msec)
{
    struct tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = min;
    t.tm_sec  = sec;
    time_t epoch = timegm(&t);
    return CFDateTime::fromMSecsSinceEpoch((cfint64)epoch * 1000 + msec);
}

}

class PSql_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<PSql_test *>(this);
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
            {"serial_test",                        [self]() { self->serial_test(); }},
            {"bigserial_test",                     [self]() { self->bigserial_test(); }},
            {"insert_prepared_test",               [self]() { self->insert_prepared_test(); }},
            {"select_prepared_insert_test",        [self]() { self->select_prepared_insert_test(); }},
            {"select_NULLs_test",                  [self]() { self->select_NULLs_test(); }},
            {"insert_special_symbols_test",        [self]() { self->insert_special_symbols_test(); }},
            {"select_special_symbols_test",        [self]() { self->select_special_symbols_test(); }},
            {"alter_table_test",                   [self]() { self->alter_table_test(); }},
            {"insert_time_with_time_zone_test",    [self]() { self->insert_time_with_time_zone_test(); }},
            {"select_time_with_time_zone_test",    [self]() { self->select_time_with_time_zone_test(); }},
            {"update_table_test",                  [self]() { self->update_table_test(); }},
            {"select_updated_table_test",          [self]() { self->select_updated_table_test(); }},
            {"select_error_test",                  [self]() { self->select_error_test(); }},
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
        cfuint32    id;
        cfuint16    x16;
        cfuint32    x32;
        cfuint64    x64;
        cfint16     s16;
        cfint32     s32;
        cfint64     s64;
        CFDateTime  t;
        CFByteArray a;
        CFString    s;
        float       f;
        double      d;
        bool        b;
    };
    TestTypes tt;

    void initTestCase()
    {
        QVERIFY(PSql::setParameter("host=127.0.0.1", "DB_TEST_DB"));
        PSqlConn;

        // drop any old exisiting table
        sql.exec("DROP TABLE cflib_db_test");
        sql.exec("DROP TABLE cflib_db_test_2");
        sql.exec("DROP TABLE cflib_db_test_3");

        QVERIFY(sql.exec(MultiLineStr(
            CREATE TABLE cflib_db_test (
                id integer NOT NULL,
                x16 smallint,
                x32 integer,
                x64 bigint,
                t timestamp with time zone,
                a bytea,
                s text,
                r real,
                d double precision,
                b boolean,
                PRIMARY KEY (id)
            )
        )));
        QVERIFY(sql.exec(
            "CREATE TABLE cflib_db_test_2 ("
                "id serial NOT NULL, "
                "x32 integer, "
                "PRIMARY KEY (id)"
            ")"));
        QVERIFY(sql.exec(
            "CREATE TABLE cflib_db_test_3 ("
                "id bigserial NOT NULL, "
                "x32 integer, "
                "PRIMARY KEY (id)"
            ")"));
    }

    void cleanupTestCase()
    {
        PSqlConn;
        QVERIFY(sql.exec("DROP TABLE cflib_db_test"));
        QVERIFY(sql.exec("DROP TABLE cflib_db_test_2"));
        QVERIFY(sql.exec("DROP TABLE cflib_db_test_3"));
    }

    // -----------------------------------------------------------

    void result_test()
    {
        PSqlConn;

        QVERIFY( sql.exec("SELECT 42"));
        QVERIFY( sql.next());
        QVERIFY(!sql.next());

        QVERIFY( sql.exec("SELECT 23;"));
        QVERIFY( sql.next());

        QVERIFY(!sql.exec("SULICT 42"));
        QVERIFY(!sql.next());
    }

    // -----------------------------------------------------------

    void check_datatype_boolean()
    {
        PSqlConn;

        bool result = false;
        QVERIFY(sql.exec("SELECT 1=1"));
        QVERIFY(sql.next());
        sql >> result;
        QVERIFY(result);

        QVERIFY(sql.exec("SELECT 0=1"));
        QVERIFY(sql.next());
        sql >> result;
        QVERIFY(!result);

        sql.prepare("SELECT $1");
        sql << false;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        QVERIFY(!sql.get<bool>(0));
        sql << true;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        QVERIFY(sql.get<bool>(0));

        sql.prepare("SELECT pg_typeof($1) = pg_typeof(False)");
        sql << true;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        sql >> result;
        QVERIFY(result);
    }

    // -----------------------------------------------------------
    void insert_test_01()
    {
        PSqlConn;

        QVERIFY(sql.exec(MultiLineStr(
            INSERT INTO
                cflib_db_test
            (
                id, x16, x32, x64, t, a, s, r, d, b
            ) VALUES (
                1, 2, -1, 4, '2017-02-27T14:47:34.123Z', E'\x41\x30', E'ABC\xC3\xB6\xC3\x9F', 1.23, 3.45, TRUE
            )
        )));
    }

    void select_test_01()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

        QVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
        QCOMPARE(tt.id,  (cfuint32)1);
        QCOMPARE(tt.x16, (cfuint16)2);
        QCOMPARE(tt.x32, (cfuint32)0xFFFFFFFF);
        QCOMPARE(tt.x64, (cfuint64)4);
        QCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
        QCOMPARE(tt.a, CFByteArray("A0"));
        QCOMPARE(tt.s, CFString::fromUtf8("ABC\xC3\xB6\xC3\x9F"));
        QVERIFY(fuzzyCompare(tt.f, 1.23f));
        QVERIFY(fuzzyCompare(tt.d, 3.45));
        QVERIFY(tt.b);
        // no futher lines
        QVERIFY(!sql.next());
    }

    void clean_test_01()
    {
        // leave table empty
        PSqlConn;
        QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=1;"));
    }

    // -----------------------------------------------------------

    void insert_test_02()
    {
        PSqlConn;

        QVERIFY(sql.exec(MultiLineStr(
            INSERT INTO
                cflib_db_test
            (
                id,
                x16, x32, x64,
                t, a, s, r, d, b
            ) VALUES (
                1,
                -32768, -2147483648, -9223372036854775807,
                '1970-01-01T00:00:00.000Z', E'\x41\x30', E'ABC\xC3\xB6\xC3\x9F', -1.23, -3.45, FALSE
            )
        )));
    }

    void select_test_02()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

        QVERIFY(sql.next());
        sql >> tt.id >> tt.s16 >> tt.s32 >> tt.s64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
        QCOMPARE(tt.id,  (cfint32)1);
        QCOMPARE(tt.s16, (cfint16)-32768);
        QCOMPARE(tt.s32, (cfint32)-2147483648l);
        QCOMPARE(tt.s64, (cfint64)(-9223372036854775807LL));
        QCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(1970, 1, 1, 0, 0, 0, 0).toMSecsSinceEpoch());
        QCOMPARE(tt.a, CFByteArray("A0"));
        QCOMPARE(tt.s, CFString::fromUtf8("ABC\xC3\xB6\xC3\x9F"));
        QVERIFY(fuzzyCompare(tt.f, -1.23f));
        QVERIFY(fuzzyCompare(tt.d, -3.45));
        QVERIFY(!tt.b);
        // no futher lines
        QVERIFY(!sql.next());
    }

    void clean_test_02()
    {
        // leave table empty
        PSqlConn;
        QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=1;"));
    }

    // -----------------------------------------------------------

    void insert_test_03()
    {
        PSqlConn;
        QVERIFY(sql.exec(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id, x16, x32, x64, t, a, s, r, d"
            ") VALUES ("
                "3, 5, 6, 7, CURRENT_TIMESTAMP, '', '', 0, 'NaN'"
            ")"
        ));
    }

    void select_test_03()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

        QVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
        QCOMPARE(tt.id,  (cfuint32)3);
        QCOMPARE(tt.x16, (cfuint16)5);
        QCOMPARE(tt.x64, (cfuint64)7);
        CFDateTime now = CFDateTime::nowUTC();
        QVERIFY(tt.t.toMSecsSinceEpoch() > now.toMSecsSinceEpoch() - 30000);
        QVERIFY(tt.t.toMSecsSinceEpoch() < now.toMSecsSinceEpoch() + 30000);
        QCOMPARE(tt.a, CFByteArray(""));
        QCOMPARE(tt.s, CFString(""));
        QCOMPARE(tt.f, 0.0f);
        QVERIFY(std::isnan(tt.d));
        QVERIFY(!tt.b);
        // no further lines
        QVERIFY(!sql.next());
    }

    void clean_test_03()
    {
        // leave table empty
        PSqlConn;
        QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=3;"));
    }

    // -----------------------------------------------------------

    void serial_test()
    {
        PSqlConn;

        sql.prepare(
            "INSERT INTO "
                "cflib_db_test_2 "
            "("
                "x32"
            ") VALUES ("
                "$1"
            ") RETURNING id"
        );

        sql << 42;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        sql >> tt.id;
        QCOMPARE(tt.id,  (cfuint32)1);

        sql << 23;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        sql >> tt.id;
        QCOMPARE(tt.id,  (cfuint32)2);

        QVERIFY(sql.exec("SELECT id, x32 FROM cflib_db_test_2"));
        QVERIFY(sql.next());
        sql >> tt.id >> tt.x32;
        QCOMPARE(tt.id,  (cfuint32)1);
        QCOMPARE(tt.x32, (cfuint32)42);
        QVERIFY(sql.next());
        sql >> tt.id >> tt.x32;
        QCOMPARE(tt.id,  (cfuint32)2);
        QCOMPARE(tt.x32, (cfuint32)23);
        QVERIFY(!sql.next());
    }

    void bigserial_test()
    {
        PSqlConn;

        sql.prepare(
            "INSERT INTO "
                "cflib_db_test_3 "
            "("
                "x32"
            ") VALUES ("
                "$1"
            ") RETURNING id"
        );

        sql << 456;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        sql >> tt.x64;
        QCOMPARE(tt.x64,  (cfuint64)1);

        sql << 765;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        sql >> tt.x64;
        QCOMPARE(tt.x64,  (cfuint64)2);

        QVERIFY(sql.exec("SELECT id, x32 FROM cflib_db_test_3"));
        QVERIFY(sql.next());
        sql >> tt.x64 >> tt.x32;
        QCOMPARE(tt.x64,  (cfuint64)1);
        QCOMPARE(tt.x32, (cfuint32)456);
        QVERIFY(sql.next());
        sql >> tt.x64 >> tt.x32;
        QCOMPARE(tt.x64,  (cfuint64)2);
        QCOMPARE(tt.x32, (cfuint32)765);
        QVERIFY(!sql.next());
    }

    // -----------------------------------------------------------

    void insert_prepared_test()
    {
        PSqlConn;

        sql.prepare(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id, x16, x32, x64, t, a, s, r, d, b"
            ") VALUES ("
                "$1, $2, $3, $4, $5, $6, $7, $8, $9, $10"
            ")"
        );
        sql << 3
            << (cfuint16)0xFFFA << 67 << 89
            << makeUTCDateTime(2017, 2, 27, 10, 47, 34, 123)
            << sql.null << "d\xC3\xB6""d\xC3\xAF""d\xC3\xBC\xC3\x9F"
            << 123.456f << 789.123
            << true;
        QVERIFY(sql.exec());
        sql << 4
            << (cfint8)-45 << sql.null << (cfint32)-89
            << sql.null
            << sql.null << sql.null
            << sql.null << sql.null
            << false;
        QVERIFY(sql.exec());
    }

    void select_prepared_insert_test()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));

        QVERIFY(sql.next());
        QVERIFY(!sql.isNull());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64;
        QCOMPARE(tt.id,  (cfuint32)3);
        QCOMPARE(tt.x16, (cfuint16)0xFFFA);
        QCOMPARE(tt.x32, (cfuint32)67);
        QCOMPARE(tt.x64, (cfuint64)89);
        QVERIFY(!sql.isNull());
        sql >> tt.t;
        QCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 10, 47, 34, 123).toMSecsSinceEpoch());
        QVERIFY(!sql.lastFieldIsNull());
        sql >> tt.a >> tt.s >> tt.f >> tt.d;
        QCOMPARE(tt.f, 123.456f);
        QCOMPARE(tt.d, 789.123);
        sql >> tt.b;
        QVERIFY(tt.b);
    }

    // -----------------------------------------------------------

    void select_NULLs_test()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d, b FROM cflib_db_test"));
        QVERIFY(sql.next());

        sql >> sql.null;    // id
        sql >> sql.null;    // x16
        sql >> sql.null;    // x32
        sql >> sql.null;    // x64
        sql >> sql.null;    // t

        QVERIFY(sql.isNull());  // a
        sql >> sql.null;
        QVERIFY(sql.lastFieldIsNull());
        QVERIFY(!sql.isNull()); // s
        sql >> tt.s;
        QVERIFY(!sql.lastFieldIsNull());
        QCOMPARE(tt.s, CFString::fromUtf8("d\xC3\xB6""d\xC3\xAF""d\xC3\xBC\xC3\x9F"));
        QVERIFY(!sql.isNull()); // r
        sql >> sql.null;
        QVERIFY(!sql.lastFieldIsNull());

        cfint16 sx16;
        cfint32 sx32;
        cfint64 sx64;

        QVERIFY(sql.next());

        sql >> tt.id >> sx16;   // id, x16
        QCOMPARE(tt.id,  (cfuint32)4);
        QCOMPARE(sx16, (cfint16)-45);
        QVERIFY(!sql.lastFieldIsNull());
        QVERIFY(sql.isNull());  // x32
        sql >> sx32;
        QVERIFY(sql.lastFieldIsNull());
        sql >> sx64;
        QCOMPARE(sx64, (cfint64)-89);
        QVERIFY(!sql.lastFieldIsNull());
        QVERIFY(!sql.isNull(1));    // x16
        QVERIFY(!sql.isNull(3));    // x64
        QVERIFY( sql.isNull(4));    // t

        sql >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d >> tt.b;
        QVERIFY(tt.t.isNull());
        QVERIFY(tt.a.isNull());
        QVERIFY(tt.s.isNull());
        QCOMPARE(tt.f, 0.0f);
        QCOMPARE(tt.d, 0.0);
        QVERIFY(!tt.b);
        QVERIFY(!sql.lastFieldIsNull());

        QVERIFY(!sql.next());

        // leave table empty
        QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=3;"));
        QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=4;"));
    }

    // -----------------------------------------------------------

    void insert_special_symbols_test()
    {
        PSqlConn;

        QVERIFY(sql.exec(CFString::fromUtf8(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id, x16, x32, x64, t, a, s, r, d"
            ") VALUES ("
                "1.0, 2.0, 3.0, 4.0, '2017-02-27T14:47:34.123Z', '\xC3\x96\xC3\x84\xC3\x9C', '\xC3\x96\xC3\x84\xC3\x9C', 1.23, 3.45"
            ")"
        )));
    }

    void select_special_symbols_test()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));

        QVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d;
        QCOMPARE(tt.id,  (cfuint32)1);
        QCOMPARE(tt.x16, (cfuint16)2);
        QCOMPARE(tt.x32, (cfuint32)3);
        QCOMPARE(tt.x64, (cfuint64)4);
        QCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
        QCOMPARE(tt.a, CFByteArray("\xC3\x96\xC3\x84\xC3\x9C"));
        QCOMPARE(tt.s, CFString::fromUtf8("\xC3\x96\xC3\x84\xC3\x9C"));
        QVERIFY(fuzzyCompare(tt.f, 1.23f));
        QVERIFY(fuzzyCompare(tt.d, 3.45));
        // no futher lines
        QVERIFY(!sql.next());
        // leave table empty
        QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=1;"));
    }

    // -----------------------------------------------------------

    void alter_table_test()
    {
        PSqlConn;

        QVERIFY(sql.exec("ALTER TABLE cflib_db_test DROP COLUMN t;"));
        QVERIFY(sql.exec("ALTER TABLE cflib_db_test ADD COLUMN t timestamp with time zone;"));
    }

    // -----------------------------------------------------------

    void insert_time_with_time_zone_test()
    {
        PSqlConn;

        QVERIFY(sql.exec(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id, x16, x32, x64, a, s, r, d, t"
            ") VALUES ("
                "1, 2, 3, 4, '\xe6\xbc\xa2\xe5\xad\x97', 'Hello World!', 1.23, 3.45, '2017-02-27T14:47:34.123Z'"
            ")"
        ));
    }


    void select_time_with_time_zone_test()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));

        QVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.a >> tt.s >> tt.f >> tt.d >> tt.t;
        QCOMPARE(tt.id,  (cfuint32)1);
        QCOMPARE(tt.x16, (cfuint16)2);
        QCOMPARE(tt.x32, (cfuint32)3);
        QCOMPARE(tt.x64, (cfuint64)4);
        QCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
        QCOMPARE(tt.a, CFByteArray("\xe6\xbc\xa2\xe5\xad\x97"));
        QCOMPARE(tt.s, CFString::fromUtf8("Hello World!"));
        QVERIFY(fuzzyCompare(tt.f, 1.23f));
        QVERIFY(fuzzyCompare(tt.d, 3.45));
        // no futher lines
        QVERIFY(!sql.next());
    }

    // -----------------------------------------------------------

    void update_table_test()
    {
        PSqlConn;

        QVERIFY(sql.exec(
            "UPDATE "
                "cflib_db_test "
            "SET "
                "id = 2, x16 = 123, x32 = 345, x64 = 1234567, a = '2017-02-27T14:47:34.123Z', s='Hello again', r = 123.456 , d = 12345.6789, t = '2017-02-27T14:47:34.123Z' "
            "where "
                "id = 1;"
        ));
    }

    void select_updated_table_test()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));

        QVERIFY(sql.next());
        sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.a >> tt.s >> tt.f >> tt.d >> tt.t;
        QCOMPARE(tt.id,  (cfuint32)2);
        QCOMPARE(tt.x16, (cfuint16)123);
        QCOMPARE(tt.x32, (cfuint32)345);
        QCOMPARE(tt.x64, (cfuint64)1234567);
        QCOMPARE(tt.t.toMSecsSinceEpoch(), makeUTCDateTime(2017, 2, 27, 14, 47, 34, 123).toMSecsSinceEpoch());
        QCOMPARE(tt.a, CFByteArray("2017-02-27T14:47:34.123Z"));
        QCOMPARE(tt.s, CFString::fromUtf8("Hello again"));
        QVERIFY(fuzzyCompare(tt.f, 123.456f));
        QVERIFY(fuzzyCompare(tt.d, 12345.6789));
        // no futher lines
        QVERIFY(!sql.next());
    }

    // -----------------------------------------------------------

    void select_error_test()
    {
        PSqlConn;
        QVERIFY(sql.lastFieldIsNull());

        QVERIFY(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));
        QVERIFY(sql.lastFieldIsNull());

        QVERIFY(sql.next());
        QVERIFY(sql.lastFieldIsNull());

        sql >> tt.x16;
        QVERIFY(sql.lastFieldIsNull());

        sql >> tt.t;
        QVERIFY(tt.t.isNull());
        QVERIFY(sql.lastFieldIsNull());

        QVERIFY(!sql.next());
        QVERIFY(sql.lastFieldIsNull());
    }

    // -----------------------------------------------------------

    void cascading_transaction_test()
    {
        PSqlConn;

        QVERIFY(!sql.commit());

        sql.begin();
        QVERIFY(sql.commit());

        sql.begin();
        sql.rollback();
        QVERIFY(!sql.commit());

        sql.begin();
        {
            PSqlConn;

            QVERIFY(!sql.commit());

            sql.begin();
            QVERIFY(sql.commit());
        }
        QVERIFY(sql.commit());

        sql.begin();
        {
            PSqlConn;

            sql.begin();
            sql.rollback();
            QVERIFY(!sql.commit());
        }
        QVERIFY(!sql.commit());
    }

    // -----------------------------------------------------------

    void keepFields_test()
    {
        PSqlConn;

        sql.prepare(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id, x32"
            ") VALUES ("
                "$2, $1"
            ")"
        );
        sql << 123 << 5;
        QVERIFY(sql.exec(1));
        sql << 6;
        QVERIFY(sql.exec());

        sql.prepare("SELECT x32 FROM cflib_db_test WHERE id = $1");

        sql << 5;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        sql >> tt.x32;
        QCOMPARE(tt.x32, (cfuint32)123);

        sql << 6;
        QVERIFY(sql.exec());
        QVERIFY(sql.next());
        sql >> tt.x32;
        QCOMPARE(tt.x32, (cfuint32)123);
    }

    // -----------------------------------------------------------

    void multi_prepare_test()
    {
        PSqlConn;

        sql.prepare(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id"
            ") VALUES ("
                "$1"
            ")"
        );

        PSqlConn2;

        sql2.prepare("SELECT id FROM cflib_db_test WHERE id = $1");

        sql << 7;
        sql2 << 6;
        QVERIFY(sql.exec());
        QVERIFY(sql2.exec());
        QVERIFY(sql2.next());
        sql2 >> tt.id;
        QCOMPARE(tt.id, (cfuint32)6);
        QVERIFY(!sql2.next());

        sql << 8;
        sql2 << 7;
        QVERIFY(sql.exec());
        QVERIFY(sql2.exec());
        QVERIFY(sql2.next());
        sql2 >> tt.id;
        QCOMPARE(tt.id, (cfuint32)7);
        QVERIFY(!sql2.next());
    }

    void multi_prepare_transaction_test()
    {
        {
            PSqlConn;
            sql.begin();
            {
                PSqlConn;
                sql.begin();
                sql.prepare(
                    "INSERT INTO "
                        "cflib_db_test "
                    "("
                        "id"
                    ") VALUES ("
                        "$1"
                    ")"
                );
                sql << 7;
                QVERIFY(!sql.exec());
                QVERIFY(sql.commit());
            }
            QVERIFY(sql.commit());
        }
        {
            PSqlConn;
            sql.begin();
            {
                PSqlConn;
                sql.begin();
                sql.prepare("SELECT id FROM cflib_db_test");
                QVERIFY(sql.exec());
                QVERIFY(sql.commit());
            }
            QVERIFY(sql.commit());
        }
    }

    void cascading_multi_prepare_test()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT COUNT(*) FROM cflib_db_test"));
        QVERIFY(sql.next());
        sql >> tt.x64;
        QVERIFY(tt.x64 > (cfuint64)1);

        sql.prepare("SELECT id FROM cflib_db_test ORDER BY id");
        QVERIFY(sql.exec());

        PSqlConn2;
        sql2.prepare("SELECT x32 FROM cflib_db_test WHERE id = $1");

        QVERIFY(sql.next());
        sql >> tt.id;
        QCOMPARE(tt.id, (cfuint32)2);

        sql2 << tt.id;
        QVERIFY(sql2.exec());
        QVERIFY(sql2.next());
        sql2 >> tt.x32;
        QCOMPARE(tt.x32, (cfuint32)345);
        QVERIFY(!sql2.next());

        // cascading queries do not work
        QVERIFY(!sql.next());
    }

    void two_connections_test()
    {
        PSqlConn;

        QVERIFY(sql.exec("SELECT COUNT(*) FROM cflib_db_test"));
        QVERIFY(sql.next());
        sql >> tt.x64;
        QVERIFY(tt.x64 > (cfuint64)2);

        sql.prepare("SELECT id FROM cflib_db_test ORDER BY id");
        QVERIFY(sql.exec());

        PSql sql2;
        sql2.prepare("SELECT x32 FROM cflib_db_test WHERE id = $1");

        QVERIFY(sql.next());
        sql >> tt.id;
        QCOMPARE(tt.id, (cfuint32)2);

        sql2 << tt.id;
        QVERIFY(sql2.exec());
        QVERIFY(sql2.next());
        sql2 >> tt.x32;
        QCOMPARE(tt.x32, (cfuint32)345);
        QVERIFY(!sql2.next());

        QVERIFY(sql.next());
        sql >> tt.id;
        QCOMPARE(tt.id, (cfuint32)5);

        sql2 << tt.id;
        QVERIFY(sql2.exec());
        QVERIFY(sql2.next());
        sql2 >> tt.x32;
        QCOMPARE(tt.x32, (cfuint32)123);
        QVERIFY(!sql2.next());

        QVERIFY(sql.next());
    }

    // -----------------------------------------------------------

    void transaction_isolation_test()
    {
        PSqlConn;
        sql.begin();
        sql.prepare(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id"
            ") VALUES ("
                "$1"
            ")"
        );
        sql  << 9;
        QVERIFY(sql.exec());

        PSql sql2;
        sql2.prepare("SELECT id FROM cflib_db_test WHERE id = $1");
        sql2 << 9;
        QVERIFY(sql2.exec(1));
        QVERIFY(!sql2.next());

        sql.commit();

        QVERIFY(sql2.exec(1));
        QVERIFY(sql2.next());
    }

    // -----------------------------------------------------------

    void transaction_blocking_commit_test()
    {
        CFSemaphore sem;
        bool threadResult = false;

        PSqlConn;
        sql.begin();
        sql.prepare(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id"
            ") VALUES ("
                "$1"
            ")"
        );
        sql << 10;
        QVERIFY(sql.exec());

        std::thread thread([&sem, &threadResult]() {
            PSqlConn;
            sql.prepare(
                "INSERT INTO "
                    "cflib_db_test "
                "("
                    "id"
                ") VALUES ("
                    "$1"
                ")"
            );

            sql << 11;
            threadResult = sql.exec();
            sem.release();

            sql << 10;
            threadResult = sql.exec();
            sem.release();
        });

        // Insert of different key works.
        sem.acquire();
        QVERIFY(threadResult);

        // Insert of same key blocks until our transaction has finished.
        // (We can't easily check sem.available() == 0 with CFSemaphore,
        //  so just sleep a bit and then commit.)
        usleep(1000000); // 1 second

        QVERIFY(sql.commit());

        sem.acquire();
        QVERIFY(!threadResult);

        thread.join();
    }

    void transaction_blocking_rollback_test()
    {
        CFSemaphore sem;
        bool threadResult = false;

        PSqlConn;
        sql.begin();
        sql.prepare(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id"
            ") VALUES ("
                "$1"
            ")"
        );
        sql << 12;
        QVERIFY(sql.exec());

        std::thread thread([&sem, &threadResult]() {
            PSqlConn;
            sql.prepare(
                "INSERT INTO "
                    "cflib_db_test "
                "("
                    "id"
                ") VALUES ("
                    "$1"
                ")"
            );

            sql << 12;
            threadResult = sql.exec();
            sem.release();
        });

        // Insert of same key blocks until our transaction has finished.
        usleep(1000000); // 1 second

        sql.rollback();

        sem.acquire();
        QVERIFY(threadResult);

        thread.join();
    }

    void transaction_failing_exec_test()
    {
        PSqlConn;
        sql.begin();
        sql.prepare(
            "INSERT INTO "
                "cflib_db_test "
            "("
                "id"
            ") VALUES ("
                "$1"
            ")"
        );
        sql << 13;
        QVERIFY(sql.exec());
        sql << 13;
        QVERIFY(!sql.exec());
        sql << 14;
        QVERIFY(!sql.exec());
        QVERIFY(sql.commit());
    }

};
ADD_TEST(PSql_test)
