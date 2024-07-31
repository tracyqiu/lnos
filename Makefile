.PHONY: all clean bochs qemu


SRC_DIR:=src
BUILD_DIR:=build
HD_IMG_NAME:="lnos.img"


all: bootloader kernel
	$(shell rm -rf $(HD_IMG_NAME))
	bximage -q -hd=16 -mode=create -sectsize=512 -imgmode=flat $(HD_IMG_NAME)
	dd if=${BUILD_DIR}/bootloader/boot.o of=$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc
	dd if=${BUILD_DIR}/bootloader/setup.o of=$(HD_IMG_NAME) bs=512 seek=1 count=1 conv=notrunc
#	dd if=${BUILD_DIR}/kernel.bin of=$(HD_IMG_NAME) bs=512 seek=2 count=2 conv=notrunc


bootloader:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(SRC_DIR)/bootloader BUILD_DIR=$(abspath $(BUILD_DIR))


kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR))


clean:
	$(MAKE) -C $(SRC_DIR)/bootloader/ BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(shell rm -rf $(BUILD_DIR))
	$(shell rm -rf $(HD_IMG_NAME))


bochs: all
	bochs -q -f bochsrc


qemu: all
	qemu-system-x86_64 -hda $(HD_IMG_NAME)