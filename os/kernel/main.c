#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "sbi.h"


// start() jumps here in supervisor mode on all CPUs.
void
main()
{
    sbi_debug_console_write(20, "\nxv6 kernel booting\n");
    printfinit();
    consoleinit();
    printf("kernel(%d): console initialized\n", cpuid());
    kinit();            // physical page allocator
    kvminit();          // create kernel page table
    kvminithart();      // turn on paging
    printf("kernel(%d): virtual memory initialized\n", cpuid());
    procinit();         // process table
    trapinit();         // trap vectors
    trapinithart();     // install kernel trap vector
    plicinit();         // set up interrupt controller
    plicinithart();     // ask PLIC for device interrupts
    printf("kernel(%d): interrupts initialized\n", cpuid());
    binit();            // buffer cache
    iinit();            // inode table
    fileinit();         // file table
    virtio_disk_init(); // emulated hard disk
    printf("kernel(%d): storage subsystem initialized\n", cpuid());
    userinit();         // first user process
    printf("kernel(%d): first program initialized\n", cpuid());

    scheduler();
}

void main_secondary()
{
    printf("kernel(%d): starting auxiliary harts\n", cpuid());
    kvminithart();        // turn on paging
    trapinithart();     // install kernel trap vector
    plicinithart();     // ask PLIC for device interrupts
    scheduler();
}