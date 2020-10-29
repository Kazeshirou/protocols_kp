# Файл содержит общие настройки для компиляции и линковки.

# --0. Выбираем место хранения долгоживущих файлов.

# Папка *.d содержит флаги компиляции предыдущей сборки (.flags),
# а также файлы с указанием зависиомстей (.d). Файлы остаются с предыдущих
# компиляций и на их основе решается - требуется ли перекомпиляция исходника.

# Папка *.objs содержит объектные файлы на основе которыйх линкуется
# конечный бинарник.

# INTERNAL_BUILD_PATH это часть пути основывется на значении BUILD самого
# главного makefile'а.
DEPS_PATH := .internals/$(INTERNAL_BUILD_PATH).$(BUILD).d
OBJS_PATH := .internals/$(INTERNAL_BUILD_PATH).$(BUILD).objs

# --1. Выбираем уровень оптимизации
# Если не задан другой уровень оптимизации то по умолчанию -Os
ifneq (,$(filter $(shell echo $(OPTIMIZATION) | tr A-Z a-z),off false disable))
OPTIMIZATION_LVL := -O0
else
OPTIMIZATION := on
OPTIMIZATION_LVL := -Os
endif

# --2. Выбираем тип сборки debug/release
# Включение отладочных функций (DEBUG/RELEASE сборка)
ifneq (,$(filter $(shell echo $(DEBUG) | tr A-Z a-z),on true enable))
TYPE_OF_BUILD = DEBUG
DEFS += _DEBUG_BUILD
CFLAGS += -g3
else
TYPE_OF_BUILD = RELEASE
DEFS += _RELEASE_BUILD
CFLAGS += -g0
endif
