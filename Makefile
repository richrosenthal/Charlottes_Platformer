PROJECT := charlottes_platformer
TITLE := CHARLOTTE
BUILD_DIR := build
SOURCES := $(wildcard src/*.c)
ROM := $(BUILD_DIR)/$(PROJECT).gb

ifdef GBDK_HOME
LCC := $(GBDK_HOME)/bin/lcc
else
LCC := lcc
endif

LCCFLAGS := -Wa-l -Wl-m -Wl-j -Wm-yn"$(TITLE)"

.PHONY: all clean run

all: $(ROM)

$(ROM): $(SOURCES) | $(BUILD_DIR)
	$(LCC) $(LCCFLAGS) -o $@ $(SOURCES)

$(BUILD_DIR):
	mkdir -p $@

run: $(ROM)
	@if [ -z "$(EMULATOR)" ]; then \
		echo "Set EMULATOR to your Game Boy emulator, for example: make run EMULATOR=sameboy"; \
		exit 1; \
	fi
	$(EMULATOR) $(ROM)

clean:
	rm -rf $(BUILD_DIR)
