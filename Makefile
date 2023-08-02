OUTPUT = test

SOURCES += vm/Vm.cpp
SOURCES += object/Storage.cpp
SOURCES += object/TypeInfo.cpp
SOURCES += compiler/ProgramBuilder.cpp

SOURCES += TestStorage.cpp
SOURCES += TestVm.cpp

SOURCES += main.cpp
SOURCES += pet/1test/TestRunnerExperimental.cpp

INCLUDE_DIRS += .

CXXFLAGS += -std=c++17 -O0 -g3 -m32 -Wall
CXXFLAGS += -fmax-errors=6

LDFLAGS += -m32

LD = $(CXX)

include pet/mod.mk
include ultimate-makefile/Makefile.ultimate
