CXX = clang++
DEPS_BIN = g++
DEPSFLAGS = -std=c++11 -I$(HOME)/.local/include
CXXFLAGS = -O2 -std=c++11 -I$(HOME)/.local/include
LDFLAGS = -O2 -L$(HOME)/.local/lib
LDLIB = -llexer
AR = ar
ARFLAGS = rc
MKDIR = mkdir
MKDIRFLAGS = -p


PREFIX = ~/.local/
BIN_DIR = bin/
INCLUDE_DIR = include/
LIB_DIR = lib/

PKG_NAME = alint

SOURCES = src/alint.cpp

HEADERS = 

BIN = bin/alint


bin/alint: build/src/alint.o


LIB = 

#lib/lib....a: build/.../.o
