all: test.z64
.PHONY: all

BUILD_DIR = build
include $(N64_INST)/include/n64.mk

CFLAGS += -Iwren/include
CFLAGS += -DWREN_OPT_META=0
CFLAGS += -DWREN_OPT_RANDOM=0

OBJS = $(BUILD_DIR)/test.o

# Wren
OBJS += $(BUILD_DIR)/wren/vm/wren_compiler.o
OBJS += $(BUILD_DIR)/wren/vm/wren_core.o
OBJS += $(BUILD_DIR)/wren/vm/wren_debug.o
OBJS += $(BUILD_DIR)/wren/vm/wren_primitive.o
OBJS += $(BUILD_DIR)/wren/vm/wren_utils.o
OBJS += $(BUILD_DIR)/wren/vm/wren_value.o
OBJS += $(BUILD_DIR)/wren/vm/wren_vm.o

test.z64: N64_ROM_TITLE = "Video Test"
test.z64: $(BUILD_DIR)/test.dfs

$(BUILD_DIR)/test.dfs: $(wildcard filesystem/*)
$(BUILD_DIR)/test.elf: $(OBJS)

clean:
	rm -rf $(BUILD_DIR) *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d))
