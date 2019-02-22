#include <assert.h>
#include "common.h"
#include "mont.h"
#include "ec.h"
#include "endianess.h"

void print_x(const char *s, const uint64_t *number, const MontContext *ctx);
Workplace *new_workplace(const MontContext *ctx);
void free_workplace(Workplace *wp);

void ec_jacobian_to_affine(uint64_t *x3, uint64_t *y3,
                         const uint64_t *x1, uint64_t *y1, uint64_t *z1,
                         Workplace *tmp,
                         const MontContext *ctx);

void ec_full_double(uint64_t *x3, uint64_t *y3, uint64_t *z3,
                    const uint64_t *x1, const uint64_t *y1, const uint64_t *z1,
                    Workplace *tmp, const MontContext *ctx);
void ec_mix_add(uint64_t *x3, uint64_t *y3, uint64_t *z3,
                       const uint64_t *x1, const uint64_t *y1, const uint64_t *z1,
                       const uint64_t *x2, const uint64_t *y2,
                       Workplace *tmp,
                       const MontContext *ctx);
void ec_full_add(uint64_t *x3, uint64_t *y3, uint64_t *z3,
                        const uint64_t *x1, const uint64_t *y1, const uint64_t *z1,
                        const uint64_t *x2, const uint64_t *y2, const uint64_t *z2,
                        Workplace *tmp,
                        const MontContext *ctx);
void ec_scalar(uint64_t *x3, uint64_t *y3, uint64_t *z3,
                   const uint64_t *x1, const uint64_t *y1, const uint64_t *z1,
                   const uint8_t *exp, size_t exp_size, uint64_t seed,
                   Workplace *wp1,
                   Workplace *wp2,
                   const MontContext *ctx);

int ec_scalar_g_p256(uint64_t *x3, uint64_t *y3, uint64_t *z3,
                      const uint8_t *exp, size_t exp_size,
                      uint64_t seed,
                      Workplace *wp1,
                      Workplace *wp2,
                      const MontContext *ctx);

