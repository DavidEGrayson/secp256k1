/**********************************************************************
 * Copyright (c) 2013, 2014 Pieter Wuille                             *
 * Distributed under the MIT software license, see the accompanying   *
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/

#if defined HAVE_CONFIG_H
#include "libsecp256k1-config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "secp256k1.c"
#include "testrand_impl.h"

#ifdef ENABLE_OPENSSL_TESTS
#include "openssl/bn.h"
#include "openssl/ec.h"
#include "openssl/ecdsa.h"
#include "openssl/obj_mac.h"
#endif

static int count = 64;

void random_field_element_test(secp256k1_fe_t *fe) {
    do {
        unsigned char b32[32];
        secp256k1_rand256_test(b32);
        if (secp256k1_fe_set_b32(fe, b32)) {
            break;
        }
    } while(1);
}

void random_field_element_magnitude(secp256k1_fe_t *fe) {
    secp256k1_fe_t zero;
    int n = secp256k1_rand32() % 9;
    secp256k1_fe_normalize(fe);
    if (n == 0) {
        return;
    }
    secp256k1_fe_clear(&zero);
    secp256k1_fe_negate(&zero, &zero, 0);
    secp256k1_fe_mul_int(&zero, n - 1);
    secp256k1_fe_add(fe, &zero);
#ifdef VERIFY
    CHECK(fe->magnitude == n);
#endif
}

void random_group_element_test(secp256k1_ge_t *ge) {
    secp256k1_fe_t fe;
    do {
        random_field_element_test(&fe);
        if (secp256k1_ge_set_xo_var(ge, &fe, secp256k1_rand32() & 1))
            break;
    } while(1);
}

void random_group_element_jacobian_test(secp256k1_gej_t *gej, const secp256k1_ge_t *ge) {
    secp256k1_fe_t z2, z3;
    do {
        random_field_element_test(&gej->z);
        if (!secp256k1_fe_is_zero(&gej->z)) {
            break;
        }
    } while(1);
    secp256k1_fe_sqr(&z2, &gej->z);
    secp256k1_fe_mul(&z3, &z2, &gej->z);
    secp256k1_fe_mul(&gej->x, &ge->x, &z2);
    secp256k1_fe_mul(&gej->y, &ge->y, &z3);
    gej->infinity = ge->infinity;
}

void random_scalar_order_test(secp256k1_scalar_t *num) {
    do {
        unsigned char b32[32];
        int overflow = 0;
        secp256k1_rand256_test(b32);
        secp256k1_scalar_set_b32(num, b32, &overflow);
        if (overflow || secp256k1_scalar_is_zero(num))
            continue;
        break;
    } while(1);
}

void random_scalar_order(secp256k1_scalar_t *num) {
    do {
        unsigned char b32[32];
        int overflow = 0;
        secp256k1_rand256(b32);
        secp256k1_scalar_set_b32(num, b32, &overflow);
        if (overflow || secp256k1_scalar_is_zero(num))
            continue;
        break;
    } while(1);
}

/***** HASH TESTS *****/

void run_sha256_tests(void) {
    static const char *inputs[8] = {
        "", "abc", "message digest", "secure hash algorithm", "SHA256 is considered to be safe",
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        "For this sample, this 63-byte string will be used as input data",
        "This is exactly 64 bytes long, not counting the terminating byte"
    };
    static const unsigned char outputs[8][32] = {
        {0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55},
        {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad},
        {0xf7, 0x84, 0x6f, 0x55, 0xcf, 0x23, 0xe1, 0x4e, 0xeb, 0xea, 0xb5, 0xb4, 0xe1, 0x55, 0x0c, 0xad, 0x5b, 0x50, 0x9e, 0x33, 0x48, 0xfb, 0xc4, 0xef, 0xa3, 0xa1, 0x41, 0x3d, 0x39, 0x3c, 0xb6, 0x50},
        {0xf3, 0x0c, 0xeb, 0x2b, 0xb2, 0x82, 0x9e, 0x79, 0xe4, 0xca, 0x97, 0x53, 0xd3, 0x5a, 0x8e, 0xcc, 0x00, 0x26, 0x2d, 0x16, 0x4c, 0xc0, 0x77, 0x08, 0x02, 0x95, 0x38, 0x1c, 0xbd, 0x64, 0x3f, 0x0d},
        {0x68, 0x19, 0xd9, 0x15, 0xc7, 0x3f, 0x4d, 0x1e, 0x77, 0xe4, 0xe1, 0xb5, 0x2d, 0x1f, 0xa0, 0xf9, 0xcf, 0x9b, 0xea, 0xea, 0xd3, 0x93, 0x9f, 0x15, 0x87, 0x4b, 0xd9, 0x88, 0xe2, 0xa2, 0x36, 0x30},
        {0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1},
        {0xf0, 0x8a, 0x78, 0xcb, 0xba, 0xee, 0x08, 0x2b, 0x05, 0x2a, 0xe0, 0x70, 0x8f, 0x32, 0xfa, 0x1e, 0x50, 0xc5, 0xc4, 0x21, 0xaa, 0x77, 0x2b, 0xa5, 0xdb, 0xb4, 0x06, 0xa2, 0xea, 0x6b, 0xe3, 0x42},
        {0xab, 0x64, 0xef, 0xf7, 0xe8, 0x8e, 0x2e, 0x46, 0x16, 0x5e, 0x29, 0xf2, 0xbc, 0xe4, 0x18, 0x26, 0xbd, 0x4c, 0x7b, 0x35, 0x52, 0xf6, 0xb3, 0x82, 0xa9, 0xe7, 0xd3, 0xaf, 0x47, 0xc2, 0x45, 0xf8}
    };
    int i;
    for (i = 0; i < 8; i++) {
        unsigned char out[32];
        secp256k1_sha256_t hasher;
        secp256k1_sha256_initialize(&hasher);
        secp256k1_sha256_write(&hasher, (const unsigned char*)(inputs[i]), strlen(inputs[i]));
        secp256k1_sha256_finalize(&hasher, out);
        CHECK(memcmp(out, outputs[i], 32) == 0);
        if (strlen(inputs[i]) > 0) {
            int split = secp256k1_rand32() % strlen(inputs[i]);
            secp256k1_sha256_initialize(&hasher);
            secp256k1_sha256_write(&hasher, (const unsigned char*)(inputs[i]), split);
            secp256k1_sha256_write(&hasher, (const unsigned char*)(inputs[i] + split), strlen(inputs[i]) - split);
            secp256k1_sha256_finalize(&hasher, out);
            CHECK(memcmp(out, outputs[i], 32) == 0);
        }
    }
}

void run_hmac_sha256_tests(void) {
    static const char *keys[6] = {
        "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
        "\x4a\x65\x66\x65",
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa",
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19",
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa",
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
    };
    static const char *inputs[6] = {
        "\x48\x69\x20\x54\x68\x65\x72\x65",
        "\x77\x68\x61\x74\x20\x64\x6f\x20\x79\x61\x20\x77\x61\x6e\x74\x20\x66\x6f\x72\x20\x6e\x6f\x74\x68\x69\x6e\x67\x3f",
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd",
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd",
        "\x54\x65\x73\x74\x20\x55\x73\x69\x6e\x67\x20\x4c\x61\x72\x67\x65\x72\x20\x54\x68\x61\x6e\x20\x42\x6c\x6f\x63\x6b\x2d\x53\x69\x7a\x65\x20\x4b\x65\x79\x20\x2d\x20\x48\x61\x73\x68\x20\x4b\x65\x79\x20\x46\x69\x72\x73\x74",
        "\x54\x68\x69\x73\x20\x69\x73\x20\x61\x20\x74\x65\x73\x74\x20\x75\x73\x69\x6e\x67\x20\x61\x20\x6c\x61\x72\x67\x65\x72\x20\x74\x68\x61\x6e\x20\x62\x6c\x6f\x63\x6b\x2d\x73\x69\x7a\x65\x20\x6b\x65\x79\x20\x61\x6e\x64\x20\x61\x20\x6c\x61\x72\x67\x65\x72\x20\x74\x68\x61\x6e\x20\x62\x6c\x6f\x63\x6b\x2d\x73\x69\x7a\x65\x20\x64\x61\x74\x61\x2e\x20\x54\x68\x65\x20\x6b\x65\x79\x20\x6e\x65\x65\x64\x73\x20\x74\x6f\x20\x62\x65\x20\x68\x61\x73\x68\x65\x64\x20\x62\x65\x66\x6f\x72\x65\x20\x62\x65\x69\x6e\x67\x20\x75\x73\x65\x64\x20\x62\x79\x20\x74\x68\x65\x20\x48\x4d\x41\x43\x20\x61\x6c\x67\x6f\x72\x69\x74\x68\x6d\x2e"
    };
    static const unsigned char outputs[6][32] = {
        {0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53, 0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b, 0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7, 0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7},
        {0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e, 0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7, 0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27, 0x39, 0x83, 0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43},
        {0x77, 0x3e, 0xa9, 0x1e, 0x36, 0x80, 0x0e, 0x46, 0x85, 0x4d, 0xb8, 0xeb, 0xd0, 0x91, 0x81, 0xa7, 0x29, 0x59, 0x09, 0x8b, 0x3e, 0xf8, 0xc1, 0x22, 0xd9, 0x63, 0x55, 0x14, 0xce, 0xd5, 0x65, 0xfe},
        {0x82, 0x55, 0x8a, 0x38, 0x9a, 0x44, 0x3c, 0x0e, 0xa4, 0xcc, 0x81, 0x98, 0x99, 0xf2, 0x08, 0x3a, 0x85, 0xf0, 0xfa, 0xa3, 0xe5, 0x78, 0xf8, 0x07, 0x7a, 0x2e, 0x3f, 0xf4, 0x67, 0x29, 0x66, 0x5b},
        {0x60, 0xe4, 0x31, 0x59, 0x1e, 0xe0, 0xb6, 0x7f, 0x0d, 0x8a, 0x26, 0xaa, 0xcb, 0xf5, 0xb7, 0x7f, 0x8e, 0x0b, 0xc6, 0x21, 0x37, 0x28, 0xc5, 0x14, 0x05, 0x46, 0x04, 0x0f, 0x0e, 0xe3, 0x7f, 0x54},
        {0x9b, 0x09, 0xff, 0xa7, 0x1b, 0x94, 0x2f, 0xcb, 0x27, 0x63, 0x5f, 0xbc, 0xd5, 0xb0, 0xe9, 0x44, 0xbf, 0xdc, 0x63, 0x64, 0x4f, 0x07, 0x13, 0x93, 0x8a, 0x7f, 0x51, 0x53, 0x5c, 0x3a, 0x35, 0xe2}
    };
    int i;
    for (i = 0; i < 6; i++) {
        secp256k1_hmac_sha256_t hasher;
        unsigned char out[32];
        secp256k1_hmac_sha256_initialize(&hasher, (const unsigned char*)(keys[i]), strlen(keys[i]));
        secp256k1_hmac_sha256_write(&hasher, (const unsigned char*)(inputs[i]), strlen(inputs[i]));
        secp256k1_hmac_sha256_finalize(&hasher, out);
        CHECK(memcmp(out, outputs[i], 32) == 0);
        if (strlen(inputs[i]) > 0) {
            int split = secp256k1_rand32() % strlen(inputs[i]);
            secp256k1_hmac_sha256_initialize(&hasher, (const unsigned char*)(keys[i]), strlen(keys[i]));
            secp256k1_hmac_sha256_write(&hasher, (const unsigned char*)(inputs[i]), split);
            secp256k1_hmac_sha256_write(&hasher, (const unsigned char*)(inputs[i] + split), strlen(inputs[i]) - split);
            secp256k1_hmac_sha256_finalize(&hasher, out);
            CHECK(memcmp(out, outputs[i], 32) == 0);
        }
    }
}

