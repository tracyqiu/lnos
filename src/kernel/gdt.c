#include "gdt.h"
#include "stdint.h"
#include "print.h"

typedef struct {
   uint16_t limit_low;     // limit (bits 0-15)
   uint16_t base_low;      // base (bits 0-15)
   uint8_t base_middle;    // base (bits 16-23)
   uint8_t access;         // access
   uint8_t granularity;    // limit (bits 16-19) | flags
   uint8_t base_high;      // base (bits 24-31)
} __attribute__((packed)) GDT_Entry;

// register GDTR
typedef struct {
   uint16_t limit;         // sizeof(gdt) - 1
   uint32_t base;          // address of GDT == GDT_Entry* addr
} __attribute__((packed)) GDT_Descriptor;


typedef enum
{
   GDT_ACCESS_CODE_READABLE_EXECUTABLE    = 0x0A,   // type = 1010
   GDT_ACCESS_DATA_READABLE_WRITEABLE     = 0x02,   // type = 0010

   GDT_ACCESS_DATA_SEGMENT                = 0x10,
   GDT_ACCESS_CODE_SEGMENT                = 0x10,

   GDT_ACCESS_DPL_0                       = 0x00,
   GDT_ACCESS_DPL_1                       = 0x20,
   GDT_ACCESS_DPL_2                       = 0x40,
   GDT_ACCESS_DPL_3                       = 0x60,

   GDT_ACCESS_PRESENT                     = 0x80,
} GDT_ACCESS;

typedef enum
{
   GDT_FLAG_64BIT                         = 0x20,
   GDT_FLAG_32BIT                         = 0x40,
   GDT_FLAG_16BIT                         = 0x00,

   GDT_FLAG_GRANULARITY_1B                = 0x00,
   GDT_FLAG_GRANULARITY_4K                = 0x80,
} GDT_FLAGS;


#define GDT_LIMIT_LOW(limit)                (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                  (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)               ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)    (((limit >> 16) & 0x0F) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                 ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) {                    \
   GDT_LIMIT_LOW(limit),                                           \
   GDT_BASE_LOW(base),                                             \
   GDT_BASE_MIDDLE(base),                                          \
   access,                                                         \
   GDT_FLAGS_LIMIT_HI(limit, flags),                               \
   GDT_BASE_HIGH(base)                                             \
}


GDT_Entry gdt[] = {
   // NULL descriptor
   GDT_ENTRY(0, 0, 0, 0),

   // Kernel 32-bit code segment
   GDT_ENTRY(0,
             0xFFFFF,
             GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE_EXECUTABLE, //10011010b  P=1存在，DPL=00特权级0，S=1代码段，TYPE=1010代码段可读可执行
             (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)), // 11001111b; G=1段界限单位4KB, G=0表示1B，D=1表示32位保护模式，L=0, AVL=0, 后4位是段界限19-16位(会与15-0位组合)

   // Kernel 32-bit data segment
   GDT_ENTRY(0,
             0xFFFFF,
             GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_READABLE_WRITEABLE, //10011010b  P=1存在，DPL=00特权级0，S=1代码段，TYPE=0010数据段可读可写
             (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)),

   // User 32-bit code segment
   GDT_ENTRY(0,
             0xFFFFF,
             GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE_EXECUTABLE,
             (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)),

   // User 32-bit data segment
   GDT_ENTRY(0,
             0xFFFFF,
             GDT_ACCESS_PRESENT | GDT_ACCESS_DPL_3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_READABLE_WRITEABLE,
             (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)),
};

GDT_Descriptor lgdt = {
   sizeof(GDT_Entry) * 5 - 1,
   (uint32_t)&gdt
};

extern void __attribute__((cdecl)) load_gdt(uint32_t descriptor, uint16_t code_segment, uint16_t data_segment);

//------------------------------------------------------------------------------
void init_gdt()
//------------------------------------------------------------------------------
{
   load_gdt((uint32_t)&lgdt, 0x08, 0x10);
}
