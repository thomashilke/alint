CXX = clang++
DEPS_BIN = clang++
DEPSFLAGS =  -std=c++1y -Iexternal/lexer/include -Iexternal/spikes/include -Iexternal/parser/include 
CXXFLAGS = -O3 -std=c++1y -Wall -Wextra -Iexternal/lexer/include -Iexternal/spikes/include -Iexternal/parser/include 
LDFLAGS = -O3 -Lexternal/lexer/lib
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

SOURCES = src/alint.cpp test/recovery.cpp

HEADERS = 

BIN = bin/alint bin/test_recovery


bin/alint: build/src/alint.o
bin/test_recovery: build/test/recovery.o


LIB = 

#lib/lib....a: build/.../.o
