/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <cflib/util/log.h>

#include <iomanip>
#include <iostream>
#include <string_view>

const char * prgName = "";

// Behaviour-Driven Development
struct BddConsoleReporter : public doctest::IReporter
{
    std::ostream & stream;

    static constexpr std::string_view RESET = "\033[0m";
    static constexpr std::string_view BOLD  = "\033[1m";
    static constexpr std::string_view GREEN = "\033[32m";
    static constexpr std::string_view RED   = "\033[31m";
    static constexpr std::string_view CYAN  = "\033[36m";
    static constexpr std::string_view GRAY  = "\033[90m";

    BddConsoleReporter(const doctest::ContextOptions & in) :
        stream(*in.cout)
    {
    }

    void report_query(const doctest::QueryData &) override
    {
    }

    void test_run_start() override
    {
        stream << BOLD << CYAN
               << "============================================================\n"
               << prgName << "\n"
               << "============================================================\n"
               << RESET;
    }

    void test_run_end(const doctest::TestRunStats & in) override
    {
        stream << "\n" << BOLD << CYAN
            << "============================================================\n"
            << RESET;

        stream << BOLD << "SUMMARY: ";
        if (in.numTestCasesFailed == 0) stream << GREEN << "✅ ALL TESTS PASSED";
        else                            stream << RED << "❌ SOME TESTS FAILED";
        stream << RESET << "\n";

        auto printStat = [&](int passed, int failed, int total) {
            stream << std::right
                << BOLD << std::setw(5) << passed << RESET << " passed | ";
            if (failed > 0) stream << RED << BOLD << std::setw(5) << failed << RESET << " failed";
            else            stream << std::setw(5) << 0 << " failed";
            stream << " | " << std::setw(5) << total << " total\n";
        };

        stream << "  Scenarios  : ";
        printStat(in.numTestCasesPassingFilters - in.numTestCasesFailed, in.numTestCasesFailed, in.numTestCasesPassingFilters);
        unsigned skipped = in.numTestCases - in.numTestCasesPassingFilters;
        if (skipped > 0) stream << "  Skipped    :                               " << std::setw(5) << skipped << " total\n";
        stream << "  Assertions : ";
        printStat(in.numAsserts - in.numAssertsFailed, in.numAssertsFailed, in.numAsserts);

        stream << BOLD << CYAN
            << "============================================================\n"
            << RESET << "\n";
    }

    void test_case_start(const doctest::TestCaseData & in) override
    {
        bool hasIndent = doctest::String(in.m_name).substr(0, 2) == "  ";
        stream << "\n" << BOLD << (hasIndent ? "" : "  ") << in.m_name << RESET << "\n";
    }

    void test_case_end(const doctest::CurrentTestCaseStats & in) override
    {
        bool passed = (in.failure_flags == doctest::TestCaseFailureReason::None);

        stream << "  " << BOLD << "Status: ";
        if (passed) {
            stream << GREEN << "✅ PASSED";
        } else {
            stream << RED << "❌ FAILED";
        }
        stream << RESET << "\n";
    }

    void subcase_start(const doctest::SubcaseSignature & in) override
    {
        bool given = in.m_name.substr(0, 10) == "   Given: ";
        stream << GRAY << (given ? "   🔹" : "    ") << in.m_name << RESET << "\n";
    }

    void subcase_end() override
    {
    }

    void log_assert(const doctest::AssertData & in) override
    {
        if (!in.m_failed) return;

        stream << "        " << BOLD << RED << "❌ FAILED: " << in.m_expr << RESET << "\n";
        if (in.m_decomp.size() > 0) stream << "           " << in.m_decomp.c_str() << "\n";
        stream << "           " << in.m_file << ":" << in.m_line << "\n";
    }

    void log_message(const doctest::MessageData &) override
    {
    }

    void test_case_skipped(const doctest::TestCaseData &) override
    {
    }

    void test_case_exception(const doctest::TestCaseException &) override
    {
    }

    void test_case_reenter(const doctest::TestCaseData &) override
    {
    }
};

REGISTER_REPORTER("bdd", 0, BddConsoleReporter);

int main(int argc, char ** argv)
{
    if (argc > 0) prgName = argv[0];
    cflib::util::Log::start("test.log");
    return doctest::Context(argc, argv).run();
}
