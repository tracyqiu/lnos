#include "disk.h"
#include "ata.h"
#include "print.h"

//------------------------------------------------------------------------------
void disk_init(void)
//------------------------------------------------------------------------------
{
}

//------------------------------------------------------------------------------
int32_t disk_read(uint32_t lba, uint32_t count, void* buffer)
//------------------------------------------------------------------------------
{
   // ATA supports a maximum of 255 sectors at a time
   uint8_t* buf = (uint8_t*)buffer;
   while (count > 0) {
      uint8_t n = (count > 255) ? 255 : (uint8_t)count;
      if (!ata_read_sectors(lba, n, buf)) return 0;
      lba += n;
      buf += n * 512;
      count -= n;
   }
   return 1;
}

//------------------------------------------------------------------------------
int32_t disk_write(uint32_t lba, uint32_t count, const void* buffer)
//------------------------------------------------------------------------------
{
   const uint8_t* buf = (const uint8_t*)buffer;
   while (count > 0) {
      uint8_t n = (count > 255) ? 255 : (uint8_t)count;
      if (!ata_write_sectors(lba, n, buf)) return 0;
      lba += n;
      buf += n * 512;
      count -= n;
   }
   return 1;
}