void test_ec_jacobian_to_affine(void)
{
    Workplace *wp;
    MontContext *ctx;
    const uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint64_t *x, *y, *z;
    uint8_t buffer[32];
    uint64_t zero[4] = { 0 };

    mont_context_init(&ctx, modulus, sizeof(modulus));
    wp = new_workplace(ctx);

    /** Arbitrary point on the curve with Z=10 **/
    mont_from_bytes(&x, (uint8_t*)"\xf3\x91\x4a\x3a\xf2\x1b\x11\x44\x58\x3e\xf2\xf8\x54\x01\x4b\x72\xfa\x94\x05\x8d\xf9\x7c\x32\x4f\x1a\xef\x49\x37\x3c\xe8\x5b\xef", 32, ctx);
    mont_from_bytes(&y, (uint8_t*)"\x23\xaa\x65\x85\x4c\xc5\xbc\x53\x0d\x4f\xe7\x3e\xd9\x58\x95\x67\xb2\xea\x79\x1a\x7c\x9b\xe5\xf6\x78\x8c\xd5\xbe\xd8\x55\x0d\xe7", 32, ctx);
    mont_from_bytes(&z, (uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32, ctx);
    ec_jacobian_to_affine(x, y, x, y, z, wp, ctx);
    mont_to_bytes(buffer, x, ctx);
    assert(0 == memcmp(buffer, (uint8_t*)"\x84\xfe\xe4\x94\x8f\x38\x97\x36\xf4\x15\x1c\x07\x9a\x70\xa7\x27\x8c\xbe\xeb\x93\xd9\x88\xec\x05\xe9\x3a\xb6\x7d\xfe\x0c\x90\x47", 32));
    mont_to_bytes(buffer, y, ctx);
    assert(0 == memcmp(buffer, (uint8_t*)"\x7a\xa8\xdf\xd7\xf9\x6b\xb7\xbe\x00\x86\x7a\xda\xf1\x5e\x8d\x76\x1e\xa4\x96\x23\x93\xd6\x2c\x01\x86\xc6\xcb\xfd\x5f\xf5\xd8\x55", 32));

    /** Point-at-infinity **/
    memset(x, 0xFF, 32);
    memset(y, 0xFF, 32);
    memset(z, 0, 32);
    ec_jacobian_to_affine(x, y, x, y, z, wp, ctx);
    assert(0 == memcmp(x, zero, 32));
    assert(0 == memcmp(y, zero, 32));

    free(x);
    free(y);
    free(z);

    free_workplace(wp);
    mont_context_free(ctx);
}

void test_ec_full_double(void)
{
    Workplace *wp;
    MontContext *ctx;
    const uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint64_t *x, *y, *z;
    uint8_t buffer[32];
    uint64_t zero[4] = { 0 };

    mont_context_init(&ctx, modulus, sizeof(modulus));
    wp = new_workplace(ctx);

    /** Arbitrary point on the curve with Z=10 **/
    mont_from_bytes(&x, (uint8_t*)"\xf3\x91\x4a\x3a\xf2\x1b\x11\x44\x58\x3e\xf2\xf8\x54\x01\x4b\x72\xfa\x94\x05\x8d\xf9\x7c\x32\x4f\x1a\xef\x49\x37\x3c\xe8\x5b\xef", 32, ctx);
    mont_from_bytes(&y, (uint8_t*)"\x23\xaa\x65\x85\x4c\xc5\xbc\x53\x0d\x4f\xe7\x3e\xd9\x58\x95\x67\xb2\xea\x79\x1a\x7c\x9b\xe5\xf6\x78\x8c\xd5\xbe\xd8\x55\x0d\xe7", 32, ctx);
    mont_from_bytes(&z, (uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32, ctx);
    ec_full_double(x, y, z, x, y, z, wp, ctx);

    mont_to_bytes(buffer, x, ctx);
    assert(0 == memcmp(buffer, "\x0d\x6b\xd2\x61\x23\x64\xf0\x20\xfd\xb0\x5c\x48\x76\xcc\xd8\xe5\xdc\x5e\x05\x05\xd0\x80\x80\x8a\xcc\x79\xca\x1d\xf7\xfb\x0a\x09", 32));
    mont_to_bytes(buffer, y, ctx);
    assert(0 == memcmp(buffer, "\x48\xa6\x95\x51\x52\x0e\xd9\x5d\xe5\x05\x22\xd7\x50\x2f\x1b\x96\x6c\xa7\x78\x03\x6c\xec\x9e\x44\x91\xc5\x1d\x92\x13\x4b\x98\x7d", 32));
    mont_to_bytes(buffer, z, ctx);
    assert(0 == memcmp(buffer, "\xc9\x4f\xee\x6b\xff\x72\xb6\x7b\x0a\x3e\x10\xe8\xfa\xeb\xac\x19\xfa\x51\x76\x0f\xbc\x2d\xf7\x41\x6b\x00\xb2\xe8\xe6\xa5\x16\x0e", 32));

    /** Point-at-infinity **/
    mont_set(x, 1, NULL, ctx);
    mont_set(y, 1, NULL, ctx);
    mont_set(z, 0, NULL, ctx);
    ec_full_double(x, y, z, x, y, z, wp, ctx);
    assert(0 == memcmp(z, zero, 32));

    /* Point with Y=0; for P-256:
       X = 0x512aecbfc85c47596a7fb7b1285159e35f22b92edfb04634ea63c40cb6134872
       X = 0xaed5133f37a3b8a79580484ed7aea61ca0dd46d2204fb9cb159c3bf349ecb790
    */
    free(x);
    free(z);
    mont_from_bytes(&x, (uint8_t*)"\xb4\xc4\x7b\x09\x44\x0b\xde\xce\x99\xe3\xc1\x33\xbf\xc7\x1c\xd1\x29\x90\x56\x30\x60\xdb\x6c\xab\x8e\xf8\x94\xf7\x1f\x88\x4c\xa7", 32, ctx);
    mont_set(y, 0, NULL, ctx);
    mont_from_bytes(&z, (uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32, ctx);
    ec_full_double(x, y, z, x, y, z, wp, ctx);
    assert(0 == memcmp(z, zero, 32));

    free(x);
    free(y);
    free(z);
    free_workplace(wp);
    mont_context_free(ctx);
}

void test_ec_mix_add(void)
{
    Workplace *wp;
    MontContext *ctx;
    const uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint64_t *x1, *y1, *z1;
    uint64_t *x2, *y2;
    uint8_t buffer[32];

    mont_context_init(&ctx, modulus, sizeof(modulus));
    wp = new_workplace(ctx);

    /* Arbitrary points */
    mont_from_bytes(&x1, (uint8_t*)"\xf3\x91\x4a\x3a\xf2\x1b\x11\x44\x58\x3e\xf2\xf8\x54\x01\x4b\x72\xfa\x94\x05\x8d\xf9\x7c\x32\x4f\x1a\xef\x49\x37\x3c\xe8\x5b\xef", 32, ctx);
    mont_from_bytes(&y1, (uint8_t*)"\x23\xaa\x65\x85\x4c\xc5\xbc\x53\x0d\x4f\xe7\x3e\xd9\x58\x95\x67\xb2\xea\x79\x1a\x7c\x9b\xe5\xf6\x78\x8c\xd5\xbe\xd8\x55\x0d\xe7", 32, ctx);
    mont_from_bytes(&z1, (uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32, ctx);
    mont_from_bytes(&x2, (uint8_t*)"\x42\x90\xeb\x55\xae\x4f\xb7\x32\x82\xa3\xaf\x03\x10\x10\xb1\x21\x4c\x14\x48\x36\x33\x68\x30\xa3\x46\x2c\xa5\x6e\x37\xf2\x7b\xcb", 32, ctx);
    mont_from_bytes(&y2, (uint8_t*)"\xc7\xd4\x9b\x02\xcc\x58\x10\x8d\x84\x94\xd2\x48\xfe\x14\xd4\x54\x6d\x2d\x87\x7d\x83\x43\x26\x90\xad\x11\x3a\xb2\x16\x2a\xaa\x2a", 32, ctx);
    ec_mix_add(x1, y1, z1, x1, y1, z1, x2, y2, wp, ctx);

    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\x92\xaa\x70\x2a\xd6\xf1\xf5\x46\x51\x46\x10\x13\xa4\xd7\x9a\x03\xdc\xa1\x18\xf4\x8d\x89\x3d\xbd\xea\xbe\x0e\x60\xef\x4f\x55\xda", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\xf3\x92\x8b\x7b\x84\xb9\xb2\xba\x43\x91\x99\x3a\x43\x28\x60\xab\xea\xf9\x6f\x7d\x5b\x96\x1f\xde\xc5\x16\x27\x38\x08\x71\xdb\xc0", 32));
    mont_to_bytes(buffer, z1, ctx);
    assert(0 == memcmp(buffer, "\x04\xd4\xc2\xbb\xe4\xa9\xdf\x50\xb9\xdc\x4c\x86\xf2\x4d\xef\x26\xca\xe3\xb6\x65\x20\x48\x0d\x66\x22\x1a\xb4\xc4\x54\x3f\xf5\x39", 32));

    /* Point at infinity */
    memset(x2, 0, 32);
    memset(y2, 0, 32);
    ec_mix_add(x1, y1, z1, x1, y1, z1, x2, y2, wp, ctx);

    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\x92\xaa\x70\x2a\xd6\xf1\xf5\x46\x51\x46\x10\x13\xa4\xd7\x9a\x03\xdc\xa1\x18\xf4\x8d\x89\x3d\xbd\xea\xbe\x0e\x60\xef\x4f\x55\xda", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\xf3\x92\x8b\x7b\x84\xb9\xb2\xba\x43\x91\x99\x3a\x43\x28\x60\xab\xea\xf9\x6f\x7d\x5b\x96\x1f\xde\xc5\x16\x27\x38\x08\x71\xdb\xc0", 32));
    mont_to_bytes(buffer, z1, ctx);
    assert(0 == memcmp(buffer, "\x04\xd4\xc2\xbb\xe4\xa9\xdf\x50\xb9\xdc\x4c\x86\xf2\x4d\xef\x26\xca\xe3\xb6\x65\x20\x48\x0d\x66\x22\x1a\xb4\xc4\x54\x3f\xf5\x39", 32));

    mont_copy(x2, x1, ctx);
    mont_copy(y2, y1, ctx);
    mont_set(z1, 0, NULL, ctx);
    ec_mix_add(x1, y1, z1, x1, y1, z1, x2, y2, wp, ctx);
    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\x92\xaa\x70\x2a\xd6\xf1\xf5\x46\x51\x46\x10\x13\xa4\xd7\x9a\x03\xdc\xa1\x18\xf4\x8d\x89\x3d\xbd\xea\xbe\x0e\x60\xef\x4f\x55\xda", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\xf3\x92\x8b\x7b\x84\xb9\xb2\xba\x43\x91\x99\x3a\x43\x28\x60\xab\xea\xf9\x6f\x7d\x5b\x96\x1f\xde\xc5\x16\x27\x38\x08\x71\xdb\xc0", 32));
    assert(mont_is_one(z1, ctx));

    free(x1);
    free(y1);
    free(z1);
    free(x2);
    free(y2);

    free_workplace(wp);
    mont_context_free(ctx);
}

