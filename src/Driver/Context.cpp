//
// Created by Albert Varaksin on 18/04/2021.
//
#include "Context.hpp"
#include "CompileOptions.hpp"
#include <llvm/Support/Host.h>
using namespace lbc;

Context::Context(const CompileOptions& options)
: m_options{ options },
  m_triple{ llvm::sys::getDefaultTargetTriple() } {
    if (m_options.is64Bit()) {
        m_triple = m_triple.get64BitArchVariant();
    } else {
        m_triple = m_triple.get32BitArchVariant();
    }

    if (!m_options.getToolchainDir().empty()) {
        m_toolchain.setBasePath(m_options.getToolchainDir());
    }
}

StringRef Context::retainCopy(StringRef str) {
    return m_retainedStrings.insert(str).first->first();
}

bool Context::import(StringRef module) {
    return m_imports.insert(module).second;
}
