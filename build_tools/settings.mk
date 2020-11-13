# Файл содержит общие настройки для компиляции и линковки.

# Выбираем место хранения долгоживущих файлов.

# Папка *.d содержит флаги компиляции предыдущей сборки (.flags),
# а также файлы с указанием зависимостей (.d). Файлы остаются с предыдущих
# компиляций и на их основе решается - требуется ли перекомпиляция исходника.

# Папка *.objs содержит объектные файлы на основе которыйх линкуется
# конечный бинарник.

# INTERNAL_BUILD_PATH это часть пути основывется на значении BUILD самого
# главного makefile'а.
DEPS_PATH := .internals/$(TARGET).d
OBJS_PATH := .internals/$(TARGET).objs

# Выбираем уровень оптимизации
# Если не задан другой уровень оптимизации то по умолчанию -Os
ifneq (,$(filter $(shell echo $(OPTIMIZATION) | tr A-Z a-z),off false disable))
OPTIMIZATION_LVL := -O0
else
OPTIMIZATION := on
OPTIMIZATION_LVL := -Os
endif

# Выбираем тип сборки debug/release
# Включение отладочных функций (DEBUG/RELEASE сборка)
ifneq (,$(filter $(shell echo $(DEBUG) | tr A-Z a-z),on true enable))
BUILD = DEBUG
DEFS = _DEBUG_BUILD
CFLAGS += -g3
else
BUILD = RELEASE
DEFS = _RELEASE_BUILD
CFLAGS += -g0
endif
