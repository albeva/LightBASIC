//
// Created by Albert Varaksin on 01/05/2021.
//
#pragma once


namespace lbc {
class SymbolTable;
class Symbol;
class Context;
struct AstModule;
struct AstFuncDecl;
struct AstFuncParamDecl;

namespace Sem {
    class TypePass;

    /**
     * Semantic pass that declares all the functions
     * and declarations in the ast
     */
    class FuncDeclarerPass final {
    public:
        NO_COPY_AND_MOVE(FuncDeclarerPass)

        explicit FuncDeclarerPass(
            Context& context,
            TypePass& typePass) noexcept
        : m_context{ context },
          m_typePass{ typePass } {}

        ~FuncDeclarerPass() noexcept = default;

        void visit(AstModule& ast);

    private:
        void visitFuncDecl(AstFuncDecl& ast, bool external);
        void visitFuncParamDecl(AstFuncParamDecl& ast);
        [[nodiscard]] Symbol* createParamSymbol(AstFuncParamDecl& ast);

        SymbolTable* m_table{};
        Context& m_context;
        TypePass& m_typePass;
    };

} // namespace Sem
} // namespace lbc
