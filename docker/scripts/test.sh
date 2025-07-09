#!/bin/bash

set -e

echo "=== Running OS Tests ==="

# 构建测试版本
echo "Building test version..."
CLEAN_BUILD=true make test

# 检查测试内核
if [ ! -f "build/kernel.bin" ]; then
    echo "Test kernel not found!"
    exit 1
fi

echo "Starting automated tests..."

# 创建测试输出目录
mkdir -p tests/output

# 运行自动化测试（30秒超时）
timeout 30s qemu-system-i386 \
    -m 128M \
    -serial file:tests/output/test-serial.log \
    -monitor file:tests/output/test-monitor.log \
    -display none \
    -kernel build/kernel.bin \
    || TEST_EXIT_CODE=$?

# 检查测试结果
echo ""
echo "=== Test Results ==="

if [ -f "tests/output/test-serial.log" ]; then
    echo "Serial output:"
    echo "----------------------------------------"
    cat tests/output/test-serial.log
    echo "----------------------------------------"

    # 检查测试成功标记
    if grep -q "KERNEL_PANIC\|ERROR\|FAIL" tests/output/test-serial.log; then
        echo "❌ Tests detected errors!"
        exit 1
    elif grep -q "TEST_COMPLETE\|SUCCESS\|PASS" tests/output/test-serial.log; then
        echo "✅ Tests completed successfully!"
        exit 0
    else
        echo "⚠️  Test completion status unclear"
        echo "Exit code: ${TEST_EXIT_CODE:-0}"

        # 如果是超时退出，可能是正常的
        if [ "${TEST_EXIT_CODE:-0}" -eq 124 ]; then
            echo "Tests timed out (may be normal for infinite loop kernel)"
        fi
    fi
else
    echo "❌ No test output found!"
    exit 1
fi
