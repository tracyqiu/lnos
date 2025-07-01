#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H


#define PAGE_BITMAP_ADDR      0x103000 //0x103000 ~ 0x110000 can mark more than 1.6G physical memory


#define PAGE_DIR_ENTRIES      1024
#define PAGE_TABLE_ENTRIES    1024

#define PAGE_DIR_ADDR         0x110000
#define FIRST_PAGE_TABLE_ADDR 0x111000

#define PAGE_TABLE_POOL_SIZE  256
#define PAGE_TABLE_POOL_START 0x120000
#define PAGE_TABLE_POOL_END   (PAGE_TABLE_POOL_START + PAGE_TABLE_POOL_SIZE * PAGE_SIZE)

#define HEAP_START_ADDR       0x220000


#define PAGE_SIZE          4096

#define PAGE_PRESENT       0x1
#define PAGE_WRITE         0x2


#define KERNEL_STARTED_VIRTUAL_ADDRESS 0xC0000000  // 1G


#define TASK_ENTRIES       1024

#define DEF_DEBUG_TRACE    0

#endif