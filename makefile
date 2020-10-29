include build_tools/common.mk

NPROC = $(shell nproc)

.PHONY: client
client:
	@echo Building $(BUILD) with j$(NPROC)
	@make $(BUILD) -j$(NPROC) --makefile=client/sources.mk

.PHONY: server
server:
	@echo Building $(BUILD) with j$(NPROC)
	@make $(BUILD) -j$(NPROC) --makefile=server/sources.mk
