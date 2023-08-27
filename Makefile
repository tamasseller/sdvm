OUTPUT = test

SOURCES += vm/Vm.cpp
SOURCES += vm/Storage.cpp

SOURCES += compiler/internal/Compiler.cpp
SOURCES += compiler/internal/AstDump.cpp
SOURCES += compiler/internal/GlobalIdentifiers.cpp
SOURCES += compiler/internal/Tacify.cpp

#SOURCES += TestVm.cpp
SOURCES += TestTacify.cpp
#SOURCES += TestBuilder.cpp
#SOURCES += TestStorage.cpp

SOURCES += main.cpp
SOURCES += pet/1test/TestRunnerExperimental.cpp

INCLUDE_DIRS += .

CXXFLAGS += -std=c++17 -O0 -g3 -m32 -Wall
CXXFLAGS += -fmax-errors=6
CXXFLAGS += -fno-inline
CXXFLAGS += --coverage

LDFLAGS += -m32

LIBS += gcov

COVROOT := .
COVFLAGS := -f vm -f program -f compiler

LD = $(CXX)

include pet/mod.mk
include ultimate-makefile/Makefile.ultimate
