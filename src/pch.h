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
#include <unordered_map>
#include <unordered_set>

using std::string;
using std::string_view;
using std::make_unique;
using std::unique_ptr;
using std::vector;
using namespace std::string_literals;

// LLVM
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Casting.h"
#include <llvm/Config/llvm-config.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Triple.h>
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Value.h"

using llvm::isa;
using llvm::dyn_cast;

// BOOST
#include <boost/core/noncopyable.hpp>

using boost::noncopyable;

// APP
#include "Utils/Utils.h"