void test_ec_full_add(void)
{
    Workplace *wp;
    MontContext *ctx;
    const uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint64_t *x1, *y1, *z1;
    uint64_t *x2, *y2, *z2;
    uint8_t buffer[32];

    mont_context_init(&ctx, modulus, sizeof(modulus));
    wp = new_workplace(ctx);

    /* Arbitrary points */
    mont_from_bytes(&x1, (uint8_t*)"\xf3\x91\x4a\x3a\xf2\x1b\x11\x44\x58\x3e\xf2\xf8\x54\x01\x4b\x72\xfa\x94\x05\x8d\xf9\x7c\x32\x4f\x1a\xef\x49\x37\x3c\xe8\x5b\xef", 32, ctx);
    mont_from_bytes(&y1, (uint8_t*)"\x23\xaa\x65\x85\x4c\xc5\xbc\x53\x0d\x4f\xe7\x3e\xd9\x58\x95\x67\xb2\xea\x79\x1a\x7c\x9b\xe5\xf6\x78\x8c\xd5\xbe\xd8\x55\x0d\xe7", 32, ctx);
    mont_from_bytes(&z1, (uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32, ctx);
    mont_from_bytes(&x2, (uint8_t*)"\x00\x9b\xed\x92\x17\x23\x8f\xa1\x07\xf0\x5d\x32\x46\x85\x31\x01\xb7\xec\x35\x12\x14\xb2\xff\xc7\x69\x70\x9f\x0d\xda\xb8\x5b\x66", 32, ctx);
    mont_from_bytes(&y2, (uint8_t*)"\x96\x7d\x85\xfa\x38\x00\xa5\xc1\xe5\x55\x6d\x20\x81\x5d\x69\xca\x79\xd9\x3f\x3c\xbe\x4e\xa5\x24\x0b\x4d\x47\xa6\x96\xa8\xb7\x1c", 32, ctx);
    mont_from_bytes(&z2, (uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32, ctx);
    ec_full_add(x1, y1, z1, x1, y1, z1, x2, y2, z2, wp, ctx);

    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\x2e\x12\x6c\x3e\x7c\xcf\x77\xb2\x52\xcf\xbe\x0a\xd1\xd5\x6d\x49\x27\x00\xd6\xba\x62\x5b\x27\x22\x86\xe6\x9b\x94\x4f\xc6\x68\x71", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\x3a\xaa\x27\x1f\xbf\xf0\x20\xb8\x7d\x6e\xad\x8f\xa2\xda\x7e\x27\x8e\xc7\x8f\x6c\xf8\x94\x71\xe0\xfc\x99\x88\xcd\x81\x9b\x8c\x7f", 32));
    mont_to_bytes(buffer, z1, ctx);
    assert(0 == memcmp(buffer, "\xdf\x18\xae\x07\x37\x90\x53\x44\x04\x8a\xef\x22\x80\x6e\x2f\x88\x89\x80\x7a\xf4\x19\x74\x56\xf5\x38\x52\x1e\xe9\x19\xd5\xe6\xba", 32));

    /* Same point */
    ec_full_add(x1, y1, z1, x1, y1, z1, x1, y1, z1, wp, ctx);

    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\x41\xa3\x18\x7a\xa9\x80\x4b\xf3\xf2\xce\x48\x49\x6f\x11\x74\xd5\xc0\x05\x1c\xbe\x9e\x62\xf7\x23\xdd\xe2\x38\x11\x74\x20\x20\xa1", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\x65\xe2\xed\x96\x30\x1c\xe0\x67\xc5\x86\xc4\xc0\x86\x08\x88\xc4\x14\x9d\xbd\x5f\xc0\x0f\x8d\x55\xcc\x5f\x85\x4c\xe5\x10\x76\x56", 32));
    mont_to_bytes(buffer, z1, ctx);
    assert(0 == memcmp(buffer, "\xf0\x3b\x9d\xad\x30\xe4\xec\x7c\xb1\xe3\xa8\xbd\x9a\xf7\x0f\x7f\x33\x61\xc8\x2c\x3f\xa2\x8c\xff\xd9\xea\x13\x7e\x01\x9f\x8d\x05", 32));

    /* Opposite point (or Y=0) */
    mont_set(y2, 1, NULL, ctx);
    mont_sub(y2, y2, y1, wp->scratch, ctx);
    ec_full_add(x1, y1, z1, x1, y1, z1, x1, y2, z1, wp, ctx);
    assert(mont_is_zero(z1, ctx));

    /* Point at infinity */
    free(x1);
    free(y1);
    free(z1);
    mont_from_bytes(&x1, (uint8_t*)"\xf3\x91\x4a\x3a\xf2\x1b\x11\x44\x58\x3e\xf2\xf8\x54\x01\x4b\x72\xfa\x94\x05\x8d\xf9\x7c\x32\x4f\x1a\xef\x49\x37\x3c\xe8\x5b\xef", 32, ctx);
    mont_from_bytes(&y1, (uint8_t*)"\x23\xaa\x65\x85\x4c\xc5\xbc\x53\x0d\x4f\xe7\x3e\xd9\x58\x95\x67\xb2\xea\x79\x1a\x7c\x9b\xe5\xf6\x78\x8c\xd5\xbe\xd8\x55\x0d\xe7", 32, ctx);
    mont_from_bytes(&z1, (uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32, ctx);
    mont_set(x2, 1, NULL, ctx);
    mont_set(y2, 1, NULL, ctx);
    mont_set(z2, 0, NULL, ctx);
    ec_full_add(x1, y1, z1, x1, y1, z1, x2, y2, z2, wp, ctx);

    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\xf3\x91\x4a\x3a\xf2\x1b\x11\x44\x58\x3e\xf2\xf8\x54\x01\x4b\x72\xfa\x94\x05\x8d\xf9\x7c\x32\x4f\x1a\xef\x49\x37\x3c\xe8\x5b\xef", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\x23\xaa\x65\x85\x4c\xc5\xbc\x53\x0d\x4f\xe7\x3e\xd9\x58\x95\x67\xb2\xea\x79\x1a\x7c\x9b\xe5\xf6\x78\x8c\xd5\xbe\xd8\x55\x0d\xe7", 32));
    mont_to_bytes(buffer, z1, ctx);
    assert(0 == memcmp(buffer, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32));

    ec_full_add(x1, y1, z1, x2, y2, z2, x1, y1, z1, wp, ctx);
    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\xf3\x91\x4a\x3a\xf2\x1b\x11\x44\x58\x3e\xf2\xf8\x54\x01\x4b\x72\xfa\x94\x05\x8d\xf9\x7c\x32\x4f\x1a\xef\x49\x37\x3c\xe8\x5b\xef", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\x23\xaa\x65\x85\x4c\xc5\xbc\x53\x0d\x4f\xe7\x3e\xd9\x58\x95\x67\xb2\xea\x79\x1a\x7c\x9b\xe5\xf6\x78\x8c\xd5\xbe\xd8\x55\x0d\xe7", 32));
    mont_to_bytes(buffer, z1, ctx);
    assert(0 == memcmp(buffer, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32));

    free(x1);
    free(y1);
    free(z1);
    free(x2);
    free(y2);
    free(z2);

    free_workplace(wp);
    mont_context_free(ctx);
}

