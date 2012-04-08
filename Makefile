#
# Makefile for lbc compiler
#

# Build type
BUILD			:= release
ifeq ($(OSTYPE),linx-gnu)
	TOOLSET			:= gcc
else
	TOOLSET			:= clang
endif
# compiler and linker flags
CXXFLAGS		:= -Wall -Werror -pedantic -Os -MMD
LDFLAGS			:= -L/usr/local/lib
# boost flags
BOOST_LDFLAGS	:= -lboost_system -lboost_filesystem
LDFLAGS			:= $(LDFLAGS) $(BOOST_LDFLAGS)
# llvm flags
LLVM_LIBS		:= core bitwriter
LLVM_CXXFLAGS	:= $(shell llvm-config --cxxflags)
LLVM_LDFLAGS	:= $(shell llvm-config --ldflags) \
				$(shell llvm-config --libs $(LLVM_LIBS))
CXXFLAGS		:= $(CXXFLAGS) $(LLVM_CXXFLAGS) -fexceptions
LDFLAGS			:= $(LDFLAGS) $(LLVM_LDFLAGS)
# input, output
TARGET_DIR		:= bin/$(BUILD)
TARGET			:= $(TARGET_DIR)/lbc
OBJDIR			:= obj/$(BUILD)
SOURCES			:= $(wildcard src/*.cpp)
OBJECTS			:= $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
DEPS			:= $(patsubst src/%.cpp,$(OBJDIR)/%.d,$(SOURCES)) \
				$(OBJDIR)/pch.d

# deal with different compilers
# clang
ifeq ($(TOOLSET),clang)
	CXX				:= clang++
	LD				:= $(CXX)
	CXXFLAGS		:= $(CXXFLAGS) -std=c++11 -stdlib=libc++
	LDFLAGS			:= $(LDFLAGS) -stdlib=libc++
	PCH				:= $(OBJDIR)/pch.hpp.gch
	PCHFLAGS		:= -include-pch $(PCH)
else ifeq ($(TOOLSET),gcc)
	CXX				:= g++
	LD				:= $(CXX)
	CXXFLAGS		:= $(CXXFLAGS) -std=gnu++0x
	LDFLAGS         := $(LDFLAGS) -ldl
	PCH				:= src/pch.hpp.gch
	PCHFLAGS		:= -include src/pch.hpp
endif

# disable checking for files
.PHONY: all clean test init-paths

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
	cp -R tests bin/$(BUILD)/
	cd bin/$(BUILD)/tests; ./run.sh

# Create folders
init-paths:
	mkdir -p $(OBJDIR)
	mkdir -p $(TARGET_DIR)

#include our project dependecy files
-include $(DEPS)

# link
$(TARGET): init-paths $(PCH) $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

# precompiled header
$(PCH): src/pch.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# compile .cpp to .o
$(OBJDIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(PCHFLAGS) -c -o $@ $<

