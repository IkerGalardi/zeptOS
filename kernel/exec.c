#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "elf.h"

static int loadseg(pde_t *, uint64, struct inode *, uint, uint);

int flags2perm(int flags)
{
        int perm = 0;
        if(flags & 0x1)
            perm = PTE_X;
        if(flags & 0x2)
            perm |= PTE_W;
        return perm;
}

int
exec(char *path, char **argv)
{
    char *s, *last;
    int i, off;
    uint64 argc, sz = 0, sp, ustack[MAXARG], stackbase;
    struct elfhdr elf;
    struct inode *ip;
    struct proghdr ph;
    pagetable_t pagetable = 0, oldpagetable;
    struct proc *p = myproc();

    begin_op();

    if((ip = namei(path)) == 0){
        end_op();
        return -1;
    }
    ilock(ip);

    // Check ELF header
    if(readi(ip, 0, (uint64)&elf, 0, sizeof(elf)) != sizeof(elf)) {
        printf("kernel(%d): exec %s: malformed header\n", cpuid(), path);
        goto bad;
    }

    if(elf.magic != ELF_MAGIC) {
        printf("kernel(%d): exec %s: wrong magic\n", cpuid(), path);
        goto bad;
    }
    if((pagetable = proc_pagetable(p)) == 0) {
        printf("kernel(%d): exec %s: could not get own pagetable\n", cpuid(), path);
        goto bad;
    }

    // Load program into memory.
    for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
        if(readi(ip, 0, (uint64)&ph, off, sizeof(ph)) != sizeof(ph)) {
            printf("kernel(%d): exec %s: could not get program header %d\n", cpuid(), path, i);
            goto bad;
        }
        if(ph.type != ELF_PROG_LOAD)
            continue;
        if(ph.memsz < ph.filesz) {
            printf("kernel(%d): exec %s: program header %d memory size is smaller than file size\n", cpuid(), path, i);
            goto bad;
        }
        if(ph.vaddr + ph.memsz < ph.vaddr) {
            printf("kernel(%d): exec %s: unknown error on program header %d", cpuid(), path, i);
            goto bad;
        }
        uint64 sz1;
        if((sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz, flags2perm(ph.flags))) == 0) {
            printf("kernel(%d): exec %s: program header %d could not be mapped\n", cpuid(), path, i);
            goto bad;
        }
        sz = sz1;
        if(loadseg(pagetable, ph.vaddr, ip, ph.off, ph.filesz) < 0)
            goto bad;
    }
    iunlockput(ip);
    end_op();
    ip = 0;

    p = myproc();
    uint64 oldsz = p->sz;

    // Allocate two pages at the next page boundary.
    // Make the first inaccessible as a stack guard.
    // Use the second as the user stack.
    sz = PGROUNDUP(sz);
    uint64 sz1;
    if((sz1 = uvmalloc(pagetable, sz, sz + 2*PGSIZE, PTE_W)) == 0)
        goto bad;
    sz = sz1;
    uvmclear(pagetable, sz-2*PGSIZE);
    sp = sz;
    stackbase = sp - PGSIZE;

    // Push argument strings, prepare rest of stack in ustack.
    for(argc = 0; argv[argc]; argc++) {
        if(argc >= MAXARG)
            goto bad;
        sp -= strlen(argv[argc]) + 1;
        sp -= sp % 16; // riscv sp must be 16-byte aligned
        if(sp < stackbase)
            goto bad;
        if(copyout(pagetable, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
            goto bad;
        ustack[argc] = sp;
    }
    ustack[argc] = 0;

    // push the array of argv[] pointers.
    sp -= (argc+1) * sizeof(uint64);
    sp -= sp % 16;
    if(sp < stackbase)
        goto bad;
    if(copyout(pagetable, sp, (char *)ustack, (argc+1)*sizeof(uint64)) < 0)
        goto bad;

    // arguments to user main(argc, argv)
    // argc is returned via the system call return
    // value, which goes in a0.
    p->trapframe->a1 = sp;

    // Save program name for debugging.
    for(last=s=path; *s; s++)
        if(*s == '/')
            last = s+1;
    safestrcpy(p->name, last, sizeof(p->name));

    // Commit to the user image.
    oldpagetable = p->pagetable;
    p->pagetable = pagetable;
    p->sz = sz;
    p->trapframe->epc = elf.entry;    // initial program counter = main
    p->trapframe->sp = sp; // initial stack pointer
    p->trapframe->ssp = SHADOW_STACK + PGSIZE - 8;
    proc_freepagetable(oldpagetable, oldsz);

    return argc; // this ends up in a0, the first argument to main(argc, argv)

 bad:
    if(pagetable)
        proc_freepagetable(pagetable, sz);
    if(ip){
        iunlockput(ip);
        end_op();
    }
    return -1;
}

static uint min_uint(uint a, uint b)
{
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

// Load a program segment into pagetable at virtual address va.
// va must be page-aligned
// and the pages from va to va+sz must already be mapped.
// Returns 0 on success, -1 on failure.
static int
loadseg(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint sz)
{
    uint i, n, forstart = 0;
    uint64 pa;

    if (va % PGSIZE != 0) {
        const uint pgleft = PGSIZE - (va & 0xfff);

        pa = walkaddr(pagetable, va);
        if (pa == 0)
            panic("loadseg: address should exist");

        n = min_uint(pgleft, sz);
        if(readi(ip, 0, (uint64)pa, offset, n) != n) {
            return -1;
        }

        if (sz <= pgleft) {
            return 0;
        }

        forstart = PGSIZE;
    }

    for(i = forstart; i < sz; i += PGSIZE){
        pa = walkaddr(pagetable, va + i);
        if(pa == 0)
            panic("loadseg: address should exist");
        if(sz - i < PGSIZE)
            n = sz - i;
        else
            n = PGSIZE;
        if(readi(ip, 0, (uint64)pa, offset+i, n) != n)
            return -1;
    }
    
    return 0;
}
