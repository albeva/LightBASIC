//
//  Utils.h
//  LightBASIC
//
//  Created by Albert Varaksin on 07/04/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

// some helpers
namespace lbc {
    
    // namespace shortcuts
    namespace FS = boost::filesystem;
    
    /**
     * Base class for exceptions
     */
    struct Exception : public std::runtime_error
    {
        // Create new instance of Exception
        explicit Exception(const std::string & message) : std::runtime_error(message) {}
    };
    
    #define STRINGIFY_IMPL(v) #v
    #define STRINGIFY(v) STRINGIFY_IMPL(v)
    // #define THROW_EXCEPTION(_msg) throw Exception(std::string(_msg) + ". \n" + __FILE__ + "(" + STRINGIFY(__LINE__) + ")\n" + __PRETTY_FUNCTION__);
    #define THROW_EXCEPTION(_msg)
    
    /**
     * NonCopyable c++11 style
     */
    struct NonCopyable
    {
        NonCopyable() = default;
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable & operator = (const NonCopyable &) = delete;
    };
    
    /**
     * Simple helper. Basically ensured that original
     * value of a variable provided is restored apon
     * exiting the scope
     */
    template<typename T> struct ScopeGuard : NonCopyable
    {
        
        // create
        ScopeGuard(T & value) : m_target(value), m_value(value)
        {
        }
        
        // restore
        ~ScopeGuard() {
            m_target = m_value;
        }
        
        // members
        T & m_target;
        T   m_value;
    };
    
    #define CONCATENATE_DETAIL(x, y) x##y
    #define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
    #define MAKE_UNIQUE(x) CONCATENATE(x, __COUNTER__)
    #define SCOPED_GUARD(V) ScopeGuard<decltype(V)> MAKE_UNIQUE(_tmp_guard) (V);
}
