//
//  Path.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 15/04/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "Path.h"
#include <llvm/Support/Path.h>
#include <llvm/Support/FileSystem.h>
using namespace lbc;


/**
 * Check if this is a valid path
 * only check that string is set. not if the file/fir
 * actually exists
 */
bool Path::isValid() const
{
    return m_path.length() > 0; 
}


/**
 * check if current file / directory exists
 */
bool Path::exists() const
{
    bool result = false;
    if (llvm::sys::fs::exists(m_path, result) == llvm::errc::success) {
        return result;
    }
    return false;
}


/**
 * Return true if current path is absolute
 */
bool Path::isAbsolute() const
{
    return llvm::sys::path::is_absolute(m_path);
}


/**
 * Return true if current path is relative
 */
bool Path::isRelative() const
{
    return llvm::sys::path::is_relative(m_path);
}


/**
 * Return true of current path points
 * to a valid directory
 */
bool Path::isDirectory() const
{
    bool result = false;
    if (llvm::sys::fs::is_directory(m_path, result) == llvm::errc::success) {
        return result;
    }
    return false;
}


/**
 * Return true if current path points
 * to a valid and accessible file
 */
bool Path::isFile() const
{
    bool result = false;
    if (llvm::sys::fs::is_regular_file(m_path, result) == llvm::errc::success) {
        return result;
    }
    return false;
}


/**
 * return true of path has a parent path
 */
bool Path::hasParent() const
{
    return llvm::sys::path::has_parent_path(m_path);
}


/**
 * Return true of path has a filename
 */
bool Path::hasFilename() const
{
    return llvm::sys::path::has_filename(m_path);
}


/**
 * Return true if path has an extension
 */
bool Path::hasExtension() const
{
    return llvm::sys::path::has_extension(m_path);
}


/**
 * get native path
 */
Path & Path::path(std::string p)
{
    llvm::SmallString<128> result;
    llvm::sys::path::native(p, result);
    m_path = result.str();
    return *this;
}


/**
 * get absolute path
 */
std::string Path::absolute() const
{
    llvm::SmallString<128> result(m_path);
    llvm::sys::fs::make_absolute(result);
    return result.str();
}


/**
 * Get currently hold file's extenions
 * or an empty string
 */
std::string Path::extension() const
{
    return llvm::sys::path::extension(m_path);
}


/**
 * Set extension to the current file.
 * Ignores if this is not a file
 * and replaces existing extension
 */
Path & Path::extension(std::string ext)
{
    llvm::SmallString<128> tmp(m_path);
    llvm::sys::path::replace_extension(tmp, ext);
    m_path = tmp.str();
    return *this;
}


/**
 * Get parent path of the current path
 */
std::string Path::parent() const
{
    return llvm::sys::path::parent_path(m_path);
}


/**
 * Set new parent to the current path
 */
Path & Path::parent(std::string p)
{
    if (p.length() && !llvm::sys::path::is_separator(p[p.length()-1])) p += '/';
    path(p + filename());
    return *this;
}


/**
 * Append part to a directory or a file
 */
Path & Path::append(std::string p)
{
    llvm::SmallString<128> tmp(m_path);
    llvm::sys::path::append(tmp, p);
    m_path = tmp.str();
    return *this;
}


/**
 * get current path file name or a directory name
 * without parent path
 */
std::string Path::filename() const
{
    return llvm::sys::path::filename(m_path);
}


/**
 * set current path or file name. */
Path & Path::filename(std::string p)
{
    path(parent() + '/' + p);
    return *this;
}
