CXX = g++
DEPS_BIN = g++
DEPSFLAGS =  -I$(HOME)/.local/include
CXXFLAGS = -O3 -std=c++1y -Wall -Wextra -I$(HOME)/.local/include
LDFLAGS = -O3 -L$(HOME)/.local/lib
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
