include common/dirs.mk

SOURCES += $(shell find client/ -name '*.c')
include build_tools/compile.mk

