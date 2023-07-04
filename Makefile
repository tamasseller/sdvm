OUTPUT = test

SOURCES += Object.cpp
SOURCES += Storage.cpp

SOURCES += TestObject.cpp
SOURCES += TestStorage.cpp

SOURCES += main.cpp
SOURCES += pet/1test/TestRunnerExperimental.cpp

INCLUDE_DIRS += .

CXXFLAGS += -std=c++17 -O0 -g3 -m32

LDFLAGS += -m32

LD = $(CXX)

include pet/mod.mk
include ultimate-makefile/Makefile.ultimate
