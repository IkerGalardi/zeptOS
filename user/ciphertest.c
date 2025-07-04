#include "aes.h"
#include "kernel/types.h"
#include "user/user.h"

#define BUFFER_SIZE_MBYTES 50
#define BUFFER_SIZE_BYTES  BUFFER_SIZE_MBYTES * 1024 * 1024
#define RDTIME_TIMER_FREQ  10000000

static uint64 rdtime()
{
    uint64 x;
    asm volatile("csrr %0, time" : "=r" (x) );
    return x;
}

static char rand_byte()
{
    const uint64 value = 12958985;
    static uint64 state = 0;

    char random_values[10] = {
        200, 120,  3, 12, 50,
        140,  60, 12, 255, 0
    };

    char random = (random_values[state % 10] + value) % 255;
    state += random;

    return random;
}

int main(int argc, char *argv[])
{
    uint8 *buffer = malloc(BUFFER_SIZE_BYTES);
    for (int i = 0; i < BUFFER_SIZE_BYTES; i++) {
        buffer[i] = rand_byte();
    }

    uint8 aes_key[AES_KEYLEN] = {0};
    for (int i = 0; i < AES_KEYLEN; i++) {
        aes_key[i] = rand_byte();
    }

    uint8 aes_iv[AES_BLOCKLEN] = {0};
    for (int i = 0; i < AES_BLOCKLEN; i++) {
        aes_iv[i] = rand_byte();
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);

    for (int i = 0; i < 50; i++) {
        uint64 start_cycles = rdtime();

        AES_CBC_encrypt_buffer(&ctx, buffer, BUFFER_SIZE_BYTES);

        uint64 end_cycles = rdtime();
        uint64 cycles = end_cycles - start_cycles;

        printf("ciphertest: test %d took %lu\n", i, cycles);
    }
}
