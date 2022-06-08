curdir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

INCLUDE_DIRS := $(INCLUDE_DIRS) $(curdir)/.

SOURCES := $(SOURCES) $(curdir)/Builder.cpp
SOURCES := $(SOURCES) $(curdir)/Program.cpp
SOURCES := $(SOURCES) $(curdir)/Expression.cpp
SOURCES := $(SOURCES) $(curdir)/StatementStream.cpp

undefine curdir