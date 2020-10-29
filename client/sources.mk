include common/dirs.mk

SOURCES += $(shell find client/ -name '*.c')
include build_tools/compile.mk

.PHONY: client
client:
	@echo Building common
	@$(MAKE) all --makefile=common/sources.mk
	@echo - Executing linker
	@$(MAKE) all --makefile=build_tools/link.mk