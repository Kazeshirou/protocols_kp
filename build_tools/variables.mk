# Выведем переменную BUILD
BUILD ?= $(MAKECMDGOALS)

# Если вызван make без цели - default
ifeq ($(BUILD),)
BUILD := default
endif

# Место для хранение файлов сборки (.d/.flags/.o)
# основывается на BULID из главного makefile'а.
INTERNAL_BUILD_PATH ?= $(BUILD)

# Создадим и проэкспортируем глобальные переменные
export CFLAGS
export BUILD
export SOURCES
export DEFS
export DIRS
export INTERNAL_BUILD_PATH
export LDFLAGS