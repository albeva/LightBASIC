//
//  utils.cpp
//  LightBASIC
//
//  Created by Albert on 23/06/2012.
//  Copyright (c) 2012 Albert. All rights reserved.
//

using namespace lbc;

// create the exception
Exception::Exception(const std::string & message) : std::runtime_error(message) {}

Exception::~Exception() = default;
