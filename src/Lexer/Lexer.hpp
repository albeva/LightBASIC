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

    [[nodiscard]] unique_ptr<Token> next();

private:
    void skipUntilLineEnd() noexcept;
    void skipToNextLine() noexcept;
    void skipMultilineComment() noexcept;

    [[nodiscard]] unique_ptr<Token> endOfFile();
    [[nodiscard]] unique_ptr<Token> endOfStatement();
    [[nodiscard]] unique_ptr<Token> invalid(const char* loc) const;
    [[nodiscard]] unique_ptr<Token> stringLiteral();
    [[nodiscard]] char escape() noexcept;
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
