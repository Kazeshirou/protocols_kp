# Выведем переменную BUILD
TARGET := $(MAKECMDGOALS)

# Создадим и проэкспортируем глобальные переменные
export TARGET
export DEFS
export SOURCES
export DIRS
export CFLAGS
export LDFLAGS