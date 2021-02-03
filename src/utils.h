#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "crypto.h"
#include "random_oracle_input.h"

#define ARRAY_LEN(array) sizeof(array)/sizeof(array[0])

uint32_t read_uint32_be(const uint8_t *buffer);
uint64_t read_uint64_be(const uint8_t *buffer);

char *amount_to_string(char *buf, const size_t len, uint64_t amount);
char *value_to_string(char *buf, const size_t len, uint64_t value);

int b58_encode(const unsigned char *in, unsigned char length, unsigned char *out,
               const unsigned char maxoutlen);
bool b58_decode(void *bin, size_t *binszp, const char *b58, size_t b58sz);

void packed_bit_array_set(uint8_t *bits, const size_t i, const bool b);
bool packed_bit_array_get(const uint8_t *bits, const size_t i);

void read_public_key_compressed(Compressed* out, const char *pubkeyBase58);
