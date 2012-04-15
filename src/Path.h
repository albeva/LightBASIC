//
//  Path.h
//  LightBASIC
//
//  Created by Albert Varaksin on 15/04/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

namespace lbc {
    
/**
 * This class encapsulates simple path into querying & manipulation
 * primarily this makes use of llvm::sys::path and llvm::sys::fs
 * namespaces.
 */
class Path {
public:
    
    /**
     * Default
     */
    Path() {}
    
    /**
     * From a string
     */
    Path(std::string p) { path(p); }
    
    /**
     * Copy constructor
     */
    Path(const Path & p) { path(p); }
    
    /**
     * Check if this is valid
     * does not check if file/directory actually exists
     */
    bool isValid() const;
    
    /**
     * check if current file / directory exists
     */
    bool exists() const;
    
    /**
     * Return true if current path is absolute
     */
    bool isAbsolute() const;
    
    /**
     * Return true if current path is relative
     */
    bool isRelative() const;
    
    /**
     * Return true of current path points
     * to a valid directory
     */
    bool isDirectory() const;
    
    /**
     * Return true if current path points
     * to a valid and accessible file
     */
    bool isFile() const;
    
    /**
     * return true of path has a parent path
     */
    bool hasParent() const;
    
    /**
     * Return true of path has a filename
     */
    bool hasFilename() const;
    
    /**
     * Return true if path has an extension
     */
    bool hasExtension() const;
    
    /**
     * get currently held path string
     */
    const std::string & str() const { return m_path; }
    const char * c_str() const { return m_path.c_str(); }
    
    /**
     * Set new path string
     */
    Path & path(std::string p);
    Path & path(const Path & p) { return path(p.m_path); }
    Path & operator = (const Path & p) { path(p.m_path); return *this; }
    Path & operator = (const std::string & p) { path(p); return *this; }
    
    /**
     * get absolute path
     */
    std::string absolute() const;
    
    /**
     * Get currently hold file's extenions
     * or an empty string
     */
    std::string extension() const;
    
    /**
     * Set extension to the current file.
     * Ignores if this is not a file
     * and replaces existing extension
     */
    Path & extension(std::string);
    
    /**
     * Get parent path of the current path
     */
    std::string parent() const;
    
    /**
     * Set new parent to the current path
     */
    Path & parent(std::string p);
    Path & parent(const Path & p) { return parent(p.m_path); }
    
    /**
     * Append part to a directory or a file
     */
    Path & append(std::string p);
    Path & append(const Path & p) { return append(p.m_path); }
    
    /**
     * get current path file name or a directory name
     * without parent path
     */
    std::string filename() const;
    
    /**
     * set current path or file name.
     */
    Path & filename(std::string p);
    Path & filename(const Path & p) { return filename(p.m_path); }
    
    /**
     * test equality. This is simple string comparison and does
     * not compare if paths actually point to the same path
     */
    friend bool operator == (const Path & lhs, const Path & rhs) {
        return lhs.m_path == rhs.m_path;
    }
    
    
    /**
     * output stream.
     */
    friend std::ostream& operator<< (std::ostream& stream, const Path & p)
    {
        stream << '"' << p.m_path << '"';
        return stream;
    }
    
private:
    std::string m_path; // internally held path
};
    
} // ~lbc namespace
