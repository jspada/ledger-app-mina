#include <stdbool.h>
#include <math.h>
#include <memory.h>

#include "utils.h"
#include "crypto.h"

static const char BASE_58_ALPHABET[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
                                        'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q',
                                        'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                                        'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                        'w', 'x', 'y', 'z'};

// >= 0 : OK
// -2   : EXCEPTION_OVERFLOW
// -1   : INVALID_PARAMETER
int encodeBase58(unsigned char *in, unsigned char length,
                     unsigned char *out, unsigned char maxoutlen) {
    unsigned char tmp[164];
    unsigned char buffer[164];
    unsigned char j;
    unsigned char startAt;
    unsigned char zeroCount = 0;
    if (length > sizeof(tmp)) {
        return -1; // INVALID_PARAMETER
    }
    // TODO: os_memmove
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
        buffer[--j] = (unsigned char)BASE_58_ALPHABET[remainder];
    }
    while ((j < (2 * length)) && (buffer[j] == BASE_58_ALPHABET[0])) {
        ++j;
    }
    while (zeroCount-- > 0) {
        buffer[--j] = BASE_58_ALPHABET[0];
    }
    length = 2 * length - j;
    if (maxoutlen < length) {
        return -2; // EXCEPTION_OVERFLOW
    }
    // TODO: os_memmove
    memcpy(out, (buffer + j), length);
    return length;
}

uint32_t readUint32BE(uint8_t *buffer) {
  return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3]);
}

uint64_t readUint64BE(uint8_t *buffer) {
    return ((uint64_t)buffer[0] << 56) | ((uint64_t)buffer[1] << 48) | ((uint64_t)buffer[2] << 40) | ((uint64_t)buffer[3] << 32) |
           ((uint64_t)buffer[4] << 24) | ((uint64_t)buffer[5] << 16) | ((uint64_t)buffer[6] << 8) | ((uint64_t)buffer[7]);
}

char *valueToString(char *buf, size_t len, uint64_t value)
{
    size_t digits = 0;
    for (uint64_t val = value; val; val /= 10) {
        digits++;
    }
    digits = digits ? digits : 1;
    size_t total_len = digits + 1;

    char *end = buf + total_len - 1;
    *end = '\0';

    for (size_t i = 0 ; value || i < 1; i++) {
        uint8_t digit = value % 10;
        *(--end) = '0' + digit;
        value /= 10;
    }

    return buf;
}

char *amountToString(char *buf, size_t len, uint64_t amount)
{
    // COIN = 1.000 000 000;
    size_t mantissa_len = 1;
    for (uint64_t value = amount, len = 9; value && len > 0; value /= 10, len--) {
        if (value % 10 != 0) {
            mantissa_len = len;
            break;
        }
    }
    size_t characteristic_len = 0;
    for (uint64_t value = amount/COIN; value; value /= 10) {
        characteristic_len++;
    }
    characteristic_len = characteristic_len ? characteristic_len : 1;
    size_t total_len = characteristic_len + 1 + mantissa_len + 1;

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
