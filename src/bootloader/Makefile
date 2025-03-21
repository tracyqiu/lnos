.PHONY: bootloader clean

ASM:=nasm
ASFLAGS=-f bin

BUILD_DIR:=../../build

SOURCES:=$(wildcard *.asm)
OBJECTS:=$(patsubst %.asm, $(BUILD_DIR)/bootloader/%.bin, $(SOURCES))

bootloader: ${OBJECTS}

${BUILD_DIR}/bootloader/%.bin: %.asm
	$(shell mkdir -p $(BUILD_DIR)/bootloader)
	$(ASM) $(ASFLAGS) $< -o $@


clean:
	$(shell rm -rf $(BUILD_DIR)/bootloader)
