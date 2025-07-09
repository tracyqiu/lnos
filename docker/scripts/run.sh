#!/bin/bash

set -e

echo "=== Running OS in QEMU ==="

# 检查是否存在内核文件
if [ ! -f "build/kernel.bin" ]; then
    echo "Kernel not found! Building first..."
    ./docker/scripts/build.sh
fi

# 检查QEMU是否可用
if ! command -v qemu-system-i386 &> /dev/null; then
    echo "Error: qemu-system-i386 not found!"
    exit 1
fi

echo "QEMU version: $(qemu-system-i386 --version | head -n1)"
echo ""

# 基础QEMU参数（与原配置保持一致）
QEMU_ARGS="-m 128M"

# 根据运行模式选择参数
MODE="${1:-normal}"

case "$MODE" in
    "debug")
        echo "Starting in debug mode..."
        echo "GDB server will be available on port 1234"
        echo "Connect with: gdb build/kernel.elf"
        echo "Then run: (gdb) target remote localhost:1234"
        echo ""
        QEMU_ARGS="$QEMU_ARGS -s -S -serial stdio"
        ;;
    "serial")
        echo "Starting with serial output..."
        QEMU_ARGS="$QEMU_ARGS -serial stdio -display none"
        ;;
    "iso")
        echo "Booting from ISO..."
        if [ -f "build/os.iso" ]; then
            echo "Using ISO: build/os.iso"
            qemu-system-i386 $QEMU_ARGS -cdrom build/os.iso
            exit 0
        else
            echo "ISO file not found! Run 'make iso' first."
            exit 1
        fi
        ;;
    "monitor")
        echo "Starting with QEMU monitor..."
        QEMU_ARGS="$QEMU_ARGS -monitor stdio"
        ;;
    "normal")
        echo "Starting in normal mode..."
        QEMU_ARGS="$QEMU_ARGS -serial file:serial.log"
        ;;
    *)
        echo "Unknown mode: $MODE"
        echo "Available modes: normal, debug, serial, iso, monitor"
        exit 1
        ;;
esac

# 启动QEMU
echo "Starting QEMU with args: $QEMU_ARGS"
echo "Press Ctrl+Alt+G to release mouse, Ctrl+Alt+2 for monitor"
echo ""

if [ -f "build/kernel.bin" ]; then
    qemu-system-i386 $QEMU_ARGS -kernel build/kernel.bin
elif [ -f "build/kernel.elf" ]; then
    qemu-system-i386 $QEMU_ARGS -kernel build/kernel.elf
else
    echo "No kernel file found!"
    exit 1
fi
