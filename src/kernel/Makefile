.PHONY: kernel clean

TARGET=kernel

ASM:=nasm
CC:=gcc
CFLAGS=-m32 -Wall -nostdlib
CFLAGS_PATH=-I./
ASFLAGS=-f elf

BUILD_DIR:=../../build

SOURCES_C:=$(wildcard *.c)
SOURCES_ASM:=$(wildcard *.asm)
OBJECTS_C:=$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_C))
OBJECTS_ASM:=$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_ASM))


$(TARGET): $(BUILD_DIR)/kernel.bin


$(BUILD_DIR)/kernel.bin: ${OBJECTS_ASM} ${OBJECTS_C}
	$(CC) $(CFLAGS) ${OBJECTS_ASM} ${OBJECTS_C} -g -o $@ -e cstart


$(BUILD_DIR)/kernel/asm/%.o: %.asm
	$(shell mkdir -p $(BUILD_DIR)/kernel/asm)
	$(ASM) $(ASFLAGS) $< -o $@


$(BUILD_DIR)/kernel/c/%.o: %.c
	$(shell mkdir -p $(BUILD_DIR)/kernel/c)
	$(CC) $(CFLAGS_PATH) $(CFLAGS) -c $< -g -o $@


clean:
	$(shell rm -rf $(BUILD_DIR)/kernel.bin)
	$(shell rm -rf $(BUILD_DIR)/kernel)







