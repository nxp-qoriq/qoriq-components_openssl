/*
 * Copyright 2015-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * TODO(3.0): Really these tests should be in evp_extra_test - but that doesn't
 * yet support testing with a non-default libctx. Once it does we should move
 * everything into one file. Consequently some things are duplicated between
 * the two files.
 */

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/provider.h>
#include "testutil.h"
#include "internal/nelem.h"

static OSSL_LIB_CTX *mainctx = NULL;
static OSSL_PROVIDER *nullprov = NULL;

/*
 * kExampleRSAKeyDER is an RSA private key in ASN.1, DER format. Of course, you
 * should never use this key anywhere but in an example.
 */
static const unsigned char kExampleRSAKeyDER[] = {
    0x30, 0x82, 0x02, 0x5c, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xf8,
    0xb8, 0x6c, 0x83, 0xb4, 0xbc, 0xd9, 0xa8, 0x57, 0xc0, 0xa5, 0xb4, 0x59,
    0x76, 0x8c, 0x54, 0x1d, 0x79, 0xeb, 0x22, 0x52, 0x04, 0x7e, 0xd3, 0x37,
    0xeb, 0x41, 0xfd, 0x83, 0xf9, 0xf0, 0xa6, 0x85, 0x15, 0x34, 0x75, 0x71,
    0x5a, 0x84, 0xa8, 0x3c, 0xd2, 0xef, 0x5a, 0x4e, 0xd3, 0xde, 0x97, 0x8a,
    0xdd, 0xff, 0xbb, 0xcf, 0x0a, 0xaa, 0x86, 0x92, 0xbe, 0xb8, 0x50, 0xe4,
    0xcd, 0x6f, 0x80, 0x33, 0x30, 0x76, 0x13, 0x8f, 0xca, 0x7b, 0xdc, 0xec,
    0x5a, 0xca, 0x63, 0xc7, 0x03, 0x25, 0xef, 0xa8, 0x8a, 0x83, 0x58, 0x76,
    0x20, 0xfa, 0x16, 0x77, 0xd7, 0x79, 0x92, 0x63, 0x01, 0x48, 0x1a, 0xd8,
    0x7b, 0x67, 0xf1, 0x52, 0x55, 0x49, 0x4e, 0xd6, 0x6e, 0x4a, 0x5c, 0xd7,
    0x7a, 0x37, 0x36, 0x0c, 0xde, 0xdd, 0x8f, 0x44, 0xe8, 0xc2, 0xa7, 0x2c,
    0x2b, 0xb5, 0xaf, 0x64, 0x4b, 0x61, 0x07, 0x02, 0x03, 0x01, 0x00, 0x01,
    0x02, 0x81, 0x80, 0x74, 0x88, 0x64, 0x3f, 0x69, 0x45, 0x3a, 0x6d, 0xc7,
    0x7f, 0xb9, 0xa3, 0xc0, 0x6e, 0xec, 0xdc, 0xd4, 0x5a, 0xb5, 0x32, 0x85,
    0x5f, 0x19, 0xd4, 0xf8, 0xd4, 0x3f, 0x3c, 0xfa, 0xc2, 0xf6, 0x5f, 0xee,
    0xe6, 0xba, 0x87, 0x74, 0x2e, 0xc7, 0x0c, 0xd4, 0x42, 0xb8, 0x66, 0x85,
    0x9c, 0x7b, 0x24, 0x61, 0xaa, 0x16, 0x11, 0xf6, 0xb5, 0xb6, 0xa4, 0x0a,
    0xc9, 0x55, 0x2e, 0x81, 0xa5, 0x47, 0x61, 0xcb, 0x25, 0x8f, 0xc2, 0x15,
    0x7b, 0x0e, 0x7c, 0x36, 0x9f, 0x3a, 0xda, 0x58, 0x86, 0x1c, 0x5b, 0x83,
    0x79, 0xe6, 0x2b, 0xcc, 0xe6, 0xfa, 0x2c, 0x61, 0xf2, 0x78, 0x80, 0x1b,
    0xe2, 0xf3, 0x9d, 0x39, 0x2b, 0x65, 0x57, 0x91, 0x3d, 0x71, 0x99, 0x73,
    0xa5, 0xc2, 0x79, 0x20, 0x8c, 0x07, 0x4f, 0xe5, 0xb4, 0x60, 0x1f, 0x99,
    0xa2, 0xb1, 0x4f, 0x0c, 0xef, 0xbc, 0x59, 0x53, 0x00, 0x7d, 0xb1, 0x02,
    0x41, 0x00, 0xfc, 0x7e, 0x23, 0x65, 0x70, 0xf8, 0xce, 0xd3, 0x40, 0x41,
    0x80, 0x6a, 0x1d, 0x01, 0xd6, 0x01, 0xff, 0xb6, 0x1b, 0x3d, 0x3d, 0x59,
    0x09, 0x33, 0x79, 0xc0, 0x4f, 0xde, 0x96, 0x27, 0x4b, 0x18, 0xc6, 0xd9,
    0x78, 0xf1, 0xf4, 0x35, 0x46, 0xe9, 0x7c, 0x42, 0x7a, 0x5d, 0x9f, 0xef,
    0x54, 0xb8, 0xf7, 0x9f, 0xc4, 0x33, 0x6c, 0xf3, 0x8c, 0x32, 0x46, 0x87,
    0x67, 0x30, 0x7b, 0xa7, 0xac, 0xe3, 0x02, 0x41, 0x00, 0xfc, 0x2c, 0xdf,
    0x0c, 0x0d, 0x88, 0xf5, 0xb1, 0x92, 0xa8, 0x93, 0x47, 0x63, 0x55, 0xf5,
    0xca, 0x58, 0x43, 0xba, 0x1c, 0xe5, 0x9e, 0xb6, 0x95, 0x05, 0xcd, 0xb5,
    0x82, 0xdf, 0xeb, 0x04, 0x53, 0x9d, 0xbd, 0xc2, 0x38, 0x16, 0xb3, 0x62,
    0xdd, 0xa1, 0x46, 0xdb, 0x6d, 0x97, 0x93, 0x9f, 0x8a, 0xc3, 0x9b, 0x64,
    0x7e, 0x42, 0xe3, 0x32, 0x57, 0x19, 0x1b, 0xd5, 0x6e, 0x85, 0xfa, 0xb8,
    0x8d, 0x02, 0x41, 0x00, 0xbc, 0x3d, 0xde, 0x6d, 0xd6, 0x97, 0xe8, 0xba,
    0x9e, 0x81, 0x37, 0x17, 0xe5, 0xa0, 0x64, 0xc9, 0x00, 0xb7, 0xe7, 0xfe,
    0xf4, 0x29, 0xd9, 0x2e, 0x43, 0x6b, 0x19, 0x20, 0xbd, 0x99, 0x75, 0xe7,
    0x76, 0xf8, 0xd3, 0xae, 0xaf, 0x7e, 0xb8, 0xeb, 0x81, 0xf4, 0x9d, 0xfe,
    0x07, 0x2b, 0x0b, 0x63, 0x0b, 0x5a, 0x55, 0x90, 0x71, 0x7d, 0xf1, 0xdb,
    0xd9, 0xb1, 0x41, 0x41, 0x68, 0x2f, 0x4e, 0x39, 0x02, 0x40, 0x5a, 0x34,
    0x66, 0xd8, 0xf5, 0xe2, 0x7f, 0x18, 0xb5, 0x00, 0x6e, 0x26, 0x84, 0x27,
    0x14, 0x93, 0xfb, 0xfc, 0xc6, 0x0f, 0x5e, 0x27, 0xe6, 0xe1, 0xe9, 0xc0,
    0x8a, 0xe4, 0x34, 0xda, 0xe9, 0xa2, 0x4b, 0x73, 0xbc, 0x8c, 0xb9, 0xba,
    0x13, 0x6c, 0x7a, 0x2b, 0x51, 0x84, 0xa3, 0x4a, 0xe0, 0x30, 0x10, 0x06,
    0x7e, 0xed, 0x17, 0x5a, 0x14, 0x00, 0xc9, 0xef, 0x85, 0xea, 0x52, 0x2c,
    0xbc, 0x65, 0x02, 0x40, 0x51, 0xe3, 0xf2, 0x83, 0x19, 0x9b, 0xc4, 0x1e,
    0x2f, 0x50, 0x3d, 0xdf, 0x5a, 0xa2, 0x18, 0xca, 0x5f, 0x2e, 0x49, 0xaf,
    0x6f, 0xcc, 0xfa, 0x65, 0x77, 0x94, 0xb5, 0xa1, 0x0a, 0xa9, 0xd1, 0x8a,
    0x39, 0x37, 0xf4, 0x0b, 0xa0, 0xd7, 0x82, 0x27, 0x5e, 0xae, 0x17, 0x17,
    0xa1, 0x1e, 0x54, 0x34, 0xbf, 0x6e, 0xc4, 0x8e, 0x99, 0x5d, 0x08, 0xf1,
    0x2d, 0x86, 0x9d, 0xa5, 0x20, 0x1b, 0xe5, 0xdf,
};