void test_ec_scalar(void)
{
    Workplace *wp1, *wp2;
    MontContext *ctx;
    const uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint64_t *x1, *y1, *z1;
    uint64_t *x2, *y2, *z2;
    uint8_t buffer[32];

    mont_context_init(&ctx, modulus, sizeof(modulus));
    wp1 = new_workplace(ctx);
    wp2 = new_workplace(ctx);

    /* 1*G */
    mont_number(&x1, 1, ctx);
    mont_number(&y1, 1, ctx);
    mont_number(&z1, 1, ctx);
    mont_from_bytes(&x2, (uint8_t*)"\xd5\x4e\x03\x08\xf5\x49\xe3\xf4\x29\xca\x31\x9a\xec\x29\x5e\xb6\x7d\x5d\x06\x4c\xef\xe0\x2a\xdf\x8e\xfa\x5f\x48\x9b\xac\x02\xc1", 32, ctx);
    mont_from_bytes(&y2, (uint8_t*)"\x0f\xbd\x47\xe8\x97\x82\x76\x6e\x39\xef\x1a\xf4\x9d\x01\x86\x9b\x1d\x78\x8c\x42\xb8\xda\x57\xcb\xbf\xeb\x97\x19\xc3\x58\x26\x40", 32, ctx);
    mont_from_bytes(&z2, (uint8_t*)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a", 32, ctx);
    ec_scalar(x1, y1, z1, x2, y2, z2, (uint8_t*)"\x01", 1, 0x4545, wp1, wp2, ctx);
    assert(mont_is_equal(x1, x2, ctx));
    assert(mont_is_equal(y1, y2, ctx));
    assert(mont_is_equal(z1, z2, ctx));

    ec_scalar(x1, y1, z1, x2, y2, z2, (uint8_t*)"\x00\x01", 2, 0x4545, wp1, wp2, ctx);
    assert(mont_is_equal(x1, x2, ctx));
    assert(mont_is_equal(y1, y2, ctx));
    assert(mont_is_equal(z1, z2, ctx));

    /* (order+1)*G */
    ec_scalar(x1, y1, z1, x2, y2, z2, (uint8_t*)"\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x52", 32, 0x4545, wp1, wp2, ctx);
    ec_jacobian_to_affine(x1, y1, x1, y1, z1, wp1, ctx);
    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5", 32));

    /* 0*G */
    ec_scalar(x1, y1, z1, x2, y2, z2, (uint8_t*)"\x00", 1, 0x4545, wp1, wp2, ctx);
    assert(mont_is_zero(z1, ctx));

    /* order*G */
    ec_scalar(x1, y1, z1, x2, y2, z2, (uint8_t*)"\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x51", 32, 0x4545, wp1, wp2, ctx);
    assert(mont_is_zero(z1, ctx));

    /* 255*O */
    ec_scalar(x1, y1, z1, x1, y1, z1, (uint8_t*)"\x00\xFF", 2, 0x4545, wp1, wp2, ctx);
    assert(mont_is_zero(z1, ctx));

    /* arb */
    free(x2);
    free(y2);
    mont_from_bytes(&x2, (uint8_t*)"\xde\x24\x44\xbe\xbc\x8d\x36\xe6\x82\xed\xd2\x7e\x0f\x27\x15\x08\x61\x75\x19\xb3\x22\x1a\x8f\xa0\xb7\x7c\xab\x39\x89\xda\x97\xc9", 32, ctx);
    mont_from_bytes(&y2, (uint8_t*)"\xc0\x93\xae\x7f\xf3\x6e\x53\x80\xfc\x01\xa5\xaa\xd1\xe6\x66\x59\x70\x2d\xe8\x0f\x53\xce\xc5\x76\xb6\x35\x0b\x24\x30\x42\xa2\x56", 32, ctx);
    mont_set(z2, 1, NULL, ctx);
    ec_scalar(x1, y1, z1, x2, y2, z2, (uint8_t*)"\xc5\x1e\x47\x53\xaf\xde\xc1\xe6\xb6\xc6\xa5\xb9\x92\xf4\x3f\x8d\xd0\xc7\xa8\x93\x30\x72\x70\x8b\x65\x22\x46\x8b\x2f\xfb\x06\xfd", 32, 0x4545, wp1, wp2, ctx);
    ec_jacobian_to_affine(x1, y1, x1, y1, z1, wp1, ctx);
    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\x51\xd0\x8d\x5f\x2d\x42\x78\x88\x29\x46\xd8\x8d\x83\xc9\x7d\x11\xe6\x2b\xec\xc3\xcf\xc1\x8b\xed\xac\xc8\x9b\xa3\x4e\xec\xa0\x3f", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\x75\xee\x68\xeb\x8b\xf6\x26\xaa\x5b\x67\x3a\xb5\x1f\x6e\x74\x4e\x06\xf8\xfc\xf8\xa6\xc0\xcf\x30\x35\xbe\xca\x95\x6a\x7b\x41\xd5", 32));

    free(x1);
    free(y1);
    free(z1);
    free(x2);
    free(y2);
    free(z2);

    free_workplace(wp1);
    free_workplace(wp2);
    mont_context_free(ctx);
}

