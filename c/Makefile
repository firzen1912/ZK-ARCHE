# auth-c — Makefile
#
# Targets:
#   make            build everything
#   make test       run conformance tests
#   make clean
#
# Deps: libsodium-dev, pkg-config, a C11 compiler.

CC       ?= cc
CFLAGS   ?= -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion \
            -Wno-sign-conversion -O2 -fno-strict-aliasing \
            -fstack-protector-strong -D_FORTIFY_SOURCE=2
LDFLAGS  ?=

SODIUM_CFLAGS := $(shell pkg-config --cflags libsodium)
SODIUM_LIBS   := $(shell pkg-config --libs   libsodium)

INCLUDES := -Iinclude $(SODIUM_CFLAGS)
LIBS     := $(SODIUM_LIBS) -lpthread

BUILD := build
LIB   := $(BUILD)/libauth.a

CRYPTO_SRCS := $(wildcard src/crypto/*.c)
WIRE_SRCS   := $(wildcard src/wire/*.c)
PROTO_SRCS  := $(wildcard src/proto/*.c)
TRANS_SRCS  := $(wildcard src/transport/*.c)
STORE_SRCS  := $(wildcard src/store/*.c)

HOST_OS := $(shell uname -s 2>/dev/null || echo unknown)
ifneq (,$(filter Windows_NT MSYS_NT% MINGW% CYGWIN%,$(OS) $(HOST_OS)))
TRANS_SRCS :=
STORE_SRCS :=
endif

LIB_SRCS := $(CRYPTO_SRCS) $(WIRE_SRCS) $(PROTO_SRCS) $(TRANS_SRCS) $(STORE_SRCS)
LIB_OBJS := $(patsubst src/%.c,$(BUILD)/%.o,$(LIB_SRCS))

TEST_SRCS := $(wildcard tests/*.c)
TEST_BINS := $(patsubst tests/%.c,$(BUILD)/tests/%,$(TEST_SRCS))

BINS_SRCS := $(wildcard bin/*.c)
ifneq (,$(filter Windows_NT MSYS_NT% MINGW% CYGWIN%,$(OS) $(HOST_OS)))
BINS_SRCS :=
endif
BINS      := $(patsubst bin/%.c,$(BUILD)/%,$(BINS_SRCS))

.PHONY: all test clean lib bins tests

all: lib bins tests

lib: $(LIB)

bins: $(BINS)

tests: $(TEST_BINS)

$(LIB): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD)/tests/%: tests/%.c $(LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIB) $(LDFLAGS) $(LIBS)

$(BUILD)/%: bin/%.c $(LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIB) $(LDFLAGS) $(LIBS)

test: $(TEST_BINS)
	@echo "--- running tests ---"
	@set -e; for t in $(TEST_BINS); do echo ">> $$t"; $$t; done

clean:
	rm -rf $(BUILD)


.PHONY: deps asan ubsan fuzz-wire fuzz-payloads reproducible

deps:
	pkg-config --exists libsodium
	@echo "libsodium: $$(pkg-config --modversion libsodium)"

asan:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -O1 -g -fsanitize=address -fno-omit-frame-pointer" LDFLAGS="$(LDFLAGS) -fsanitize=address" all test

ubsan:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -O1 -g -fsanitize=undefined -fno-omit-frame-pointer" LDFLAGS="$(LDFLAGS) -fsanitize=undefined" all test

fuzz-wire: $(LIB) fuzz/fuzz_wire.c
	clang -std=c11 -g -O1 -fsanitize=fuzzer,address,undefined $(INCLUDES) -o $(BUILD)/fuzz_wire fuzz/fuzz_wire.c $(LIB) $(LIBS)
	$(BUILD)/fuzz_wire -max_total_time=300

fuzz-payloads: $(LIB) fuzz/fuzz_payloads.c
	clang -std=c11 -g -O1 -fsanitize=fuzzer,address,undefined $(INCLUDES) -o $(BUILD)/fuzz_payloads fuzz/fuzz_payloads.c $(LIB) $(LIBS)
	$(BUILD)/fuzz_payloads -max_total_time=300

reproducible:
	$(MAKE) clean
	$(MAKE) all test
	$(MAKE) asan
	$(MAKE) ubsan
