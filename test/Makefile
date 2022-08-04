OUTPUT = sdvm-test

SOURCES += main.cpp
SOURCES += TestAsm.cpp
SOURCES += TestIsn.cpp
SOURCES += TestDisasm.cpp
SOURCES += TestPrinter.cpp
SOURCES += TestBuilder.cpp
SOURCES += TestAnalyzer.cpp
SOURCES += TestCompiler.cpp
SOURCES += TestInterpreter.cpp
SOURCES += TestErrorHandler.cpp
SOURCES += TestImmediateSynth.cpp
SOURCES += TestRegisterAllocator.cpp

SOURCES += ../analyzer/Analyzer.cpp

SOURCES += ../jit/Compiler.cpp
SOURCES += ../jit/Assembler.cpp
SOURCES += ../jit/RegisterAllocator.cpp

SOURCES += ../pet/1test/TestRunnerExperimental.cpp
SOURCES += ../pet/ubiquitous/PrintfWriter.cpp

INCLUDE_DIRS += .
INCLUDE_DIRS += ../jit
INCLUDE_DIRS += ../analyzer

CXXFLAGS += -O0 -g3
CXXFLAGS += -std=c++17
CXXFLAGS += --coverage

COVROOT = .
COVFLAGS = --filter ../vm --filter ../tools --filter ../jit --filter ../analyzer

LIBS += gcov

LD = $(CXX)

include ../vm/mod.mk
include ../tools/mod.mk
include ../pet/mod.mk

include ../ultimate-makefile/Makefile.ultimate