void test_ec_scalar_g_p256(void)
{
    Workplace *wp1, *wp2;
    MontContext *ctx;
    const uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    const uint8_t Gx[32] = "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96";
    const uint8_t Gy[32] = "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5";
    int res;

    uint64_t *x1, *y1, *z1;
    uint64_t *xw, *yw;
    uint64_t *Gx_mont, *Gy_mont;
    uint8_t buffer[32];

    mont_context_init(&ctx, modulus, sizeof(modulus));
    wp1 = new_workplace(ctx);
    wp2 = new_workplace(ctx);

    mont_from_bytes(&Gx_mont, Gx, sizeof Gx, ctx);
    mont_from_bytes(&Gy_mont, Gy, sizeof Gy, ctx);
    mont_number(&xw, 1, ctx);
    mont_number(&yw, 1, ctx);

    /* 1*G */
    mont_number(&x1, 1, ctx);
    mont_number(&y1, 1, ctx);
    mont_number(&z1, 1, ctx);
    res = ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\x01", 1, 0x4545, wp1, wp2, ctx);
    assert(res == 0);
    ec_jacobian_to_affine(xw, yw, x1, y1, z1, wp1, ctx);
    assert(mont_is_equal(xw, Gx_mont, ctx));
    assert(mont_is_equal(yw, Gy_mont, ctx));

    ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\x00\x01", 2, 0x4545, wp1, wp2, ctx);
    ec_jacobian_to_affine(xw, yw, x1, y1, z1, wp1, ctx);
    assert(mont_is_equal(xw, Gx_mont, ctx));
    assert(mont_is_equal(yw, Gy_mont, ctx));

    /* 0*G */
    ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\x00", 1, 0x4545, wp1, wp2, ctx);
    assert(mont_is_zero(z1, ctx));

    /* 31*G */
    ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\x1F", 1, 0x4545, wp1, wp2, ctx);
    ec_jacobian_to_affine(xw, yw, x1, y1, z1, wp1, ctx);
    mont_to_bytes(buffer, xw, ctx);
    assert(0 == memcmp(buffer, "\x30\x1d\x9e\x50\x2d\xc7\xe0\x5d\xa8\x5d\xa0\x26\xa7\xae\x9a\xa0\xfa\xc9\xdb\x7d\x52\xa9\x5b\x3e\x3e\x3f\x9a\xa0\xa1\xb4\x5b\x8b", 32));
    mont_to_bytes(buffer, yw, ctx);
    assert(0 == memcmp(buffer, "\x65\x51\xb6\xf6\xb3\x06\x12\x23\xe0\xd2\x3c\x02\x6b\x01\x7d\x72\x29\x8d\x9a\xe4\x68\x87\xca\x61\xd5\x8d\xb6\xae\xa1\x7e\xe2\x67", 32));

    /* 32*G */
    ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\x20", 1, 0x4545, wp1, wp2, ctx);
    ec_jacobian_to_affine(xw, yw, x1, y1, z1, wp1, ctx);
    mont_to_bytes(buffer, xw, ctx);
    assert(0 == memcmp(buffer, "\x23\x77\xc7\xd6\x90\xa2\x42\xca\x6c\x45\x07\x4e\x8e\xa5\xbe\xef\xaa\x55\x7f\xd5\xb6\x83\x71\xd9\xd1\x47\x5b\xd5\x2a\x7e\xd0\xe1", 32));
    mont_to_bytes(buffer, yw, ctx);
    assert(0 == memcmp(buffer, "\x47\xa1\x3f\xb9\x84\x13\xa4\x39\x3f\x8d\x90\xe9\xbf\x90\x1b\x7e\x66\x58\xa6\xcd\xec\xf4\x67\x16\xe7\xc0\x67\xb1\xdd\xb8\xd2\xb2", 32));

    /* (order+1)*G */
    ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x52", 32, 0x4545, wp1, wp2, ctx);
    ec_jacobian_to_affine(x1, y1, x1, y1, z1, wp1, ctx);
    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5", 32));

    /* order*G */
    ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x51", 32, 0x4545, wp1, wp2, ctx);
    assert(mont_is_zero(z1, ctx));

    /* arbirtrary */
    ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\x73\x87\x34\x34\x3F\xF8\x93\x87", 8, 0x6776, wp1, wp2, ctx);
    ec_jacobian_to_affine(x1, y1, x1, y1, z1, wp1, ctx);
    mont_to_bytes(buffer, x1, ctx);
    assert(0 == memcmp(buffer, "\xfc\x85\x6a\x26\x35\x51\x2a\x83\x44\x35\x55\x97\xbd\xbf\xa9\x3d\x33\x70\x2a\x48\xb0\x9d\x02\xbd\x1d\xc4\xfd\x4b\x5a\x4c\x6c\x09", 32));
    mont_to_bytes(buffer, y1, ctx);
    assert(0 == memcmp(buffer, "\xcf\x0d\xc7\x68\x18\x61\xa0\xb7\x29\x22\xa9\xce\x17\xf1\x58\x22\x31\x1a\xab\x2a\x14\xc4\xbd\xb0\xc4\x32\xea\xfe\x93\x9a\x4a\x47", 32));

    /* exponent is too long */
    res = ec_scalar_g_p256(x1, y1, z1, (uint8_t*)"\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x52\xFF", 33, 0x4545, wp1, wp2, ctx);
    assert(res == ERR_VALUE);

    free(x1);
    free(y1);
    free(z1);
    free(xw);
    free(yw);
    free(Gx_mont);
    free(Gy_mont);
    free_workplace(wp1);
    free_workplace(wp2);
    mont_context_free(ctx);

}


