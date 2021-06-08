//
// Created by Albert Varaksin on 23/05/2021.
//
#if defined(__CLION_IDE__)
#    pragma ide diagnostic ignored "cppcoreguidelines-avoid-non-const-global-variables"
#    pragma ide diagnostic ignored "cppcoreguidelines-owning-memory"
#    pragma ide diagnostic ignored "cppcoreguidelines-non-private-member-variables-in-classes"
#    pragma ide diagnostic ignored "cert-err58-cpp"
#    pragma ide diagnostic ignored "cppcoreguidelines-special-member-functions"
#    pragma ide diagnostic ignored "cppcoreguidelines-avoid-magic-numbers"
#endif

#include "Driver/Context.hpp"
#include "Lexer/Lexer.hpp"
#include "Lexer/Token.hpp"
#include <gtest/gtest.h>
namespace {

class LexerTests : public testing::Test {
protected:
    lbc::Lexer& load(llvm::StringRef source) {
        auto& srcMgr = m_context.getSourceMrg();
        auto buffer = llvm::MemoryBuffer::getMemBuffer(source);
        auto fileId = srcMgr.AddNewSourceBuffer(std::move(buffer), {});
        m_lexer = std::make_unique<lbc::Lexer>(m_context, fileId);
        return *m_lexer;
    }

    void expect(lbc::TokenKind kind, const std::string& lexeme = "", unsigned line = 0, unsigned col = 0, unsigned len = 0) {
        auto token = m_lexer->next();
        EXPECT_EQ(token->kind(), kind);

        if (!lexeme.empty()) {
            EXPECT_EQ(token->asString(), lexeme);
        }

        auto start = m_context.getSourceMrg().getLineAndColumn(token->range().Start);
        auto end = m_context.getSourceMrg().getLineAndColumn(token->range().End);

        if (line > 0) {
            EXPECT_EQ(start.first, line);
        }
        if (col > 0) {
            EXPECT_EQ(start.second, col);
        }
        if (len > 0) {
            auto token_len = end.second - start.second;
            EXPECT_EQ(token_len, len);
        }
    }

private:
    std::unique_ptr<lbc::Lexer> m_lexer;
    lbc::Context m_context{};
};

#define EXPECT_TOKEN(KIND, ...)      \
    {                                \
        SCOPED_TRACE(#KIND);         \
        expect(KIND, ##__VA_ARGS__); \
    }

TEST_F(LexerTests, NoInput) {
    auto& lexer = load("");
    EXPECT_TRUE(lexer.next()->kind() == lbc::TokenKind::EndOfFile);
    EXPECT_TRUE(lexer.next()->kind() == lbc::TokenKind::EndOfFile);
}

TEST_F(LexerTests, EmptyInputs) {
    constexpr std::array inputs{
        "   ",
        "\t\t",
        "\n   \n   ",
        "\r\n",
        "   \r   \n  \t  ",
        "'comment string",
        " /' stream \n '/ ",
        "/'somethign",
        "/'/' doubly nested '/'/",
        " \t _ this should be ignored \n_ ignored again"
    };
    for (const auto* input : inputs) {
        load(input);
        EXPECT_TOKEN(lbc::TokenKind::EndOfFile)
    }
}

TEST_F(LexerTests, MultiLineComments) {
    constexpr std::array inputs{
        "a/''/b",
        "a/' '/b",
        "a /''/ b",
        "a /' '/ b",
        "a /'\n'/ b",
        "a /'\r\n'/b",
        "a/'\r'/b",
        "a/'/''/'/b",
        "a/' / ' ' / '/b",
        "/' \n '/a/' /' '/\n '/b/'",
        "a _\n /' some multiline coment \n on a new line '/ _\n /' \n /' cont '/ \r\n '/ b"
    };
    for (const auto* input : inputs) {
        load(input);
        EXPECT_TOKEN(lbc::TokenKind::Identifier, "A")
        EXPECT_TOKEN(lbc::TokenKind::Identifier, "B")
        EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)
        EXPECT_TOKEN(lbc::TokenKind::EndOfFile)
    }
}

TEST_F(LexerTests, TokenLocation) {
    constexpr auto source =
        "one \"two\" three 42 = <= ...\n"
        "four \t IF a = b THEN \r\n"
        "five /'/' nested '/'/ six\n"
        "seven/' trash\n"
        "trash /' nested\n"
        "'/\n"
        "end?'/eight";
    load(source);

    // clang-format off

    // line 1
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "ONE",   1, 1,  3)
    EXPECT_TOKEN(lbc::TokenKind::StringLiteral,  "two",   1, 5,  5)
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "THREE", 1, 11, 5)
    EXPECT_TOKEN(lbc::TokenKind::IntegerLiteral, "42",    1, 17, 2)
    EXPECT_TOKEN(lbc::TokenKind::Assign,         "=",     1, 20, 1)
    EXPECT_TOKEN(lbc::TokenKind::LessOrEqual,    "<=",    1, 22, 2)
    EXPECT_TOKEN(lbc::TokenKind::Ellipsis,       "...",   1, 25, 3)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt,      "",      1, 28, 0)

    // line 2
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "FOUR",  2, 1,  4)
    EXPECT_TOKEN(lbc::TokenKind::If,             "IF",    2, 8,  2)
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "A",     2, 11, 1)
    EXPECT_TOKEN(lbc::TokenKind::Assign,         "=",     2, 13, 1)
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "B",     2, 15, 1)
    EXPECT_TOKEN(lbc::TokenKind::Then,           "THEN",  2, 17, 4)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt,      "",      2, 22, 0)

    // line 3
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "FIVE",  3, 1,  4)
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "SIX",   3, 23, 3)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt,      "",      3, 26, 0)

    // line 4
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "SEVEN", 4, 1,  5)

    // line 7
    EXPECT_TOKEN(lbc::TokenKind::Identifier,     "EIGHT", 7, 7,  5)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt,      "",      7, 12, 0)
    EXPECT_TOKEN(lbc::TokenKind::EndOfFile,      "",      7, 12, 0)

    // clang-format on
}

