//
//  LightBASIC.h.pch
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

// C
#include <stdint.h>
#include <inttypes.h>

// c++11
#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

// llvm
#include <llvm/Config/config.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Bitcode/ReaderWriter.h>


// lbc internal
#include "Utils.h"
