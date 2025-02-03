#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "sbi.h"
#include "dtb.h"

extern void _entry();

static uint64 ram_start = 0;
static uint64 ram_size = 0;

static void dtbparse(void *fdt)
{
    dtb *devicetree = dtb_fromptr(fdt);
    if (devicetree == 0) {
        panic("dtbparse");
    }

    printf("kernel(%d): parsing device tree:\n", cpuid());

    dtb_node memory_node = dtb_find(devicetree, "/memory");
    //uint32 address_cells = DTB_ADDRESS_CELLS_DEFAULT;
    //uint32 size_cells = DTB_SIZE_CELLS_DEFAULT;
    dtb_foreach_property(memory_node, prop) {
        char *propname = dtb_property_name(devicetree, prop);
        if (strncmp("#address-cells", propname, 15) == 0) {
            // address_cells = dtb_property_uint32(prop);
        } else if (strncmp("#size-cells", propname, 12) == 0) {
            // size_cells = dtb_property_uint32(prop);
        } else if (strncmp("reg", propname, 4) == 0) {
            uint64 *property = (uint64 *)dtb_property_array(prop);
            ram_start = DTB_BYTESWAP64(property[0]);
            ram_size = DTB_BYTESWAP64(property[1]);
            printf(" · Ram starts at 0x%lx with size 0x%lx\n", ram_start, ram_size);
        }
    }
}

// start() jumps here in supervisor mode on all CPUs.
void
main(void *fdt)
{
    sbi_debug_console_write(20, "\nxv6 kernel booting\n");
    printfinit();
    consoleinit();
    printf("kernel(%d): console initialized\n", cpuid());
    dtbparse(fdt);
    kinit(ram_start, ram_size);            // physical page allocator
    printf("kernel(%d): physical page allocator initialized\n", cpuid());
    kvminit();          // create kernel page table
    printf("kernel(%d): kernel page table initialized \n", cpuid());
    kvminithart();      // turn on paging
    printf("kernel(%d): virtual memory initialized\n", cpuid());
    procinit();         // process table
    printf("kernel(%d): process table initialized\n", cpuid());
    trapinit();         // trap vectors
    trapinithart();     // install kernel trap vector
    printf("kernel(%d): traps initialized\n", cpuid());
    plicinit();         // set up interrupt controller
    printf("kernel(%d): global interrupt controller initialized\n", cpuid());
    plicinithart();     // ask PLIC for device interrupts
    printf("kernel(%d): local interrupt controller initialized\n", cpuid());
    binit();            // buffer cache
    printf("kernel(%d): buffer cache initialized\n", cpuid());
    iinit();            // inode table
    printf("kernel(%d): inode table initialized\n", cpuid());
    fileinit();         // file table
    printf("kernel(%d): file table initialized\n", cpuid());
    virtio_disk_init(); // emulated hard disk
    printf("kernel(%d): virtio disk driver initialized\n", cpuid());
    userinit();         // first user process
    printf("kernel(%d): first program initialized\n", cpuid());

    if (sbi_probe_extension(SBI_EXTENSION_HSM) != 0) {
        printf("kernel(%d): HSM extension available, starting other cores\n",
                cpuid());

        for (int i = 0; i < NCPU; i++) {
            if (i == cpuid())
                continue;

            struct sbiret res = sbi_hart_start(i, (uint64)(&_entry), 0);

            // INVALID_PARAM is returned when hartid is an invalid hart. Assuming
            // that all valid hartid's are contiguous, we've already runned out
            // of cores so we should stop asking for more startings.
            if (res.error == SBI_ERR_INVALID_PARAM)
                break;

            // TODO: should we panic?
            if (res.error != SBI_SUCCESS) {
                printf("kernel(%d): could not start hart %d, with code %d\n",
                        cpuid(), i, (int)res.error);
            }
        }
    } else {
        printf("kernel(%d): HSM extension not available, only single core used\n",
                cpuid());
    }

    // ask for clock interrupts.
    sbi_set_timer(r_time() + 1000000);

    scheduler();
}

void main_secondary()
{
    printf("kernel(%d): starting auxiliary harts\n", cpuid());
    kvminithart();        // turn on paging
    printf("kernel(%d): virtual memory initialized\n", cpuid());
    trapinithart();     // install kernel trap vector
    printf("kernel(%d): traps initialized\n", cpuid());
    plicinithart();     // ask PLIC for device interrupts
    printf("kernel(%d): local interrupt controller initialized\n", cpuid());

    // ask for clock interrupts.
    sbi_set_timer(r_time() + 1000000);

    scheduler();
}
