include common/dirs.mk

SOURCES += $(shell find server/ -name '*.c')
include build_tools/compile.mk