# Файл осуществляет непосредственную линковку объектных файлов в бинарник.

# Необходимые переменные для совершения линковки:
# LD         - Линковщик (бинарник).
# OBJDUMP    - Бинарник используется для генерации асемблерного файла (man objdump).
# READELF    - Бинарник не используется (man readelf).
# PRINT_SIZE - Юниксовая команда для вывода текущего размера.
# CFLAGS     - Сишные флаги линковки.
# LDFLAGS    - Общие флаги линковки.

include build_tools/settings.mk

# Собираем объектники для компиляции.
OBJS := $(shell find $(OBJS_PATH) -name "*.o" )

# Пути расположения конечных файлов.
BINARY_FILE := bin/$(BUILD)
MAP_FILE := bin/$(BUILD).map

# Добавляем общие флаги линковки
CFLAGS += -Wall -Wextra -Werror
LDFLAGS += -Xlinker --gc-sections
LDFLAGS += -o $@ -Wl,-Map="$(MAP_FILE)" $(MEMORY_PRINT)

# Добавляем зависимость бинарника от мейкфайов, которые встречались
# до текущего момента
COMMON_OBJ_DEPS := $(MAKEFILE_LIST)

$(BINARY_FILE): $(OBJS) $(COMMON_OBJ_DEPS)
	@mkdir -p bin
	@echo [LD] $@
	@echo
	@$(MESSAGE)
	@$(LD) $(CFLAGS) $(OBJS) $(LDFLAGS)
	@echo 'Size summary:'
	@$(PRINT_SIZE)

LD := gcc
OBJDUMP := objdump
READELF := readelf
SIZE := size

# зачем --no-as-needed - https://gcc.gnu.org/ml/gcc-help/2017-03/msg00081.html
CFLAGS = -Wl,--no-as-needed -pthread

ifeq ($(BUILD),test)
LDFLAGS += -lgcov -lgtest -lgmock
endif

MEMORY_PRINT :=
MESSAGE :=
PRINT_SIZE = $(SIZE) --format=berkeley $(BINARY_FILE)

all: $(BINARY_FILE)
