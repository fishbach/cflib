/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/types.h>

#include <chrono>
#include <ctime>

namespace cflib::base {

class ElapsedTimer
{
public:
    ElapsedTimer() : started_(false) {}

    void start() {
        tp_ = std::chrono::steady_clock::now();
        started_ = true;
    }

    int64 elapsed() const {
        if (!started_) return 0;
        auto d = std::chrono::steady_clock::now() - tp_;
        return (int64)std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
    }

    int64 nsecsElapsed() const {
        if (!started_) return 0;
        auto d = std::chrono::steady_clock::now() - tp_;
        return (int64)std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
    }

    bool isValid() const { return started_; }

private:
    std::chrono::steady_clock::time_point tp_;
    bool started_;
};

class DateTime
{
public:
    DateTime() : valid_(false) {}

    static DateTime currentDateTimeUtc() { return nowUTC(); }
    static DateTime nowUTC() {
        DateTime dt;
        dt.tp_ = std::chrono::system_clock::now();
        dt.valid_ = true;
        dt.fillTm();
        return dt;
    }

    static DateTime fromSecsSinceEpoch(int64 secs) {
        DateTime dt;
        dt.tp_ = std::chrono::system_clock::time_point(std::chrono::seconds(secs));
        dt.valid_ = true;
        dt.fillTm();
        return dt;
    }

    static DateTime fromMSecsSinceEpoch(int64 msecs) {
        DateTime dt;
        dt.tp_ = std::chrono::system_clock::time_point(std::chrono::milliseconds(msecs));
        dt.valid_ = true;
        dt.fillTm();
        return dt;
    }

    bool isValid() const { return valid_; }
    bool isNull()  const { return !valid_; }

    // UTC accessors
    int year()   const { return tm_.tm_year + 1900; }
    int month()  const { return tm_.tm_mon + 1; }
    int day()    const { return tm_.tm_mday; }
    int hour()   const { return tm_.tm_hour; }
    int minute() const { return tm_.tm_min; }
    int second() const { return tm_.tm_sec; }
    int msec()   const {
        auto secs = std::chrono::time_point_cast<std::chrono::seconds>(tp_);
        auto ms   = std::chrono::duration_cast<std::chrono::milliseconds>(tp_ - secs);
        return (int)ms.count();
    }

    int64 toSecsSinceEpoch() const {
        return (int64)std::chrono::duration_cast<std::chrono::seconds>(tp_.time_since_epoch()).count();
    }

    int64 toMSecsSinceEpoch() const {
        return (int64)std::chrono::duration_cast<std::chrono::milliseconds>(tp_.time_since_epoch()).count();
    }

    int64 msecsTo(const DateTime & other) const {
        return other.toMSecsSinceEpoch() - toMSecsSinceEpoch();
    }

    int64 secsTo(const DateTime & other) const {
        return other.toSecsSinceEpoch() - toSecsSinceEpoch();
    }

    DateTime addMSecs(int64 ms) const {
        return fromMSecsSinceEpoch(toMSecsSinceEpoch() + ms);
    }

    // Mon=1..Sun=7
    int dayOfWeek() const { return tm_.tm_wday == 0 ? 7 : tm_.tm_wday; }

    bool operator==(const DateTime & other) const { return toMSecsSinceEpoch() == other.toMSecsSinceEpoch(); }
    bool operator!=(const DateTime & other) const { return !(*this == other); }

    std::chrono::system_clock::time_point timePoint() const { return tp_; }

private:
    void fillTm() {
        time_t t = std::chrono::system_clock::to_time_t(tp_);
        gmtime_r(&t, &tm_);
    }

    std::chrono::system_clock::time_point tp_;
    struct tm tm_ = {};
    bool valid_;
};

} // namespace
