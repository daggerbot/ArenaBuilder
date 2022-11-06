# Game ID used in file names
GAME = arena-builder

# Options passed to the game executable when running
RUN_ARGS = --data-path=${DATA_PK3}

# Directory containing this source tree
TOP_DIR := $(patsubst %/,%,$(dir $(lastword ${MAKEFILE_LIST})))

# Directory where intermediate and output files are placed
BUILD_DIR := $(patsubst ./%,%,${TOP_DIR}/build)

# Use rustup to determine the default TARGET if not specified
ifndef TARGET
 TARGET := $(shell rustup show | grep '^Default host:' | sed 's/^.*: //')
endif

ifneq ($(words ${TARGET}),1)
 $(error invalid TARGET: ${TARGET})
endif

# Use RELEASE=1 for release builds
PROFILE := $(if ${RELEASE},release,debug)
RELEASE_FLAG := $(if ${RELEASE},--release)

# Use VERBOSE=1 for verbose builds
VERBOSE_FLAG := $(if ${VERBOSE},--verbose)

# Options passed to cargo when building/running
BUILD_ARGS := \
	--manifest-path=${TOP_DIR}/code/Cargo.toml \
	--target=${TARGET} \
	--target-dir=${BUILD_DIR}/cargo \
	${RELEASE_FLAG} \
	${VERBOSE_FLAG}

#---------------------------------------------------------------------------------------------------

all: build

.PHONY: all build check clean run

build:
	cargo build ${BUILD_ARGS}

check:
	cargo check ${BUILD_ARGS}

clean:
	rm -rf ${BUILD_DIR}

run:
	cargo run ${BUILD_ARGS} --bin=${GAME} -- ${RUN_ARGS}

include ${TOP_DIR}/data/data.mk
