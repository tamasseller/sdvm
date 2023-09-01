OUTPUT = test

SOURCES += vm/Vm.cpp
SOURCES += vm/Storage.cpp

SOURCES += compiler/ast/ProgramObjectSet.cpp
SOURCES += compiler/ast/ValueType.cpp
SOURCES += compiler/ast/Function.cpp
SOURCES += compiler/ast/Class.cpp
SOURCES += compiler/ast/Field.cpp

SOURCES += compiler/ir/BasicBlock.cpp
SOURCES += compiler/ir/Function.cpp

SOURCES += compiler/internal/Dump.cpp
SOURCES += compiler/internal/IrGen.cpp
SOURCES += compiler/internal/JumpOptimization.cpp

#SOURCES += TestStorage.cpp
#SOURCES += TestVm.cpp
#SOURCES += TestBuilder.cpp
SOURCES += TestIrGen.cpp

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
