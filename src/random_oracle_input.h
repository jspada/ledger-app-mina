#pragma once

#include "crypto.h"
#include "utils.h"

#define FIELD_BITS 255

struct roinput_t {
    Field   *fields;
    uint8_t *bits;
    size_t  fields_len;      // in bytes
    size_t  fields_capacity; // in bytes
    size_t  bits_len;        // in bits
    size_t  bits_capacity;   // in bytes
};

#define roinput_create(fs, bs) { \
    .fields = fs, \
    .fields_capacity = ARRAY_LEN(fs), \
    .bits = bs, \
    .bits_capacity = ARRAY_LEN(bs) \
}

void roinput_add_field(ROInput *input, const Field a);
void roinput_add_scalar(ROInput *input, const Scalar a);
void roinput_add_bit(ROInput *input, const bool b);
void roinput_add_bytes(ROInput *input, const uint8_t *bytes, size_t len);
void roinput_add_uint32(ROInput *input, const uint32_t x);
void roinput_add_uint64(ROInput *input, const uint64_t x);
int roinput_derive_message(uint8_t *out, const size_t len, const Keypair *kp, const ROInput *msg, const uint8_t network_id);
int roinput_hash_message(Field *out, const size_t len, const Affine *pub, const Field rx, const ROInput *msg);
