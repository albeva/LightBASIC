//
// Created by Albert on 03/06/2021.
//
#include "DiagnosticEngine.hpp"
#include "Driver/Context.hpp"
using namespace lbc;

namespace {
constexpr const char* messages[]{
#define DIAG(LEVEL, ID, STR) STR,
#include "Diagnostics.def.hpp"
};

constexpr llvm::SourceMgr::DiagKind diagKind[]{
#define DIAG(LEVEL, ID, STR) llvm::SourceMgr::DiagKind::DK_##LEVEL,
#include "Diagnostics.def.hpp"
};
} // namespace

DiagnosticEngine::DiagnosticEngine(Context& context) noexcept
: m_context{ context },
  m_sourceMgr{ context.getSourceMrg() } {}

const char* DiagnosticEngine::getDiagnosticText(Diag diag) noexcept {
    return messages[static_cast<size_t>(diag)];
}

llvm::SourceMgr::DiagKind DiagnosticEngine::getDiagKind(Diag diag) noexcept {
    return diagKind[static_cast<size_t>(diag)];
}

void DiagnosticEngine::print(Diag diag, llvm::SMLoc loc, const string& str, llvm::ArrayRef<llvm::SMRange> ranges) noexcept {
    auto kind = getDiagKind(diag);
    if (kind == llvm::SourceMgr::DK_Error) {
        m_errorCounter++;
    }
    m_sourceMgr.PrintMessage(loc, kind, str, ranges);
}
