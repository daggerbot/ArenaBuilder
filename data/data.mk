PYTHON := python3
DATA_PK3 := ${BUILD_DIR}/${GAME}.pk3
PK3_TOOL := ${TOP_DIR}/tools/pk3.py

COMPRESS_FILES := \
	shaders/glsl-1.30/unlit.frag \
	shaders/glsl-1.30/unlit.vert

DATA_DEPENDS := \
	${PK3_TOOL} \
	$(addprefix ${TOP_DIR}/data/,${COMPRESS_FILES})

DATA_ARGS := \
	$(foreach f,${COMPRESS_FILES},${TOP_DIR}/data/$f:$f:zstd)

all: data

.PHONY: data

data: ${DATA_PK3}

${DATA_PK3}: ${DATA_DEPENDS}
	${PYTHON} ${PK3_TOOL} -o $@ ${DATA_ARGS}
