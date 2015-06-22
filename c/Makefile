# OWF: the Open Waveform Format
# @file Makefile
# @author Morgan Jones <morgan@medicalinformaticscorp.com>

# Set the C compiler
CC ?= clang

# Set the CFLAGS and LDFLAGS
override CFLAGS := -Wall -std=c99 -g -D_GNU_SOURCE $(CFLAGS)
override LDFLAGS := $(LDFLAGS)

# Define includes and linker libs
INCLUDES = -Iinclude
LIBS = 

LIBOWF_SRCS = $(shell find src -type f -name '*.c')
LIBOWF_OBJS = $(LIBOWF_SRCS:.c=.o)

LIBOWF_TEST_SRCS = $(shell find test -type f -name '*.c')
LIBOWF_TEST_OBJS = $(LIBOWF_TEST_SRCS:.c=.o)

# Export variables for subprocesses
export PATH := ./build/lib:./build/bin:$(PATH)
export LD_LIBRARY_PATH := ./build/lib:$(LD_LIBRARY_PATH)

# Perform some basic platform detection
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	DL_EXT := .dylib
	BIN_EXT := 
	CFLAGS += -fPIC
else ifeq ($(UNAME), Linux)
	DL_EXT := .so
	BIN_EXT := 
	CFLAGS += -fPIC
else ifneq (,$(findstring CYGWIN, $(UNAME)))
	DL_EXT := .dll
	BIN_EXT := .exe
else
	$(error Unknown development platform, we support Darwin, Linux, and Cygwin)
endif

# Define the executable file
LIBOWF = build/lib/libowf$(DL_EXT)
LIBOWF_TEST = build/bin/libowf-test$(BIN_EXT)

.PHONY: depend clean test

all: $(LIBOWF)
tests: $(LIBOWF) $(LIBOWF_TEST)

test: $(LIBOWF) $(LIBOWF_TEST)
	$(LIBOWF_TEST)

$(LIBOWF): 
$(LIBOWF): $(LIBOWF_OBJS)
	$(CC) $(CFLAGS) -shared $(INCLUDES) -o $(LIBOWF) $(LIBOWF_OBJS) $(LDFLAGS) $(LIBS)

$(LIBOWF_TEST): LDFLAGS += -Lbuild/lib -lm -lowf
$(LIBOWF_TEST): $(LIBOWF) $(LIBOWF_TEST_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(LIBOWF_TEST) $(LIBOWF_TEST_OBJS) $(LDFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	rm -f $(LIBOWF) $(LIBOWF_TEST)
	find src test -name '*.o' -exec rm -f {} \;
