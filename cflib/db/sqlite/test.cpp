#include <sqlite3.h>

#include <iostream>

int sqlite_test()
{
    sqlite3* db;
    char* zErrMsg = 0;
    int rc;

    rc = sqlite3_open(":memory:", &db);
    if (rc) {
        std::cerr << "Fehler beim Öffnen: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    } else {
        std::cout << "SQLite erfolgreich geöffnet!" << std::endl;
        std::cout << "Version: " << sqlite3_libversion() << std::endl;
    }

    rc = sqlite3_exec(db, "CREATE TABLE TEST(ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL);", 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Fehler: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return 2;
    } else {
        std::cout << "Test-Tabelle erfolgreich erstellt." << std::endl;
    }

    sqlite3_close(db);

    return 0;
}
