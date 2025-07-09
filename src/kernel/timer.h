#ifndef TIMER_H
#define TIMER_H

#include "stdint.h"

// PIT (Programmable Interval Timer) 相关定义
#define PIT_FREQUENCY   1193182     // PIT基准频率 (Hz)
#define PIT_CHANNEL0    0x40        // 通道0数据端口
#define PIT_CHANNEL1    0x41        // 通道1数据端口
#define PIT_CHANNEL2    0x42        // 通道2数据端口
#define PIT_COMMAND     0x43        // 命令寄存器

// PIT命令字位定义
#define PIT_CMD_CHANNEL0    0x00    // 选择通道0
#define PIT_CMD_CHANNEL1    0x40    // 选择通道1
#define PIT_CMD_CHANNEL2    0x80    // 选择通道2
#define PIT_CMD_ACCESS_LATCH 0x00   // 锁存计数值
#define PIT_CMD_ACCESS_LOW  0x10    // 只访问低字节
#define PIT_CMD_ACCESS_HIGH 0x20    // 只访问高字节
#define PIT_CMD_ACCESS_BOTH 0x30    // 先低字节后高字节
#define PIT_CMD_MODE0       0x00    // 模式0：计数结束中断
#define PIT_CMD_MODE1       0x02    // 模式1：硬件可重触发单稳
#define PIT_CMD_MODE2       0x04    // 模式2：比率发生器
#define PIT_CMD_MODE3       0x06    // 模式3：方波发生器
#define PIT_CMD_MODE4       0x08    // 模式4：软件触发选通
#define PIT_CMD_MODE5       0x0A    // 模式5：硬件触发选通
#define PIT_CMD_BINARY      0x00    // 二进制计数
#define PIT_CMD_BCD         0x01    // BCD计数


#define TIMER_FREQUENCY     100     // generate 100 clock interrupts per second == every 10 milliseconds an interrupt.
#define TIMER_INTERVAL_MS   (1000 / TIMER_FREQUENCY)

void init_timer(uint32_t frequency);

#endif
