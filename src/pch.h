//
// Created by Albert on 03/07/2020.
//
#pragma once

// STL
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <optional>
#include <limits>
#include <cassert>

using std::string;
using std::string_view;
using std::make_unique;
using std::unique_ptr;
using namespace std::string_literals;

// LLVM
#include "llvm/Support/SourceMgr.h"

// BOOST
#include <boost/core/noncopyable.hpp>

using boost::noncopyable;

// APP
#include "Utils/Utils.h"
