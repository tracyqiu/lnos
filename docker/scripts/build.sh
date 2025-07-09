#!/bin/bash

set -e

echo "=== Building OS Kernel ==="

# 检查交叉编译工具链
echo "Checking cross-compiler toolchain..."
if ! command -v i686-elf-gcc &> /dev/null; then
    echo "Error: i686-elf-gcc not found!"
    exit 1
fi

echo "Using toolchain:"
echo "  GCC: $(i686-elf-gcc --version | head -n1)"
echo "  Binutils: $(i686-elf-ld --version | head -n1)"
echo "  NASM: $(nasm --version)"
echo ""

# 创建构建目录
mkdir -p build

# 清理之前的构建（可选）
if [ "${CLEAN_BUILD:-false}" = "true" ]; then
    echo "Cleaning previous build..."
    make clean
fi

# 构建内核
echo "Building kernel..."
make all

echo ""
echo "=== Build Summary ==="

# 检查构建产物
if [ -f "build/kernel.bin" ]; then
    echo "✓ Kernel binary: build/kernel.bin"
    ls -la build/kernel.bin

    # 显示内核信息
    echo "Kernel size: $(stat -c%s build/kernel.bin) bytes"
    echo "Kernel type: $(file build/kernel.bin)"
else
    echo "✗ Kernel binary not found!"
    exit 1
fi

if [ -f "build/kernel.elf" ]; then
    echo "✓ Kernel ELF: build/kernel.elf"
    # 显示符号表信息
    echo "Symbols: $(i686-elf-nm build/kernel.elf | wc -l) symbols"
fi

if [ -f "build/os.iso" ]; then
    echo "✓ ISO image: build/os.iso"
    ls -la build/os.iso
fi

echo ""
echo "Build completed successfully!"
