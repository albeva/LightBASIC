//
// Created by Albert on 18/04/2021.
//
#include "Context.h"
#include <llvm/Support/Host.h>
using namespace lbc;

Context::Context()
: m_triple{ llvm::sys::getDefaultTargetTriple() } {
}

Context::~Context() = default;

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
    FileType type = FileType::Source;

    constexpr std::array types{
        FileType::Assembly,
        FileType::Object,
        FileType::LLVMIr,
        FileType::BitCode
    };
    for (auto ft : types) {
        if (getFileExt(ft) == ext) {
            type = ft;
            break;
        }
    }

    m_inputFiles.at(static_cast<size_t>(type)).emplace_back(path);
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

fs::path Context::resolveOutputPath(const fs::path& path, const string& ext, bool single, bool final) const {
    if (m_outputFilePath.empty()) {
        fs::path output{ path };
        output.replace_extension(ext);
        if (!output.is_absolute()) {
            output = fs::absolute(m_workingDir / output);
        }
        fs::create_directories(output.parent_path());
        return output;
    }

    if (fs::is_directory(m_outputFilePath) || m_outputFilePath.filename().empty()) {
        fs::path relative = fs::relative(path, m_workingDir);
        relative.replace_extension(ext);
        auto fullPath = fs::absolute(m_outputFilePath / relative);
        fs::create_directories(fullPath.parent_path());
        return fullPath;
    }

    if (!final) {
        fs::path relative = fs::relative(path, m_workingDir);
        relative.replace_extension(ext);
        auto fullPath = fs::absolute(m_workingDir / relative);
        fs::create_directories(fullPath.parent_path());
        return fullPath;
    }

    if (!single) {
        fatalError("Output path '"s + m_outputFilePath.string() + "' can have only one input file");
    }

    fs::create_directories(m_outputFilePath.parent_path());
    return m_outputFilePath;
}

fs::path Context::resolvePath(const fs::path& path) const {
    if (path.is_absolute()) {
        if (validateFile(path, true)) {
            return path;
        }
    }

    auto p = fs::absolute(m_workingDir / path);
    if (validateFile(p, false)) {
        return p;
    }

    if (validateFile(path, true)) {
        return path;
    }

    llvm_unreachable("file resolving failed");
}

[[nodiscard]] bool Context::validateFile(const fs::path& path, bool mustExist) {
    if (!fs::exists(path)) {
        if (mustExist) {
            fatalError("File '"s + path.string() + "' not found");
        }
        return false;
    }

    if (!fs::is_regular_file(path)) {
        fatalError("File '"s + path.string() + "' is not regular");
    }

    return true;
}