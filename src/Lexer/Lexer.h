//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "pch.h"
#include "Token.h"

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
    [[nodiscard]] unique_ptr<Token> ellipsis() noexcept;
    [[nodiscard]] unique_ptr<Token> identifier() noexcept;
    [[nodiscard]] unique_ptr<Token> number() noexcept;
    [[nodiscard]] unique_ptr<Token> string() noexcept;
    [[nodiscard]] unique_ptr<Token> op(TokenKind kind) noexcept;
    [[nodiscard]] unique_ptr<Token> endOfStatement() noexcept;
    [[nodiscard]] unique_ptr<Token> endOfFile() noexcept;
    [[nodiscard]] static unique_ptr<Token> invalid(const char* loc) noexcept;

    void skipUntilLineEnd() noexcept;
    bool move() noexcept;
    bool move(int steps) noexcept;
    [[nodiscard]] bool isValid() const noexcept;
    [[nodiscard]] char peek(int ahead = 1) const noexcept;
    void handleLineEnd() noexcept;

    Context& m_context;
    const llvm::MemoryBuffer* m_buffer;
    const char* m_input;
    char m_char;
    bool m_hasStmt;
};

} // namespace lbc
