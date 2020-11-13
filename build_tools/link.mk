# Файл осуществляет непосредственную линковку объектных файлов в бинарник.

# Необходимые переменные для совершения линковки:
# LD         - Линковщик (бинарник).
# PRINT_SIZE - Юниксовая команда для вывода текущего размера.
# CFLAGS     - Сишные флаги линковки.
# LDFLAGS    - Общие флаги линковки.

include build_tools/settings.mk

# Собираем объектники для компиляции.
OBJS := $(shell find $(OBJS_PATH) -name "*.o" )

# Пути расположения конечных файлов.
BINARY_FILE := bin/$(TARGET)
MAP_FILE := bin/$(TARGET).map

# Добавляем общие флаги линковки
# зачем --no-as-needed - https://gcc.gnu.org/ml/gcc-help/2017-03/msg00081.html
CFLAGS += -Wall -Wextra -Werror -Wl,--no-as-needed -pthread
LDFLAGS += -Xlinker --gc-sections
LDFLAGS += -o $@ -Wl,-Map="$(MAP_FILE)" 

# Добавляем зависимость бинарника от мейкфайов, которые встречались
# до текущего момента
COMMON_OBJ_DEPS := $(MAKEFILE_LIST)

$(BINARY_FILE): $(OBJS) $(COMMON_OBJ_DEPS)
	@mkdir -p bin
	@echo [LD] $@
	@echo
	@$(LD) $(CFLAGS) $(OBJS) $(LDFLAGS)
	@echo 'Size summary:'
	@$(PRINT_SIZE)

LD := gcc
SIZE := size


ifeq ($(TARGET),test)
LDFLAGS += -lgcov -lgtest -lgmock
endif


PRINT_SIZE = $(SIZE) --format=berkeley $(BINARY_FILE)

all: $(BINARY_FILE)
