//
// Created by Albert Varaksin on 23/05/2021.
//
#if defined(_MSC_VER)
#    pragma warning(disable : 4068)
#endif
#pragma ide diagnostic ignored "cppcoreguidelines-avoid-non-const-global-variables"
#pragma ide diagnostic ignored "cppcoreguidelines-owning-memory"
#pragma ide diagnostic ignored "cppcoreguidelines-non-private-member-variables-in-classes"
#pragma ide diagnostic ignored "cert-err58-cpp"
#pragma ide diagnostic ignored "cppcoreguidelines-special-member-functions"

#include "Driver/Context.h"
#include "Lexer/Lexer.h"
#include <gtest/gtest.h>
namespace {

class LexerTests: public testing::Test {
protected:
    unsigned load(StringRef source) {
        auto& srcMgr = context.getSourceMrg();
        auto buffer = llvm::MemoryBuffer::getMemBuffer(source);
        return srcMgr.AddNewSourceBuffer(std::move(buffer), {});
    }

    unique_ptr<lbc::Lexer> getLexer(unsigned fileId) {
        return make_unique<lbc::Lexer>(context, fileId);
    }

    lbc::Context context{};
};

TEST_F(LexerTests, NoInput) {
    auto lexer = getLexer(load(""));
    EXPECT_TRUE(lexer->next()->kind() == lbc::TokenKind::EndOfFile);
    EXPECT_TRUE(lexer->next()->kind() == lbc::TokenKind::EndOfFile);
}

TEST_F(LexerTests, EmptyInputs) {
    constexpr std::array inputs {
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
    for (const auto* input: inputs) {
        auto lexer = getLexer(load(input));
        auto token = lexer->next();
        EXPECT_TRUE(token->kind() == lbc::TokenKind::EndOfFile);
    }
}

} // namespace
