//
// Created by Albert on 29/05/2021.
//
#include "TypeUdt.hpp"
#include "Driver/Context.hpp"
#include "Symbol/Symbol.hpp"
#include "Symbol/SymbolTable.hpp"
using namespace lbc;

namespace {
std::vector<unique_ptr<TypeUDT>> udts; // NOLINT
} // namespace

TypeUDT::TypeUDT(Symbol& symbol, SymbolTable& symbolTable)
: TypeRoot{ TypeFamily::UDT },
  m_symbol{ symbol },
  m_symbolTable{ symbolTable } {
    symbol.setType(this);
}

const TypeUDT* TypeUDT::get(Symbol& symbol, SymbolTable& symbolTable) {
    if (const auto* type = symbol.type()) {
        if (const auto* udt = dyn_cast<TypeUDT>(type)) {
            return udt;
        }
        fatalError("Symbol should hold UDT type pointer!");
    }
    return udts.emplace_back(new TypeUDT(symbol, symbolTable)).get(); // NOLINT
}

string TypeUDT::asString() const {
    return m_symbol.name().str();
}

llvm::Type* TypeUDT::genLlvmType(Context& context) const {
    llvm::LLVMContext& llvmContext = context.getLlvmContext();
    std::vector<llvm::Type*> elems;
    for (const auto& symbol : m_symbolTable) {
        auto* ty = symbol.second->type()->getLlvmType(context);
        elems.emplace_back(ty);
    }
    return llvm::StructType::create(llvmContext, elems, m_symbol.identifier());
}