/*
 * kExampleRSAKeyPKCS8 is kExampleRSAKeyDER encoded in a PKCS #8
 * PrivateKeyInfo.
 */
static const unsigned char kExampleRSAKeyPKCS8[] = {
    0x30, 0x82, 0x02, 0x76, 0x02, 0x01, 0x00, 0x30, 0x0d, 0x06, 0x09, 0x2a,
    0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x82,
    0x02, 0x60, 0x30, 0x82, 0x02, 0x5c, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81,
    0x00, 0xf8, 0xb8, 0x6c, 0x83, 0xb4, 0xbc, 0xd9, 0xa8, 0x57, 0xc0, 0xa5,
    0xb4, 0x59, 0x76, 0x8c, 0x54, 0x1d, 0x79, 0xeb, 0x22, 0x52, 0x04, 0x7e,
    0xd3, 0x37, 0xeb, 0x41, 0xfd, 0x83, 0xf9, 0xf0, 0xa6, 0x85, 0x15, 0x34,
    0x75, 0x71, 0x5a, 0x84, 0xa8, 0x3c, 0xd2, 0xef, 0x5a, 0x4e, 0xd3, 0xde,
    0x97, 0x8a, 0xdd, 0xff, 0xbb, 0xcf, 0x0a, 0xaa, 0x86, 0x92, 0xbe, 0xb8,
    0x50, 0xe4, 0xcd, 0x6f, 0x80, 0x33, 0x30, 0x76, 0x13, 0x8f, 0xca, 0x7b,
    0xdc, 0xec, 0x5a, 0xca, 0x63, 0xc7, 0x03, 0x25, 0xef, 0xa8, 0x8a, 0x83,
    0x58, 0x76, 0x20, 0xfa, 0x16, 0x77, 0xd7, 0x79, 0x92, 0x63, 0x01, 0x48,
    0x1a, 0xd8, 0x7b, 0x67, 0xf1, 0x52, 0x55, 0x49, 0x4e, 0xd6, 0x6e, 0x4a,
    0x5c, 0xd7, 0x7a, 0x37, 0x36, 0x0c, 0xde, 0xdd, 0x8f, 0x44, 0xe8, 0xc2,
    0xa7, 0x2c, 0x2b, 0xb5, 0xaf, 0x64, 0x4b, 0x61, 0x07, 0x02, 0x03, 0x01,
    0x00, 0x01, 0x02, 0x81, 0x80, 0x74, 0x88, 0x64, 0x3f, 0x69, 0x45, 0x3a,
    0x6d, 0xc7, 0x7f, 0xb9, 0xa3, 0xc0, 0x6e, 0xec, 0xdc, 0xd4, 0x5a, 0xb5,
    0x32, 0x85, 0x5f, 0x19, 0xd4, 0xf8, 0xd4, 0x3f, 0x3c, 0xfa, 0xc2, 0xf6,
    0x5f, 0xee, 0xe6, 0xba, 0x87, 0x74, 0x2e, 0xc7, 0x0c, 0xd4, 0x42, 0xb8,
    0x66, 0x85, 0x9c, 0x7b, 0x24, 0x61, 0xaa, 0x16, 0x11, 0xf6, 0xb5, 0xb6,
    0xa4, 0x0a, 0xc9, 0x55, 0x2e, 0x81, 0xa5, 0x47, 0x61, 0xcb, 0x25, 0x8f,
    0xc2, 0x15, 0x7b, 0x0e, 0x7c, 0x36, 0x9f, 0x3a, 0xda, 0x58, 0x86, 0x1c,
    0x5b, 0x83, 0x79, 0xe6, 0x2b, 0xcc, 0xe6, 0xfa, 0x2c, 0x61, 0xf2, 0x78,
    0x80, 0x1b, 0xe2, 0xf3, 0x9d, 0x39, 0x2b, 0x65, 0x57, 0x91, 0x3d, 0x71,
    0x99, 0x73, 0xa5, 0xc2, 0x79, 0x20, 0x8c, 0x07, 0x4f, 0xe5, 0xb4, 0x60,
    0x1f, 0x99, 0xa2, 0xb1, 0x4f, 0x0c, 0xef, 0xbc, 0x59, 0x53, 0x00, 0x7d,
    0xb1, 0x02, 0x41, 0x00, 0xfc, 0x7e, 0x23, 0x65, 0x70, 0xf8, 0xce, 0xd3,
    0x40, 0x41, 0x80, 0x6a, 0x1d, 0x01, 0xd6, 0x01, 0xff, 0xb6, 0x1b, 0x3d,
    0x3d, 0x59, 0x09, 0x33, 0x79, 0xc0, 0x4f, 0xde, 0x96, 0x27, 0x4b, 0x18,
    0xc6, 0xd9, 0x78, 0xf1, 0xf4, 0x35, 0x46, 0xe9, 0x7c, 0x42, 0x7a, 0x5d,
    0x9f, 0xef, 0x54, 0xb8, 0xf7, 0x9f, 0xc4, 0x33, 0x6c, 0xf3, 0x8c, 0x32,
    0x46, 0x87, 0x67, 0x30, 0x7b, 0xa7, 0xac, 0xe3, 0x02, 0x41, 0x00, 0xfc,
    0x2c, 0xdf, 0x0c, 0x0d, 0x88, 0xf5, 0xb1, 0x92, 0xa8, 0x93, 0x47, 0x63,
    0x55, 0xf5, 0xca, 0x58, 0x43, 0xba, 0x1c, 0xe5, 0x9e, 0xb6, 0x95, 0x05,
    0xcd, 0xb5, 0x82, 0xdf, 0xeb, 0x04, 0x53, 0x9d, 0xbd, 0xc2, 0x38, 0x16,
    0xb3, 0x62, 0xdd, 0xa1, 0x46, 0xdb, 0x6d, 0x97, 0x93, 0x9f, 0x8a, 0xc3,
    0x9b, 0x64, 0x7e, 0x42, 0xe3, 0x32, 0x57, 0x19, 0x1b, 0xd5, 0x6e, 0x85,
    0xfa, 0xb8, 0x8d, 0x02, 0x41, 0x00, 0xbc, 0x3d, 0xde, 0x6d, 0xd6, 0x97,
    0xe8, 0xba, 0x9e, 0x81, 0x37, 0x17, 0xe5, 0xa0, 0x64, 0xc9, 0x00, 0xb7,
    0xe7, 0xfe, 0xf4, 0x29, 0xd9, 0x2e, 0x43, 0x6b, 0x19, 0x20, 0xbd, 0x99,
    0x75, 0xe7, 0x76, 0xf8, 0xd3, 0xae, 0xaf, 0x7e, 0xb8, 0xeb, 0x81, 0xf4,
    0x9d, 0xfe, 0x07, 0x2b, 0x0b, 0x63, 0x0b, 0x5a, 0x55, 0x90, 0x71, 0x7d,
    0xf1, 0xdb, 0xd9, 0xb1, 0x41, 0x41, 0x68, 0x2f, 0x4e, 0x39, 0x02, 0x40,
    0x5a, 0x34, 0x66, 0xd8, 0xf5, 0xe2, 0x7f, 0x18, 0xb5, 0x00, 0x6e, 0x26,
    0x84, 0x27, 0x14, 0x93, 0xfb, 0xfc, 0xc6, 0x0f, 0x5e, 0x27, 0xe6, 0xe1,
    0xe9, 0xc0, 0x8a, 0xe4, 0x34, 0xda, 0xe9, 0xa2, 0x4b, 0x73, 0xbc, 0x8c,
    0xb9, 0xba, 0x13, 0x6c, 0x7a, 0x2b, 0x51, 0x84, 0xa3, 0x4a, 0xe0, 0x30,
    0x10, 0x06, 0x7e, 0xed, 0x17, 0x5a, 0x14, 0x00, 0xc9, 0xef, 0x85, 0xea,
    0x52, 0x2c, 0xbc, 0x65, 0x02, 0x40, 0x51, 0xe3, 0xf2, 0x83, 0x19, 0x9b,
    0xc4, 0x1e, 0x2f, 0x50, 0x3d, 0xdf, 0x5a, 0xa2, 0x18, 0xca, 0x5f, 0x2e,
    0x49, 0xaf, 0x6f, 0xcc, 0xfa, 0x65, 0x77, 0x94, 0xb5, 0xa1, 0x0a, 0xa9,
    0xd1, 0x8a, 0x39, 0x37, 0xf4, 0x0b, 0xa0, 0xd7, 0x82, 0x27, 0x5e, 0xae,
    0x17, 0x17, 0xa1, 0x1e, 0x54, 0x34, 0xbf, 0x6e, 0xc4, 0x8e, 0x99, 0x5d,
    0x08, 0xf1, 0x2d, 0x86, 0x9d, 0xa5, 0x20, 0x1b, 0xe5, 0xdf,
};

