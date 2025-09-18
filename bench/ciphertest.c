#include "aes.h"
#include "kernel/types.h"
#include "user/user.h"

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

void do_test(uint8 *buffer, uint64 buffer_size_mb, struct AES_ctx *ctx)
{
    uint64 buffer_size = buffer_size_mb * 1024 * 1024;

    // Fill the buffer with random data
    for (int i = 0; i < buffer_size; i++) {
        buffer[i] = rand_byte();
    }

    for (int i = 0; i < 10; i++) {
        uint64 start_cycles = rdtime();
        AES_CBC_encrypt_buffer(ctx, buffer, buffer_size);

        uint64 end_cycles = rdtime();
        uint64 cycles = end_cycles - start_cycles;

        printf("ciphertest: %lu MB took %lu cycles\n", buffer_size_mb, cycles);
    }
}

int main(int argc, char *argv[])
{
    uint8 *buffer = malloc(50 * 1024 * 1024);
    for (int i = 0; i < 50 * 1024 * 1024; i++) {
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

    do_test(buffer, 1, &ctx);
    do_test(buffer, 25, &ctx);
    do_test(buffer, 50, &ctx);
}
