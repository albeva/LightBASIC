//
// Created by Albert on 03/06/2021.
//
#pragma once

namespace lbc {
class Context;

class DiagnosticEngine final {
public:
    NO_COPY_AND_MOVE(DiagnosticEngine)
    explicit DiagnosticEngine(Context& context) noexcept;
    ~DiagnosticEngine() noexcept = default;

private:
    Context& m_context;
    llvm::SourceMgr& m_sourceMgr;
};

} // namespace lbc