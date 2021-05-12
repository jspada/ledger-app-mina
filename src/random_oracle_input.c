#include <string.h>

#ifdef LEDGER_BUILD
    #include <os.h>
#endif

#include "random_oracle_input.h"
#include "utils.h"

// Add a field to the roinput.  The field to be added is converted to
// little endian byte order for compatibility with Mina
void roinput_add_field(ROInput *input, const Field a)
{
    size_t remaining = (int)input->fields_capacity - (int)input->fields_len;
    if (remaining < 1) {
        return;
    }

    // Mina field elements are little endian
    for (size_t i = FIELD_BYTES; i > 0; i--) {
        input->fields[input->fields_len][FIELD_BYTES - i] = a[i - 1];
    }
    input->fields_len += 1;
}

void roinput_add_bit(ROInput *input, bool b)
{
    size_t remaining = (int)input->bits_capacity * 8 - (int)input->bits_len;

    if (remaining < 1) {
        return;
    }

    packed_bit_array_set(input->bits, input->bits_len, b);
    input->bits_len += 1;
}

// Add a scalar to the roinput.  The scalar_eq to be added is converted to
// little endian byte order for compatibility with Mina
void roinput_add_scalar(ROInput *input, const Scalar a)
{
    size_t remaining = (int)input->bits_capacity * 8 - (int)input->bits_len;
    const size_t len = FIELD_BITS;

    if (remaining < len) {
        return;
    }

    size_t offset = input->bits_len;
    for (size_t i = 0; i < len; ++i) {
        size_t byte_idx = 32 - i / 8 - 1; // Mina scalars are little endian
        size_t bit_idx = (i % 8);
        bool b = (a[byte_idx] >> bit_idx) & 1;
        packed_bit_array_set(input->bits, offset + i, b);
    }

    input->bits_len += len;
}

void roinput_add_bytes(ROInput *input, const uint8_t *bytes, size_t len)
{
    size_t remaining = (int)input->bits_capacity * 8 - (int)input->bits_len;
    if (remaining < 8 * len) {
        return;
    }

    // LSB bits
    size_t k = input->bits_len;
    for (size_t i = 0; i < len; ++i) {
        const uint8_t b = bytes[i];

        for (size_t j = 0; j < 8; ++j) {
            packed_bit_array_set(input->bits, k, (b >> j) & 1);
            ++k;
        }
    }

    input->bits_len += 8 * len;
}

void roinput_add_uint32(ROInput *input, const uint32_t x)
{
    const size_t NUM_BYTES = 4;
    uint8_t le[NUM_BYTES];

    for (size_t i = 0; i < NUM_BYTES; ++i) {
        le[i] = (uint8_t) (0xff & (x >> (8 * i)));
    }

    roinput_add_bytes(input, le, NUM_BYTES);
}

void roinput_add_uint64(ROInput *input, const uint64_t x)
{
    const size_t NUM_BYTES = 8;
    uint8_t le[NUM_BYTES];

    for (size_t i = 0; i < NUM_BYTES; ++i) {
        le[i] = (uint8_t) (0xff & (x >> (8 * i)));
    }

    roinput_add_bytes(input, le, NUM_BYTES);
}

void roinput_to_bytes(uint8_t *out, const ROInput *input)
{
    size_t bit_idx = 0;

    // first the field elements, then the bitstrings
    for (size_t i = 0; i < input->fields_len; ++i) {
        uint8_t *field_bits = (uint8_t *)input->fields[i];

        for (size_t j = 0; j < FIELD_BITS; ++j) {
            packed_bit_array_set(
                out
                , bit_idx
                , packed_bit_array_get(field_bits, j));
                bit_idx += 1;
            }
    }

    for (size_t i = 0; i < input->bits_len; ++i) {
        packed_bit_array_set(out, bit_idx, packed_bit_array_get(input->bits, i));
        bit_idx += 1;
    }
}

int roinput_to_fields(Field *out, size_t len, const ROInput *input)
{
    size_t output_len = 0;

    if (len < input->fields_len) {
        return -1;
    }

    // Copy over the field elements
    for (size_t i = 0; i < input->fields_len; i++) {
        for (size_t j = FIELD_BYTES; j > 0; j--) {
            out[i][j - 1] = input->fields[i][FIELD_BYTES - j];
        }
    }
    output_len += input->fields_len;

    size_t bits_consumed = 0;

    // pack in the bits
    const size_t MAX_CHUNK_SIZE = FIELD_BITS - 1;
    while (bits_consumed < input->bits_len) {
        Field tmp = { };

        size_t remaining = input->bits_len - bits_consumed;
        size_t chunk_size_in_bits = remaining >= MAX_CHUNK_SIZE ? MAX_CHUNK_SIZE : remaining;

        for (size_t i = 0; i < chunk_size_in_bits; ++i) {
            packed_bit_array_set(
                tmp,
                i,
                packed_bit_array_get(input->bits, bits_consumed + i));
        }
        for (size_t i = FIELD_BYTES; i > 0; i--) {
            out[output_len][i - 1] = tmp[FIELD_BYTES - i];
        }
        output_len += 1;
        bits_consumed += chunk_size_in_bits;
    }

    return output_len;
}

int roinput_derive_message(uint8_t *out, const size_t len, const Keypair *kp, const ROInput *msg, const uint8_t network_id)
{
    Field   input_fields[msg->fields_capacity + 2];
    uint8_t input_bits[msg->bits_capacity + SCALAR_BYTES + 1];
    ROInput input = roinput_create(input_fields, input_bits);

    if (msg->fields_len > input.fields_capacity) {
        return -1;
    }
    if (msg->bits_len > input.bits_capacity*8) {
        return -1;
    }

    memcpy(input.fields, msg->fields, FIELD_BYTES * msg->fields_len);
    memcpy(input.bits, msg->bits, sizeof(uint8_t) * (msg->bits_len + 7)/8);
    input.fields_len = msg->fields_len;
    input.bits_len = msg->bits_len;

    roinput_add_field(&input, kp->pub.x);
    roinput_add_field(&input, kp->pub.y);
    roinput_add_scalar(&input, kp->priv); // Secret is now on stack!
    roinput_add_bytes(&input, &network_id, 1);

    size_t input_size_in_bytes = (input.bits_len + FIELD_BITS * input.fields_len + 7) / 8;
    if (input_size_in_bytes <= len) {
        roinput_to_bytes(out, &input);
    }

    // Clear privkey material
    explicit_bzero(&input_bits, sizeof(input_bits));

    return input_size_in_bytes > len ? -1 : input_size_in_bytes;
}

int roinput_hash_message(Field *out, const size_t len, const Affine *pub, const Field rx, const ROInput *msg)
{
    Field   input_fields[msg->fields_capacity + 3];
    uint8_t input_bits[msg->bits_capacity];
    ROInput input = roinput_create(input_fields, input_bits);

    if (msg->fields_len > input.fields_capacity) {
        return -1;
    }
    if (msg->bits_len > input.bits_capacity*8) {
        return -1;
    }

    memcpy(input.fields, msg->fields, FIELD_BYTES * msg->fields_len);
    memcpy(input.bits, msg->bits, sizeof(uint8_t) * (msg->bits_len + 7)/8);
    input.fields_len = msg->fields_len;
    input.bits_len = msg->bits_len;

    roinput_add_field(&input, pub->x);
    roinput_add_field(&input, pub->y);
    roinput_add_field(&input, rx);

    return roinput_to_fields(out, len, &input);
}
