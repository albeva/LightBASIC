//
// Created by Albert on 05/07/2020.
//
#include "CodeGen.h"
#include "Ast/Ast.h"
#include "Lexer/Token.h"
#include "llvm/IR/IRPrintingPasses.h"
using namespace lbc;

[[noreturn]]
static void error(const string& message) {
    std::cerr << message << std::endl;
    std::exit(EXIT_FAILURE);
}

CodeGen::CodeGen()
: m_context{},
  m_builder{m_context},
  m_module{nullptr},
  m_value{nullptr} {

}

CodeGen::~CodeGen() {
}

void CodeGen::visit(const AstProgram *ast) {
    m_module = make_unique<llvm::Module>("app", m_context);
    m_module->setTargetTriple(llvm::sys::getDefaultTargetTriple());

    ast->body->accept(this);

    if (llvm::verifyModule(*m_module, &llvm::outs())) {
        std::cerr << "Failed to verify modeul" << '\n';
        std::exit(EXIT_FAILURE);
    }

    auto *printer = llvm::createPrintModulePass(llvm::outs());
    printer->runOnModule(*m_module);
}

void CodeGen::visit(const AstStmtList *ast) {
    for (const auto& stmt: ast->stmts)
        stmt->accept(this);
}

void CodeGen::visit(const AstAssignStmt *ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(const AstExprStmt *ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(const AstVarDecl *ast) {
    llvm::Constant* constant = nullptr;
    if (const auto *expr = dyn_cast<AstLiteralExpr>(ast->expr.get())) {
        switch (expr->token->kind()) {
        case TokenKind::StringLiteral:
            constant = llvm::ConstantDataArray::getString(
                m_context,
                view_to_stringRef(expr->token->lexeme()),
                true
            );
            break;
        default:
            error("Unsupported expression type");
            break;
        }
    } else {
        error("Unsupported expression type");
    }

    auto *value = new llvm::GlobalVariable(
        *m_module,
        constant->getType(),
        true,
        llvm::GlobalValue::PrivateLinkage,
        constant,
        std::string(ast->ident->token->lexeme())
    );
}

void CodeGen::visit(const AstAttributeList *ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(const AstAttribute *ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(const AstIdentExpr *ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(const AstCallExpr *ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(const AstLiteralExpr *ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}


