#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Interpret virtual addresses on the Sv39 virtual memory system.\n");
        fprintf(stderr, "Usage: rvvaddr <vaddr>.\n");
        return 1;
    }

    // TODO: detect if its a decimal number or hex (by checking if it starts
    // with 0x or not)
    uint64_t vaddr = (uint64_t)strtol(argv[1], NULL, 16);

    uint64_t pgoff = vaddr & 0xFFF;
    uint64_t vpn0  = (vaddr >> 12) & 0x1FF;
    uint64_t vpn1  = (vaddr >> (12 + 9)) & 0x1FF;
    uint64_t vpn2  = (vaddr >> (12 + 9 + 9)) & 0x1FF;
    uint64_t vpn = vpn0 | (vpn1 << 9) | (vpn2 << 18);

    printf("offset = %lx\n", pgoff);
    printf("vpn = %lx (vpn0 = %lx, vpn1 = %lx, vpn2 = %lx)\n", vpn, vpn0, vpn1, vpn2);
}
