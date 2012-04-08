#
# Makefile for lbc compiler
#

# Build type
BUILD			:= release
ARCH			:= x86_64
# compiler and linker flags
CXX				:= c++
CXXFLAGS		:= -Wall -Werror -pedantic -Os -std=c++11 -stdlib=libc++ \
				-arch $(ARCH) -MMD
LDFLAGS			:= -arch $(ARCH) -stdlib=libc++
# boost flags
BOOST_LDFLAGS	:= -lboost_filesystem -lboost_system
LDFLAGS			:= $(LDFLAGS) $(BOOST_LDFLAGS)
# llvm flags
LLVM_LIBS		:= core bitwriter
LLVM_CXXFLAGS	:= $(shell llvm-config --cxxflags)
LLVM_LDFLAGS	:= $(shell llvm-config --ldflags) \
				$(shell llvm-config --libs $(LLVM_LIBS))
CXXFLAGS		:= $(CXXFLAGS) $(LLVM_CXXFLAGS)
LDFLAGS			:= $(LDFLAGS) $(LLVM_LDFLAGS)
# input, output
TARGET			:= bin/$(BUILD)/lbc
OBJDIR			:= obj/$(BUILD)
SOURCES			:= $(wildcard src/*.cpp)
OBJECTS			:= $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
PCH				:= $(OBJDIR)/pch.gch
DEPS			:= $(patsubst src/%.cpp,$(OBJDIR)/%.d,$(SOURCES)) \
				$(OBJDIR)/pch.d

#include our project dependecy files
-include $(DEPS)

# disable checking for files
.PHONY: all clean

# the final
all: $(TARGET)

# cleanup
clean:
	rm -f $(OBJECTS)
	rm -f $(DEPS)
	rm -f $(PCH)

# link
$(TARGET): $(PCH) $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

# precompiled header
$(PCH): src/pch.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# compile .cpp to .o
$(OBJDIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -include-pch $(PCH) -c $< -o $@

