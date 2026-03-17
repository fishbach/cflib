/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/db/psql/psql.h>
#include <cflib/db/psql/schema.h>
#include <cflib/util/log.h>
#include <cflib/util/test.h>
#include <cflib/util/util.h>

using namespace cflib::db;
using namespace cflib::util;

USE_LOG(LogCat::Db)

namespace {

class Migrator
{
public:
    bool migrate(const CFByteArray & name)
    {
        if (name == "test1") return test1();
        if (name == "test2") return test2();
        if (name == "test3") return test3();
        if (name == "test4") return test4();
        if (name == "test5") return test5();
        return false;
    }

    bool test1()
    {
        PSqlConn;
        return sql.exec("INSERT INTO config (key, value) VALUES ('test1', 'val1')");
    }

    bool test2()
    {
        PSqlConn;
        return sql.exec("INSERT INTO config (key, value, value2) VALUES ('test2', 'val2', 2)");
    }

    bool test3()
    {
        PSqlConn;
        return sql.exec("INSERT INTO config (key, value, value2) VALUES ('test3', 'val3', 3)");
    }

    bool test4()
    {
        PSqlConn;
        return sql.exec("INSERT INTO config (key, value, value2, value3) VALUES ('test4', 'val4', 4, 4)");
    }

    bool test5()
    {
        PSqlConn;
        return sql.exec("INSERT INTO config (key, value, value2, value3, value4) VALUES ('test5', 'val5', 5, 5, 5)");
    }
};

CFString connParam(const CFString & dbName)
{
    const char * envVal = getenv("DB_TEST_DB");
    CFString baseConn = envVal ? CFString(envVal) : CFString("host=127.0.0.1");
    return PSql::setDBName(baseConn, dbName);
}

}

class Schema_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Schema_test *>(this);
        return {
            {"initTestCase",    [self]() { self->initTestCase(); }},
            {"basic_test",      [self]() { self->basic_test(); }},
            {"update_test",     [self]() { self->update_test(); }},
            {"resetDB",         [self]() { self->resetDB(); }},
            {"empty_head_test", [self]() { self->empty_head_test(); }},
            {"cleanupTestCase", [self]() { self->cleanupTestCase(); }},
        };
    }

    void initTestCase()
    {
        PSql::closeThreadConnection();
        PSql sql(connParam("postgres"));
        sql.exec("DROP DATABASE cflib_db_test");
        QVERIFY(sql.exec("CREATE DATABASE cflib_db_test"));
        QVERIFY(PSql::setParameter(connParam("cflib_db_test")));
    }

    void cleanupTestCase()
    {
        PSql::closeThreadConnection();
        PSql sql(connParam("postgres"));
        QVERIFY(sql.exec("DROP DATABASE cflib_db_test"));
    }

    void basic_test()
    {
        QVERIFY((schema::update<Migrator>(CFString(SCHEMA_SQL_PATH))));

        PSqlConn;
        QVERIFY(sql.exec("SELECT key, value, value2, value3, value4 FROM config ORDER BY key"));
        CFString key, value;
        cfint32 value2, value3, value4;

        QVERIFY(sql.next());
        sql >> key >> value >> value2 >> value3 >> value4;
        QCOMPARE(key, CFString("test1"));
        QCOMPARE(value, CFString("val1"));
        QCOMPARE(value2, 0);
        QCOMPARE(value3, 0);
        QCOMPARE(value4, 0);

        QVERIFY(sql.next());
        sql >> key >> value >> value2 >> value3 >> value4;
        QCOMPARE(key, CFString("test2"));
        QCOMPARE(value, CFString("val2"));
        QCOMPARE(value2, 2);
        QCOMPARE(value3, 0);
        QCOMPARE(value4, 0);

        QVERIFY(sql.next());
        QVERIFY(sql.next());
        QVERIFY(sql.next());
        QVERIFY(!sql.next());
    }

    void update_test()
    {
        CFByteArray schema = readFile(CFString(SCHEMA_SQL_PATH));
        QVERIFY(schema::update(schema));

        schema +=
            "-- REVISION neu\n"
            "\n"
            "INSERT INTO config (key) VALUES ('neu')\n"
        ;
        QVERIFY(schema::update(schema));
        PSqlConn;
        QVERIFY(sql.exec("SELECT COUNT(*) FROM config WHERE key = 'neu'"));
        QVERIFY(sql.next());
        QCOMPARE(sql.get<cfint64>(0), (cfint64)1);
    }

    void resetDB()
    {
        PSql::closeThreadConnection();
        PSql sql(connParam("postgres"));
        QVERIFY(sql.exec("DROP DATABASE cflib_db_test"));
        QVERIFY(sql.exec("CREATE DATABASE cflib_db_test"));
        QVERIFY(PSql::setParameter(connParam("cflib_db_test")));
    }

    void empty_head_test()
    {
        QVERIFY(schema::update(CFByteArray(
            "-- REVISION first\n"
            "CREATE TABLE config (\n"
            "  key   text NOT NULL, \n"
            "  value text, \n"
            "  PRIMARY KEY (key)\n"
            ");\n"
        )));

        PSqlConn;
        QVERIFY(sql.exec("SELECT key, value FROM config"));
    }

};
ADD_TEST(Schema_test)