void test_ec_ws_new_point(void)
{
    EcContext *ec_ctx;
    EcPoint *ecp;
    int res;
    uint8_t Gx[32] = "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96";
    uint8_t Gx_wrong[32] = "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x97";
    uint8_t Gy[32] = "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5";
    const uint8_t  b[32] = "\x5a\xc6\x35\xd8\xaa\x3a\x93\xe7\xb3\xeb\xbd\x55\x76\x98\x86\xbc\x65\x1d\x06\xb0\xcc\x53\xb0\xf6\x3b\xce\x3c\x3e\x27\xd2\x60\x4b";
    const uint8_t order[32] = "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x51";
    uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint8_t zero[32] = { 0 };

    res = ec_ws_new_context(&ec_ctx, modulus, b, order, 32);
    assert(res == 0);
    res = ec_ws_new_point(NULL, Gx, Gy, 32, ec_ctx);
    assert(res == ERR_NULL);
    res = ec_ws_new_point(&ecp, NULL, Gy, 32, ec_ctx);
    assert(res == ERR_NULL);
    res = ec_ws_new_point(&ecp, Gx, NULL, 32, ec_ctx);
    assert(res == ERR_NULL);
    res = ec_ws_new_point(&ecp, Gx, Gy, 32, NULL);
    assert(res == ERR_NULL);

    res = ec_ws_new_point(&ecp, Gx, Gy, 0, ec_ctx);
    assert(res == ERR_VALUE);

    res = ec_ws_new_point(&ecp, Gx_wrong, Gy, 32, ec_ctx);
    assert(res == ERR_EC_POINT);

    res = ec_ws_new_point(&ecp, Gx, Gy, 32, ec_ctx);
    assert(res == 0);

    ec_free_point(ecp);
    res = ec_ws_new_point(&ecp, zero, zero, 32, ec_ctx);
    assert(res == 0);

    ec_free_point(ecp);
    ec_free_context(ec_ctx);
}