#ifndef OPENSSL_NO_EC
/*
 * kExampleECKeyDER is a sample EC private key encoded as an ECPrivateKey
 * structure.
 */
static const unsigned char kExampleECKeyDER[] = {
    0x30, 0x77, 0x02, 0x01, 0x01, 0x04, 0x20, 0x07, 0x0f, 0x08, 0x72, 0x7a,
    0xd4, 0xa0, 0x4a, 0x9c, 0xdd, 0x59, 0xc9, 0x4d, 0x89, 0x68, 0x77, 0x08,
    0xb5, 0x6f, 0xc9, 0x5d, 0x30, 0x77, 0x0e, 0xe8, 0xd1, 0xc9, 0xce, 0x0a,
    0x8b, 0xb4, 0x6a, 0xa0, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x03, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0xe6, 0x2b, 0x69,
    0xe2, 0xbf, 0x65, 0x9f, 0x97, 0xbe, 0x2f, 0x1e, 0x0d, 0x94, 0x8a, 0x4c,
    0xd5, 0x97, 0x6b, 0xb7, 0xa9, 0x1e, 0x0d, 0x46, 0xfb, 0xdd, 0xa9, 0xa9,
    0x1e, 0x9d, 0xdc, 0xba, 0x5a, 0x01, 0xe7, 0xd6, 0x97, 0xa8, 0x0a, 0x18,
    0xf9, 0xc3, 0xc4, 0xa3, 0x1e, 0x56, 0xe2, 0x7c, 0x83, 0x48, 0xdb, 0x16,
    0x1a, 0x1c, 0xf5, 0x1d, 0x7e, 0xf1, 0x94, 0x2d, 0x4b, 0xcf, 0x72, 0x22,
    0xc1,
};