void run_rfc6979_hmac_sha256_tests(void) {
    static const unsigned char key1[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x00};
    static const unsigned char msg1[32] = {0x4b, 0xf5, 0x12, 0x2f, 0x34, 0x45, 0x54, 0xc5, 0x3b, 0xde, 0x2e, 0xbb, 0x8c, 0xd2, 0xb7, 0xe3, 0xd1, 0x60, 0x0a, 0xd6, 0x31, 0xc3, 0x85, 0xa5, 0xd7, 0xcc, 0xe2, 0x3c, 0x77, 0x85, 0x45, 0x9a};
    static const unsigned char out1[3][32] = {
        {0x4f, 0xe2, 0x95, 0x25, 0xb2, 0x08, 0x68, 0x09, 0x15, 0x9a, 0xcd, 0xf0, 0x50, 0x6e, 0xfb, 0x86, 0xb0, 0xec, 0x93, 0x2c, 0x7b, 0xa4, 0x42, 0x56, 0xab, 0x32, 0x1e, 0x42, 0x1e, 0x67, 0xe9, 0xfb},
        {0x2b, 0xf0, 0xff, 0xf1, 0xd3, 0xc3, 0x78, 0xa2, 0x2d, 0xc5, 0xde, 0x1d, 0x85, 0x65, 0x22, 0x32, 0x5c, 0x65, 0xb5, 0x04, 0x49, 0x1a, 0x0c, 0xbd, 0x01, 0xcb, 0x8f, 0x3a, 0xa6, 0x7f, 0xfd, 0x4a},
        {0xf5, 0x28, 0xb4, 0x10, 0xcb, 0x54, 0x1f, 0x77, 0x00, 0x0d, 0x7a, 0xfb, 0x6c, 0x5b, 0x53, 0xc5, 0xc4, 0x71, 0xea, 0xb4, 0x3e, 0x46, 0x6d, 0x9a, 0xc5, 0x19, 0x0c, 0x39, 0xc8, 0x2f, 0xd8, 0x2e}
    };

    static const unsigned char key2[32] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    static const unsigned char msg2[32] = {0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
    static const unsigned char out2[3][32] = {
        {0x9c, 0x23, 0x6c, 0x16, 0x5b, 0x82, 0xae, 0x0c, 0xd5, 0x90, 0x65, 0x9e, 0x10, 0x0b, 0x6b, 0xab, 0x30, 0x36, 0xe7, 0xba, 0x8b, 0x06, 0x74, 0x9b, 0xaf, 0x69, 0x81, 0xe1, 0x6f, 0x1a, 0x2b, 0x95},
        {0xdf, 0x47, 0x10, 0x61, 0x62, 0x5b, 0xc0, 0xea, 0x14, 0xb6, 0x82, 0xfe, 0xee, 0x2c, 0x9c, 0x02, 0xf2, 0x35, 0xda, 0x04, 0x20, 0x4c, 0x1d, 0x62, 0xa1, 0x53, 0x6c, 0x6e, 0x17, 0xae, 0xd7, 0xa9},
        {0x75, 0x97, 0x88, 0x7c, 0xbd, 0x76, 0x32, 0x1f, 0x32, 0xe3, 0x04, 0x40, 0x67, 0x9a, 0x22, 0xcf, 0x7f, 0x8d, 0x9d, 0x2e, 0xac, 0x39, 0x0e, 0x58, 0x1f, 0xea, 0x09, 0x1c, 0xe2, 0x02, 0xba, 0x94}
    };

    secp256k1_rfc6979_hmac_sha256_t rng;
    unsigned char out[32];
    int i;

    secp256k1_rfc6979_hmac_sha256_initialize(&rng, key1, 32, msg1, 32);
    for (i = 0; i < 3; i++) {
        secp256k1_rfc6979_hmac_sha256_generate(&rng, out, 32);
        CHECK(memcmp(out, out1[i], 32) == 0);
    }
    secp256k1_rfc6979_hmac_sha256_finalize(&rng);

    secp256k1_rfc6979_hmac_sha256_initialize(&rng, key2, 32, msg2, 32);
    for (i = 0; i < 3; i++) {
        secp256k1_rfc6979_hmac_sha256_generate(&rng, out, 32);
        CHECK(memcmp(out, out2[i], 32) == 0);
    }
    secp256k1_rfc6979_hmac_sha256_finalize(&rng);
}

/***** NUM TESTS *****/

#ifndef USE_NUM_NONE
void random_num_negate(secp256k1_num_t *num) {
    if (secp256k1_rand32() & 1)
        secp256k1_num_negate(num);
}

void random_num_order_test(secp256k1_num_t *num) {
    secp256k1_scalar_t sc;
    random_scalar_order_test(&sc);
    secp256k1_scalar_get_num(num, &sc);
}

void random_num_order(secp256k1_num_t *num) {
    secp256k1_scalar_t sc;
    random_scalar_order(&sc);
    secp256k1_scalar_get_num(num, &sc);
}

void test_num_negate(void) {
    secp256k1_num_t n1;
    secp256k1_num_t n2;
    random_num_order_test(&n1); /* n1 = R */
    random_num_negate(&n1);
    secp256k1_num_copy(&n2, &n1); /* n2 = R */
    secp256k1_num_sub(&n1, &n2, &n1); /* n1 = n2-n1 = 0 */
    CHECK(secp256k1_num_is_zero(&n1));
    secp256k1_num_copy(&n1, &n2); /* n1 = R */
    secp256k1_num_negate(&n1); /* n1 = -R */
    CHECK(!secp256k1_num_is_zero(&n1));
    secp256k1_num_add(&n1, &n2, &n1); /* n1 = n2+n1 = 0 */
    CHECK(secp256k1_num_is_zero(&n1));
    secp256k1_num_copy(&n1, &n2); /* n1 = R */
    secp256k1_num_negate(&n1); /* n1 = -R */
    CHECK(secp256k1_num_is_neg(&n1) != secp256k1_num_is_neg(&n2));
    secp256k1_num_negate(&n1); /* n1 = R */
    CHECK(secp256k1_num_eq(&n1, &n2));
}

void test_num_add_sub(void) {
    secp256k1_num_t n1;
    secp256k1_num_t n2;
    secp256k1_num_t n1p2, n2p1, n1m2, n2m1;
    int r = secp256k1_rand32();
    random_num_order_test(&n1); /* n1 = R1 */
    if (r & 1) {
        random_num_negate(&n1);
    }
    random_num_order_test(&n2); /* n2 = R2 */
    if (r & 2) {
        random_num_negate(&n2);
    }
    secp256k1_num_add(&n1p2, &n1, &n2); /* n1p2 = R1 + R2 */
    secp256k1_num_add(&n2p1, &n2, &n1); /* n2p1 = R2 + R1 */
    secp256k1_num_sub(&n1m2, &n1, &n2); /* n1m2 = R1 - R2 */
    secp256k1_num_sub(&n2m1, &n2, &n1); /* n2m1 = R2 - R1 */
    CHECK(secp256k1_num_eq(&n1p2, &n2p1));
    CHECK(!secp256k1_num_eq(&n1p2, &n1m2));
    secp256k1_num_negate(&n2m1); /* n2m1 = -R2 + R1 */
    CHECK(secp256k1_num_eq(&n2m1, &n1m2));
    CHECK(!secp256k1_num_eq(&n2m1, &n1));
    secp256k1_num_add(&n2m1, &n2m1, &n2); /* n2m1 = -R2 + R1 + R2 = R1 */
    CHECK(secp256k1_num_eq(&n2m1, &n1));
    CHECK(!secp256k1_num_eq(&n2p1, &n1));
    secp256k1_num_sub(&n2p1, &n2p1, &n2); /* n2p1 = R2 + R1 - R2 = R1 */
    CHECK(secp256k1_num_eq(&n2p1, &n1));
}

void run_num_smalltests(void) {
    int i;
    for (i = 0; i < 100*count; i++) {
        test_num_negate();
        test_num_add_sub();
    }
}
#endif

/***** SCALAR TESTS *****/

void scalar_test(void) {
    secp256k1_scalar_t s;
    secp256k1_scalar_t s1;
    secp256k1_scalar_t s2;
#ifndef USE_NUM_NONE
    secp256k1_num_t snum, s1num, s2num;
    secp256k1_num_t order, half_order;
#endif
    unsigned char c[32];

    /* Set 's' to a random scalar, with value 'snum'. */
    random_scalar_order_test(&s);

    /* Set 's1' to a random scalar, with value 's1num'. */
    random_scalar_order_test(&s1);

    /* Set 's2' to a random scalar, with value 'snum2', and byte array representation 'c'. */
    random_scalar_order_test(&s2);
    secp256k1_scalar_get_b32(c, &s2);

#ifndef USE_NUM_NONE
    secp256k1_scalar_get_num(&snum, &s);
    secp256k1_scalar_get_num(&s1num, &s1);
    secp256k1_scalar_get_num(&s2num, &s2);

    secp256k1_scalar_order_get_num(&order);
    half_order = order;
    secp256k1_num_shift(&half_order, 1);
#endif

    {
        int i;
        /* Test that fetching groups of 4 bits from a scalar and recursing n(i)=16*n(i-1)+p(i) reconstructs it. */
        secp256k1_scalar_t n;
        secp256k1_scalar_set_int(&n, 0);
        for (i = 0; i < 256; i += 4) {
            secp256k1_scalar_t t;
            int j;
            secp256k1_scalar_set_int(&t, secp256k1_scalar_get_bits(&s, 256 - 4 - i, 4));
            for (j = 0; j < 4; j++) {
                secp256k1_scalar_add(&n, &n, &n);
            }
            secp256k1_scalar_add(&n, &n, &t);
        }
        CHECK(secp256k1_scalar_eq(&n, &s));
    }

    {
        /* Test that fetching groups of randomly-sized bits from a scalar and recursing n(i)=b*n(i-1)+p(i) reconstructs it. */
        secp256k1_scalar_t n;
        int i = 0;
        secp256k1_scalar_set_int(&n, 0);
        while (i < 256) {
            secp256k1_scalar_t t;
            int j;
            int now = (secp256k1_rand32() % 15) + 1;
            if (now + i > 256) {
                now = 256 - i;
            }
            secp256k1_scalar_set_int(&t, secp256k1_scalar_get_bits_var(&s, 256 - now - i, now));
            for (j = 0; j < now; j++) {
                secp256k1_scalar_add(&n, &n, &n);
            }
            secp256k1_scalar_add(&n, &n, &t);
            i += now;
        }
        CHECK(secp256k1_scalar_eq(&n, &s));
    }

#ifndef USE_NUM_NONE
    {
        /* Test that adding the scalars together is equal to adding their numbers together modulo the order. */
        secp256k1_num_t rnum;
        secp256k1_num_t r2num;
        secp256k1_scalar_t r;
        secp256k1_num_add(&rnum, &snum, &s2num);
        secp256k1_num_mod(&rnum, &order);
        secp256k1_scalar_add(&r, &s, &s2);
        secp256k1_scalar_get_num(&r2num, &r);
        CHECK(secp256k1_num_eq(&rnum, &r2num));
    }

    {
        /* Test that multipying the scalars is equal to multiplying their numbers modulo the order. */
        secp256k1_scalar_t r;
        secp256k1_num_t r2num;
        secp256k1_num_t rnum;
        secp256k1_num_mul(&rnum, &snum, &s2num);
        secp256k1_num_mod(&rnum, &order);
        secp256k1_scalar_mul(&r, &s, &s2);
        secp256k1_scalar_get_num(&r2num, &r);
        CHECK(secp256k1_num_eq(&rnum, &r2num));
        /* The result can only be zero if at least one of the factors was zero. */
        CHECK(secp256k1_scalar_is_zero(&r) == (secp256k1_scalar_is_zero(&s) || secp256k1_scalar_is_zero(&s2)));
        /* The results can only be equal to one of the factors if that factor was zero, or the other factor was one. */
        CHECK(secp256k1_num_eq(&rnum, &snum) == (secp256k1_scalar_is_zero(&s) || secp256k1_scalar_is_one(&s2)));
        CHECK(secp256k1_num_eq(&rnum, &s2num) == (secp256k1_scalar_is_zero(&s2) || secp256k1_scalar_is_one(&s)));
    }

    {
        secp256k1_scalar_t neg;
        secp256k1_num_t negnum;
        secp256k1_num_t negnum2;
        /* Check that comparison with zero matches comparison with zero on the number. */
        CHECK(secp256k1_num_is_zero(&snum) == secp256k1_scalar_is_zero(&s));
        /* Check that comparison with the half order is equal to testing for high scalar. */
        CHECK(secp256k1_scalar_is_high(&s) == (secp256k1_num_cmp(&snum, &half_order) > 0));
        secp256k1_scalar_negate(&neg, &s);
        secp256k1_num_sub(&negnum, &order, &snum);
        secp256k1_num_mod(&negnum, &order);
        /* Check that comparison with the half order is equal to testing for high scalar after negation. */
        CHECK(secp256k1_scalar_is_high(&neg) == (secp256k1_num_cmp(&negnum, &half_order) > 0));
        /* Negating should change the high property, unless the value was already zero. */
        CHECK((secp256k1_scalar_is_high(&s) == secp256k1_scalar_is_high(&neg)) == secp256k1_scalar_is_zero(&s));
        secp256k1_scalar_get_num(&negnum2, &neg);
        /* Negating a scalar should be equal to (order - n) mod order on the number. */
        CHECK(secp256k1_num_eq(&negnum, &negnum2));
        secp256k1_scalar_add(&neg, &neg, &s);
        /* Adding a number to its negation should result in zero. */
        CHECK(secp256k1_scalar_is_zero(&neg));
        secp256k1_scalar_negate(&neg, &neg);
        /* Negating zero should still result in zero. */
        CHECK(secp256k1_scalar_is_zero(&neg));
    }

    {
        /* Test secp256k1_scalar_mul_shift_var. */
        secp256k1_scalar_t r;
        secp256k1_num_t one;
        secp256k1_num_t rnum;
        secp256k1_num_t rnum2;
        unsigned char cone[1] = {0x01};
        unsigned int shift = 256 + (secp256k1_rand32() % 257);
        secp256k1_scalar_mul_shift_var(&r, &s1, &s2, shift);
        secp256k1_num_mul(&rnum, &s1num, &s2num);
        secp256k1_num_shift(&rnum, shift - 1);
        secp256k1_num_set_bin(&one, cone, 1);
        secp256k1_num_add(&rnum, &rnum, &one);
        secp256k1_num_shift(&rnum, 1);
        secp256k1_scalar_get_num(&rnum2, &r);
        CHECK(secp256k1_num_eq(&rnum, &rnum2));
    }
#endif

    {
        /* Test that scalar inverses are equal to the inverse of their number modulo the order. */
        if (!secp256k1_scalar_is_zero(&s)) {
            secp256k1_scalar_t inv;
#ifndef USE_NUM_NONE
            secp256k1_num_t invnum;
            secp256k1_num_t invnum2;
#endif
            secp256k1_scalar_inverse(&inv, &s);
#ifndef USE_NUM_NONE
            secp256k1_num_mod_inverse(&invnum, &snum, &order);
            secp256k1_scalar_get_num(&invnum2, &inv);
            CHECK(secp256k1_num_eq(&invnum, &invnum2));
#endif
            secp256k1_scalar_mul(&inv, &inv, &s);
            /* Multiplying a scalar with its inverse must result in one. */
            CHECK(secp256k1_scalar_is_one(&inv));
            secp256k1_scalar_inverse(&inv, &inv);
            /* Inverting one must result in one. */
            CHECK(secp256k1_scalar_is_one(&inv));
        }
    }

    {
        /* Test commutativity of add. */
        secp256k1_scalar_t r1, r2;
        secp256k1_scalar_add(&r1, &s1, &s2);
        secp256k1_scalar_add(&r2, &s2, &s1);
        CHECK(secp256k1_scalar_eq(&r1, &r2));
    }

    {
        secp256k1_scalar_t r1, r2;
        secp256k1_scalar_t b;
        int i;
        /* Test add_bit. */
        int bit = secp256k1_rand32() % 256;
        secp256k1_scalar_set_int(&b, 1);
        CHECK(secp256k1_scalar_is_one(&b));
        for (i = 0; i < bit; i++) {
            secp256k1_scalar_add(&b, &b, &b);
        }
        r1 = s1;
        r2 = s1;
        if (!secp256k1_scalar_add(&r1, &r1, &b)) {
            /* No overflow happened. */
            secp256k1_scalar_add_bit(&r2, bit);
            CHECK(secp256k1_scalar_eq(&r1, &r2));
        }
    }

    {
        /* Test commutativity of mul. */
        secp256k1_scalar_t r1, r2;
        secp256k1_scalar_mul(&r1, &s1, &s2);
        secp256k1_scalar_mul(&r2, &s2, &s1);
        CHECK(secp256k1_scalar_eq(&r1, &r2));
    }

    {
        /* Test associativity of add. */
        secp256k1_scalar_t r1, r2;
        secp256k1_scalar_add(&r1, &s1, &s2);
        secp256k1_scalar_add(&r1, &r1, &s);
        secp256k1_scalar_add(&r2, &s2, &s);
        secp256k1_scalar_add(&r2, &s1, &r2);
        CHECK(secp256k1_scalar_eq(&r1, &r2));
    }

    {
        /* Test associativity of mul. */
        secp256k1_scalar_t r1, r2;
        secp256k1_scalar_mul(&r1, &s1, &s2);
        secp256k1_scalar_mul(&r1, &r1, &s);
        secp256k1_scalar_mul(&r2, &s2, &s);
        secp256k1_scalar_mul(&r2, &s1, &r2);
        CHECK(secp256k1_scalar_eq(&r1, &r2));
    }

    {
        /* Test distributitivity of mul over add. */
        secp256k1_scalar_t r1, r2, t;
        secp256k1_scalar_add(&r1, &s1, &s2);
        secp256k1_scalar_mul(&r1, &r1, &s);
        secp256k1_scalar_mul(&r2, &s1, &s);
        secp256k1_scalar_mul(&t, &s2, &s);
        secp256k1_scalar_add(&r2, &r2, &t);
        CHECK(secp256k1_scalar_eq(&r1, &r2));
    }

    {
        /* Test square. */
        secp256k1_scalar_t r1, r2;
        secp256k1_scalar_sqr(&r1, &s1);
        secp256k1_scalar_mul(&r2, &s1, &s1);
        CHECK(secp256k1_scalar_eq(&r1, &r2));
    }

    {
        /* Test multiplicative identity. */
        secp256k1_scalar_t r1, v1;
        secp256k1_scalar_set_int(&v1,1);
        secp256k1_scalar_mul(&r1, &s1, &v1);
        CHECK(secp256k1_scalar_eq(&r1, &s1));
    }

    {
        /* Test additive identity. */
        secp256k1_scalar_t r1, v0;
        secp256k1_scalar_set_int(&v0,0);
        secp256k1_scalar_add(&r1, &s1, &v0);
        CHECK(secp256k1_scalar_eq(&r1, &s1));
    }

    {
        /* Test zero product property. */
        secp256k1_scalar_t r1, v0;
        secp256k1_scalar_set_int(&v0,0);
        secp256k1_scalar_mul(&r1, &s1, &v0);
        CHECK(secp256k1_scalar_eq(&r1, &v0));
    }

}

void run_scalar_tests(void) {
    int i;
    for (i = 0; i < 128 * count; i++) {
        scalar_test();
    }

    {
        /* (-1)+1 should be zero. */
        secp256k1_scalar_t s, o;
        secp256k1_scalar_set_int(&s, 1);
        CHECK(secp256k1_scalar_is_one(&s));
        secp256k1_scalar_negate(&o, &s);
        secp256k1_scalar_add(&o, &o, &s);
        CHECK(secp256k1_scalar_is_zero(&o));
        secp256k1_scalar_negate(&o, &o);
        CHECK(secp256k1_scalar_is_zero(&o));
    }

#ifndef USE_NUM_NONE
    {
        /* A scalar with value of the curve order should be 0. */
        secp256k1_num_t order;
        secp256k1_scalar_t zero;
        unsigned char bin[32];
        int overflow = 0;
        secp256k1_scalar_order_get_num(&order);
        secp256k1_num_get_bin(bin, 32, &order);
        secp256k1_scalar_set_b32(&zero, bin, &overflow);
        CHECK(overflow == 1);
        CHECK(secp256k1_scalar_is_zero(&zero));
    }
#endif
}

/***** FIELD TESTS *****/

void random_fe(secp256k1_fe_t *x) {
    unsigned char bin[32];
    do {
        secp256k1_rand256(bin);
        if (secp256k1_fe_set_b32(x, bin)) {
            return;
        }
    } while(1);
}

void random_fe_non_zero(secp256k1_fe_t *nz) {
    int tries = 10;
    while (--tries >= 0) {
        random_fe(nz);
        secp256k1_fe_normalize(nz);
        if (!secp256k1_fe_is_zero(nz))
            break;
    }
    /* Infinitesimal probability of spurious failure here */
    CHECK(tries >= 0);
}

void random_fe_non_square(secp256k1_fe_t *ns) {
    secp256k1_fe_t r;
    random_fe_non_zero(ns);
    if (secp256k1_fe_sqrt_var(&r, ns)) {
        secp256k1_fe_negate(ns, ns, 1);
    }
}

int check_fe_equal(const secp256k1_fe_t *a, const secp256k1_fe_t *b) {
    secp256k1_fe_t an = *a;
    secp256k1_fe_t bn = *b;
    secp256k1_fe_normalize_weak(&an);
    secp256k1_fe_normalize_var(&bn);
    return secp256k1_fe_equal_var(&an, &bn);
}

int check_fe_inverse(const secp256k1_fe_t *a, const secp256k1_fe_t *ai) {
    secp256k1_fe_t x;
    secp256k1_fe_t one;
    secp256k1_fe_mul(&x, a, ai);
    secp256k1_fe_set_int(&one, 1);
    return check_fe_equal(&x, &one);
}

void run_field_convert(void) {
    static const unsigned char b32[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x40
    };
    static const char *c64 = "0001020304050607111213141516171822232425262728293334353637383940";
    static const secp256k1_fe_storage_t fes = SECP256K1_FE_STORAGE_CONST(
        0x00010203UL, 0x04050607UL, 0x11121314UL, 0x15161718UL,
        0x22232425UL, 0x26272829UL, 0x33343536UL, 0x37383940UL
    );
    static const secp256k1_fe_t fe = SECP256K1_FE_CONST(
        0x00010203UL, 0x04050607UL, 0x11121314UL, 0x15161718UL,
        0x22232425UL, 0x26272829UL, 0x33343536UL, 0x37383940UL
    );
    secp256k1_fe_t fe2;
    unsigned char b322[32];
    char c642[64];
    secp256k1_fe_storage_t fes2;
    /* Check conversions to fe. */
    CHECK(secp256k1_fe_set_b32(&fe2, b32));
    CHECK(secp256k1_fe_equal_var(&fe, &fe2));
    CHECK(secp256k1_fe_set_hex(&fe2, c64));
    CHECK(secp256k1_fe_equal_var(&fe, &fe2));
    secp256k1_fe_from_storage(&fe2, &fes);
    CHECK(secp256k1_fe_equal_var(&fe, &fe2));
    /* Check conversion from fe. */
    secp256k1_fe_get_b32(b322, &fe);
    CHECK(memcmp(b322, b32, 32) == 0);
    secp256k1_fe_get_hex(c642, &fe);
    CHECK(memcmp(c642, c64, 64) == 0);
    secp256k1_fe_to_storage(&fes2, &fe);
    CHECK(memcmp(&fes2, &fes, sizeof(fes)) == 0);
}

void run_field_misc(void) {
    const unsigned char f32_5[32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
    };
    secp256k1_fe_t x;
    secp256k1_fe_t y;
    secp256k1_fe_t z;
    secp256k1_fe_t q;
    secp256k1_fe_t fe5;
    int i;
    CHECK(secp256k1_fe_set_b32(&fe5, f32_5));
    for (i = 0; i < 5*count; i++) {
        secp256k1_fe_storage_t xs, ys, zs;
        random_fe(&x);
        random_fe_non_zero(&y);
        /* Test the fe equality and comparison operations. */
        CHECK(secp256k1_fe_cmp_var(&x, &x) == 0);
        CHECK(secp256k1_fe_equal_var(&x, &x));
        z = x;
        secp256k1_fe_add(&z,&y);
        secp256k1_fe_normalize(&z);
        /* Test storage conversion and conditional moves. */
        secp256k1_fe_to_storage(&xs, &x);
        secp256k1_fe_to_storage(&ys, &y);
        secp256k1_fe_to_storage(&zs, &z);
        secp256k1_fe_storage_cmov(&zs, &xs, 0);
        CHECK(memcmp(&xs, &zs, sizeof(xs)) != 0);
        secp256k1_fe_storage_cmov(&ys, &xs, 1);
        CHECK(memcmp(&xs, &ys, sizeof(xs)) == 0);
        secp256k1_fe_from_storage(&x, &xs);
        secp256k1_fe_from_storage(&y, &ys);
        secp256k1_fe_from_storage(&z, &zs);
        /* Test that mul_int, mul, and add agree. */
        secp256k1_fe_add(&y, &x);
        secp256k1_fe_add(&y, &x);
        z = x;
        secp256k1_fe_mul_int(&z, 3);
        CHECK(check_fe_equal(&y, &z));
        secp256k1_fe_add(&y, &x);
        secp256k1_fe_add(&z, &x);
        CHECK(check_fe_equal(&z, &y));
        z = x;
        secp256k1_fe_mul_int(&z, 5);
        secp256k1_fe_mul(&q, &x, &fe5);
        CHECK(check_fe_equal(&z, &q));
        secp256k1_fe_negate(&x, &x, 1);
        secp256k1_fe_add(&z, &x);
        secp256k1_fe_add(&q, &x);
        CHECK(check_fe_equal(&y, &z));
        CHECK(check_fe_equal(&q, &y));
    }
}

void run_field_inv(void) {
    secp256k1_fe_t x, xi, xii;
    int i;
    for (i = 0; i < 10*count; i++) {
        random_fe_non_zero(&x);
        secp256k1_fe_inv(&xi, &x);
        CHECK(check_fe_inverse(&x, &xi));
        secp256k1_fe_inv(&xii, &xi);
        CHECK(check_fe_equal(&x, &xii));
    }
}

void run_field_inv_var(void) {
    secp256k1_fe_t x, xi, xii;
    int i;
    for (i = 0; i < 10*count; i++) {
        random_fe_non_zero(&x);
        secp256k1_fe_inv_var(&xi, &x);
        CHECK(check_fe_inverse(&x, &xi));
        secp256k1_fe_inv_var(&xii, &xi);
        CHECK(check_fe_equal(&x, &xii));
    }
}

void run_field_inv_all_var(void) {
    secp256k1_fe_t x[16], xi[16], xii[16];
    int i;
    /* Check it's safe to call for 0 elements */
    secp256k1_fe_inv_all_var(0, xi, x);
    for (i = 0; i < count; i++) {
        size_t j;
        size_t len = (secp256k1_rand32() & 15) + 1;
        for (j = 0; j < len; j++)
            random_fe_non_zero(&x[j]);
        secp256k1_fe_inv_all_var(len, xi, x);
        for (j = 0; j < len; j++)
            CHECK(check_fe_inverse(&x[j], &xi[j]));
        secp256k1_fe_inv_all_var(len, xii, xi);
        for (j = 0; j < len; j++)
            CHECK(check_fe_equal(&x[j], &xii[j]));
    }
}

void run_sqr(void) {
    secp256k1_fe_t x, s;

    {
        int i;
        secp256k1_fe_set_int(&x, 1);
        secp256k1_fe_negate(&x, &x, 1);

        for (i = 1; i <= 512; ++i) {
            secp256k1_fe_mul_int(&x, 2);
            secp256k1_fe_normalize(&x);
            secp256k1_fe_sqr(&s, &x);
        }
    }
}

void test_sqrt(const secp256k1_fe_t *a, const secp256k1_fe_t *k) {
    secp256k1_fe_t r1, r2;
    int v = secp256k1_fe_sqrt_var(&r1, a);
    CHECK((v == 0) == (k == NULL));

    if (k != NULL) {
        /* Check that the returned root is +/- the given known answer */
        secp256k1_fe_negate(&r2, &r1, 1);
        secp256k1_fe_add(&r1, k); secp256k1_fe_add(&r2, k);
        secp256k1_fe_normalize(&r1); secp256k1_fe_normalize(&r2);
        CHECK(secp256k1_fe_is_zero(&r1) || secp256k1_fe_is_zero(&r2));
    }
}

void run_sqrt(void) {
    secp256k1_fe_t ns, x, s, t;
    int i;

    /* Check sqrt(0) is 0 */
    secp256k1_fe_set_int(&x, 0);
    secp256k1_fe_sqr(&s, &x);
    test_sqrt(&s, &x);

    /* Check sqrt of small squares (and their negatives) */
    for (i = 1; i <= 100; i++) {
        secp256k1_fe_set_int(&x, i);
        secp256k1_fe_sqr(&s, &x);
        test_sqrt(&s, &x);
        secp256k1_fe_negate(&t, &s, 1);
        test_sqrt(&t, NULL);
    }

    /* Consistency checks for large random values */
    for (i = 0; i < 10; i++) {
        int j;
        random_fe_non_square(&ns);
        for (j = 0; j < count; j++) {
            random_fe(&x);
            secp256k1_fe_sqr(&s, &x);
            test_sqrt(&s, &x);
            secp256k1_fe_negate(&t, &s, 1);
            test_sqrt(&t, NULL);
            secp256k1_fe_mul(&t, &s, &ns);
            test_sqrt(&t, NULL);
        }
    }
}

/***** GROUP TESTS *****/

void ge_equals_ge(const secp256k1_ge_t *a, const secp256k1_ge_t *b) {
    CHECK(a->infinity == b->infinity);
    if (a->infinity)
        return;
    CHECK(secp256k1_fe_equal_var(&a->x, &b->x));
    CHECK(secp256k1_fe_equal_var(&b->y, &b->y));
}

void ge_equals_gej(const secp256k1_ge_t *a, const secp256k1_gej_t *b) {
    secp256k1_fe_t z2s;
    secp256k1_fe_t u1, u2, s1, s2;
    CHECK(a->infinity == b->infinity);
    if (a->infinity)
        return;
    /* Check a.x * b.z^2 == b.x && a.y * b.z^3 == b.y, to avoid inverses. */
    secp256k1_fe_sqr(&z2s, &b->z);
    secp256k1_fe_mul(&u1, &a->x, &z2s);
    u2 = b->x; secp256k1_fe_normalize_weak(&u2);
    secp256k1_fe_mul(&s1, &a->y, &z2s); secp256k1_fe_mul(&s1, &s1, &b->z);
    s2 = b->y; secp256k1_fe_normalize_weak(&s2);
    CHECK(secp256k1_fe_equal_var(&u1, &u2));
    CHECK(secp256k1_fe_equal_var(&s1, &s2));
}

void test_ge(void) {
    int i, i1;
    int runs = 4;
    /* Points: (infinity, p1, p1, -p1, -p1, p2, p2, -p2, -p2, p3, p3, -p3, -p3, p4, p4, -p4, -p4).
     * The second in each pair of identical points uses a random Z coordinate in the Jacobian form.
     * All magnitudes are randomized.
     * All 17*17 combinations of points are added to eachother, using all applicable methods.
     */
    secp256k1_ge_t *ge = malloc(sizeof(secp256k1_ge_t) * (1 + 4 * runs));
    secp256k1_gej_t *gej = malloc(sizeof(secp256k1_gej_t) * (1 + 4 * runs));
    secp256k1_gej_set_infinity(&gej[0]);
    secp256k1_ge_clear(&ge[0]);
    secp256k1_ge_set_gej_var(&ge[0], &gej[0]);
    for (i = 0; i < runs; i++) {
        int j;
        secp256k1_ge_t g;
        random_group_element_test(&g);
        ge[1 + 4 * i] = g;
        ge[2 + 4 * i] = g;
        secp256k1_ge_neg(&ge[3 + 4 * i], &g);
        secp256k1_ge_neg(&ge[4 + 4 * i], &g);
        secp256k1_gej_set_ge(&gej[1 + 4 * i], &ge[1 + 4 * i]);
        random_group_element_jacobian_test(&gej[2 + 4 * i], &ge[2 + 4 * i]);
        secp256k1_gej_set_ge(&gej[3 + 4 * i], &ge[3 + 4 * i]);
        random_group_element_jacobian_test(&gej[4 + 4 * i], &ge[4 + 4 * i]);
        for (j = 0; j < 4; j++) {
            random_field_element_magnitude(&ge[1 + j + 4 * i].x);
            random_field_element_magnitude(&ge[1 + j + 4 * i].y);
            random_field_element_magnitude(&gej[1 + j + 4 * i].x);
            random_field_element_magnitude(&gej[1 + j + 4 * i].y);
            random_field_element_magnitude(&gej[1 + j + 4 * i].z);
        }
    }

    for (i1 = 0; i1 < 1 + 4 * runs; i1++) {
        int i2;
        for (i2 = 0; i2 < 1 + 4 * runs; i2++) {
            /* Compute reference result using gej + gej (var). */
            secp256k1_gej_t refj, resj;
            secp256k1_ge_t ref;
            secp256k1_gej_add_var(&refj, &gej[i1], &gej[i2]);
            secp256k1_ge_set_gej_var(&ref, &refj);

            /* Test gej + ge (var). */
            secp256k1_gej_add_ge_var(&resj, &gej[i1], &ge[i2]);
            ge_equals_gej(&ref, &resj);

            /* Test gej + ge (const). */
            if (i2 != 0) {
                /* secp256k1_gej_add_ge does not support its second argument being infinity. */
                secp256k1_gej_add_ge(&resj, &gej[i1], &ge[i2]);
                ge_equals_gej(&ref, &resj);
            }

            /* Test doubling (var). */
            if ((i1 == 0 && i2 == 0) || ((i1 + 3)/4 == (i2 + 3)/4 && ((i1 + 3)%4)/2 == ((i2 + 3)%4)/2)) {
                /* Normal doubling. */
                secp256k1_gej_double_var(&resj, &gej[i1]);
                ge_equals_gej(&ref, &resj);
                secp256k1_gej_double_var(&resj, &gej[i2]);
                ge_equals_gej(&ref, &resj);
            }

            /* Test adding opposites. */
            if ((i1 == 0 && i2 == 0) || ((i1 + 3)/4 == (i2 + 3)/4 && ((i1 + 3)%4)/2 != ((i2 + 3)%4)/2)) {
                CHECK(secp256k1_ge_is_infinity(&ref));
            }

            /* Test adding infinity. */
            if (i1 == 0) {
                CHECK(secp256k1_ge_is_infinity(&ge[i1]));
                CHECK(secp256k1_gej_is_infinity(&gej[i1]));
                ge_equals_gej(&ref, &gej[i2]);
            }
            if (i2 == 0) {
                CHECK(secp256k1_ge_is_infinity(&ge[i2]));
                CHECK(secp256k1_gej_is_infinity(&gej[i2]));
                ge_equals_gej(&ref, &gej[i1]);
            }
        }
    }

    /* Test adding all points together in random order equals infinity. */
    {
        secp256k1_gej_t sum;
        secp256k1_gej_t *gej_shuffled = malloc((4 * runs + 1) * sizeof(secp256k1_gej_t));
        for (i = 0; i < 4 * runs + 1; i++) {
            gej_shuffled[i] = gej[i];
        }
        for (i = 0; i < 4 * runs + 1; i++) {
            int swap = i + secp256k1_rand32() % (4 * runs + 1 - i);
            if (swap != i) {
                secp256k1_gej_t t = gej_shuffled[i];
                gej_shuffled[i] = gej_shuffled[swap];
                gej_shuffled[swap] = t;
            }
        }
        secp256k1_gej_set_infinity(&sum);
        for (i = 0; i < 4 * runs + 1; i++) {
            secp256k1_gej_add_var(&sum, &sum, &gej_shuffled[i]);
        }
        CHECK(secp256k1_gej_is_infinity(&sum));
        free(gej_shuffled);
    }

    /* Test batch gej -> ge conversion. */
    {
        secp256k1_ge_t *ge_set_all = malloc((4 * runs + 1) * sizeof(secp256k1_ge_t));
        secp256k1_ge_set_all_gej_var(4 * runs + 1, ge_set_all, gej);
        for (i = 0; i < 4 * runs + 1; i++) {
            ge_equals_gej(&ge_set_all[i], &gej[i]);
        }
        free(ge_set_all);
    }

    free(ge);
    free(gej);
}

void run_ge(void) {
    int i;
    for (i = 0; i < count * 32; i++) {
        test_ge();
    }
}

/***** ECMULT TESTS *****/

void run_ecmult_chain(void) {
    secp256k1_gej_t a;
    secp256k1_gej_t x;
    secp256k1_gej_t x2;
    secp256k1_fe_t ax;
    secp256k1_fe_t ay;
    secp256k1_scalar_t xn;
    secp256k1_scalar_t gn;
    secp256k1_scalar_t xf;
    secp256k1_scalar_t gf;
    secp256k1_scalar_t ae;
    secp256k1_scalar_t ge;
    int i;
    /* two random initial factors xn and gn */
    static const unsigned char xni[32] = {
        0x84, 0xcc, 0x54, 0x52, 0xf7, 0xfd, 0xe1, 0xed,
        0xb4, 0xd3, 0x8a, 0x8c, 0xe9, 0xb1, 0xb8, 0x4c,
        0xce, 0xf3, 0x1f, 0x14, 0x6e, 0x56, 0x9b, 0xe9,
        0x70, 0x5d, 0x35, 0x7a, 0x42, 0x98, 0x54, 0x07
    };
    static const unsigned char gni[32] = {
        0xa1, 0xe5, 0x8d, 0x22, 0x55, 0x3d, 0xcd, 0x42,
        0xb2, 0x39, 0x80, 0x62, 0x5d, 0x4c, 0x57, 0xa9,
        0x6e, 0x93, 0x23, 0xd4, 0x2b, 0x31, 0x52, 0xe5,
        0xca, 0x2c, 0x39, 0x90, 0xed, 0xc7, 0xc9, 0xde
    };
    /* two small multipliers to be applied to xn and gn in every iteration: */
    static const unsigned char xfi[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x13,0x37};
    static const unsigned char gfi[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x71,0x13};
    char res[131];
    char res2[131];
    /* random starting point A (on the curve) */
    VERIFY_CHECK(secp256k1_fe_set_hex(&ax, "8b30bbe9ae2a990696b22f670709dff3727fd8bc04d3362c6c7bf458e2846004"));
    VERIFY_CHECK(secp256k1_fe_set_hex(&ay, "a357ae915c4a65281309edf20504740f0eb3343990216b4f81063cb65f2f7e0f"));
    secp256k1_gej_set_xy(&a, &ax, &ay);
    secp256k1_scalar_set_b32(&xn, xni, NULL);
    secp256k1_scalar_set_b32(&gn, gni, NULL);
    secp256k1_scalar_set_b32(&xf, xfi, NULL);
    secp256k1_scalar_set_b32(&gf, gfi, NULL);
    /* accumulators with the resulting coefficients to A and G */
    secp256k1_scalar_set_int(&ae, 1);
    secp256k1_scalar_set_int(&ge, 0);
    /* the point being computed */
    x = a;
    for (i = 0; i < 200*count; i++) {
        /* in each iteration, compute X = xn*X + gn*G; */
        secp256k1_ecmult(&x, &x, &xn, &gn);
        /* also compute ae and ge: the actual accumulated factors for A and G */
        /* if X was (ae*A+ge*G), xn*X + gn*G results in (xn*ae*A + (xn*ge+gn)*G) */
        secp256k1_scalar_mul(&ae, &ae, &xn);
        secp256k1_scalar_mul(&ge, &ge, &xn);
        secp256k1_scalar_add(&ge, &ge, &gn);
        /* modify xn and gn */
        secp256k1_scalar_mul(&xn, &xn, &xf);
        secp256k1_scalar_mul(&gn, &gn, &gf);

        /* verify */
        if (i == 19999) {
            secp256k1_gej_get_hex(res, &x);
            CHECK(memcmp(res, "(D6E96687F9B10D092A6F35439D86CEBEA4535D0D409F53586440BD74B933E830,B95CBCA2C77DA786539BE8FD53354D2D3B4F566AE658045407ED6015EE1B2A88)", 131) == 0);
        }
    }
    /* redo the computation, but directly with the resulting ae and ge coefficients: */
    secp256k1_ecmult(&x2, &a, &ae, &ge);
    secp256k1_gej_get_hex(res, &x);
    secp256k1_gej_get_hex(res2, &x2);
    CHECK(memcmp(res, res2, 131) == 0);
}

void test_point_times_order(const secp256k1_gej_t *point) {
    /* X * (point + G) + (order-X) * (pointer + G) = 0 */
    secp256k1_scalar_t x;
    secp256k1_scalar_t nx;
    secp256k1_gej_t res1, res2;
    secp256k1_ge_t res3;
    unsigned char pub[65];
    int psize = 65;
    random_scalar_order_test(&x);
    secp256k1_scalar_negate(&nx, &x);
    secp256k1_ecmult(&res1, point, &x, &x); /* calc res1 = x * point + x * G; */
    secp256k1_ecmult(&res2, point, &nx, &nx); /* calc res2 = (order - x) * point + (order - x) * G; */
    secp256k1_gej_add_var(&res1, &res1, &res2);
    CHECK(secp256k1_gej_is_infinity(&res1));
    CHECK(secp256k1_gej_is_valid_var(&res1) == 0);
    secp256k1_ge_set_gej(&res3, &res1);
    CHECK(secp256k1_ge_is_infinity(&res3));
    CHECK(secp256k1_ge_is_valid_var(&res3) == 0);
    CHECK(secp256k1_eckey_pubkey_serialize(&res3, pub, &psize, 0) == 0);
    psize = 65;
    CHECK(secp256k1_eckey_pubkey_serialize(&res3, pub, &psize, 1) == 0);
}

void run_point_times_order(void) {
    int i;
    char c[64];
    secp256k1_fe_t x; VERIFY_CHECK(secp256k1_fe_set_hex(&x, "0000000000000000000000000000000000000000000000000000000000000002"));
    for (i = 0; i < 500; i++) {
        secp256k1_ge_t p;
        if (secp256k1_ge_set_xo_var(&p, &x, 1)) {
            secp256k1_gej_t j;
            CHECK(secp256k1_ge_is_valid_var(&p));
            secp256k1_gej_set_ge(&j, &p);
            CHECK(secp256k1_gej_is_valid_var(&j));
            test_point_times_order(&j);
        }
        secp256k1_fe_sqr(&x, &x);
    }
    secp256k1_fe_get_hex(c, &x);
    CHECK(memcmp(c, "7603CB59B0EF6C63FE6084792A0C378CDB3233A80F8A9A09A877DEAD31B38C45", 64) == 0);
}

void test_wnaf(const secp256k1_scalar_t *number, int w) {
    secp256k1_scalar_t x, two, t;
    int wnaf[256];
    int zeroes = -1;
    int i;
    int bits;
    secp256k1_scalar_set_int(&x, 0);
    secp256k1_scalar_set_int(&two, 2);
    bits = secp256k1_ecmult_wnaf(wnaf, number, w);
    CHECK(bits <= 256);
    for (i = bits-1; i >= 0; i--) {
        int v = wnaf[i];
        secp256k1_scalar_mul(&x, &x, &two);
        if (v) {
            CHECK(zeroes == -1 || zeroes >= w-1); /* check that distance between non-zero elements is at least w-1 */
            zeroes=0;
            CHECK((v & 1) == 1); /* check non-zero elements are odd */
            CHECK(v <= (1 << (w-1)) - 1); /* check range below */
            CHECK(v >= -(1 << (w-1)) - 1); /* check range above */
        } else {
            CHECK(zeroes != -1); /* check that no unnecessary zero padding exists */
            zeroes++;
        }
        if (v >= 0) {
            secp256k1_scalar_set_int(&t, v);
        } else {
            secp256k1_scalar_set_int(&t, -v);
            secp256k1_scalar_negate(&t, &t);
        }
        secp256k1_scalar_add(&x, &x, &t);
    }
    CHECK(secp256k1_scalar_eq(&x, number)); /* check that wnaf represents number */
}

void run_wnaf(void) {
    int i;
    secp256k1_scalar_t n;
    for (i = 0; i < count; i++) {
        random_scalar_order(&n);
        if (i % 1)
            secp256k1_scalar_negate(&n, &n);
        test_wnaf(&n, 4+(i%10));
    }
}

void random_sign(secp256k1_ecdsa_sig_t *sig, const secp256k1_scalar_t *key, const secp256k1_scalar_t *msg, int *recid) {
    secp256k1_scalar_t nonce;
    do {
        random_scalar_order_test(&nonce);
    } while(!secp256k1_ecdsa_sig_sign(sig, key, msg, &nonce, recid));
}

void test_ecdsa_sign_verify(void) {
    secp256k1_gej_t pubj;
    secp256k1_ge_t pub;
    secp256k1_scalar_t one;
    secp256k1_scalar_t msg, key;
    secp256k1_ecdsa_sig_t sig;
    int recid;
    int getrec;
    random_scalar_order_test(&msg);
    random_scalar_order_test(&key);
    secp256k1_ecmult_gen(&pubj, &key);
    secp256k1_ge_set_gej(&pub, &pubj);
    getrec = secp256k1_rand32()&1;
    random_sign(&sig, &key, &msg, getrec?&recid:NULL);
    if (getrec) CHECK(recid >= 0 && recid < 4);
    CHECK(secp256k1_ecdsa_sig_verify(&sig, &pub, &msg));
    secp256k1_scalar_set_int(&one, 1);
    secp256k1_scalar_add(&msg, &msg, &one);
    CHECK(!secp256k1_ecdsa_sig_verify(&sig, &pub, &msg));
}

void run_ecdsa_sign_verify(void) {
    int i;
    for (i = 0; i < 10*count; i++) {
        test_ecdsa_sign_verify();
    }
}

/** Dummy nonce generation function that just uses a precomputed nonce, and fails if it is not accepted. Use only for testing. */
static int precomputed_nonce_function(unsigned char *nonce32, const unsigned char *msg32, const unsigned char *key32, unsigned int counter, const void *data) {
    (void)msg32;
    (void)key32;
    memcpy(nonce32, data, 32);
    return (counter == 0);
}

static int nonce_function_test_fail(unsigned char *nonce32, const unsigned char *msg32, const unsigned char *key32, unsigned int counter, const void *data) {
   /* Dummy nonce generator that has a fatal error on the first counter value. */
   if (counter == 0) return 0;
   return nonce_function_rfc6979(nonce32, msg32, key32, counter - 1, data);
}

static int nonce_function_test_retry(unsigned char *nonce32, const unsigned char *msg32, const unsigned char *key32, unsigned int counter, const void *data) {
   /* Dummy nonce generator that produces unacceptable nonces for the first several counter values. */
   if (counter < 3) {
       memset(nonce32, counter==0 ? 0 : 255, 32);
       if (counter == 2) nonce32[31]--;
       return 1;
   }
   if (counter < 5) {
       static const unsigned char order[] = {
           0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
           0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
           0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,
           0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x41
       };
       memcpy(nonce32, order, 32);
       if (counter == 4) nonce32[31]++;
       return 1;
   }
   /* Retry rate of 6979 is negligible esp. as we only call this in determinstic tests. */
   /* If someone does fine a case where it retries for secp256k1, we'd like to know. */
   if (counter > 5) return 0;
   return nonce_function_rfc6979(nonce32, msg32, key32, counter - 5, data);
}

void test_ecdsa_end_to_end(void) {
    unsigned char privkey[32];
    unsigned char message[32];
    unsigned char privkey2[32];
    unsigned char csignature[64];
    unsigned char signature[72];
    unsigned char pubkey[65];
    unsigned char recpubkey[65];
    unsigned char seckey[300];
    int signaturelen = 72;
    int recid = 0;
    int recpubkeylen = 0;
    int pubkeylen = 65;
    int seckeylen = 300;

    /* Generate a random key and message. */
    {
        secp256k1_scalar_t msg, key;
        random_scalar_order_test(&msg);
        random_scalar_order_test(&key);
        secp256k1_scalar_get_b32(privkey, &key);
        secp256k1_scalar_get_b32(message, &msg);
    }

    /* Construct and verify corresponding public key. */
    CHECK(secp256k1_ec_seckey_verify(privkey) == 1);
    CHECK(secp256k1_ec_pubkey_create(pubkey, &pubkeylen, privkey, (secp256k1_rand32() & 3) != 0) == 1);
    if (secp256k1_rand32() & 1) {
        CHECK(secp256k1_ec_pubkey_decompress(pubkey, &pubkeylen));
    }
    CHECK(secp256k1_ec_pubkey_verify(pubkey, pubkeylen));

    /* Verify private key import and export. */
    CHECK(secp256k1_ec_privkey_export(privkey, seckey, &seckeylen, secp256k1_rand32() % 2) == 1);
    CHECK(secp256k1_ec_privkey_import(privkey2, seckey, seckeylen) == 1);
    CHECK(memcmp(privkey, privkey2, 32) == 0);

    /* Optionally tweak the keys using addition. */
    if (secp256k1_rand32() % 3 == 0) {
        int ret1;
        int ret2;
        unsigned char rnd[32];
        unsigned char pubkey2[65];
        int pubkeylen2 = 65;
        secp256k1_rand256_test(rnd);
        ret1 = secp256k1_ec_privkey_tweak_add(privkey, rnd);
        ret2 = secp256k1_ec_pubkey_tweak_add(pubkey, pubkeylen, rnd);
        CHECK(ret1 == ret2);
        if (ret1 == 0) return;
        CHECK(secp256k1_ec_pubkey_create(pubkey2, &pubkeylen2, privkey, pubkeylen == 33) == 1);
        CHECK(memcmp(pubkey, pubkey2, pubkeylen) == 0);
    }

    /* Optionally tweak the keys using multiplication. */
    if (secp256k1_rand32() % 3 == 0) {
        int ret1;
        int ret2;
        unsigned char rnd[32];
        unsigned char pubkey2[65];
        int pubkeylen2 = 65;
        secp256k1_rand256_test(rnd);
        ret1 = secp256k1_ec_privkey_tweak_mul(privkey, rnd);
        ret2 = secp256k1_ec_pubkey_tweak_mul(pubkey, pubkeylen, rnd);
        CHECK(ret1 == ret2);
        if (ret1 == 0) return;
        CHECK(secp256k1_ec_pubkey_create(pubkey2, &pubkeylen2, privkey, pubkeylen == 33) == 1);
        CHECK(memcmp(pubkey, pubkey2, pubkeylen) == 0);
    }

    /* Sign. */
    CHECK(secp256k1_ecdsa_sign(message, signature, &signaturelen, privkey, NULL, NULL) == 1);
    /* Verify. */
    CHECK(secp256k1_ecdsa_verify(message, signature, signaturelen, pubkey, pubkeylen) == 1);
    /* Destroy signature and verify again. */
    signature[signaturelen - 1 - secp256k1_rand32() % 20] += 1 + (secp256k1_rand32() % 255);
    CHECK(secp256k1_ecdsa_verify(message, signature, signaturelen, pubkey, pubkeylen) != 1);

    /* Compact sign. */
    CHECK(secp256k1_ecdsa_sign_compact(message, csignature, privkey, NULL, NULL, &recid) == 1);
    /* Recover. */
    CHECK(secp256k1_ecdsa_recover_compact(message, csignature, recpubkey, &recpubkeylen, pubkeylen == 33, recid) == 1);
    CHECK(recpubkeylen == pubkeylen);
    CHECK(memcmp(pubkey, recpubkey, pubkeylen) == 0);
    /* Destroy signature and verify again. */
    csignature[secp256k1_rand32() % 64] += 1 + (secp256k1_rand32() % 255);
    CHECK(secp256k1_ecdsa_recover_compact(message, csignature, recpubkey, &recpubkeylen, pubkeylen == 33, recid) != 1 ||
          memcmp(pubkey, recpubkey, pubkeylen) != 0);
    CHECK(recpubkeylen == pubkeylen);

}

void test_random_pubkeys(void) {
    secp256k1_ge_t elem;
    secp256k1_ge_t elem2;
    unsigned char in[65];
    /* Generate some randomly sized pubkeys. */
    uint32_t r = secp256k1_rand32();
    int len = (r & 3) == 0 ? 65 : 33;
    r>>=2;
    if ((r & 3) == 0) len = (r & 252) >> 3;
    r>>=8;
    if (len == 65) {
      in[0] = (r & 2) ? 4 : (r & 1? 6 : 7);
    } else {
      in[0] = (r & 1) ? 2 : 3;
    }
    r>>=2;
    if ((r & 7) == 0) in[0] = (r & 2040) >> 3;
    r>>=11;
    if (len > 1) secp256k1_rand256(&in[1]);
    if (len > 33) secp256k1_rand256(&in[33]);
    if (secp256k1_eckey_pubkey_parse(&elem, in, len)) {
        unsigned char out[65];
        unsigned char firstb;
        int res;
        int size = len;
        firstb = in[0];
        /* If the pubkey can be parsed, it should round-trip... */
        CHECK(secp256k1_eckey_pubkey_serialize(&elem, out, &size, len == 33));
        CHECK(size == len);
        CHECK(memcmp(&in[1], &out[1], len-1) == 0);
        /* ... except for the type of hybrid inputs. */
        if ((in[0] != 6) && (in[0] != 7)) CHECK(in[0] == out[0]);
        size = 65;
        CHECK(secp256k1_eckey_pubkey_serialize(&elem, in, &size, 0));
        CHECK(size == 65);
        CHECK(secp256k1_eckey_pubkey_parse(&elem2, in, size));
        ge_equals_ge(&elem,&elem2);
        /* Check that the X9.62 hybrid type is checked. */
        in[0] = (r & 1) ? 6 : 7;
        res = secp256k1_eckey_pubkey_parse(&elem2, in, size);
        if (firstb == 2 || firstb == 3) {
            if (in[0] == firstb + 4) CHECK(res);
            else CHECK(!res);
        }
        if (res) {
            ge_equals_ge(&elem,&elem2);
            CHECK(secp256k1_eckey_pubkey_serialize(&elem, out, &size, 0));
            CHECK(memcmp(&in[1], &out[1], 64) == 0);
        }
    }
}

void run_random_pubkeys(void) {
    int i;
    for (i = 0; i < 10*count; i++) {
        test_random_pubkeys();
    }
}

void run_ecdsa_end_to_end(void) {
    int i;
    for (i = 0; i < 64*count; i++) {
        test_ecdsa_end_to_end();
    }
}

/* Tests several edge cases. */
void test_ecdsa_edge_cases(void) {
    const unsigned char msg32[32] = {
        'T', 'h', 'i', 's', ' ', 'i', 's', ' ',
        'a', ' ', 'v', 'e', 'r', 'y', ' ', 's',
        'e', 'c', 'r', 'e', 't', ' ', 'm', 'e',
        's', 's', 'a', 'g', 'e', '.', '.', '.'
    };
    const unsigned char sig64[64] = {
        /* Generated by signing the above message with nonce 'This is the nonce we will use...'
         * and secret key 0 (which is not valid), resulting in recid 0. */
        0x67, 0xCB, 0x28, 0x5F, 0x9C, 0xD1, 0x94, 0xE8,
        0x40, 0xD6, 0x29, 0x39, 0x7A, 0xF5, 0x56, 0x96,
        0x62, 0xFD, 0xE4, 0x46, 0x49, 0x99, 0x59, 0x63,
        0x17, 0x9A, 0x7D, 0xD1, 0x7B, 0xD2, 0x35, 0x32,
        0x4B, 0x1B, 0x7D, 0xF3, 0x4C, 0xE1, 0xF6, 0x8E,
        0x69, 0x4F, 0xF6, 0xF1, 0x1A, 0xC7, 0x51, 0xDD,
        0x7D, 0xD7, 0x3E, 0x38, 0x7E, 0xE4, 0xFC, 0x86,
        0x6E, 0x1B, 0xE8, 0xEC, 0xC7, 0xDD, 0x95, 0x57
    };
    unsigned char pubkey[65];
    int pubkeylen = 65;
    /* signature (r,s) = (4,4), which can be recovered with all 4 recids. */
    const unsigned char sigb64[64] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
    };
    unsigned char pubkeyb[33];
    int pubkeyblen = 33;
    int recid;

    CHECK(!secp256k1_ecdsa_recover_compact(msg32, sig64, pubkey, &pubkeylen, 0, 0));
    CHECK(secp256k1_ecdsa_recover_compact(msg32, sig64, pubkey, &pubkeylen, 0, 1));
    CHECK(!secp256k1_ecdsa_recover_compact(msg32, sig64, pubkey, &pubkeylen, 0, 2));
    CHECK(!secp256k1_ecdsa_recover_compact(msg32, sig64, pubkey, &pubkeylen, 0, 3));

    for (recid = 0; recid < 4; recid++) {
        int i;
        int recid2;
        /* (4,4) encoded in DER. */
        unsigned char sigbder[8] = {0x30, 0x06, 0x02, 0x01, 0x04, 0x02, 0x01, 0x04};
        unsigned char sigcder_zr[7] = {0x30, 0x05, 0x02, 0x00, 0x02, 0x01, 0x01};
        unsigned char sigcder_zs[7] = {0x30, 0x05, 0x02, 0x01, 0x01, 0x02, 0x00};
        unsigned char sigbderalt1[39] = {
            0x30, 0x25, 0x02, 0x20, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x04, 0x02, 0x01, 0x04,
        };
        unsigned char sigbderalt2[39] = {
            0x30, 0x25, 0x02, 0x01, 0x04, 0x02, 0x20, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
        };
        unsigned char sigbderalt3[40] = {
            0x30, 0x26, 0x02, 0x21, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x01, 0x04,
        };
        unsigned char sigbderalt4[40] = {
            0x30, 0x26, 0x02, 0x01, 0x04, 0x02, 0x21, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
        };
        /* (order + r,4) encoded in DER. */
        unsigned char sigbderlong[40] = {
            0x30, 0x26, 0x02, 0x21, 0x00, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xBA, 0xAE, 0xDC,
            0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E,
            0x8C, 0xD0, 0x36, 0x41, 0x45, 0x02, 0x01, 0x04
        };
        CHECK(secp256k1_ecdsa_recover_compact(msg32, sigb64, pubkeyb, &pubkeyblen, 1, recid));
        CHECK(secp256k1_ecdsa_verify(msg32, sigbder, sizeof(sigbder), pubkeyb, pubkeyblen) == 1);
        for (recid2 = 0; recid2 < 4; recid2++) {
            unsigned char pubkey2b[33];
            int pubkey2blen = 33;
            CHECK(secp256k1_ecdsa_recover_compact(msg32, sigb64, pubkey2b, &pubkey2blen, 1, recid2));
            /* Verifying with (order + r,4) should always fail. */
            CHECK(secp256k1_ecdsa_verify(msg32, sigbderlong, sizeof(sigbderlong), pubkey2b, pubkey2blen) != 1);
        }
        /* DER parsing tests. */
        /* Zero length r/s. */
        CHECK(secp256k1_ecdsa_verify(msg32, sigcder_zr, sizeof(sigcder_zr), pubkeyb, pubkeyblen) == -2);
        CHECK(secp256k1_ecdsa_verify(msg32, sigcder_zs, sizeof(sigcder_zs), pubkeyb, pubkeyblen) == -2);
        /* Leading zeros. */
        CHECK(secp256k1_ecdsa_verify(msg32, sigbderalt1, sizeof(sigbderalt1), pubkeyb, pubkeyblen) == 1);
        CHECK(secp256k1_ecdsa_verify(msg32, sigbderalt2, sizeof(sigbderalt2), pubkeyb, pubkeyblen) == 1);
        CHECK(secp256k1_ecdsa_verify(msg32, sigbderalt3, sizeof(sigbderalt3), pubkeyb, pubkeyblen) == 1);
        CHECK(secp256k1_ecdsa_verify(msg32, sigbderalt4, sizeof(sigbderalt4), pubkeyb, pubkeyblen) == 1);
        sigbderalt3[4] = 1;
        CHECK(secp256k1_ecdsa_verify(msg32, sigbderalt3, sizeof(sigbderalt3), pubkeyb, pubkeyblen) == -2);
        sigbderalt4[7] = 1;
        CHECK(secp256k1_ecdsa_verify(msg32, sigbderalt4, sizeof(sigbderalt4), pubkeyb, pubkeyblen) == -2);
        /* Damage signature. */
        sigbder[7]++;
        CHECK(secp256k1_ecdsa_verify(msg32, sigbder, sizeof(sigbder), pubkeyb, pubkeyblen) == 0);
        sigbder[7]--;
        CHECK(secp256k1_ecdsa_verify(msg32, sigbder, 6, pubkeyb, pubkeyblen) == -2);
        CHECK(secp256k1_ecdsa_verify(msg32, sigbder, sizeof(sigbder)-1, pubkeyb, pubkeyblen) == -2);
        for(i = 0; i < 8; i++) {
            int c;
            unsigned char orig = sigbder[i];
            /*Try every single-byte change.*/
            for (c = 0; c < 256; c++) {
                if (c == orig ) continue;
                sigbder[i] = c;
                CHECK(secp256k1_ecdsa_verify(msg32, sigbder, sizeof(sigbder), pubkeyb, pubkeyblen) ==
                  (i==4 || i==7) ? 0 : -2 );
            }
            sigbder[i] = orig;
        }
    }

    /* Test the case where ECDSA recomputes a point that is infinity. */
    {
        secp256k1_gej_t keyj;
        secp256k1_ge_t key;
        secp256k1_scalar_t msg;
        secp256k1_ecdsa_sig_t sig;
        secp256k1_scalar_set_int(&sig.s, 1);
        secp256k1_scalar_negate(&sig.s, &sig.s);
        secp256k1_scalar_inverse(&sig.s, &sig.s);
        secp256k1_scalar_set_int(&sig.r, 1);
        secp256k1_ecmult_gen(&keyj, &sig.r);
        secp256k1_ge_set_gej(&key, &keyj);
        msg = sig.s;
        CHECK(secp256k1_ecdsa_sig_verify(&sig, &key, &msg) == 0);
    }

    /* Test r/s equal to zero */
    {
        /* (1,1) encoded in DER. */
        unsigned char sigcder[8] = {0x30, 0x06, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01};
        unsigned char sigc64[64] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        };
        unsigned char pubkeyc[65];
        int pubkeyclen = 65;
        CHECK(secp256k1_ecdsa_recover_compact(msg32, sigc64, pubkeyc, &pubkeyclen, 0, 0) == 1);
        CHECK(secp256k1_ecdsa_verify(msg32, sigcder, sizeof(sigcder), pubkeyc, pubkeyclen) == 1);
        sigcder[4] = 0;
        sigc64[31] = 0;
        CHECK(secp256k1_ecdsa_recover_compact(msg32, sigc64, pubkeyb, &pubkeyblen, 1, 0) == 0);
        CHECK(secp256k1_ecdsa_verify(msg32, sigcder, sizeof(sigcder), pubkeyc, pubkeyclen) == 0);
        sigcder[4] = 1;
        sigcder[7] = 0;
        sigc64[31] = 1;
        sigc64[63] = 0;
        CHECK(secp256k1_ecdsa_recover_compact(msg32, sigc64, pubkeyb, &pubkeyblen, 1, 0) == 0);
        CHECK(secp256k1_ecdsa_verify(msg32, sigcder, sizeof(sigcder), pubkeyc, pubkeyclen) == 0);
    }

    /*Signature where s would be zero.*/
    {
        const unsigned char nonce[32] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        };
        static const unsigned char nonce2[32] = {
            0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
            0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
            0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,
            0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x40
        };
        const unsigned char key[32] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        };
        unsigned char msg[32] = {
            0x86, 0x41, 0x99, 0x81, 0x06, 0x23, 0x44, 0x53,
            0xaa, 0x5f, 0x9d, 0x6a, 0x31, 0x78, 0xf4, 0xf7,
            0xb8, 0x12, 0xe0, 0x0b, 0x81, 0x7a, 0x77, 0x62,
            0x65, 0xdf, 0xdd, 0x31, 0xb9, 0x3e, 0x29, 0xa9,
        };
        unsigned char sig[72];
        int siglen = 72;
        CHECK(secp256k1_ecdsa_sign(msg, sig, &siglen, key, precomputed_nonce_function, nonce) == 0);
        CHECK(secp256k1_ecdsa_sign(msg, sig, &siglen, key, precomputed_nonce_function, nonce2) == 0);
        msg[31] = 0xaa;
        siglen = 72;
        CHECK(secp256k1_ecdsa_sign(msg, sig, &siglen, key, precomputed_nonce_function, nonce) == 1);
        CHECK(secp256k1_ecdsa_sign(msg, sig, &siglen, key, precomputed_nonce_function, nonce2) == 1);
        siglen = 10;
        CHECK(secp256k1_ecdsa_sign(msg, sig, &siglen, key, precomputed_nonce_function, nonce) != 1);
    }

    /* Nonce function corner cases. */
    {
        int i;
        unsigned char key[32];
        unsigned char msg[32];
        unsigned char sig[72];
        unsigned char sig2[72];
        secp256k1_ecdsa_sig_t s[512];
        int siglen = 72;
        int siglen2 = 72;
        int recid2;
        memset(key, 0, 32);
        memset(msg, 0, 32);
        key[31] = 1;
        msg[31] = 1;
        /* Nonce function failure results in signature failure. */
        CHECK(secp256k1_ecdsa_sign(msg, sig, &siglen, key, nonce_function_test_fail, NULL) == 0);
        CHECK(secp256k1_ecdsa_sign_compact(msg, sig, key, nonce_function_test_fail, NULL, &recid) == 0);
        /* The retry loop successfully makes its way to the first good value. */
        siglen = 72;
        CHECK(secp256k1_ecdsa_sign(msg, sig, &siglen, key, nonce_function_test_retry, NULL) == 1);
        CHECK(secp256k1_ecdsa_sign(msg, sig2, &siglen2, key, nonce_function_rfc6979, NULL) == 1);
        CHECK((siglen == siglen2) && (memcmp(sig, sig2, siglen) == 0));
        CHECK(secp256k1_ecdsa_sign_compact(msg, sig, key, nonce_function_test_retry, NULL, &recid) == 1);
        CHECK(secp256k1_ecdsa_sign_compact(msg, sig2, key, nonce_function_rfc6979, NULL, &recid2) == 1);
        CHECK((recid == recid2) && (memcmp(sig, sig2, 64) == 0));
        /* The default nonce function is determinstic. */
        siglen = 72;
        siglen2 = 72;
        CHECK(secp256k1_ecdsa_sign(msg, sig, &siglen, key, NULL, NULL) == 1);
        CHECK(secp256k1_ecdsa_sign(msg, sig2, &siglen2, key, NULL, NULL) == 1);
        CHECK((siglen == siglen2) && (memcmp(sig, sig2, siglen) == 0));
        CHECK(secp256k1_ecdsa_sign_compact(msg, sig, key, NULL, NULL, &recid) == 1);
        CHECK(secp256k1_ecdsa_sign_compact(msg, sig2, key, NULL, NULL, &recid2) == 1);
        CHECK((recid == recid2) && (memcmp(sig, sig2, 64) == 0));
        /* The default nonce function changes output with different messages. */
        for(i = 0; i < 256; i++) {
            int j;
            siglen2 = 72;
            msg[0] = i;
            CHECK(secp256k1_ecdsa_sign(msg, sig2, &siglen2, key, NULL, NULL) == 1);
            CHECK(secp256k1_ecdsa_sig_parse(&s[i], sig2, siglen2));
            for (j = 0; j < i; j++) {
                CHECK(!secp256k1_scalar_eq(&s[i].r, &s[j].r));
            }
        }
        msg[0] = 0;
        msg[31] = 2;
        /* The default nonce function changes output with different keys. */
        for(i = 256; i < 512; i++) {
            int j;
            siglen2 = 72;
            key[0] = i - 256;
            CHECK(secp256k1_ecdsa_sign(msg, sig2, &siglen2, key, NULL, NULL) == 1);
            CHECK(secp256k1_ecdsa_sig_parse(&s[i], sig2, siglen2));
            for (j = 0; j < i; j++) {
                CHECK(!secp256k1_scalar_eq(&s[i].r, &s[j].r));
            }
        }
        key[0] = 0;
    }

    /* Privkey export where pubkey is the point at infinity. */
    {
        unsigned char privkey[300];
        unsigned char seckey[32] = {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
            0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48, 0xa0, 0x3b,
            0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41,
        };
        int outlen = 300;
        CHECK(!secp256k1_ec_privkey_export(seckey, privkey, &outlen, 0));
        CHECK(!secp256k1_ec_privkey_export(seckey, privkey, &outlen, 1));
    }
}

