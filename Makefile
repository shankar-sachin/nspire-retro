# Ndless sample convention: compile -> nspire-ld -> genzehn -> make-prg.
.DEFAULT_GOAL := all
GCC = nspire-gcc
LD = nspire-ld
GENZEHN = genzehn
MAKE_PRG = make-prg
HOST_CC ?= cc
DEBUG ?= FALSE
EXE := nspire-retro
BUILD := build
SOURCES := src/main.c src/game.c src/input.c src/render.c src/app.c src/season.c src/save.c
HEADERS := $(wildcard src/*.h)
OBJS := $(SOURCES:src/%.c=$(BUILD)/%.o)
GCCFLAGS := -std=gnu99 -Wall -Wextra -marm -mcpu=arm926ej-s -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS := -Wl,--gc-sections
ZEHNFLAGS := --name "Nspire Retro" --uses-lcd-blit true
ifeq ($(DEBUG),TRUE)
GCCFLAGS += -O0 -g
else
GCCFLAGS += -O2
endif
.PHONY: all clean test check
all: $(BUILD)/$(EXE).tns
$(BUILD):
	mkdir -p $@
$(BUILD)/%.o: src/%.c | $(BUILD)
	$(GCC) $(GCCFLAGS) -c $< -o $@
$(BUILD)/$(EXE).elf: $(OBJS)
	$(LD) $^ -o $@ $(LDFLAGS)
$(BUILD)/$(EXE).tns: $(BUILD)/$(EXE).elf
	$(GENZEHN) --input $< --output $@.zehn $(ZEHNFLAGS)
	$(MAKE_PRG) $@.zehn $@
	rm -f $@.zehn
$(BUILD)/test_game: tests/test_game.c src/game.c $(HEADERS) | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -pedantic -O1 -g -fsanitize=address,undefined -Isrc tests/test_game.c src/game.c -o $@
$(BUILD)/test_platform: tests/test_platform.c tests/stubs/libndls.h $(SOURCES) $(HEADERS) | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -pedantic -O1 -g -fsanitize=address,undefined -Itests/stubs -Isrc tests/test_platform.c src/game.c src/input.c src/render.c src/app.c src/season.c -o $@
$(BUILD)/test_app: tests/test_app.c src/app.c src/game.c src/season.c src/save.c $(HEADERS) | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -pedantic -O1 -g -fsanitize=address,undefined -Isrc tests/test_app.c src/app.c src/game.c src/season.c src/save.c -o $@
test: $(BUILD)/test_game $(BUILD)/test_platform $(BUILD)/test_app
	./$(BUILD)/test_game
	./$(BUILD)/test_platform
	./$(BUILD)/test_app
check: test
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -pedantic -Itests/stubs -Isrc -fsyntax-only $(SOURCES)
clean:
	rm -rf $(BUILD)
-include $(OBJS:.o=.d)
