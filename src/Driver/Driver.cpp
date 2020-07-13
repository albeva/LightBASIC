//
// Created by Albert on 13/07/2020.
//
#include "Driver.h"
using namespace lbc;

Driver::Driver() = default;
Driver::~Driver() = default;

string Driver::getOptionString(Driver::CompileResult result) {
    switch (result) {
    case CompileResult::None:
        return "";
    case CompileResult::Assembly:
        return "-S";
    case CompileResult::LLVMIr:
        return "-llvm";
    case CompileResult::Object:
        return "-C";
    case CompileResult::Executable:
    case CompileResult::Library:
        return "";
    }
}

string Driver::getOptionString(Driver::OptimizationLevel result) {
    switch (result) {
    case OptimizationLevel::O0:
        return "-O0";
    case OptimizationLevel::O1:
        return "-O1";
    case OptimizationLevel::O2:
        return "-O2";
    case OptimizationLevel::O3:
        return "-O3";
    }
}

string Driver::getOptionString(Driver::BuildMode mode) {
    switch (mode) {
    case BuildMode::Debug:
        return "-g";
    case BuildMode::Release:
        return "";
    }
}

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

string Driver::getOptionsString() const {
    return "";
}
