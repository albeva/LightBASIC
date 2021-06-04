//
// Created by Albert on 03/06/2021.
//
#include "DiagnosticEngine.hpp"
#include "Driver/Context.hpp"
using namespace lbc;

DiagnosticEngine::DiagnosticEngine(Context& context) noexcept
: m_context{ context },
  m_sourceMgr{ context.getSourceMrg() } {}
