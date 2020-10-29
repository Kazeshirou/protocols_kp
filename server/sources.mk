include common/dirs.mk

SOURCES += $(shell find server/ -name '*.c')
include build_tools/compile.mk

.PHONY: server
server: all
	@echo Building common
	@$(MAKE) all --makefile=common/sources.mk
	@echo - Executing linker
	@$(MAKE) all --makefile=build_tools/link.mk