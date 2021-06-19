//
// Created by Albert Varaksin on 18/04/2021.
//
#include "Context.hpp"
#include "CompileOptions.hpp"
#include "Diag/DiagnosticEngine.hpp"
#include "Driver/Toolchain/Toolchain.hpp"
#include <llvm/Support/Host.h>
using namespace lbc;

struct Context::Pimpl {
    Pimpl(Context& context) noexcept
    : diag{ context },
      toolchain{ context } {}

    DiagnosticEngine diag;
    Toolchain toolchain;
};

Context::Context(const CompileOptions& options)
: m_pimpl{ std::make_unique<Pimpl>(*this) },
  m_options{ options },
  m_diag{ m_pimpl->diag },
  m_toolchain{ m_pimpl->toolchain },
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

Context::~Context() noexcept = default;

StringRef Context::retainCopy(StringRef str) {
    return m_retainedStrings.insert(str).first->first();
}

bool Context::import(StringRef module) {
    return m_imports.insert(module).second;
}