/* P-384 sample EC private key in PKCS8 format (no public key) */
static const unsigned char kExampleECKey2DER[] = {
    0x30, 0x4E, 0x02, 0x01, 0x00, 0x30, 0x10, 0x06, 0x07, 0x2A, 0x86, 0x48,
    0xCE, 0x3D, 0x02, 0x01, 0x06, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x22, 0x04,
    0x37, 0x30, 0x35, 0x02, 0x01, 0x01, 0x04, 0x30, 0x73, 0xE3, 0x3A, 0x05,
    0xF2, 0xB6, 0x99, 0x6D, 0x0C, 0x33, 0x7F, 0x15, 0x9E, 0x10, 0xA9, 0x17,
    0x4C, 0x0A, 0x82, 0x57, 0x71, 0x13, 0x7A, 0xAC, 0x46, 0xA2, 0x5E, 0x1C,
    0xE0, 0xC7, 0xB2, 0xF8, 0x20, 0x40, 0xC2, 0x27, 0xC8, 0xBE, 0x02, 0x7E,
    0x96, 0x69, 0xE0, 0x04, 0xCB, 0x89, 0x0B, 0x42
};
#endif

typedef struct APK_DATA_st {
    const unsigned char *kder;
    size_t size;
    int evptype;
} APK_DATA;

