// Physical memory layout

// qemu -machine virt is set up like this,
// based on qemu's hw/riscv/virt.c:
//
// 00001000 -- boot ROM, provided by qemu
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0 
// 10001000 -- virtio disk 
// 80000000 -- boot ROM jumps here in machine mode
//                         -kernel loads the kernel here
// unused RAM after 80000000.

// the kernel uses physical memory thus:
// 80000000 -- entry.S, then kernel text and data
// end -- start of kernel page allocation area
// PHYSTOP -- end RAM used by the kernel

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

// core local interruptor (CLINT), which contains the timer.
#define CLINT_MTIMECMP(hartid) (clint + 0x4000 + 8*(hartid))
#define CLINT_MTIME (clint + 0xBFF8) // cycles since boot.

// qemu puts platform-level interrupt controller (PLIC) here.
#define PLIC_PRIORITY (plic + 0x0)
#define PLIC_PENDING (plic + 0x1000)
#define PLIC_SENABLE(hart) (plic + 0x2080 + (hart)*0x100)
#define PLIC_SPRIORITY(hart) (plic + 0x201000 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (plic + 0x201004 + (hart)*0x2000)

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80200000 to PHYSTOP.
#define KERNBASE 0x80200000L
#define RAMBASE  0x80000000L
#define PHYSTOP (RAMBASE + 128*1024*1024)

// map the trampoline page to the highest address,
// in both user and kernel space.
#define TRAMPOLINE (MAXVA - PGSIZE)

// map kernel stacks beneath the trampoline,
// each surrounded by invalid guard pages.
#define KSTACK(p) (TRAMPOLINE - ((p)+1)* 2*PGSIZE)

// User memory layout.
// Address zero first:
//     text
//     original data and bss
//     fixed-size stack
//     expandable heap
//     ...
//     TRAPFRAME (p->trapframe, used by the trampoline)
//     TRAMPOLINE (the same page as in the kernel)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)
