#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "globals.h"
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

    dtb_node chosen_node = dtb_find(devicetree, "/chosen");
    if (chosen_node == 0) {
        sbi_debug_console_write(16, "no /chosen node");
        while(1);
    }

    dtb_node stdout_node = 0;
    char *stdout_path = 0;
    dtb_foreach_property(chosen_node, prop) {
        char *propname = dtb_property_name(devicetree, prop);
        if (strncmp("stdout-path", propname, 12) == 0) {
            stdout_path = dtb_property_string(prop);
            stdout_node = dtb_find(devicetree, stdout_path);
        }
    }
    if (stdout_node == 0) {
        sbi_debug_console_write(28, "no /chosen:stdout-path node");
        while(1);
    }

    dtb_foreach_property(stdout_node, prop) {
        char *propname = dtb_property_name(devicetree, prop);

        if (strncmp("compatible", propname, 11) == 0) {
            char *value = dtb_property_string(prop);
            if (strncmp("ns16550a", value, 9) != 0) {
                sbi_debug_console_write(17, "incompatible uart");
                while(1);
            }
        } else if (strncmp("reg", propname, 4) == 0) {
            uint64 *regs = (uint64 *)dtb_property_array(prop);
            uart0 = DTB_BYTESWAP64(regs[0]);
        } else if (strncmp("interrupts", propname, 11) == 0) {
            uart0_irq = dtb_property_uint32(prop);
        }
    }

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
        }
    }

    int found_virtio_disk = 0;
    dtb_node soc_node = dtb_find(devicetree, "/soc");
    dtb_foreach_child(soc_node, dev_node) {
        char *compatible = 0;
        uint64 reg = 0;
        uint64 irq = 0;
        dtb_foreach_property(dev_node, prop) {
            char *propname = dtb_property_name(devicetree, prop);

            if (strncmp("compatible", propname, 11) == 0) {
                compatible = dtb_property_string(prop);
            } else if (strncmp("reg", propname, 4) == 0) {
                reg = dtb_property_uint64(prop);
            } else if (strncmp("interrupts", propname, 11) == 0) {
                irq = dtb_property_uint32(prop);
            }
        }

        if (strncmp("sifive,plic-1.0.0", compatible, 18) == 0) {
            plic = reg;
        } else if (strncmp("sifive,clint0", compatible, 14) == 0) {
            clint = reg;
        } else if (strncmp("virtio,mmio", compatible, 12) == 0) {
            virtio0 = reg;
            virtio0_irq = irq;
            if (found_virtio_disk == 0 && virtio_disk_probe() == 0) {
                found_virtio_disk = 1;
            }
        }
    }
    if (found_virtio_disk == 0) {
        panic("virtio disk not found");
    }
}

// start() jumps here in supervisor mode on all CPUs.
void
kmain(void *fdt)
{
    sbi_debug_console_write(20, "\nxv6 kernel booting\n");
    dtbparse(fdt);

    printfinit();
    consoleinit();
    printf("kernel(%d): console initialized\n", cpuid());
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

#ifdef CONFIG_KERNEL_LANDING_PAD_ENABLED
    sbi_fwft_set(SBI_FEAT_LANDING_PAD, 1, 0);
    printf("kernel(%d): kernel landing pads enabled\n", cpuid());
#endif // CONFIG_KERNEL_LANDING_PAD_ENABLED

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

#ifdef CONFIG_USER_LANDING_PAD_ENABLED
    uint64 senvcfg_lp = r_senvcfg();
    senvcfg_lp |= 1 << 2;
    w_senvcfg(senvcfg_lp);
    printf("kernel(%d): enabled landing pads for user mode\n", cpuid());
#endif // CONFIG_USER_LANDING_PAD_ENABLED

#ifdef CONFIG_USER_SHADOW_STACK_HARDWARE
    uint64 senvcfg_ss = r_senvcfg();
    senvcfg_ss |= 1 << 3;
    w_senvcfg(senvcfg_ss);
    sbi_fwft_set(SBI_FEAT_SHADOW_STACK, 1, 0);
    printf("kernel(%d): enabled shadow stack for user mode\n", cpuid());
#endif // CONFIG_USER_SHADOW_STACK_HARDWARE

    // ask for clock interrupts.
    sbi_set_timer(r_time() + 1000000);

    scheduler();
}

void kmain_secondary()
{
    printf("kernel(%d): starting auxiliary harts\n", cpuid());
    kvminithart();        // turn on paging
    printf("kernel(%d): virtual memory initialized\n", cpuid());
    trapinithart();     // install kernel trap vector
    printf("kernel(%d): traps initialized\n", cpuid());
    plicinithart();     // ask PLIC for device interrupts
    printf("kernel(%d): local interrupt controller initialized\n", cpuid());

#ifdef CONFIG_KERNEL_LANDING_PAD_ENABLED
    sbi_fwft_set(SBI_FEAT_LANDING_PAD, 1, 0);
    printf("kernel(%d): kernel landing pads enabled\n", cpuid());
#endif // CONFIG_KERNEL_LANDING_PAD_ENABLED

#ifdef CONFIG_USER_LANDING_PAD_ENABLED
    uint64 senvcfg_lp = r_senvcfg();
    senvcfg_lp |= 1 << 2;
    w_senvcfg(senvcfg_lp);
    printf("kernel(%d): enabled landing pads for user mode\n", cpuid());
#endif // CONFIG_USER_LANDING_PAD_ENABLED

#ifdef CONFIG_USER_SHADOW_STACK_HARDWARE
    uint64 senvcfg_ss = r_senvcfg();
    senvcfg_ss |= 1 << 3;
    w_senvcfg(senvcfg_ss);
    sbi_fwft_set(SBI_FEAT_SHADOW_STACK, 1, 0);
    printf("kernel(%d): enabled shadow stack for user mode\n", cpuid());
#endif // CONFIG_USER_SHADOW_STACK_HARDWARE

    // ask for clock interrupts.
    sbi_set_timer(r_time() + 1000000);

    scheduler();
}
