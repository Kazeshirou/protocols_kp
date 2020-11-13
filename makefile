include build_tools/common.mk

.PHONY: client
client:
	@make client --makefile=client/makefile

.PHONY: server
server:
	@make server --makefile=server/makefile

all: clean client server