static APK_DATA keydata[] = {
    {kExampleRSAKeyDER, sizeof(kExampleRSAKeyDER), EVP_PKEY_RSA},
    {kExampleRSAKeyPKCS8, sizeof(kExampleRSAKeyPKCS8), EVP_PKEY_RSA},
#ifndef OPENSSL_NO_EC
    {kExampleECKeyDER, sizeof(kExampleECKeyDER), EVP_PKEY_EC},
    {kExampleECKey2DER, sizeof(kExampleECKey2DER), EVP_PKEY_EC}
#endif
};

/* This is the equivalent of test_d2i_AutoPrivateKey in evp_extra_test */
static int test_d2i_AutoPrivateKey_ex(int i)
{
    int ret = 0;
    const unsigned char *p;
    EVP_PKEY *pkey = NULL;
    const APK_DATA *ak = &keydata[i];
    const unsigned char *input = ak->kder;
    size_t input_len = ak->size;
    int expected_id = ak->evptype;

    p = input;
    if (!TEST_ptr(pkey = d2i_AutoPrivateKey_ex(NULL, &p, input_len, mainctx,
                                               NULL))
            || !TEST_ptr_eq(p, input + input_len)
            || !TEST_int_eq(EVP_PKEY_id(pkey), expected_id))
        goto done;

    ret = 1;

 done:
    EVP_PKEY_free(pkey);
    return ret;
}

