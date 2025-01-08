#include "dtb.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "sbi.h"

#define BYTESWAP32(num) (((num>>24)&0xff) | ((num<<8)&0xff0000) | \
                        ((num>>8)&0xff00) | ((num<<24)&0xff000000))

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
}
