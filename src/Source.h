//
//  Source.h
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

namespace lbc {
    
    // forward decl
    class SourceLocation;
    
    /**
     * Base class for Source providers (for exmaple SourceString and SourceFile)
     */
    class Source : NonCopyable
    {
    public:
        
        // character type used by the Source
        typedef char CharT;
        
        /**
         * Source iterator
         */
        class Iterator
        {
        public:
            
            // category
            typedef std::random_access_iterator_tag iterator_category;
            // type of the value
            typedef CharT                           value_type;
            // difference type
            typedef ptrdiff_t                       difference_type;
            // reference. BREAK iterator. no need for reference here in this context
            typedef CharT                     reference;
            // pointer
            typedef const CharT *                   pointer;
            // node
            typedef const CharT *                   nodeptr;
            
        private:
            friend class Source;
            
            // create new iterator
            // ensure that this constructor is private and can only be created
            // by the Source . Reason for this is to protect the Cache line generator
            Iterator(nodeptr p, Source * src) : m_p(p), m_src(src)
            {
                if (m_p != nullptr) {
                    // add first line
                    m_src->setLine(m_p);
                    
                    // CR + LF
                    if (*m_p == '\r' && m_p[1] == '\n') m_p++;
                }
            };
            
        public:
            
            /// Default constructor
            Iterator() : m_p(nullptr), m_src(nullptr) {};
            
            /// copy iterator
            Iterator(const Iterator & iter) : m_p(iter.m_p), m_src(iter.m_src) {};
            
            // the * operator
            inline reference operator * () const
            {
                // LLVM doesn't complain about returning a temporary reference
                // while GCC does. creating static const CharT fails in the linker...
                // CR + LF cases are taken care of in the constructor and in ++ operator
                if (*m_p == '\r') return '\n';
                return *m_p;
            }
            
            // the -> operator
            inline pointer operator -> () const { return m_p; }
            
            // advance on self
            inline Iterator & operator++()
            {
                m_p++;
                // CR + LF ?
                if (*m_p == '\r') {
                    if (m_p[1] == '\n') m_p++;
                    m_src->setLine(m_p + 1);
                } else if (*m_p == '\n') {
                    m_src->setLine(m_p + 1);
                }
                // return self
                return *this;
            }
            
            // advance tmp
            inline Iterator operator++(int)
            {
                Iterator tmp(*this);
                ++*this;
                return tmp;
            }
            
            // amove back
            inline Iterator & operator--()
            {
                // deal with CR + LF
                // IS this safe? what if it goes before 1st point?
                // Only way to be safe is to add another position counter...
                if (*m_p == '\n' && *(m_p - 1) == '\r') m_p--;
                // move back
                m_p--;
                // reference to self
                return *this;
            }
            
            // move back
            inline Iterator operator--(int)
            {
                Iterator tmp(*this);
                --*this;
                return tmp;
            }
            
            // self increment
            inline Iterator & operator +=(int n)
            {
                while(n--) ++*this;
                return *this;
            }
            
            // self decrement
            inline Iterator & operator -=(int n)
            {
                while(n--) --*this;
                return *this;
            }
            
            // subtract
            inline friend Iterator operator - (const Iterator & lhs, int n)
            {
                Iterator tmp(lhs);
                while(n--) tmp--;
                return tmp;
            }
            
            // get the difference
            // NB is there were CR + LF line breaks then size will inclode both while
            // * operator will skip one...
            inline friend difference_type operator - (const Iterator & lhs, const Iterator & rhs)
            {
                return lhs.m_p - rhs.m_p;
            }
            
            // add iter + n
            inline friend Iterator operator + (const Iterator & lhs, int n)
            {
                Iterator tmp(lhs);
                while(n--) tmp++;
                return tmp;
            }
            
            // add iter n + ter
            inline friend Iterator operator + (int n, const Iterator & rhs)
            {
                return rhs + n;
            }
            
            // reachable ? <
            inline friend bool operator < (const Iterator & lhs, const Iterator & rhs)
            {
                return lhs.m_p < rhs.m_p;
            }
            
            // random access. forward only. is this wrong???
            inline reference operator[] (int index) {
                return m_p[index];
            }
            
            // test eqiality
            friend bool operator == (const Iterator & lhs, const Iterator & rhs)
            {
                if (rhs.m_p == 0) return lhs.m_p == 0 || *(lhs.m_p) == '\0';
                if (lhs.m_p == 0) return *(rhs.m_p) == '\0';
                return lhs.m_p == rhs.m_p;
            }
            
            // test inequality
            friend bool operator != (const Iterator & lhs, const Iterator & rhs)
            {
                return !(lhs == rhs);
            }
            
            // the data pointer
            nodeptr m_p;
            
            // the reference to source
            Source * m_src;
        };
        
        /// const_iterator
        typedef Iterator const_iterator;
        friend class Iterator;
        
        /// Create instance of the Source
        Source(const std::string & name) : m_data(nullptr), m_name(name) {}
        
        /// Virtual destructor
        virtual ~Source() {}
        
        /// Get Source name
        inline const std::string & getName() const { return m_name; }
        
        /// begin iterator marker
        inline const_iterator begin() const {
            return const_iterator(m_data, const_cast<Source *>(this));
        }
        
        /// end iterator
        inline const_iterator end() const {
            return const_iterator(nullptr, const_cast<Source *>(this));
        }
        
        // Get line as a string
        // line numbers start from 1
        std::string getLine(unsigned int line);
        
        // get string
        std::string getString(const SourceLocation & loc);
        
    protected:
        
        /// pointer to character array
        CharT * m_data;
        
    private:
        typedef uint32_t OffsetType;
        
        /// Add cache line
        /// This relies on the fact that Iterator will call this in order from 0 position
        /// thus the line numbers should increase sequentally.
        /// lines are stored as offsets because on 64bit platforms it allows to save 4 bytes per line
        /// but also should make using buffered data easier as it stored an offset not a pointer
        inline void setLine(const CharT * p)
        {
            assert(p >= m_data && "Somethign is wrong. P can't be smaller!");
            OffsetType offs = (OffsetType)(p - m_data);
            if (m_lineCache.size() > 0 && offs <= m_lineCache.back()) return;
            m_lineCache.push_back(offs);
        }
        
        /// get line ptr
        const CharT * getLinePtr(unsigned int line);
        
        /// source name (can be filename or just a tag)
        std::string m_name;
        std::vector<OffsetType> m_lineCache;
    };

    
}
