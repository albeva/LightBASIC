#
# Makefile for lbc compiler
#

# Build type
BUILD		:= release
OS		:= $(shell uname)
ifeq ($(OS),Linux)
	# Linux specifics
	TOOLSET		:= gcc
else ifeq ($(OS),Darwin)
	# OSX specifics
	TOOLSET		:= clang
endif
# compiler and linker flags
CXXFLAGS	:= $(CXXFLAGS) -Wall -Werror -pedantic -Wextra -Os -MMD
LDFLAGS		:= $(LDFLAGS) -L/usr/local/lib
# llvm flags
LLVM_LIBS	:= core bitwriter
LLVM_CXXFLAGS	:= $(shell llvm-config --cxxflags)
LLVM_LDFLAGS	:= $(shell llvm-config --ldflags) \
		$(shell llvm-config --libs $(LLVM_LIBS))
CXXFLAGS	:= $(CXXFLAGS) $(LLVM_CXXFLAGS) -fexceptions
LDFLAGS		:= $(LDFLAGS) $(LLVM_LDFLAGS)
# input, output
TARGET_DIR	:= bin/$(BUILD)
TARGET		:= $(TARGET_DIR)/lbc
OBJDIR		:= obj/$(BUILD)
SOURCES		:= $(wildcard src/*.cpp)
OBJECTS		:= $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
DEPS		:= $(patsubst src/%.cpp,$(OBJDIR)/%.d,$(SOURCES)) \
		$(OBJDIR)/pch.hpp.d
PCH_FILE	:= pch.hpp
# deal with different compilers
ifeq ($(TOOLSET),clang)
	CXX		:= clang++
	LD		:= $(CXX)
	CXXFLAGS	:= $(CXXFLAGS) -std=c++11 -stdlib=libc++ \
			-Weverything \
			-Wno-c++98-compat \
			-Wno-c++98-compat-pedantic \
			-Wno-global-constructors \
			-Wno-exit-time-destructors \
			-Wno-padded \
			-Wno-switch-enum \
			-Wno-unused-macros
	LDFLAGS		:= $(LDFLAGS) -stdlib=libc++
	PCH		:= $(OBJDIR)/$(PCH_FILE).gch
	PCHFLAGS	:= -include-pch $(PCH)
else ifeq ($(TOOLSET),gcc)
	CXX		:= g++
	LD		:= $(CXX)
	CXXFLAGS	:= $(CXXFLAGS) -std=gnu++0x
	LDFLAGS		:= $(LDFLAGS) -ldl
	PCH		:= src/$(PCH_FILE).gch
	PCHFLAGS	:= -include src/$(PCH_FILE)
endif

# create paths
$(shell [ -d "$(OBJDIR)" ] || mkdir -p $(OBJDIR))
$(shell [ -d "$(TARGET_DIR)" ] || mkdir -p $(TARGET_DIR))

# disable checking for files
.PHONY: all clean test

# default target. Make the binary
all: $(TARGET)

# cleanup
clean:
	rm -f $(OBJECTS)
	rm -f $(DEPS)
	rm -f $(PCH)
	rm -f $(TARGET)

# Run the testcases
test: $(TARGET)
	cp -R samples bin/$(BUILD)/
	cd bin/$(BUILD)/samples/tests; ./run.sh

#include our project dependecy files
-include $(DEPS)

# link
$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

# precompiled header
$(PCH): src/$(PCH_FILE)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# compile .cpp to .o
$(OBJDIR)/%.o: src/%.cpp | $(PCH)
	$(CXX) $(CXXFLAGS) $(PCHFLAGS) -c -o $@ $<
