OUTPUT = sdvm-test

SOURCES += main.cpp
SOURCES += TestAsm.cpp
SOURCES += TestIsn.cpp
SOURCES += TestPrinter.cpp
SOURCES += TestBuilder.cpp
SOURCES += TestInterpreter.cpp
SOURCES += TestErrorHandler.cpp

SOURCES += jit/Assembler.cpp

SOURCES += pet/1test/TestRunnerExperimental.cpp
SOURCES += pet/ubiquitous/PrintfWriter.cpp

INCLUDE_DIRS += .

CXXFLAGS += -O0 -g3
CXXFLAGS += -std=c++17
CXXFLAGS += --coverage

COVROOT = .
COVFLAGS = --filter vm --filter tools --filter jit

LIBS += gcov

LD = $(CXX)

include vm/mod.mk
include tools/mod.mk
include pet/mod.mk

include ultimate-makefile/Makefile.ultimate
