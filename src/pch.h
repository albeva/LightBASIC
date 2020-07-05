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
using std::vector;
using namespace std::string_literals;

// LLVM
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Casting.h"

using llvm::isa;
using llvm::dyn_cast;

// BOOST
#include <boost/core/noncopyable.hpp>

using boost::noncopyable;

// APP
#include "Utils/Utils.h"