void run_ecdsa_edge_cases(void) {
    test_ecdsa_edge_cases();
}

#ifdef ENABLE_OPENSSL_TESTS
EC_KEY *get_openssl_key(const secp256k1_scalar_t *key) {
    unsigned char privkey[300];
    int privkeylen;
    const unsigned char* pbegin = privkey;
    int compr = secp256k1_rand32() & 1;
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_secp256k1);
    CHECK(secp256k1_eckey_privkey_serialize(privkey, &privkeylen, key, compr));
    CHECK(d2i_ECPrivateKey(&ec_key, &pbegin, privkeylen));
    CHECK(EC_KEY_check_key(ec_key));
    return ec_key;
}

void test_ecdsa_openssl(void) {
    secp256k1_gej_t qj;
    secp256k1_ge_t q;
    secp256k1_ecdsa_sig_t sig;
    secp256k1_scalar_t one;
    secp256k1_scalar_t msg2;
    secp256k1_scalar_t key, msg;
    EC_KEY *ec_key;
    unsigned int sigsize = 80;
    int secp_sigsize = 80;
    unsigned char message[32];
    unsigned char signature[80];
    secp256k1_rand256_test(message);
    secp256k1_scalar_set_b32(&msg, message, NULL);
    random_scalar_order_test(&key);
    secp256k1_ecmult_gen(&qj, &key);
    secp256k1_ge_set_gej(&q, &qj);
    ec_key = get_openssl_key(&key);
    CHECK(ec_key);
    CHECK(ECDSA_sign(0, message, sizeof(message), signature, &sigsize, ec_key));
    CHECK(secp256k1_ecdsa_sig_parse(&sig, signature, sigsize));
    CHECK(secp256k1_ecdsa_sig_verify(&sig, &q, &msg));
    secp256k1_scalar_set_int(&one, 1);
    secp256k1_scalar_add(&msg2, &msg, &one);
    CHECK(!secp256k1_ecdsa_sig_verify(&sig, &q, &msg2));

    random_sign(&sig, &key, &msg, NULL);
    CHECK(secp256k1_ecdsa_sig_serialize(signature, &secp_sigsize, &sig));
    CHECK(ECDSA_verify(0, message, sizeof(message), signature, secp_sigsize, ec_key) == 1);

    EC_KEY_free(ec_key);
}

