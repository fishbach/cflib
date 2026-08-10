/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <cflib/util/log.h>

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

    void test_run_end(const doctest::TestRunStats &) override
    {
        stream << "\n" << BOLD << CYAN
            << "============================================================\n"
            << RESET << "\n";
    }

    void test_case_start(const doctest::TestCaseData & in) override
    {
        stream << "\n" << BOLD << "📌 Scenario: " << in.m_name << RESET << "\n";
    }

    void test_case_end(const doctest::CurrentTestCaseStats& in) override
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
        stream << "   " << GRAY << "🔹 " << RESET << in.m_name << "\n";
    }

    void subcase_end() override
    {
    }

    void log_assert(const doctest::AssertData& in) override
    {
        if (in.m_failed) {
            stream << "      " << BOLD << RED << "❌ FAILED: "
                << in.m_expr << " (in " << in.m_file << ":" << in.m_line << ")"
                << RESET << "\n";
            if (in.m_decomp.size() > 0) {
                stream << "         " << GRAY << in.m_decomp.c_str()
                    << RESET << "\n";
            }
        }
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
