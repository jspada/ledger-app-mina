#pragma once

#include <stdlib.h>
#include <stdint.h>

uint32_t readUint32BE(uint8_t *buffer);
uint64_t readUint64BE(uint8_t *buffer);

char *amountToString(char *buf, size_t len, uint64_t amount);
char *valueToString(char *buf, size_t len, uint64_t value);

int encodeBase58(unsigned char *in, unsigned char length,
                 unsigned char *out, unsigned char maxoutlen);
