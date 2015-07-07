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
	override CFLAGS := -fPIC $(CFLAGS)
else ifeq ($(UNAME), Linux)
	DL_EXT := .so
	BIN_EXT := 
	override CFLAGS := -fPIC $(CFLAGS)
else ifneq (,$(findstring CYGWIN, $(UNAME)))
	DL_EXT := .dll
	BIN_EXT := .exe
else
	$(error Unknown development platform, we support Darwin, Linux, and Cygwin)
endif

# Define the executable file
LIBOWF = build/lib/libowf$(DL_EXT)
LIBOWF_TEST = build/bin/libowf-test$(BIN_EXT)

# Define `make all`, `make tests`, and `make test`.
all: $(LIBOWF)
tests: $(LIBOWF) $(LIBOWF_TEST)
test: $(LIBOWF) $(LIBOWF_TEST)
	@echo "TEST $(LIBOWF_TEST)"
	@$(LIBOWF_TEST)
.PHONY: clean test

# Define the libowf target
$(LIBOWF): $(LIBOWF_OBJS)
	@echo "LD $(LIBOWF)"
	@$(CC) $(CFLAGS) $(LIBOWF_CFLAGS) -shared $(INCLUDES) -o $(LIBOWF) $(LIBOWF_OBJS) $(LDFLAGS) $(LIBS)

# Define the libowf-test target
$(LIBOWF_TEST): LDFLAGS += -Lbuild/lib -lm -lowf
$(LIBOWF_TEST): $(LIBOWF) $(LIBOWF_TEST_OBJS)
	@echo "LD $(LIBOWF_TEST)"
	@$(CC) $(CFLAGS) $(LIBOWF_TEST_CFLAGS) $(INCLUDES) -o $(LIBOWF_TEST) $(LIBOWF_TEST_OBJS) $(LDFLAGS) $(LIBS)

# Rule for translating .c into .o
.c.o:
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

# Rule for cleaning build files
clean:
	@echo "RM $(LIBOWF) $(LIBOWF_TEST)"
	@rm -f $(LIBOWF) $(LIBOWF_TEST)
	@echo "RM *.o"
	@find src test -name '*.o' -exec rm -f {} \;
