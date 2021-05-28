//
// Created by Albert Varaksin on 28/05/2021.
//
#include "Helpers.hpp"
#include "Lexer/Token.h"
#include "Type/Type.h"
using namespace lbc;
using namespace Gen;

llvm::CmpInst::Predicate lbc::Gen::getCmpPred(const TypeRoot* type, TokenKind op) noexcept {
    if (const auto* integral = dyn_cast<TypeIntegral>(type)) {
        bool isSigned = integral->isSigned();
        switch (op) {
        case TokenKind::Equal:
            return llvm::CmpInst::Predicate::ICMP_EQ;
        case TokenKind::NotEqual:
            return llvm::CmpInst::Predicate::ICMP_NE;
        case TokenKind::LessThan:
            return isSigned ? llvm::CmpInst::Predicate::ICMP_SLT : llvm::CmpInst::Predicate::ICMP_ULT;
        case TokenKind::LessOrEqual:
            return isSigned ? llvm::CmpInst::Predicate::ICMP_SLE : llvm::CmpInst::Predicate::ICMP_ULE;
        case TokenKind::GreaterOrEqual:
            return isSigned ? llvm::CmpInst::Predicate::ICMP_SGE : llvm::CmpInst::Predicate::ICMP_UGE;
        case TokenKind::GreaterThan:
            return isSigned ? llvm::CmpInst::Predicate::ICMP_SGT : llvm::CmpInst::Predicate::ICMP_UGT;
        default:
            llvm_unreachable("Unkown comparison op");
        }
    }

    if (type->isFloatingPoint()) {
        switch (op) {
        case TokenKind::Equal:
            return llvm::CmpInst::Predicate::FCMP_OEQ;
        case TokenKind::NotEqual:
            return llvm::CmpInst::Predicate::FCMP_UNE;
        case TokenKind::LessThan:
            return llvm::CmpInst::Predicate::FCMP_OLT;
        case TokenKind::LessOrEqual:
            return llvm::CmpInst::Predicate::FCMP_OLE;
        case TokenKind::GreaterOrEqual:
            return llvm::CmpInst::Predicate::FCMP_OGE;
        case TokenKind::GreaterThan:
            return llvm::CmpInst::Predicate::FCMP_OGT;
        default:
            llvm_unreachable("Unkown comparison op");
        }
    }

    if (type->isBoolean() || type->isPointer()) {
        switch (op) {
        case TokenKind::Equal:
            return llvm::CmpInst::Predicate::ICMP_EQ;
        case TokenKind::NotEqual:
            return llvm::CmpInst::Predicate::ICMP_NE;
        default:
            llvm_unreachable("Unkown comparison op");
        }
    }

    llvm_unreachable("Unsupported type");
}