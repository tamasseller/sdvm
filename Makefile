OUTPUT = test

SOURCES += object/Storage.cpp
SOURCES += vm/Vm.cpp
SOURCES += compiler/FunctionBuilder.cpp

SOURCES += TestStorage.cpp
SOURCES += TestVm.cpp

SOURCES += main.cpp
SOURCES += pet/1test/TestRunnerExperimental.cpp

INCLUDE_DIRS += .

CXXFLAGS += -std=c++17 -O0 -g3 -m32 -Wall

LDFLAGS += -m32

LD = $(CXX)

include pet/mod.mk
include ultimate-makefile/Makefile.ultimate
