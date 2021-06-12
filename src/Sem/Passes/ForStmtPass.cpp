//
// Created by Albert on 29/05/2021.
//
#include "ForStmtPass.hpp"
#include "Ast/Ast.hpp"
#include "Sem/SemanticAnalyzer.hpp"
#include "Driver/Context.hpp"
#include "Type/Type.hpp"
using namespace lbc;
using namespace Sem;

ForStmtPass::ForStmtPass(SemanticAnalyzer& sem, AstForStmt& ast)
: m_sem{ sem }, m_ast{ ast } {
    auto* current = sem.getSymbolTable();
    m_ast.symbolTable = sem.getContext().create<SymbolTable>(current);
    sem.setSymbolTable(m_ast.symbolTable);

    ceclare();
    analyze();
    determineForDirection();

    sem.setSymbolTable(current);
}

void ForStmtPass::ceclare() {
    for (auto& var : m_ast.decls) {
        m_sem.visit(*var);
    }
    m_sem.visit(*m_ast.iterator);
    auto flags = m_ast.iterator->symbol->getFlags();
    flags.dereferencable = false;
    flags.addressable = false;
    flags.assignable = false;
    m_ast.iterator->symbol->setFlags(flags);
}

void ForStmtPass::analyze() {
    m_sem.expression(m_ast.limit);

    if (m_ast.step) {
        m_sem.expression(m_ast.step);
    }

    const auto* type = m_ast.iterator->symbol->type();
    if (!type->isNumeric()) {
        fatalError("NEXT iterator must be of numeric type");
    }

    // type TO type check
    switch (type->compare(m_ast.limit->type)) {
    case TypeComparison::Incompatible:
        fatalError("Incompatible types in FOR");
    case TypeComparison::Downcast:
        m_sem.convert(m_ast.limit, type);
        break;
    case TypeComparison::Equal:
        break;
    case TypeComparison::Upcast:
        if (m_ast.iterator->typeExpr) {
            m_sem.convert(m_ast.limit, type);
        } else {
            m_sem.convert(m_ast.iterator->expr, m_ast.limit->type);
            m_ast.iterator->symbol->setType(m_ast.limit->type);
        }
        break;
    }

    // type STEP type check
    if (m_ast.step) {
        switch (type->compare(m_ast.step->type)) {
        case TypeComparison::Incompatible:
            fatalError("Incompatible types in STEP");
        case TypeComparison::Downcast:
        case TypeComparison::Upcast: {
            const auto* dstTy = type;
            const auto* iterTy = dyn_cast<TypeIntegral>(type);
            if (iterTy != nullptr && !iterTy->isSigned()) {
                if (const auto* stepIntTy = dyn_cast<TypeIntegral>(m_ast.step->type)) {
                    if (stepIntTy->isSigned()) {
                        if (auto* literal = dyn_cast<AstLiteralExpr>(m_ast.step)) {
                            if (static_cast<int64_t>(std::get<uint64_t>(literal->value)) < 0) {
                                dstTy = iterTy->getSigned();
                            }
                        } else {
                            dstTy = iterTy->getSigned();
                        }
                    }
                } else if (isa<TypeFloatingPoint>(m_ast.step->type)) {
                    if (auto* literal = dyn_cast<AstLiteralExpr>(m_ast.step)) {
                        if (std::get<double>(literal->value) < 0.0) {
                            dstTy = iterTy->getSigned();
                        }
                    } else {
                        dstTy = iterTy->getSigned();
                    }
                }
            }
            m_sem.convert(m_ast.step, dstTy);
            break;
        }
        case TypeComparison::Equal:
            break;
        }
    }

    m_sem.getControlStack().push(ControlFlowStatement::For);
    m_sem.visit(*m_ast.stmt);
    m_sem.getControlStack().pop();

    if (!m_ast.next.empty()) {
        if (m_ast.next != m_ast.iterator->name) {
            fatalError("NEXT iterator names must match");
        }
    }
}

