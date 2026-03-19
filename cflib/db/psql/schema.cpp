/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "schema.h"

#include <cflib/db/psql/psql.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Db)

namespace cflib::db::schema {

namespace {

bool insertRevision(const String & rev)
{
    PSqlConn;
    sql.prepare(
        "INSERT INTO "
            "__scheme_revisions__ "
        "("
            "rev, applied"
        ") VALUES ("
            "$1, $2"
        ") ON CONFLICT DO NOTHING"
    );
    sql << rev << DateTime::nowUTC();
    return sql.exec();
}

bool confirmRevision(const String & rev)
{
    PSqlConn;
    sql.prepare(
        "UPDATE "
            "__scheme_revisions__ "
        "SET "
            "applied = $1, success = 1 "
        "WHERE "
            "rev = $2"
    );
    sql << DateTime::nowUTC() << rev;
    return sql.exec();
}

// Remove SQL comments (lines starting with --)
String removeComments(const String & query)
{
    String result;
    size_t pos = 0;
    const String & s = query;
    size_t len = s.size();

    while (pos < len) {
        // Check for comment at start of string or after newline
        bool isLineStart = (pos == 0) || (s.c_str()[pos - 1] == '\n');
        if (isLineStart && pos + 1 < len && s.c_str()[pos] == '-' && s.c_str()[pos + 1] == '-') {
            // Skip to end of line
            while (pos < len && s.c_str()[pos] != '\n') ++pos;
            if (pos < len) ++pos; // skip the newline
            continue;
        }
        // Also check for -- after newline within the string
        if (pos + 1 < len && s.c_str()[pos] == '\n' && pos + 2 < len && s.c_str()[pos + 1] == '-' && s.c_str()[pos + 2] == '-') {
            result += '\n';
            pos += 1;
            // Skip the comment
            while (pos < len && s.c_str()[pos] != '\n') ++pos;
            continue;
        }
        result += s.c_str()[pos];
        ++pos;
    }
    return result;
}

bool execSql(const String & query)
{
    String cleanQuery = removeComments(query).trimmed();
    if (cleanQuery.isEmpty()) return true;

    PSqlConn;
    logDebug("executing: %1", cleanQuery);
    return sql.execMultiple(cleanQuery);
}

// Find "-- EXEC <name>" pattern at the beginning of a line.
// Returns -1 if not found, otherwise the position of the start of the match.
// Sets matchEnd to the end of the match and methodName to the captured name.
size_t findExecDirective(const String & query, size_t startPos, size_t & matchEnd, ByteArray & methodName)
{
    const char * data = query.c_str();
    size_t len = query.size();
    size_t pos = startPos;

    while (pos < len) {
        // Must be at start of line
        bool isLineStart = (pos == 0) || (data[pos - 1] == '\n');
        if (!isLineStart) {
            // advance to next newline
            while (pos < len && data[pos] != '\n') ++pos;
            if (pos < len) ++pos;
            continue;
        }

        // Check for "-- EXEC "
        if (pos + 8 <= len &&
            data[pos] == '-' && data[pos+1] == '-' && data[pos+2] == ' ' &&
            data[pos+3] == 'E' && data[pos+4] == 'X' && data[pos+5] == 'E' && data[pos+6] == 'C' && data[pos+7] == ' ')
        {
            size_t nameStart = pos + 8;
            size_t nameEnd = nameStart;
            while (nameEnd < len && data[nameEnd] != '\n' && data[nameEnd] != '\r') ++nameEnd;

            String name = query.mid(nameStart, nameEnd - nameStart).trimmed();
            methodName = name.toUtf8();
            matchEnd = nameEnd;
            return pos;
        }

        // advance past current character
        ++pos;
    }
    return (size_t)-1;
}

// Find "-- REVISION <name>" pattern at the beginning of a line.
size_t findRevisionDirective(const String & query, size_t startPos, size_t & matchEnd, String & revName)
{
    const char * data = query.c_str();
    size_t len = query.size();
    size_t pos = startPos;

    while (pos < len) {
        bool isLineStart = (pos == 0) || (data[pos - 1] == '\n');
        if (!isLineStart) {
            while (pos < len && data[pos] != '\n') ++pos;
            if (pos < len) ++pos;
            continue;
        }

        // Check for "-- REVISION "
        if (pos + 12 <= len &&
            data[pos] == '-' && data[pos+1] == '-' && data[pos+2] == ' ' &&
            data[pos+3] == 'R' && data[pos+4] == 'E' && data[pos+5] == 'V' &&
            data[pos+6] == 'I' && data[pos+7] == 'S' && data[pos+8] == 'I' &&
            data[pos+9] == 'O' && data[pos+10] == 'N' && data[pos+11] == ' ')
        {
            size_t nameStart = pos + 12;
            size_t nameEnd = nameStart;
            while (nameEnd < len && data[nameEnd] != '\n' && data[nameEnd] != '\r') ++nameEnd;

            revName = query.mid(nameStart, nameEnd - nameStart);
            matchEnd = nameEnd;
            return pos;
        }

        ++pos;
    }
    return (size_t)-1;
}

bool execRevision(const String & query, Migrator & migrator)
{
    size_t start = 0;
    size_t matchEnd;
    ByteArray method;
    size_t matchStart = findExecDirective(query, start, matchEnd, method);

    while (matchStart != (size_t)-1) {
        if (!execSql(query.mid(start, matchStart - start))) return false;

        if (!migrator) {
            logWarn("found EXEC in SQL, but no migrator given");
            return false;
        }

        if (!migrator(method)) {
            logWarn("migration %1 failed", method);
            return false;
        }

        logInfo("migration %1 finished successfully", method);

        start = matchEnd;
        matchStart = findExecDirective(query, start, matchEnd, method);
    }
    return execSql(query.mid(start));
}

}

bool update(Migrator migrator, const String & filename)
{
    return update(util::readFile(filename), migrator);
}

bool update(const ByteArray & schema, Migrator migrator)
{
    PSqlConn;

    // get existing revisions
    Set<String> existingRevisions;
    if (!sql.exec("SELECT rev FROM __scheme_revisions__ WHERE success = 1")) {
        logInfo("creating table __scheme_revisions__");
        if (!sql.exec(
            "CREATE TABLE __scheme_revisions__ ("
                "rev text NOT NULL, "
                "applied timestamp with time zone NOT NULL, "
                "success smallint NOT NULL DEFAULT 0, "
                "PRIMARY KEY (rev)"
            ")"
        )) return false;
    } else {
        while (sql.next()) {
            existingRevisions.insert(sql.get<String>(0));
        }
    }

    const String utf8Schema = String::fromUtf8(schema);

    size_t start = 0;
    size_t matchEnd;
    String revName;
    size_t matchStart = findRevisionDirective(utf8Schema, start, matchEnd, revName);
    String lastRev = "__initial__";

    while (matchStart != (size_t)-1) {
        if (existingRevisions.find(lastRev) == existingRevisions.end()) {
            logInfo("applying revision %1", lastRev);
            if (!insertRevision(lastRev)) return false;
            PSqlConn;
            sql.begin();
            if (!execRevision(utf8Schema.mid(start, matchStart - start), migrator)) return false;
            if (!confirmRevision(lastRev)) return false;
            if (!sql.commit()) return false;
        }

        lastRev = revName;
        start = matchEnd;
        matchStart = findRevisionDirective(utf8Schema, start, matchEnd, revName);
    }

    if (existingRevisions.find(lastRev) == existingRevisions.end()) {
        logInfo("applying revision %1", lastRev);
        if (!insertRevision(lastRev)) return false;
        PSqlConn;
        sql.begin();
        if (!execRevision(utf8Schema.mid(start), migrator)) return false;
        if (!confirmRevision(lastRev)) return false;
        if (!sql.commit()) return false;
    }

    return true;
}

} // namespace
