#ifndef ATA_H
#define ATA_H

#include <stdint.h>

int32_t ata_read_sectors(uint32_t lba, uint8_t sector_count, uint8_t* buffer);
int32_t ata_write_sectors(uint32_t lba, uint8_t sector_count, const uint8_t* buffer);

#endif