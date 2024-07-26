ASM:=nasm

SRC_DIR:=src
BUILD_DIR:=build
HD_IMG_NAME:="lnos.img"

all: ${BUILD_DIR}/boot.o
	$(shell rm -rf $(HD_IMG_NAME))
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $(HD_IMG_NAME)
	dd if=${BUILD_DIR}/boot.o of=$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc
#	dd if=${BUILD_DIR}/boot/setup.o of=$(HD_IMG_NAME) bs=512 seek=1 count=2 conv=notrunc

${BUILD_DIR}/%.o: $(SRC_DIR)/%.asm
	$(shell mkdir -p ${BUILD_DIR})
	$(ASM) $< -o $@

clean:
	$(shell rm -rf $(BUILD_DIR))
	$(shell rm -rf $(HD_IMG_NAME))

bochs: all
	bochs -q -f bochsrc

qemu: all
	qemu-system-x86_64 -hda $(HD_IMG_NAME)