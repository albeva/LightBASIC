//
// Created by Albert Varaksin on 18/04/2021.
//
#pragma once
#include "Driver/Toolchain/Toolchain.hpp"
#include "llvm/Support/Allocator.h"

namespace lbc {
class CompileOptions;

/**
 * Context holds various data and memory allocations required for the compilation process.
 * While it is not thread safe, as long as no more than 1 thread accesses it, it acts
 * similar to `thread_local` storage.
 */
class Context final {
public:
    NO_COPY_AND_MOVE(Context)

    explicit Context(const CompileOptions& options);
    ~Context() noexcept = default;

    [[nodiscard]] const CompileOptions& getOptions() noexcept { return m_options; }
    [[nodiscard]] Toolchain& getToolchain() noexcept { return m_toolchain; }
    [[nodiscard]] llvm::Triple& getTriple() noexcept { return m_triple; }
    [[nodiscard]] llvm::SourceMgr& getSourceMrg() noexcept { return m_sourceMgr; }
    [[nodiscard]] llvm::LLVMContext& getLlvmContext() noexcept { return m_llvmContext; }

    /**
     * Retain a copy of the string in the context and return a StringRef that
     * we can pass around safely without worry of it expiring (as long as context lives)
     * @param str string to retain
     */
    [[nodiscard]] StringRef retainCopy(StringRef str);

    /**
     * Store imported modules
     * @return true if module is newly added, false otherwise
     */
    [[nodiscard]] bool import(StringRef module);

    /**
     * Allocate memory, this memory is not expected to be deallocated
     */
    void* allocate(size_t bytes, unsigned alignment);

    /**
     * Allocate & construct object from the context
     * @tparam T object to create
     * @tparam Args arguments to pass to T constructor
     */
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        T* mem = allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }

private:
    const CompileOptions& m_options;
    Toolchain m_toolchain{ *this };

    llvm::Triple m_triple;
    llvm::SourceMgr m_sourceMgr{};
    llvm::LLVMContext m_llvmContext{};

    llvm::StringSet<> m_retainedStrings{};
    llvm::StringSet<> m_imports;

    // Allocations
    llvm::BumpPtrAllocator m_allocator;
};

} // namespace lbc
