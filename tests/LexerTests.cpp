//
// Created by Albert Varaksin on 23/05/2021.
//
#include "Driver/Context.h"
#include "Lexer/Lexer.h"
#include <gtest/gtest.h>
namespace {

TEST(LexerTests, EmptyInput) { // NOLINT
    auto context = lbc::Context();
    auto& srcMgr = context.getSourceMrg();
    auto buffer = llvm::MemoryBuffer::getMemBuffer("");
    auto fileId = srcMgr.AddNewSourceBuffer(std::move(buffer), {});
    auto lexer = lbc::Lexer(context, fileId);

    EXPECT_TRUE(lexer.next()->kind() == lbc::TokenKind::EndOfFile);
    EXPECT_TRUE(lexer.next()->kind() == lbc::TokenKind::EndOfFile);
}

} // namespace
