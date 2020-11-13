# Общие настройки для сборки проектов
.PHONY: clean
clean:
	rm -rf bin
	rm -rf .internals

# Глобальные переменные для сборки
include build_tools/variables.mk