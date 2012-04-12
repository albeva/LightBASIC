//
//  MemoryPool.h
//  LightBASIC
//
//  Created by Albert Varaksin on 12/04/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once
#include <llvm/Support/Allocator.h>
#include <llvm/Support/Recycler.h>

namespace lbc {

/**
 * Simple memory pool using llvm::Recycler and llvm::BumpPtrAllocator
 */
template<class T> class MemoryPool {
    /**
     * The recucler object
     */
    llvm::Recycler<T, sizeof(T), llvm::AlignOf<T>::Alignment> m_recycler;
    /**
     * Memory allocator
     */
    llvm::BumpPtrAllocator m_allocator;
public:
    /**
     * Clean up any used memory
     */
    ~MemoryPool()
    {
        m_recycler.clear(m_allocator);
    }
    
    /**
     * Allocate objct
     */
    inline T * allocate() {
        return m_recycler.Allocate(m_allocator);
    }
    
    /**
     * Deallocate
     */
    inline void deallocate(void * obj) {
        m_recycler.Deallocate(m_allocator, obj);
    }
};
    
} // ~lbc namespace