void ForStmtPass::determineForDirection() {
    auto* from = dyn_cast<AstLiteralExpr>(m_ast.iterator->expr);
    auto* to = dyn_cast<AstLiteralExpr>(m_ast.limit);
    const auto* type = m_ast.iterator->symbol->type();
    bool equal = false;

    if (from != nullptr && to != nullptr) {
        if (const auto* integral = dyn_cast<TypeIntegral>(type)) {
            auto lhs = std::get<uint64_t>(from->value);
            auto rhs = std::get<uint64_t>(to->value);
            if (lhs == rhs) {
                m_ast.direction = AstForStmt::Direction::Increment;
                equal = true;
            } else if (integral->isSigned()) {
                auto slhs = static_cast<int64_t>(lhs);
                auto srhs = static_cast<int64_t>(rhs);
                if (slhs < srhs) {
                    m_ast.direction = AstForStmt::Direction::Increment;
                } else if (slhs > srhs) {
                    m_ast.direction = AstForStmt::Direction::Decrement;
                }
            } else {
                if (lhs < rhs) {
                    m_ast.direction = AstForStmt::Direction::Increment;
                } else if (lhs > rhs) {
                    m_ast.direction = AstForStmt::Direction::Decrement;
                }
            }
        } else if (isa<TypeFloatingPoint>(type)) {
            auto lhs = std::get<double>(from->value);
            auto rhs = std::get<double>(to->value);
            if (lhs == rhs) {
                m_ast.direction = AstForStmt::Direction::Increment;
                equal = true;
            } else if (lhs < rhs) {
                m_ast.direction = AstForStmt::Direction::Increment;
            } else {
                m_ast.direction = AstForStmt::Direction::Decrement;
            }
        }
    }

    if (!m_ast.step) {
        return;
    }

    auto* step = dyn_cast<AstLiteralExpr>(m_ast.step);
    if (step == nullptr) {
        return;
    }

    if (step->type->isSignedIntegral()) {
        auto val = static_cast<int64_t>(std::get<uint64_t>(step->value));
        if (val < 0) {
            if (m_ast.direction == AstForStmt::Direction::Increment) {
                if (equal) {
                    m_ast.direction = AstForStmt::Direction::Decrement;
                } else {
                    m_ast.direction = AstForStmt::Direction::Skip;
                }
            } else if (m_ast.direction == AstForStmt::Direction::Unknown) {
                m_ast.direction = AstForStmt::Direction::Decrement;
            }
        } else if (val > 0) {
            if (m_ast.direction == AstForStmt::Direction::Decrement) {
                m_ast.direction = AstForStmt::Direction::Skip;
            } else if (m_ast.direction == AstForStmt::Direction::Unknown) {
                m_ast.direction = AstForStmt::Direction::Increment;
            }
        } else {
            // TODO emit warning
            if (m_ast.direction == AstForStmt::Direction::Unknown) {
                m_ast.direction = AstForStmt::Direction::Increment;
            }
        }
    } else if (step->type->isUnsignedIntegral()) {
        auto val = std::get<uint64_t>(step->value);
        if (val == 0 || m_ast.direction == AstForStmt::Direction::Decrement) {
            m_ast.direction = AstForStmt::Direction::Skip;
        } else if (m_ast.direction == AstForStmt::Direction::Unknown) {
            m_ast.direction = AstForStmt::Direction::Increment;
        }
    } else if (step->type->isFloatingPoint()) {
        auto val = std::get<double>(step->value);
        if (val < 0.0) {
            if (m_ast.direction == AstForStmt::Direction::Increment) {
                if (equal) {
                    m_ast.direction = AstForStmt::Direction::Decrement;
                } else {
                    m_ast.direction = AstForStmt::Direction::Skip;
                }
            } else if (m_ast.direction == AstForStmt::Direction::Unknown) {
                m_ast.direction = AstForStmt::Direction::Decrement;
            }
        } else if (val > 0.0) {
            if (m_ast.direction == AstForStmt::Direction::Decrement) {
                m_ast.direction = AstForStmt::Direction::Skip;
            } else if (m_ast.direction == AstForStmt::Direction::Unknown) {
                m_ast.direction = AstForStmt::Direction::Increment;
            }
        } else {
            // TODO emit warning
            if (m_ast.direction == AstForStmt::Direction::Unknown) {
                m_ast.direction = AstForStmt::Direction::Increment;
            }
        }
    }
}
