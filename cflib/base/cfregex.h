/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfstring.h>
#include <cflib/base/cfcontainers.h>

#include <regex>

class CFRegex
{
public:
    CFRegex() : valid_(false) {}

    explicit CFRegex(const char * pattern)
        : re_(pattern), valid_(true) {}

    explicit CFRegex(const CFString & pattern)
        : re_(pattern.str()), valid_(true) {}

    // Case-insensitive constructor
    CFRegex(const char * pattern, bool caseInsensitive)
        : re_(pattern, caseInsensitive ? std::regex::ECMAScript | std::regex::icase : std::regex::ECMAScript)
        , valid_(true) {}

    CFRegex(const CFString & pattern, bool caseInsensitive)
        : re_(pattern.str(), caseInsensitive ? std::regex::ECMAScript | std::regex::icase : std::regex::ECMAScript)
        , valid_(true) {}

    bool isValid() const { return valid_; }

    bool match(const CFString & subject) const {
        if (!valid_) return false;
        return std::regex_search(subject.str(), re_);
    }

    bool match(const CFByteArray & subject) const {
        if (!valid_) return false;
        return std::regex_search(subject.toStdString(), re_);
    }

    // Match returning captured groups (0 = whole match, 1+ = groups)
    struct MatchResult {
        bool hasMatch() const { return matched_; }
        CFString captured(int n = 0) const {
            if (!matched_ || n < 0 || n >= (int)groups_.size()) return CFString();
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
        CFList<CFString> groups_;
        CFList<int> positions_;
        CFList<int> lengths_;
        friend class CFRegex;
    };

    MatchResult matchResult(const CFString & subject) const {
        MatchResult result;
        if (!valid_) return result;
        std::smatch m;
        if (!std::regex_search(subject.str(), m, re_)) return result;
        result.matched_ = true;
        for (auto & s : m) {
            result.groups_.push_back(CFString(s.str()));
            result.positions_.push_back((int)(s.first - subject.str().begin()));
            result.lengths_.push_back((int)s.length());
        }
        return result;
    }

    MatchResult matchResult(const CFByteArray & subject) const {
        MatchResult result;
        if (!valid_) return result;
        std::smatch m;
        const std::string & s = subject.toStdString();
        if (!std::regex_search(s, m, re_)) return result;
        result.matched_ = true;
        for (auto & sub : m) {
            result.groups_.push_back(CFString(sub.str()));
            result.positions_.push_back((int)(sub.first - s.begin()));
            result.lengths_.push_back((int)sub.length());
        }
        return result;
    }

    // Replace first match
    CFString replace(const CFString & subject, const CFString & replacement) const {
        if (!valid_) return subject;
        return CFString(std::regex_replace(subject.str(), re_, replacement.str(),
            std::regex_constants::format_first_only));
    }

    // Replace all matches
    CFString replaceAll(const CFString & subject, const CFString & replacement) const {
        if (!valid_) return subject;
        return CFString(std::regex_replace(subject.str(), re_, replacement.str()));
    }

private:
    std::regex re_;
    bool valid_;
};

// CFString::replace overload that takes CFRegex (defined here since CFString is declared before CFRegex)
inline CFString & cfStringReplace(CFString & s, const CFRegex & re, const char * replacement) {
    s = re.replaceAll(s, CFString(replacement));
    return s;
}

// Allow chaining: content.replace(CFRegex(...), "...")
// We do this by making CFString implicitly work with a helper
// Actually, for the fileserver pattern content.replace(re, str).replace(re2, str2)
// we provide a chainable wrapper:
class CFStringRegexReplacer {
public:
    CFStringRegexReplacer(CFString & s) : s_(s) {}
    CFStringRegexReplacer & replace(const CFRegex & re, const char * replacement) {
        s_ = re.replaceAll(s_, CFString(replacement));
        return *this;
    }
    operator CFString &() { return s_; }
private:
    CFString & s_;
};
