/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/db/psql/psql.h>
#include <cflib/db/schema.h>
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
    bool migrate(const ByteArray & name)
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

String connParam(const String & dbName)
{
    const char * envVal = getenv("DB_TEST_DB");
    String baseConn = envVal ? String(envVal) : String("host=127.0.0.1");
    return PSql::setDBName(baseConn, dbName);
}

}

TEST_SUITE("Schema") {

TEST_CASE("Schema: initTestCase")
{
    PSql::closeThreadConnection();
    PSql sql(connParam("postgres"));
    sql.exec("DROP DATABASE cflib_db_test");
    REQUIRE(sql.exec("CREATE DATABASE cflib_db_test"));
    REQUIRE(PSql::setParameter(connParam("cflib_db_test")));
}

TEST_CASE("Schema: basic")
{
    REQUIRE((schema::update<PSql, Migrator>()));

    PSqlConn;
    REQUIRE(sql.exec("SELECT key, value, value2, value3, value4 FROM config ORDER BY key"));
    String key, value;
    int32 value2, value3, value4;

    REQUIRE(sql.next());
    sql >> key >> value >> value2 >> value3 >> value4;
    REQUIRE_EQ(key, String("test1"));
    REQUIRE_EQ(value, String("val1"));
    REQUIRE_EQ(value2, 0);
    REQUIRE_EQ(value3, 0);
    REQUIRE_EQ(value4, 0);

    REQUIRE(sql.next());
    sql >> key >> value >> value2 >> value3 >> value4;
    REQUIRE_EQ(key, String("test2"));
    REQUIRE_EQ(value, String("val2"));
    REQUIRE_EQ(value2, 2);
    REQUIRE_EQ(value3, 0);
    REQUIRE_EQ(value4, 0);

    REQUIRE(sql.next());
    REQUIRE(sql.next());
    REQUIRE(sql.next());
    REQUIRE(!sql.next());
}

TEST_CASE("Schema: update")
{
    ByteArray schema = File::read(":/schema.sql");
    REQUIRE(schema::update<PSql>(schema));

    schema +=
        "-- REVISION neu\n"
        "\n"
        "INSERT INTO config (key) VALUES ('neu')\n"
    ;
    REQUIRE(schema::update<PSql>(schema));
    PSqlConn;
    REQUIRE(sql.exec("SELECT COUNT(*) FROM config WHERE key = 'neu'"));
    REQUIRE(sql.next());
    REQUIRE_EQ(sql.get<int64>(0), (int64)1);
}

TEST_CASE("Schema: resetDB")
{
    PSql::closeThreadConnection();
    PSql sql(connParam("postgres"));
    REQUIRE(sql.exec("DROP DATABASE cflib_db_test"));
    REQUIRE(sql.exec("CREATE DATABASE cflib_db_test"));
    REQUIRE(PSql::setParameter(connParam("cflib_db_test")));
}

TEST_CASE("Schema: empty_head")
{
    REQUIRE(schema::update<PSql>(ByteArray(
        "-- REVISION first\n"
        "CREATE TABLE config (\n"
        "  key   text NOT NULL, \n"
        "  value text, \n"
        "  PRIMARY KEY (key)\n"
        ");\n"
    )));

    PSqlConn;
    REQUIRE(sql.exec("SELECT key, value FROM config"));
}

TEST_CASE("Schema: cleanupTestCase")
{
    PSql::closeThreadConnection();
    PSql sql(connParam("postgres"));
    REQUIRE(sql.exec("DROP DATABASE cflib_db_test"));
}

}