TEST_F(LexerTests, StringLiterals) {
    constexpr auto source = R"BAS(
"hello"
	"hello\nWorld!"
"	Hello \"world\"\t"
"Hello""World"
""
    )BAS";
    load(source);

    EXPECT_TOKEN(lbc::TokenKind::StringLiteral, "hello", 2, 1, 7)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)
    EXPECT_TOKEN(lbc::TokenKind::StringLiteral, "hello\nWorld!", 3, 2, 15)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)
    EXPECT_TOKEN(lbc::TokenKind::StringLiteral, "\tHello \"world\"\t", 4, 1, 20)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)
    EXPECT_TOKEN(lbc::TokenKind::StringLiteral, "Hello", 5, 1, 7)
    EXPECT_TOKEN(lbc::TokenKind::StringLiteral, "World", 5, 8, 7)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)
    EXPECT_TOKEN(lbc::TokenKind::StringLiteral, "", 6, 1, 2)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)
    EXPECT_TOKEN(lbc::TokenKind::EndOfFile)
}

TEST_F(LexerTests, NumberLiterals) {
    constexpr auto source = "10 .5 3.14 6. 0.10 5";
    load(source);

    // clang-format off
    EXPECT_TOKEN(lbc::TokenKind::IntegerLiteral,       "10",       1, 1, 2)
    EXPECT_TOKEN(lbc::TokenKind::FloatingPointLiteral, "0.500000", 1, 4, 2)
    EXPECT_TOKEN(lbc::TokenKind::FloatingPointLiteral, "3.140000", 1, 7, 4)
    EXPECT_TOKEN(lbc::TokenKind::FloatingPointLiteral, "6.000000", 1, 12, 2)
    EXPECT_TOKEN(lbc::TokenKind::FloatingPointLiteral, "0.100000", 1, 15, 4)
    EXPECT_TOKEN(lbc::TokenKind::IntegerLiteral,       "5",        1, 20, 1)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)
    EXPECT_TOKEN(lbc::TokenKind::EndOfFile)
    // clang-format on
}

TEST_F(LexerTests, MiscLiterals) {
    constexpr auto source = R"BAS(
true false null
True False Null
TRUE FALSE NULL
    )BAS";
    load(source);

    // clang-format off
    EXPECT_TOKEN(lbc::TokenKind::BooleanLiteral, "TRUE",  2, 1,  4)
    EXPECT_TOKEN(lbc::TokenKind::BooleanLiteral, "FALSE", 2, 6,  5)
    EXPECT_TOKEN(lbc::TokenKind::NullLiteral,    "NULL",  2, 12, 4)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)

    EXPECT_TOKEN(lbc::TokenKind::BooleanLiteral, "TRUE",  3, 1,  4)
    EXPECT_TOKEN(lbc::TokenKind::BooleanLiteral, "FALSE", 3, 6,  5)
    EXPECT_TOKEN(lbc::TokenKind::NullLiteral,    "NULL",  3, 12, 4)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)

    EXPECT_TOKEN(lbc::TokenKind::BooleanLiteral, "TRUE",  4, 1,  4)
    EXPECT_TOKEN(lbc::TokenKind::BooleanLiteral, "FALSE", 4, 6,  5)
    EXPECT_TOKEN(lbc::TokenKind::NullLiteral,    "NULL",  4, 12, 4)
    EXPECT_TOKEN(lbc::TokenKind::EndOfStmt)
    EXPECT_TOKEN(lbc::TokenKind::EndOfFile)
    // clang-format on
}

} // namespace
