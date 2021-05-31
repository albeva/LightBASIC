//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "pch.hpp"

namespace lbc {
class Token;
class Context;
enum class TokenKind;

class Lexer final {
public:
    NO_COPY_AND_MOVE(Lexer)

    Lexer(Context& context, unsigned fileID);
    ~Lexer() noexcept = default;

    [[nodiscard]] unique_ptr<Token> next();

private:
    void skipUntilLineEnd();
    void skipToNextLine();
    void skipMultilineComment();

    [[nodiscard]] unique_ptr<Token> endOfFile();
    [[nodiscard]] unique_ptr<Token> endOfStatement();
    [[nodiscard]] unique_ptr<Token> invalid(const char* loc) const noexcept;
    [[nodiscard]] unique_ptr<Token> stringLiteral();
    [[nodiscard]] char escape();
    [[nodiscard]] unique_ptr<Token> token(TokenKind kind, int len = 1);
    [[nodiscard]] unique_ptr<Token> numberLiteral();
    [[nodiscard]] unique_ptr<Token> identifier();

    Context& m_context;
    const llvm::MemoryBuffer* m_buffer;
    const char* m_input;
    const char* m_eolPos;
    bool m_hasStmt;
};

} // namespace lbc
