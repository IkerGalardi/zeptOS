#include "dtb.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "sbi.h"

extern int strncmp(const char *p, const char *q, uint n);

#define BYTESWAP32(num) ((((num)>>24)&0xff) | (((num)<<8)&0xff0000) | \
                        (((num)>>8)&0xff00) | (((num)<<24)&0xff000000))

#define BYTESWAP64(num) (BYTESWAP32(num) << 32 | BYTESWAP32((num) >> 32))

#define FDT_BEGIN_NODE BYTESWAP32((uint32)0x1)
#define FDT_END_NODE   BYTESWAP32((uint32)0x2)
#define FDT_PROP       BYTESWAP32((uint32)0x3)
#define FDT_NOP        BYTESWAP32((uint32)0x4)
#define FDT_END        BYTESWAP32((uint32)0x9)

uint64 ram_start = 0;
uint64 ram_size = 0;

struct fdtheader {
    uint32 magic;
    uint32 totalsize;
    uint32 off_dt_struct;
    uint32 off_dt_strings;
    uint32 off_mem_rsvmap;
    uint32 version;
    uint32 last_comp_version;
    uint32 boot_cpuid_phys;
    uint32 size_dt_strings;
    uint32 size_dt_struct;
} __attribute__((packed));

struct fdtreserved {
    uint64 address;
    uint64 size;
};

static void memory_property(char *name, uint32 *property)
{
    if (strncmp(name, "reg", 4) == 0) {
        uint64 *property64 = (uint64 *)property;

        /// NOTE: documentation of the device tree file tells us that this
        ///       should be a pair array (first element being the start and the
        ///       second the size). Instead, Qemu's device tree gives us garbage
        ///       as the first value and then the start/size pair.
        ram_start = BYTESWAP64(*(property64 + 1));
        ram_size = BYTESWAP64(*(property64 + 2));
    }
}

void dtbparse(void *fdt)
{
    struct fdtheader *header = fdt;

    if (header->magic != BYTESWAP32(0xd00dfeed)) {
        printf("kernel(%d): wrong device tree magic number %x\n", cpuid(),
               BYTESWAP32(header->magic));
        return;
    }

    printf("kernel(%d): version %d, struct off %x, strings off %x\n", cpuid(),
           BYTESWAP32(header->version), BYTESWAP32(header->off_dt_struct),
           BYTESWAP32(header->off_dt_strings));

    struct fdtreserved *reserved = (struct fdtreserved *)((char *)fdt +
                                   BYTESWAP32(header->off_mem_rsvmap));
    while (!(reserved->address == 0 && reserved->size == 0)) {
        printf("kernel(%d): reserved zone at %lx with size %ld", cpuid(),
               reserved->address, reserved->size);

        reserved++;
    }

    enum {
        DEVICE_ROOT,
        DEVICE_MEMORY,
        DEVICE_UNKNOWN
    } node = DEVICE_ROOT;

    uint32 *property = (uint32 *)((char *)fdt + BYTESWAP32(header->off_dt_struct));
    char *strings = (char *)fdt + BYTESWAP32(header->off_dt_strings);
    while (*property != FDT_END) {
        if (*property == FDT_BEGIN_NODE) {
            property++;
            printf("kernel(%d): node %s\n", cpuid(), (char *)property);

            if (node == DEVICE_ROOT && strncmp((char *)property, "memory", 6) == 0) {
                node = DEVICE_MEMORY;
            }
        } else if (*property == FDT_PROP) {
            uint32 len = BYTESWAP32(*(property + 1));
            uint32 str_off = BYTESWAP32(*(property + 2));

            printf("kernel(%d): property %s\n", cpuid(), strings + str_off);

            if (node == DEVICE_MEMORY) {
                memory_property(strings + str_off, property+1);
            }

            property += len / sizeof(uint32) + 2;
        } else if (*property == FDT_END_NODE) {
            printf("kernel(%d): end node\n", cpuid());

            if (node == DEVICE_MEMORY) {
                node = DEVICE_ROOT;
            }
        } else if (*property == FDT_NOP) {
            property++;
            printf("kernel(%d): nop\n", cpuid());
        }

        property++;
    }
}
