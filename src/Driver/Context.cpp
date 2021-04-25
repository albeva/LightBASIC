//
// Created by Albert Varaksin on 18/04/2021.
//
#include "Context.h"
#include <llvm/Support/Host.h>
using namespace lbc;

Context::Context()
: m_triple{ llvm::sys::getDefaultTargetTriple() } {
}

void Context::validate() const noexcept {
    size_t count = std::accumulate(m_inputFiles.begin(), m_inputFiles.end(), size_t{}, [](auto cnt, const auto& vec) {
        return cnt + vec.size();
    });

    if (count == 0) {
        fatalError("no input.");
    }

    if (count > 1 && !isTargetLinkable() && !m_outputFilePath.empty()) {
        fatalError("cannot specify -o when generating multiple output files.");
    }

    if (m_outputType == OutputType::LLVM && isTargetNative()) {
        fatalError("flag -emit-llvm must be combined with -S or -c");
    }

    // .s > `.o`
    if (!getInputFiles(FileType::Assembly).empty()) {
        if (m_outputType == OutputType::LLVM) {
            fatalError("Cannot emit llvm from native assembly");
        }
        if (m_compilationTarget == CompilationTarget::Assembly) {
            fatalError("Invalid output: assembly to assembly");
        }
    }

    // .o > only native linkable target
    if (!getInputFiles(FileType::Object).empty()) {
        if (m_outputType == OutputType::LLVM) {
            fatalError("Cannot emit llvm from native objects");
        }
        if (!isTargetLinkable()) {
            fatalError(".o files can only be added to a linkable target");
        }
    }

    // .ll > everything
    if (!getInputFiles(FileType::LLVMIr).empty()) {
        if (m_outputType == OutputType::LLVM && m_compilationTarget == CompilationTarget::Assembly) {
            fatalError("Invalid output: llvm ir to llvm ir");
        }
    }

    // .bc > everything
    if (!getInputFiles(FileType::BitCode).empty()) {
        if (m_outputType == OutputType::LLVM && m_compilationTarget == CompilationTarget::Object) {
            fatalError("Invalid output: bitcode to bitcode");
        }
    }
}

string Context::getFileExt(FileType type) {
    switch (type) {
    case FileType::Source:
        return ".bas";
    case FileType::Assembly:
        return ".s";
    case FileType::Object:
        return ".o";
    case FileType::LLVMIr:
        return ".ll";
    case FileType::BitCode:
        return ".bc";
    default:
        llvm_unreachable("Invalid file type");
    }
}

void Context::addInputFile(const fs::path& path) {
    auto ext = path.extension();
    auto index = static_cast<size_t>(FileType::Source);

    for (size_t typeIdx = 0; typeIdx < fileTypeCount; typeIdx++) {
        if (getFileExt(static_cast<FileType>(typeIdx)) == ext) {
            index = typeIdx;
            break;
        }
    }

    m_inputFiles.at(index).emplace_back(path);
}

const std::vector<fs::path>& Context::getInputFiles(FileType type) const {
    return m_inputFiles.at(static_cast<size_t>(type));
}

void Context::setWorkingDir(const fs::path& path) {
    if (!path.is_absolute()) {
        fatalError("Working dir not a full path");
    }

    if (!fs::exists(path)) {
        fatalError("Working dir does not exist");
    }

    if (!fs::is_directory(path)) {
        fatalError("Working dir must point to a directory");
    }

    m_workingDir = path;
}

void Context::setOutputFilePath(const fs::path& path) {
    if (path.is_absolute()) {
        m_outputFilePath = path;
    } else {
        m_outputFilePath = fs::absolute(m_workingDir / path);
    }
}

fs::path Context::resolveOutputPath(const fs::path& path, const string& ext) const {
    if (!fs::exists(path)) {
        fatalError("File '"s + path.string() + "' not found");
    }
    if (!path.is_absolute()) {
        fatalError("Path '"s + path.string() + "' is not absolute");
    }
    if (fs::is_directory(path)) {
        fatalError("Path '"s + path.string() + "' is not a file");
    }

    if (m_outputFilePath.empty()) {
        fs::path output{ path };
        output.replace_extension(ext);
        return output;
    }

    fatalError("output path handling is not implemented");
}

fs::path Context::resolveFilePath(const fs::path& path) const {
    if (path.is_absolute()) {
        if (validateFile(path)) {
            return path;
        }
    } else if (auto relToWorkingDir = fs::absolute(m_workingDir / path); validateFile(relToWorkingDir)) {
        return relToWorkingDir;
    } else if (auto relToCompiler = fs::absolute(m_compilerPath / path); validateFile(relToCompiler)) {
        return relToCompiler;
    }

    fatalError("File '"s + path.string() + "' not found");
}

[[nodiscard]] bool Context::validateFile(const fs::path& path) {
    if (!fs::exists(path)) {
        return false;
    }

    if (!fs::is_regular_file(path)) {
        fatalError("File '"s + path.string() + "' is not regular");
    }

    return true;
}
