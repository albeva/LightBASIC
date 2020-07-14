//
// Created by Albert on 13/07/2020.
//
#include "Driver.h"
using namespace lbc;

Driver::Driver()
: m_triple{ llvm::sys::getDefaultTargetTriple() } {
}

Driver::~Driver() = default;

void Driver::addResource(Driver::ResourceType type, const fs::path& path) {
    auto index = static_cast<size_t>(type);
    m_resources.at(index).insert(path);
}

const Driver::ResourceContainer& Driver::getResources(Driver::ResourceType type) const {
    auto index = static_cast<size_t>(type);
    return m_resources.at(index);
}

void Driver::setTool(Driver::Tool tool, const fs::path& path) {
    auto index = static_cast<size_t>(tool);
    m_tools.at(index) = path;
}

const fs::path& Driver::getTool(Driver::Tool tool) const {
    auto index = static_cast<size_t>(tool);
    return m_tools.at(index);
}

int Driver::execute() {
    return EXIT_SUCCESS;
}
