# Общие настройки для сборки проектов
.PHONY: clean
clean: clean_all
	rm -rf bin

.PHONY: clean_all
clean_all:
	rm -rf .internals

# Глобальные переменные для сборки
include build_tools/variables.mk