static int test_alternative_default(void)
{
    OSSL_LIB_CTX *oldctx;
    EVP_MD *sha256;
    int ok = 0;

    /*
     * setup_tests() loaded the "null" provider in the current default, so
     * we know this fetch should fail.
     */
    if (!TEST_ptr_null(sha256 = EVP_MD_fetch(NULL, "SHA2-256", NULL)))
        goto err;

    /*
     * Now we switch to our main library context, and try again.  Since no
     * providers are loaded in this one, it should fall back to the default.
     */
    if (!TEST_ptr(oldctx = OSSL_LIB_CTX_set0_default(mainctx))
        || !TEST_ptr(sha256 = EVP_MD_fetch(NULL, "SHA2-256", NULL)))
        goto err;
    EVP_MD_free(sha256);
    sha256 = NULL;

    /*
     * Switching back should give us our main library context back, and
     * fetching SHA2-256 should fail again.
     */
    if (!TEST_ptr_eq(OSSL_LIB_CTX_set0_default(oldctx), mainctx)
        || !TEST_ptr_null(sha256 = EVP_MD_fetch(NULL, "SHA2-256", NULL)))
        goto err;

    ok = 1;
 err:
    EVP_MD_free(sha256);
    return ok;
}

static int test_d2i_PrivateKey_ex(void) {
    int ok;
    OSSL_PROVIDER *provider;
    BIO *key_bio;
    EVP_PKEY* pkey;
    ok = 0;

    provider = OSSL_PROVIDER_load(NULL, "default");
    key_bio = BIO_new_mem_buf((&keydata[0])->kder, (&keydata)[0]->size);

    ok = TEST_ptr(pkey = PEM_read_bio_PrivateKey(key_bio, NULL, NULL, NULL));
    TEST_int_eq(ERR_peek_error(), 0);
    test_openssl_errors();

    EVP_PKEY_free(pkey);
    BIO_free(key_bio);
    OSSL_PROVIDER_unload(provider);

    return ok;
}

int setup_tests(void)
{
    if (!test_get_libctx(&mainctx, &nullprov, NULL, NULL, NULL)) {
        OSSL_LIB_CTX_free(mainctx);
        mainctx = NULL;
        return 0;
    }

    ADD_TEST(test_alternative_default);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey_ex, OSSL_NELEM(keydata));
    ADD_TEST(test_d2i_PrivateKey_ex);

    return 1;
}

void cleanup_tests(void)
{
    OSSL_LIB_CTX_free(mainctx);
    OSSL_PROVIDER_unload(nullprov);
}