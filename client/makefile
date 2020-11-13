include build_tools/common.mk

NPROC = $(shell nproc)

.PHONY: client
client:
	@echo Building $(TARGET) with -j$(NPROC)
	@$(MAKE) all -j$(NPROC) --makefile=client/sources.mk
	@echo Building common
	@$(MAKE) all -j$(NPROC) --makefile=common/sources.mk
	@echo - Executing linker
	@$(MAKE) all -j$(NPROC) --makefile=build_tools/link.mk