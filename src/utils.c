#include <stdbool.h>
#include <memory.h>
#include <stdio.h> // REMOVE

#include "utils.h"
#include "crypto.h"

#ifdef LEDGER_BUILD
    #include <os.h>
#endif

static const char B58_ALPHABET[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

// >= 0 : OK
// -2   : EXCEPTION_OVERFLOW
// -1   : INVALID_PARAMETER
int b58_encode(const unsigned char *in, unsigned char length,
               unsigned char *out, const unsigned char maxoutlen)
{
    unsigned char tmp[164];
    unsigned char buffer[164];
    unsigned char j;
    unsigned char startAt;
    unsigned char zeroCount = 0;
    if (length > sizeof(tmp)) {
        // Input buffer too big
        return -1;
    }
    memcpy(tmp, in, length);
    while ((zeroCount < length) && (tmp[zeroCount] == 0)) {
        ++zeroCount;
    }
    j = 2 * length;
    startAt = zeroCount;
    while (startAt < length) {
        unsigned short remainder = 0;
        unsigned char divLoop;
        for (divLoop = startAt; divLoop < length; divLoop++) {
            unsigned short digit256 = (unsigned short)(tmp[divLoop] & 0xff);
            unsigned short tmpDiv = remainder * 256 + digit256;
            tmp[divLoop] = (unsigned char)(tmpDiv / 58);
            remainder = (tmpDiv % 58);
        }
        if (tmp[startAt] == 0) {
            ++startAt;
        }
        buffer[--j] = (unsigned char)B58_ALPHABET[remainder];
    }
    while ((j < (2 * length)) && (buffer[j] == B58_ALPHABET[0])) {
        ++j;
    }
    while (zeroCount-- > 0) {
        buffer[--j] = B58_ALPHABET[0];
    }
    length = 2 * length - j;
    if (maxoutlen < length) {
        // Output buffer too small
        return -1;
    }
    memcpy(out, (buffer + j), length);
    return length;
}

/*
 * Copyright 2012-2014 Luke Dashjr
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the standard MIT license.  See COPYING for more details.
 */
static const int8_t B58_DIGITS_MAP[] = {
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
	-1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
	22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
	-1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
	47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
};
typedef uint64_t b58_maxint_t;
typedef uint32_t b58_almostmaxint_t;
#define b58_almostmaxint_bits (sizeof(b58_almostmaxint_t) * 8)
static const b58_almostmaxint_t b58_almostmaxint_mask = ((((b58_maxint_t)1) << b58_almostmaxint_bits) - 1);
bool b58_decode(void *bin, size_t *binszp, const char *b58, size_t b58sz)
{
	size_t binsz = *binszp;
	const unsigned char *b58u = (void*)b58;
	unsigned char *binu = bin;
	size_t outisz = (binsz + sizeof(b58_almostmaxint_t) - 1) / sizeof(b58_almostmaxint_t);
	b58_almostmaxint_t outi[outisz];
	b58_maxint_t t;
	b58_almostmaxint_t c;
	size_t i, j;
	uint8_t bytesleft = binsz % sizeof(b58_almostmaxint_t);
	b58_almostmaxint_t zeromask = bytesleft ? (b58_almostmaxint_mask << (bytesleft * 8)) : 0;
	unsigned zerocount = 0;

	if (!b58sz)
		b58sz = strlen(b58);

	for (i = 0; i < outisz; ++i) {
		outi[i] = 0;
	}

	// Leading zeros, just count
	for (i = 0; i < b58sz && b58u[i] == '1'; ++i)
		++zerocount;

	for ( ; i < b58sz; ++i)	{
		if (b58u[i] & 0x80)
			// High-bit set on invalid digit
			return false;
		if (B58_DIGITS_MAP[b58u[i]] == -1)
			// Invalid base58 digit
			return false;
		c = (unsigned)B58_DIGITS_MAP[b58u[i]];
		for (j = outisz; j--; ) {
			t = ((b58_maxint_t)outi[j]) * 58 + c;
			c = t >> b58_almostmaxint_bits;
			outi[j] = t & b58_almostmaxint_mask;
		}
		if (c)
			// Output number too big (carry to the next int32)
			return false;
		if (outi[0] & zeromask)
			// Output number too big (last int32 filled too far)
			return false;
	}

	j = 0;
	if (bytesleft) {
		for (i = bytesleft; i > 0; --i) {
			*(binu++) = (outi[0] >> (8 * (i - 1))) & 0xff;
		}
		++j;
	}

	for (; j < outisz; ++j)	{
		for (i = sizeof(*outi); i > 0; --i) {
			*(binu++) = (outi[j] >> (8 * (i - 1))) & 0xff;
		}
	}

	// Count canonical base58 byte count
	binu = bin;
	for (i = 0; i < binsz; ++i)	{
		if (binu[i])
			break;
		--*binszp;
	}
	*binszp += zerocount;

	return true;
}

uint32_t read_uint32_be(const uint8_t *buffer)
{
  return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3]);
}

