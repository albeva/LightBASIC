//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "Token.hpp"
#include "pch.hpp"

namespace lbc {

class Context;
enum class TokenKind;

class Lexer final {
public:
    NO_COPY_AND_MOVE(Lexer)

    Lexer(Context& context, unsigned fileID) noexcept;
    ~Lexer() noexcept = default;

    [[nodiscard]] unique_ptr<Token> next() noexcept;

private:
    void skipUntilLineEnd() noexcept;
    void skipToNextLine() noexcept;
    void skipMultilineComment() noexcept;

    [[nodiscard]] unique_ptr<Token> endOfFile() noexcept;
    [[nodiscard]] unique_ptr<Token> endOfStatement() noexcept;
    [[nodiscard]] unique_ptr<Token> invalid(const char* loc) const noexcept;
    [[nodiscard]] unique_ptr<Token> stringLiteral() noexcept;
    [[nodiscard]] char escape() noexcept;
    [[nodiscard]] unique_ptr<Token> token(TokenKind kind, int len = 1) noexcept;
    [[nodiscard]] unique_ptr<Token> numberLiteral() noexcept;
    [[nodiscard]] unique_ptr<Token> identifier() noexcept;

    Context& m_context;
    const llvm::MemoryBuffer* m_buffer;
    const char* m_input;
    const char* m_eolPos;
    bool m_hasStmt;
};

} // namespace lbc