void test_ec_ws_get_xy(void)
{
    EcContext *ec_ctx;
    EcPoint *ecp;
    int res;
    uint8_t Gx[32] = "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96";
    uint8_t Gy[32] = "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5";
    uint8_t  b[32] = "\x5a\xc6\x35\xd8\xaa\x3a\x93\xe7\xb3\xeb\xbd\x55\x76\x98\x86\xbc\x65\x1d\x06\xb0\xcc\x53\xb0\xf6\x3b\xce\x3c\x3e\x27\xd2\x60\x4b";
    const uint8_t order[32] = "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x51";
    uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint8_t bufx[32], bufy[32];

    res = ec_ws_new_context(&ec_ctx, modulus, b, order, 32);
    assert(res == 0);
    res = ec_ws_new_point(&ecp, Gx, Gy, 32, ec_ctx);
    assert(res == 0);
    assert(ecp != NULL);

    res = ec_ws_get_xy(NULL, bufy, 32, ecp);
    assert(res == ERR_NULL);
    res = ec_ws_get_xy(bufx, NULL, 32, ecp);
    assert(res == ERR_NULL);
    res = ec_ws_get_xy(bufx, bufy, 32, NULL);
    assert(res == ERR_NULL);

    res = ec_ws_get_xy(bufx, bufy, 31, ecp);
    assert(res == ERR_VALUE);

    res = ec_ws_get_xy(bufx, bufy, 32, ecp);
    assert(res == 0);

    assert(0 == memcmp(bufx, Gx, 32));
    assert(0 == memcmp(bufy, Gy, 32));

    ec_free_point(ecp);
    ec_free_context(ec_ctx);
}

void test_ec_ws_double(void)
{
    EcContext *ec_ctx;
    EcPoint *ecp;
    int res;
    uint8_t Gx[32] = "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96";
    uint8_t Gy[32] = "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5";
    uint8_t  b[32] = "\x5a\xc6\x35\xd8\xaa\x3a\x93\xe7\xb3\xeb\xbd\x55\x76\x98\x86\xbc\x65\x1d\x06\xb0\xcc\x53\xb0\xf6\x3b\xce\x3c\x3e\x27\xd2\x60\x4b";
    const uint8_t order[32] = "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x51";
    uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint8_t bufx[32], bufy[32];

    ec_ws_new_context(&ec_ctx, modulus, b, order, 32);
    ec_ws_new_point(&ecp, Gx, Gy, 32, ec_ctx);

    res = ec_ws_double(NULL);
    assert(res == ERR_NULL);

    res = ec_ws_double(ecp);
    assert(res == 0);
    ec_ws_get_xy(bufx, bufy, 32, ecp);
    assert(0 == memcmp(bufx, "\x7c\xf2\x7b\x18\x8d\x03\x4f\x7e\x8a\x52\x38\x03\x04\xb5\x1a\xc3\xc0\x89\x69\xe2\x77\xf2\x1b\x35\xa6\x0b\x48\xfc\x47\x66\x99\x78", 32));
    assert(0 == memcmp(bufy, "\x07\x77\x55\x10\xdb\x8e\xd0\x40\x29\x3d\x9a\xc6\x9f\x74\x30\xdb\xba\x7d\xad\xe6\x3c\xe9\x82\x29\x9e\x04\xb7\x9d\x22\x78\x73\xd1", 32));

    ec_free_point(ecp);
    ec_free_context(ec_ctx);
}

void test_ec_ws_add(void)
{
    EcContext *ec_ctx;
    EcPoint *ecp, *ecp2;
    int res;
    uint8_t Gx[32] = "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96";
    uint8_t Gy[32] = "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5";
    uint8_t  b[32] = "\x5a\xc6\x35\xd8\xaa\x3a\x93\xe7\xb3\xeb\xbd\x55\x76\x98\x86\xbc\x65\x1d\x06\xb0\xcc\x53\xb0\xf6\x3b\xce\x3c\x3e\x27\xd2\x60\x4b";
    const uint8_t order[32] = "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x51";
    uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint8_t bufx[32], bufy[32];

    ec_ws_new_context(&ec_ctx, modulus, b, order, 32);
    ec_ws_new_point(&ecp, Gx, Gy, 32, ec_ctx);
    ec_ws_new_point(&ecp2, Gx, Gy, 32, ec_ctx);
    ec_ws_double(ecp2);

    res = ec_ws_add(NULL, ecp);
    assert(res == ERR_NULL);
    res = ec_ws_add(ecp, NULL);
    assert(res == ERR_NULL);

    res = ec_ws_add(ecp, ecp2);
    assert(res == 0);
    ec_ws_get_xy(bufx, bufy, 32, ecp);
    assert(0 == memcmp(bufx, "\x5e\xcb\xe4\xd1\xa6\x33\x0a\x44\xc8\xf7\xef\x95\x1d\x4b\xf1\x65\xe6\xc6\xb7\x21\xef\xad\xa9\x85\xfb\x41\x66\x1b\xc6\xe7\xfd\x6c", 32));
    assert(0 == memcmp(bufy, "\x87\x34\x64\x0c\x49\x98\xff\x7e\x37\x4b\x06\xce\x1a\x64\xa2\xec\xd8\x2a\xb0\x36\x38\x4f\xb8\x3d\x9a\x79\xb1\x27\xa2\x7d\x50\x32", 32));

    ec_free_point(ecp);
    ec_free_point(ecp2);
    ec_free_context(ec_ctx);
}