uint64_t read_uint64_be(const uint8_t *buffer)
{
    return ((uint64_t)buffer[0] << 56) | ((uint64_t)buffer[1] << 48) |
            ((uint64_t)buffer[2] << 40) | ((uint64_t)buffer[3] << 32) |
           ((uint64_t)buffer[4] << 24) | ((uint64_t)buffer[5] << 16) |
            ((uint64_t)buffer[6] << 8) | ((uint64_t)buffer[7]);
}

char *amount_to_string(char *buf, const size_t len, uint64_t amount)
{
    // COIN = 1.000 000 000;
    size_t mantissa_len = 1;
    for (uint64_t value = amount, len = 9; value && len > 0; value /= 10, len--) {
        if (value % 10 != 0) {
            mantissa_len = len;
            break;
        }
    }

    // don't use log10() on ledger
    size_t characteristic_len = 0;
    for (uint64_t value = amount/COIN; value; value /= 10) {
        characteristic_len++;
    }
    characteristic_len = characteristic_len ? characteristic_len : 1;
    size_t total_len = characteristic_len + 1 + mantissa_len + 1;
    if (total_len > len) {
        *buf = '\0';
        return buf;
    }

    char *end = buf + total_len - 1;
    *end = '\0';

    int show_digit = 0;
    for (size_t i = 0; amount || i < 11; i++) {
        uint8_t digit = amount % 10;

        if (i == 9) {
            *(--end) = '.';
        }
        else {
            if (i > 7 || (digit != 0 && i < 8) || show_digit) {
                *(--end) = '0' + digit;
                show_digit = 1;
            }

            amount /= 10;
        }
    }

    return buf;
}

char *value_to_string(char *buf, const size_t len, uint64_t value)
{
    // don't use log10() on ledger
    size_t digits = 0;
    for (uint64_t val = value; val; val /= 10) {
        digits++;
    }
    digits = digits ? digits : 1;
    size_t total_len = digits + 1;
    if (total_len > len) {
        return NULL;
    }

    char *end = buf + total_len - 1;
    *end = '\0';

    for (size_t i = 0 ; value || i < 1; i++) {
        uint8_t digit = value % 10;
        *(--end) = '0' + digit;
        value /= 10;
    }

    return buf;
}

void packed_bit_array_set(uint8_t *bits, const size_t i, const bool b)
{
    size_t byte_idx = i / 8;
    size_t in_byte_idx = i % 8;

    if (b) {
        bits[byte_idx] |= (1 << in_byte_idx);
    }
    else {
        bits[byte_idx] &= ~( (uint8_t)(1 << in_byte_idx) );
    }
}

bool packed_bit_array_get(const uint8_t *bits, const size_t i)
{
    size_t byte_idx = i / 8;
    size_t in_byte_idx = i % 8;

    return (bits[byte_idx] >> in_byte_idx) & 1;
}

// Note: does not validate the address
void read_public_key_compressed(Compressed *out, const char *address)
{
    if (strnlen(address, MINA_ADDRESS_LEN) != MINA_ADDRESS_LEN - 1) {
        return;
    }

    uint8_t bytes[40];
    size_t bytes_len = 40;
    b58_decode(bytes, &bytes_len, address, MINA_ADDRESS_LEN - 1);

    struct bytes {
        uint8_t version;
        uint8_t payload[35];
        uint8_t checksum[4];
    } *raw = (struct bytes *)bytes;

    // Extract x-coordinate and swap big endian order
    for (size_t i = 2; i < sizeof(raw->payload) - 1; i++) {
        out->x[FIELD_BYTES - (i - 1)] = raw->payload[i];
    }
    out->is_odd = (bool)raw->payload[34];
}