void run_ecdsa_openssl(void) {
    int i;
    for (i = 0; i < 10*count; i++) {
        test_ecdsa_openssl();
    }
}
#endif

int main(int argc, char **argv) {
    uint64_t seed;
    /* find iteration count */
    if (argc > 1) {
        count = strtol(argv[1], NULL, 0);
    }

    /* find random seed */
    if (argc > 2) {
        sscanf(argv[2], "%" I64uFORMAT, (unsigned long long*)&seed);
    } else {
        FILE *frand = fopen("/dev/urandom", "r");
        if (!frand || !fread(&seed, sizeof(seed), 1, frand)) {
            seed = time(NULL) * 1337;
        }
        fclose(frand);
    }
    secp256k1_rand_seed(seed);

    printf("test count = %i\n", count);
    printf("random seed = %" I64uFORMAT "\n", (unsigned long long)seed);

    /* initialize */
    secp256k1_start(SECP256K1_START_SIGN | SECP256K1_START_VERIFY);

    /* initializing a second time shouldn't cause any harm or memory leaks. */
    secp256k1_start(SECP256K1_START_SIGN | SECP256K1_START_VERIFY);

    run_sha256_tests();
    run_hmac_sha256_tests();
    run_rfc6979_hmac_sha256_tests();

#ifndef USE_NUM_NONE
    /* num tests */
    run_num_smalltests();
#endif

    /* scalar tests */
    run_scalar_tests();

    /* field tests */
    run_field_inv();
    run_field_inv_var();
    run_field_inv_all_var();
    run_field_misc();
    run_field_convert();
    run_sqr();
    run_sqrt();

    /* group tests */
    run_ge();

    /* ecmult tests */
    run_wnaf();
    run_point_times_order();
    run_ecmult_chain();

    /* ecdsa tests */
    run_random_pubkeys();
    run_ecdsa_sign_verify();
    run_ecdsa_end_to_end();
    run_ecdsa_edge_cases();
#ifdef ENABLE_OPENSSL_TESTS
    run_ecdsa_openssl();
#endif

    printf("random run = %llu\n", (unsigned long long)secp256k1_rand32() + ((unsigned long long)secp256k1_rand32() << 32));

    /* shutdown */
    secp256k1_stop();

    /* shutting down twice shouldn't cause any double frees. */
    secp256k1_stop();
    return 0;
}
