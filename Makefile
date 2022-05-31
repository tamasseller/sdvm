OUTPUT = sdvm-test

SOURCES += main.cpp
SOURCES += TestInterpreter.cpp

SOURCES += pet/1test/TestRunnerExperimental.cpp
SOURCES += pet/ubiquitous/PrintfWriter.cpp

INCLUDE_DIRS += .

CXXFLAGS += -O0 -g3
CXXFLAGS += -std=c++17
CXXFLAGS += --coverage

COVROOT = .
COVFLAGS = --filter vm

LIBS += gcov

LD = $(CXX)

include vm/mod.mk
include pet/mod.mk

include ultimate-makefile/Makefile.ultimate