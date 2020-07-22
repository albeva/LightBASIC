//
// Created by Albert on 03/07/2020.
//
#pragma once

// STL
#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <any>

using std::make_unique;
using std::string;
using std::string_view;
using std::unique_ptr;
namespace fs = std::filesystem;
using namespace std::literals::string_literals;

// LLVM
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

using llvm::dyn_cast;
using llvm::isa;

// BOOST

// APP
#include "Utils/Utils.h"
#include "Utils/Version.h"
