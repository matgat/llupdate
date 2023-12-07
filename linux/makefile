PRJNAME = llupdate
SRCDIR = ../source
BLDDIR = ./build
TARGET = $(BLDDIR)/$(PRJNAME)
VPATH := $(SRCDIR)
MAIN = $(SRCDIR)/main.cpp
HEADERS = $(wildcard $(SRCDIR)/*.hpp)

CXX = g++
CXXFLAGS = -std=c++2b -Wall -Wextra -Wpedantic -Wconversion -Wfatal-errors -O3 -lfmt
#MSVCFLAGS = /std:c++latest /utf-8 /W4 /O2

TEST_DIR = ../test
TEST_MAIN = $(TEST_DIR)/test.cpp
TEST_TARGET = $(TEST_DIR)/build/$(PRJNAME)_test
TEST_FLAGS = -DTESTING -pthread -lgtest -lgtest_main

all: executable test

debug: CXXFLAGS += -DDEBUG -D_DEBUG -g
debug: executable

executable: $(MAIN) $(HEADERS) makefile
	$(CXX) -o $(TARGET) $(CXXFLAGS) $(MAIN)

test: $(TEST_MAIN) $(MAIN) $(HEADERS) makefile
	$(CXX) -o $(TEST_TARGET) $(CXXFLAGS) $(TEST_FLAGS) $(TEST_MAIN)

clean:
	rm *.o
