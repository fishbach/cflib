/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/log.h>

#include <QtSql>

#define Transaction cflib::db::ScopedTransaction ta(::cflib_util_logFileInfo, __LINE__)
#define execQuery(query) cflib::db::exec((query), ::cflib_util_logFileInfo, __LINE__)
#define execQueryCommit(query) (cflib::db::exec((query), ::cflib_util_logFileInfo, __LINE__) && ta.commit())

namespace cflib { namespace db {

class ScopedTransaction
{
    Q_DISABLE_COPY(ScopedTransaction)
private:
    class ThreadData;
    static QThreadStorage<ThreadData *> threadData_;
    ThreadData & td_;

public:
    ScopedTransaction(const cflib::util::LogFileInfo & lfi, int line);
    ~ScopedTransaction();

    QSqlDatabase & db;

    bool commit();

private:
    const cflib::util::LogFileInfo & lfi_;
    const int line_;
    bool nested_;
    bool committed_;
    QElapsedTimer watch_;
    friend void closeDBConnection();
};

void closeDBConnection();

void setParameter(const QString & database, const QString & user, const QString & password);

bool exec(QSqlQuery & query, const cflib::util::LogFileInfo & lfi, int line);

QDateTime getUTC(const QSqlQuery & query, int index);

quint64 lastInsertId();

}}    // namespace
