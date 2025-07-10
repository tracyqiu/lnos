#include "ata.h"
#include "x86.h"

#define ATA_DATA_PORT      0x1F0
#define ATA_SECTOR_COUNT   0x1F2
#define ATA_LBA_LOW        0x1F3
#define ATA_LBA_MID        0x1F4
#define ATA_LBA_HIGH       0x1F5
#define ATA_DRIVE_SELECT   0x1F6
#define ATA_COMMAND_PORT   0x1F7
#define ATA_STATUS_PORT    0x1F7

#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30

#define ATA_STATUS_BSY    0x80
#define ATA_STATUS_DRQ    0x08

//------------------------------------------------------------------------------
static bool ata_wait_bsy()
//------------------------------------------------------------------------------
{
   int timeout = 1000000;
   while ((x86_inb(ATA_STATUS_PORT) & ATA_STATUS_BSY) && --timeout);
   return timeout > 0;
}

//------------------------------------------------------------------------------
static bool ata_wait_drq()
//------------------------------------------------------------------------------
{
   int timeout = 1000000;
   while (!(x86_inb(ATA_STATUS_PORT) & ATA_STATUS_DRQ) && --timeout);
   return timeout > 0;
}

//------------------------------------------------------------------------------
int32_t ata_read_sectors(uint32_t lba, uint8_t sector_count, uint8_t* buffer)
//------------------------------------------------------------------------------
{
   if (sector_count == 0) return 0;
   ata_wait_bsy();

   x86_outb(ATA_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));
   x86_outb(ATA_SECTOR_COUNT, sector_count);
   x86_outb(ATA_LBA_LOW, (uint8_t)lba);
   x86_outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
   x86_outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
   x86_outb(ATA_COMMAND_PORT, ATA_CMD_READ_SECTORS);

   for (uint8_t s = 0; s < sector_count; s++) {
      if (!ata_wait_bsy()) return 0;
      if (!ata_wait_drq()) return 0;
      uint16_t* buf16 = (uint16_t*)buffer;
      for (int i = 0; i < 256; i++) {
         buf16[i] = x86_inw(ATA_DATA_PORT);
      }
      buffer += 512;
   }
   return 1;
}

//------------------------------------------------------------------------------
int32_t ata_write_sectors(uint32_t lba, uint8_t sector_count, const uint8_t* buffer)
//------------------------------------------------------------------------------
{
   if (sector_count == 0) return 0;
   ata_wait_bsy();

   x86_outb(ATA_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));
   x86_outb(ATA_SECTOR_COUNT, sector_count);
   x86_outb(ATA_LBA_LOW, (uint8_t)lba);
   x86_outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
   x86_outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
   x86_outb(ATA_COMMAND_PORT, ATA_CMD_WRITE_SECTORS);

   for (uint8_t s = 0; s < sector_count; s++) {
      if (!ata_wait_bsy()) return 0;
      if (!ata_wait_drq()) return 0;
      for (int i = 0; i < 256; i++) {
         x86_outw(ATA_DATA_PORT, ((const uint16_t*)buffer)[i]);
      }
      buffer += 512;
   }
   return 1;
}