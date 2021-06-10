//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once

namespace lbc {
class Token;
class Context;
enum class TokenKind;

class Lexer final {
public:
    NO_COPY_AND_MOVE(Lexer)

    Lexer(Context& context, unsigned fileID) noexcept;
    ~Lexer() noexcept = default;

    void next(Token& result);
    void peek(Token& result);

private:
    void skipUntilLineEnd() noexcept;
    void skipToNextLine() noexcept;
    void skipMultilineComment() noexcept;

    void endOfFile(Token& result) noexcept;
    void endOfStatement(Token& result) noexcept;
    void invalid(Token& result, const char* loc) const noexcept;
    void stringLiteral(Token& result);
    [[nodiscard]] char escape() noexcept;
    void token(Token& result, TokenKind kind, int len = 1) noexcept;
    void numberLiteral(Token& result);
    void identifier(Token& result);

    Context& m_context;
    const llvm::MemoryBuffer* m_buffer;
    const char* m_input;
    const char* m_eolPos;
    bool m_hasStmt;
};

} // namespace lbc
