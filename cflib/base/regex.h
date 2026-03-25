/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/util.h>

#include <regex>

namespace cflib::base {

class Regex
{
public:
    Regex() : valid_(false) {}

    explicit Regex(const char * pattern)
        : re_(pattern), valid_(true) {}

    explicit Regex(const String & pattern)
        : re_(pattern.str()), valid_(true) {}

    // Case-insensitive constructor
    Regex(const char * pattern, bool caseInsensitive)
        : re_(pattern, caseInsensitive ? std::regex::ECMAScript | std::regex::icase : std::regex::ECMAScript)
        , valid_(true) {}

    Regex(const String & pattern, bool caseInsensitive)
        : re_(pattern.str(), caseInsensitive ? std::regex::ECMAScript | std::regex::icase : std::regex::ECMAScript)
        , valid_(true) {}

    bool isValid() const { return valid_; }

    bool match(const String & subject) const {
        if (!valid_) return false;
        return std::regex_search(subject.str(), re_);
    }

    bool match(const ByteArray & subject) const {
        if (!valid_) return false;
        return std::regex_search(subject.toStdString(), re_);
    }

    // Match returning captured groups (0 = whole match, 1+ = groups)
    struct MatchResult {
        bool hasMatch() const { return matched_; }
        String captured(int n = 0) const {
            if (!matched_ || n < 0 || n >= (int)groups_.size()) return String();
            return groups_[n];
        }
        int capturedStart(int n = 0) const {
            if (!matched_ || n < 0 || n >= (int)positions_.size()) return -1;
            return positions_[n];
        }
        int capturedLength(int n = 0) const {
            if (!matched_ || n < 0 || n >= (int)lengths_.size()) return 0;
            return lengths_[n];
        }
    private:
        bool matched_ = false;
        StringList groups_;
        List<int> positions_;
        List<int> lengths_;
        friend class Regex;
    };

    MatchResult matchResult(const String & subject) const {
        MatchResult result;
        if (!valid_) return result;
        std::smatch m;
        if (!std::regex_search(subject.str(), m, re_)) return result;
        result.matched_ = true;
        for (auto & s : m) {
            result.groups_.push_back(String(s.str()));
            result.positions_.push_back((int)(s.first - subject.str().begin()));
            result.lengths_.push_back((int)s.length());
        }
        return result;
    }

    MatchResult matchResult(const ByteArray & subject) const {
        MatchResult result;
        if (!valid_) return result;
        std::smatch m;
        const std::string & s = subject.toStdString();
        if (!std::regex_search(s, m, re_)) return result;
        result.matched_ = true;
        for (auto & sub : m) {
            result.groups_.push_back(String(sub.str()));
            result.positions_.push_back((int)(sub.first - s.begin()));
            result.lengths_.push_back((int)sub.length());
        }
        return result;
    }

    // Replace first match
    String replace(const String & subject, const String & replacement) const {
        if (!valid_) return subject;
        return String(std::regex_replace(subject.str(), re_, replacement.str(),
            std::regex_constants::format_first_only));
    }

    // Replace all matches
    String replaceAll(const String & subject, const String & replacement) const {
        if (!valid_) return subject;
        return String(std::regex_replace(subject.str(), re_, replacement.str()));
    }

private:
    std::regex re_;
    bool valid_;
};

// String::replace overload that takes Regex (defined here since String is declared before Regex)
inline String & stringReplace(String & s, const Regex & re, const char * replacement) {
    s = re.replaceAll(s, String(replacement));
    return s;
}

// Allow chaining: content.replace(Regex(...), "...")
// We do this by making String implicitly work with a helper
// Actually, for the fileserver pattern content.replace(re, str).replace(re2, str2)
// we provide a chainable wrapper:
class StringRegexReplacer {
public:
    StringRegexReplacer(String & s) : s_(s) {}
    StringRegexReplacer & replace(const Regex & re, const char * replacement) {
        s_ = re.replaceAll(s_, String(replacement));
        return *this;
    }
    operator String &() { return s_; }
private:
    String & s_;
};

} // namespace
