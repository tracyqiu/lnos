#ifndef DISK_H
#define DISK_H

#include <stdint.h>

void disk_init(void);
int32_t disk_read(uint32_t lba, uint32_t count, void* buffer);
int32_t disk_write(uint32_t lba, uint32_t count, const void* buffer);

#endif