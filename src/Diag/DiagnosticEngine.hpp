//
// Created by Albert on 03/06/2021.
//
#pragma once

namespace lbc {
class Context;

enum class Diag {
#define DIAG(LEVEL, ID, ...) ID,
#include "Diagnostics.def.hpp"
};

class DiagnosticEngine final {
public:
    NO_COPY_AND_MOVE(DiagnosticEngine)

    explicit DiagnosticEngine(Context& context) noexcept;
    ~DiagnosticEngine() noexcept = default;

    [[nodiscard]] bool hasErrors() const noexcept { return m_errorCounter > 0; }

    template<typename... Args>
    void report(Diag diag, llvm::SMRange range, Args... args) noexcept {
        auto formatted = llvm::formatv(
            getDiagnosticText(diag),
            std::forward<Args>(args)...);

        auto kind = getDiagKind(diag);
        if (kind == llvm::SourceMgr::DK_Error) {
            m_errorCounter++;
        }

        m_sourceMgr.PrintMessage(range.Start, kind, formatted.str(), range);
    }

    template<typename... Args>
    void report(Diag diag, llvm::SMLoc loc, Args... args) noexcept {
        auto formatted = llvm::formatv(
            getDiagnosticText(diag),
            std::forward<Args>(args)...);

        auto kind = getDiagKind(diag);
        if (kind == llvm::SourceMgr::DK_Error) {
            m_errorCounter++;
        }

        m_sourceMgr.PrintMessage(loc, kind, formatted.str());
    }

private:
    static const char* getDiagnosticText(Diag diag) noexcept;
    static llvm::SourceMgr::DiagKind getDiagKind(Diag diag) noexcept;

    Context& m_context;
    llvm::SourceMgr& m_sourceMgr;
    int m_errorCounter = 0;
};

} // namespace lbc