void test_ec_ws_scalar(void)
{
    EcContext *ec_ctx;
    EcPoint *ecp;
    int res;
    uint8_t Gx[32] = "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96";
    uint8_t Gy[32] = "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5";
    uint8_t  b[32] = "\x5a\xc6\x35\xd8\xaa\x3a\x93\xe7\xb3\xeb\xbd\x55\x76\x98\x86\xbc\x65\x1d\x06\xb0\xcc\x53\xb0\xf6\x3b\xce\x3c\x3e\x27\xd2\x60\x4b";
    const uint8_t order[32] = "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x51";
    uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint8_t bufx[32], bufy[32];

    ec_ws_new_context(&ec_ctx, modulus, b, order, 32);
    ec_ws_new_point(&ecp, Gx, Gy, 32, ec_ctx);

    res = ec_ws_scalar(NULL, (uint8_t*)"\xFF\xFF", 2, 0xFFFF);
    assert(res == ERR_NULL);
    res = ec_ws_scalar(ecp, NULL, 2, 0xFFFF);
    assert(res == ERR_NULL);

    res = ec_ws_scalar(ecp, (uint8_t*)"\xFF\xFF", 2, 0xFFFF);
    assert(res == 0);
    ec_ws_get_xy(bufx, bufy, 32, ecp);
    assert(0 == memcmp(bufx, "\xf2\x49\x10\x4d\x0e\x6f\x8f\x29\xe6\x01\x62\x77\x78\x0c\xda\x84\xdc\x84\xb8\x3b\xc3\xd8\x99\xdf\xb7\x36\xca\x08\x31\xfb\xe8\xcf", 32));
    assert(0 == memcmp(bufy, "\xb5\x7e\x12\xfc\xdb\x03\x1f\x59\xca\xb8\x1b\x1c\x6b\x1e\x1c\x07\xe4\x51\x2e\x52\xce\x83\x2f\x1a\x0c\xed\xef\xff\x8b\x43\x40\xe9", 32));

    ec_free_point(ecp);
    ec_free_context(ec_ctx);
}

void test_ec_ws_neg(void)
{
    EcContext *ec_ctx;
    EcPoint *ecp;
    int res;
    uint8_t Gx[32] = "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96";
    uint8_t Gy[32] = "\x4f\xe3\x42\xe2\xfe\x1a\x7f\x9b\x8e\xe7\xeb\x4a\x7c\x0f\x9e\x16\x2b\xce\x33\x57\x6b\x31\x5e\xce\xcb\xb6\x40\x68\x37\xbf\x51\xf5";
    uint8_t  b[32] = "\x5a\xc6\x35\xd8\xaa\x3a\x93\xe7\xb3\xeb\xbd\x55\x76\x98\x86\xbc\x65\x1d\x06\xb0\xcc\x53\xb0\xf6\x3b\xce\x3c\x3e\x27\xd2\x60\x4b";
    const uint8_t order[32] = "\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xbc\xe6\xfa\xad\xa7\x17\x9e\x84\xf3\xb9\xca\xc2\xfc\x63\x25\x51";
    uint8_t modulus[32] = "\xff\xff\xff\xff\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";
    uint8_t bufx[32], bufy[32];

    ec_ws_new_context(&ec_ctx, modulus, b, order, 32);
    ec_ws_new_point(&ecp, Gx, Gy, 32, ec_ctx);

    res = ec_ws_neg(NULL);
    assert(res == ERR_NULL);

    res = ec_ws_neg(ecp);
    assert(res == 0);
    ec_ws_get_xy(bufx, bufy, 32, ecp);
    assert(0 == memcmp(bufx, "\x6b\x17\xd1\xf2\xe1\x2c\x42\x47\xf8\xbc\xe6\xe5\x63\xa4\x40\xf2\x77\x03\x7d\x81\x2d\xeb\x33\xa0\xf4\xa1\x39\x45\xd8\x98\xc2\x96", 32));
    assert(0 == memcmp(bufy, "\xb0\x1c\xbd\x1c\x01\xe5\x80\x65\x71\x18\x14\xb5\x83\xf0\x61\xe9\xd4\x31\xcc\xa9\x94\xce\xa1\x31\x34\x49\xbf\x97\xc8\x40\xae\x0a", 32));

    ec_free_point(ecp);
    ec_free_context(ec_ctx);
}


int main(void) {
    test_ec_jacobian_to_affine();
    test_ec_full_double();
    test_ec_mix_add();
    test_ec_full_add();
    test_ec_scalar();
    test_ec_scalar_g_p256();
    test_ec_ws_new_point();
    test_ec_ws_get_xy();
    test_ec_ws_double();
    test_ec_ws_add();
    test_ec_ws_scalar();
    test_ec_ws_neg();
    return 0;
}
