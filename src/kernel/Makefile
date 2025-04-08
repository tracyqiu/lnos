.PHONY: kernel clean

TARGET=kernel

ASM := nasm
CC := i686-elf-gcc
OBJCOPY = objcopy
LD := ld
CFLAGS=-m32 -Wall -nostdlib -ffreestanding -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wextra -Werror -c 
CFLAGS_PATH=-I./
ASFLAGS=-f elf
LDFLAGS = -m elf_i386 -T linker.ld

BUILD_DIR := ../../build

SOURCES_C := $(wildcard *.c)
SOURCES_ASM := $(wildcard *.asm)
OBJECTS_C := $(patsubst %.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_C))
OBJECTS_ASM := $(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_ASM))
KERNEL_ELF := $(BUILD_DIR)/kernel/kernel.elf


$(TARGET): $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(KERNEL_ELF)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $@

$(BUILD_DIR)/kernel/asm/%.o: %.asm
	$(shell mkdir -p $(BUILD_DIR)/kernel/asm)
	$(ASM) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel/c/%.o: %.c
	$(shell mkdir -p $(BUILD_DIR)/kernel/c)
	$(CC) $(CFLAGS_PATH) $(CFLAGS) -c $< -g -o $@

# Make sure the entry point code is placed at the very beginning, 
# otherwise the execution entry point will become random when the assembly code file is added.
$(KERNEL_ELF): ${OBJECTS_ASM} ${OBJECTS_C}
	$(LD) $(LDFLAGS) -o $@ $(BUILD_DIR)/kernel/c/main.o $(filter-out $(BUILD_DIR)/kernel/c/main.o, $^)

clean:
	$(shell rm -rf $(BUILD_DIR)/kernel.bin)
	$(shell rm -rf $(BUILD_DIR)/kernel)

