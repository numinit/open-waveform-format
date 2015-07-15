# OWF: the Open Waveform Format
# @file Makefile
# @author Morgan Jones <morgan@medicalinformaticscorp.com>

# Set the C compiler
CC ?= clang

# Set the CFLAGS and LDFLAGS
override CFLAGS := -Wall -g -D_GNU_SOURCE $(CFLAGS)
override LDFLAGS := $(LDFLAGS)

# Define includes and linker libs
INCLUDES = -Iinclude
LIBS = 

LIBOWF_SRCS = $(shell find src -type f -name '*.c')
LIBOWF_OBJS = $(LIBOWF_SRCS:.c=.o)

LIBOWF_TEST_SRCS = $(shell find test -type f -name '*.c')
LIBOWF_TEST_OBJS = $(LIBOWF_TEST_SRCS:.c=.o)

LIBOWF_SERVER_SRCS = $(shell find server -type f -name '*.c')
LIBOWF_SERVER_OBJS = $(LIBOWF_SERVER_SRCS:.c=.o)

LIBOWF_DOC_SRCS = $(shell find include -type f -name '*.h')

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

# Define output paths
LIBOWF = build/lib/libowf$(DL_EXT)
LIBOWF_TEST = build/bin/libowf-test$(BIN_EXT)
LIBOWF_SERVER = build/bin/libowf-server$(BIN_EXT)
LIBOWF_DOC = doc

# Define `make all`, `make test`, and `make server`
all: $(LIBOWF) $(LIBOWF_TEST) $(LIBOWF_SERVER)
test: $(LIBOWF) $(LIBOWF_TEST)
server: $(LIBOWF) $(LIBOWF_SERVER)

# Phony targets
clean:
	@echo "RM $(LIBOWF) $(LIBOWF_TEST) $(LIBOWF_SERVER)"
	@rm -f "$(LIBOWF)" "$(LIBOWF_TEST)" "$(LIBOWF_SERVER)"
	@echo "RM *.o"
	@find src test server -name '*.o' -exec rm -f {} \;

test-run: $(LIBOWF) $(LIBOWF_TEST)
	@echo
	@echo "TEST $(LIBOWF_TEST)"
	@$(LIBOWF_TEST)

server-run: $(LIBOWF) $(LIBOWF_SERVER)
	@echo
	@echo "SERVER $(LIBOWF_SERVER) $(protocol) $(host) $(port)"
	@$(LIBOWF_SERVER) $(protocol) $(host) $(port)

doc: -cldoc-installed doc-clean
	@echo "CLDOC $(LIBOWF_DOC_SRCS)"
	@cldoc generate $(CFLAGS) $(LIBOWF_CFLAGS) $(INCLUDES) -- --output $(LIBOWF_DOC) $(LIBOWF_DOC_SRCS)

doc-static: -cldoc-installed -cldoc-static-installed doc-clean
	@echo "CLDOC $(LIBOWF_DOC_SRCS)"
	@cldoc generate $(CFLAGS) $(LIBOWF_CFLAGS) $(INCLUDES) -- --static --output $(LIBOWF_DOC) $(LIBOWF_DOC_SRCS)

doc-server: doc -cldoc-server
doc-static-server: doc-static -cldoc-server

doc-clean:
	@echo "RMDIR $(LIBOWF_DOC)"
	@rm -rf "$(LIBOWF_DOC)"

-cldoc-server:
	@echo "CLDOC $(LIBOWF_DOC)"
	@cldoc serve $(LIBOWF_DOC)

-cldoc-installed:
	@which cldoc > /dev/null || (echo 'cldoc is not installed, install it with `pip install cldoc`.' && exit 1)

-cldoc-static-installed:
	@which cldoc-static > /dev/null || (echo 'cldoc-static is not installed, install it globally with `npm install -g cldoc-static`' && exit 1)

.PHONY: clean test-run server-run doc doc-static doc-server doc-clean -cldoc-server -cldoc-installed -cldoc-static-installed

# Define the libowf target
$(LIBOWF): $(LIBOWF_OBJS)
	@echo "LD $(LIBOWF)"
	@$(CC) $(CFLAGS) -std=c99 $(LIBOWF_CFLAGS) -shared $(INCLUDES) -o $(LIBOWF) $(LIBOWF_OBJS) $(LDFLAGS) $(LIBS)

# Define the libowf-test target
$(LIBOWF_TEST): LDFLAGS += -Lbuild/lib -lm -lowf
$(LIBOWF_TEST): $(LIBOWF) $(LIBOWF_TEST_OBJS)
	@echo "LD $(LIBOWF_TEST)"
	@$(CC) $(CFLAGS) -std=c99 $(LIBOWF_TEST_CFLAGS) $(INCLUDES) -o $(LIBOWF_TEST) $(LIBOWF_TEST_OBJS) $(LDFLAGS) $(LIBS)

# Define the libowf-server target
$(LIBOWF_SERVER): LDFLAGS += -Lbuild/lib -lm -lowf
$(LIBOWF_SERVER): $(LIBOWF) $(LIBOWF_SERVER_OBJS)
	@echo "LD $(LIBOWF_SERVER)"
	@$(CC) $(CFLAGS) -std=c99 $(LIBOWF_SERVER_CFLAGS) $(INCLUDES) -o $(LIBOWF_SERVER) $(LIBOWF_SERVER_OBJS) $(LDFLAGS) $(LIBS)

# Rule for translating .c into .o
.c.o:
	@echo "CC $<"
	@$(CC) $(CFLAGS) -std=c99 $(INCLUDES) -c $<  -o $@
