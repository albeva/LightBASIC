//
//  LightBASIC.h.pch
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//


// use c++11 standard library for std::shared_ptr, hash maps, etc...
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <cctype>
#include <string>
#include <inttypes.h>
#include <unordered_map>
// boost libraries
#include <boost/filesystem.hpp>
#include <boost/pool/pool.hpp>
#include <boost/algorithm/string/join.hpp>

// llvm
#include "stdint.h"
#include <llvm/Config/config.h>
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/IRBuilder.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>

#include "Utils.h"
