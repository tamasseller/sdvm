curdir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

INCLUDE_DIRS := $(INCLUDE_DIRS) $(curdir)/.

#SOURCES := $(SOURCES) $(curdir)/

undefine curdir