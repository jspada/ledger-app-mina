// Mina schnorr signatures and elliptic curve arithmetic
//
//     * Produces a schnorr signature according to the specification here:
//       https://github.com/MinaProtocol/mina/blob/develop/docs/specs/signatures/description.md
//
//     * Signer reference here: https://github.com/MinaProtocol/signer-reference
//
//     * Curve details
//         Pasta.Pallas (https://github.com/zcash/pasta)
//         E1/Fp : y^2 = x^3 + 5
//         GROUP_ORDER   = 28948022309329048855892746252171976963363056481941647379679742748393362948097 (Fq, 0x94)
//         FIELD_MODULUS = 28948022309329048855892746252171976963363056481941560715954676764349967630337 (Fp, 0x4c)

#include <os.h>

#include "crypto.h"
#include "poseidon.h"
#include "utils.h"
#include "globals.h"
#include "random_oracle_input.h"

// Base field Fp
static const Field FIELD_MODULUS = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x46, 0x98, 0xfc, 0x09, 0x4c, 0xf9, 0x1b,
    0x99, 0x2d, 0x30, 0xed, 0x00, 0x00, 0x00, 0x01
};

// Scalar field Fq
static const Scalar GROUP_ORDER = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x46, 0x98, 0xfc, 0x09, 0x94, 0xa8, 0xdd,
    0x8c, 0x46, 0xeb, 0x21, 0x00, 0x00, 0x00, 0x01
};

// a = 0, b = 5
static const Field GROUP_COEFF_B = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05
};


static const Field FIELD_ZERO = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const Field FIELD_ONE = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

static const Field FIELD_TWO = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
};

static const Field FIELD_THREE = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03
};

static const Field FIELD_FOUR = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04
};

static const Field FIELD_EIGHT = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08
};

static const Scalar SCALAR_ZERO = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// (X : Y : Z) = (0 : 1 : 0)
static const Group GROUP_ZERO = {
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    },
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
};

// g_generator = (1 : 12418654782883325593414442427049395787963493412651469444558597405572177144507)
static const Affine AFFINE_ONE = {
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    },
    {
        0x1b, 0x74, 0xb5, 0xa3, 0x0a, 0x12, 0x93, 0x7c,
        0x53, 0xdf, 0xa9, 0xf0, 0x63, 0x78, 0xee, 0x54,
        0x8f, 0x65, 0x5b, 0xd4, 0x33, 0x3d, 0x47, 0x71,
        0x19, 0xcf, 0x7a, 0x23, 0xca, 0xed, 0x2a, 0xbb
    }
};

void field_copy(Field a, const Field b)
{
    for (size_t i = 0; i < sizeof(Field); i++) {
        a[i] = b[i];
    }
}

void field_add(Field c, const Field a, const Field b)
{
    cx_math_addm(c, a, b, FIELD_MODULUS, FIELD_BYTES);
}

void field_sub(Field c, const Field a, const Field b)
{
    cx_math_subm(c, a, b, FIELD_MODULUS, FIELD_BYTES);
}

void field_mul(Field c, const Field a, const Field b)
{
    cx_math_multm(c, a, b, FIELD_MODULUS, FIELD_BYTES);
}

void field_sq(Field c, const Field a)
{
    cx_math_multm(c, a, a, FIELD_MODULUS, FIELD_BYTES);
}

void field_inv(Field c, const Field a)
{
    cx_math_invprimem(c, a, FIELD_MODULUS, FIELD_BYTES);
}

void field_negate(Field c, const Field a)
{
    // Ledger API expects inputs to be in range [0, FIELD_MODULUS)
    cx_math_subm(c, FIELD_ZERO, a, FIELD_MODULUS, FIELD_BYTES);
}

// c = a^e mod m
void field_pow(Field c, const Field a, const Field e)
{
    cx_math_powm(c, a, e, FIELD_BYTES, FIELD_MODULUS, FIELD_BYTES);
}

unsigned int field_eq(const Field a, const Field b)
{
    return (os_memcmp(a, b, FIELD_BYTES) == 0);
}

void scalar_copy(Scalar a, const Scalar b)
{
    for (size_t i = 0; i < sizeof(Scalar); i++) {
        a[i] = b[i];
    }
}

void scalar_add(Scalar c, const Scalar a, const Scalar b)
{
    cx_math_addm(c, a, b, GROUP_ORDER, SCALAR_BYTES);
}

void scalar_sub(Scalar c, const Scalar a, const Scalar b)
{
    cx_math_subm(c, a, b, GROUP_ORDER, SCALAR_BYTES);
}

void scalar_mul(Scalar c, const Scalar a, const Scalar b)
{
    cx_math_multm(c, a, b, GROUP_ORDER, SCALAR_BYTES);
}

void scalar_sq(Scalar c, const Scalar a)
{
    cx_math_multm(c, a, a, GROUP_ORDER, SCALAR_BYTES);
}

void scalar_negate(Field c, const Field a)
{
    // Ledger API expects inputs to be in range [0, GROUP_ORDER)
    cx_math_subm(c, SCALAR_ZERO, a, GROUP_ORDER, SCALAR_BYTES);
}

// c = a^e mod m
void scalar_pow(Scalar c, const Scalar a, const Scalar e)
{
    cx_math_powm(c, a, e, SCALAR_BYTES, GROUP_ORDER, SCALAR_BYTES);
}

unsigned int scalar_eq(const Scalar a, const Scalar b)
{
    return (os_memcmp(a, b, SCALAR_BYTES) == 0);
}

// zero is the only point with Z = 0 in jacobian coordinates
unsigned int is_zero(const Group *p)
{
    return field_eq(p->Z, FIELD_ZERO);
}

unsigned int affine_is_zero(const Affine *p)
{
    return (field_eq(p->x, FIELD_ZERO) && field_eq(p->y, FIELD_ZERO));
}

unsigned int is_on_curve(const Group *p)
{
    if (is_zero(p)) {
        return 1;
    }

    Field lhs, rhs;
    if (field_eq(p->Z, FIELD_ONE)) {
        // we can check y^2 == x^3 + ax + b
        field_sq(lhs, p->Y);                // y^2
        field_sq(rhs, p->X);                // x^2
        field_mul(rhs, rhs, p->X);          // x^3
        field_add(rhs, rhs, GROUP_COEFF_B); // x^3 + b
    }
    else {
        // we check (y/z^3)^2 == (x/z^2)^3 + b
        // => y^2 == x^3 + bz^6
        Field x3, z6;
        field_sq(x3, p->X);                 // x^2
        field_mul(x3, x3, p->X);            // x^3
        field_sq(lhs, p->Y);                // y^2
        field_sq(z6, p->Z);                 // z^2
        field_sq(z6, z6);                   // z^4
        field_mul(z6, z6, p->Z);            // z^5
        field_mul(z6, z6, p->Z);            // z^6

        field_mul(rhs, z6, GROUP_COEFF_B);  // bz^6
        field_add(rhs, x3, rhs);            // x^3 + bz^6
    }

    return field_eq(lhs, rhs);
}

void affine_to_projective(Group *r, const Affine *p)
{
    if (field_eq(p->x, FIELD_ZERO) && field_eq(p->y, FIELD_ZERO)) {
        os_memcpy(r->X, FIELD_ZERO, FIELD_BYTES);
        os_memcpy(r->Y, FIELD_ONE, FIELD_BYTES);
        os_memcpy(r->Z, FIELD_ZERO, FIELD_BYTES);
        return;
    }

    os_memcpy(r->X, p->x, FIELD_BYTES);
    os_memcpy(r->Y, p->y, FIELD_BYTES);
    os_memcpy(r->Z, FIELD_ONE, FIELD_BYTES);
}

void projective_to_affine(Affine *r, const Group *p)
{
    if (field_eq(p->Z, FIELD_ZERO)) {
        os_memcpy(r->x, FIELD_ZERO, FIELD_BYTES);
        os_memcpy(r->y, FIELD_ZERO, FIELD_BYTES);
        return;
    }

    Field zi, zi2, zi3;
    field_inv(zi, p->Z);        // 1/Z
    field_mul(zi2, zi, zi);     // 1/Z^2
    field_mul(zi3, zi2, zi);    // 1/Z^3
    field_mul(r->x, p->X, zi2); // X/Z^2
    field_mul(r->y, p->Y, zi3); // Y/Z^3
}

// https://www.hyperelliptic.org/EFD/g1p/auto-code/shortw/jacobian-0/doubling/dbl-1986-cc.op3
// cost 3M + 3S + 24 + 1*a + 4add + 2*2 + 1*3 + 1*4 + 1*8
void group_dbl(Group *r, const Group *p)
{
    if (is_zero(p)) {
        *r = *p;
        return;
    }

    Field t0, t1, S;
    field_sq(t0, p->Y);              // t0 = Y1^2
    field_mul(t1, p->X, t0);         // t1 = X1*t0
    field_mul(S, FIELD_FOUR, t1);    // S = 4*t1

    Field t2, t3;
    field_sq(t2, p->X);              // t2 = X1^2
                                     // t3 = Z1^4
                                     // t4 = a*t3 [a = 0]
    field_mul(t3, FIELD_THREE, t2);  // t3 = 3*t2

    Field t4, t5;
                                     // M = t3+t4
    field_sq(t4, t3);                // t4 = M^2
    field_mul(t5, FIELD_TWO, S);     // t5 = 2*S
    field_sub(r->X, t4, t5);         // T = t4-t5
                                     // X3 = T

    Field t6, t7, t8, t9, t10;
    field_sub(t6, S, r->X);          // t6 = S-T
    field_sq(t7, t0);                // t7 = Y1^4
    field_mul(t8, FIELD_EIGHT, t7);  // t8 = 8*t7
    field_mul(t9, t3, t6);           // t9 = M*t6
    field_sub(r->Y, t9, t8);         // Y3 = t11-t10
    field_mul(t10, p->Y, p->Z);      // t10 = Y1*Z1
    field_mul(r->Z, FIELD_TWO, t10); // Z3 = 2*t12
}

// https://www.hyperelliptic.org/EFD/g1p/auto-code/shortw/jacobian-0/addition/add-1986-cc.op3
// cost 10M + 5S + 33 + 6add
void group_add(Group *r, const Group *p, const Group *q)
{
    if (is_zero(p)) {
        *r = *q;
        return;
    }

    if (is_zero(q)) {
        *r = *p;
        return;
    }

    if (field_eq(p->X, q->X) && field_eq(p->Y, q->Y) && field_eq(p->Z, q->Z)) {
        return group_dbl(r, p);
    }

    Field t0, U1, t1, U2, t2;
    field_sq(t0, q->Z);        // t0 = Z2^2
    field_mul(U1, p->X, t0);   // U1 = X1*t0
    field_sq(t1, p->Z);        // t1 = Z1^2
    field_mul(U2, q->X, t1);   // U2 = X2*t1
    field_mul(t2, t0, q->Z);   // t2 = Z2^3

    Field S1, S2, P, R;
    field_mul(S1, p->Y, t2);   // S1 = Y1*t2
    field_mul(t0, t1, p->Z);   // t0 = Z1^3
    field_mul(S2, q->Y, t0);   // S2 = Y2*t0
    field_sub(P, U2, U1);      // P = U2-U1
    field_sub(R, S2, S1);      // R = S2-S1
    field_add(t1, U1, U2);     // t1 = U1+U2

    field_sq(t2, R);           // t2 = R^2
    field_sq(U2, P);           // U2 = P^2
    field_mul(S2, t1, U2);     // S2 = t1*U2
    field_sub(r->X, t2, S2);   // X3 = t2-S2

                               // t8 = P^2 [t8 = U2]
    field_mul(t1, U1, U2);     // t1 = U1*U2
    field_sub(t2, t1, r->X);   // t2 = t1-X3
    field_mul(t0, U2, P);      // t0 = P^3
    field_mul(S2, S1, t0);     // S2 = S1*t0

    Field t3, t4;

    field_mul(t3, R, t2);      // t3 = R*t2
    field_sub(r->Y, t3, S2);   // Y3 = t3-S2
    field_mul(t4, q->Z, P);    // t4 = Z2*P
    field_mul(r->Z, p->Z, t4); // Z3 = Z1*t4
}

// Montgomery ladder scalar multiplication (this could be optimized further)
void group_scalar_mul(Group *r, const Scalar k, const Group *p)
{
    *r = GROUP_ZERO;
    if (is_zero(p)) {
        return;
    }
    if (scalar_eq(k, SCALAR_ZERO)) {
        return;
    }

    Group r1 = *p;
    for (size_t i = SCALAR_OFFSET; i < SCALAR_BITS; i++) {
        uint8_t di = k[i / 8] & (1 << (7 - (i % 8)));
        Group q0;
        if (!di) {
            group_add(&q0, r, &r1); // r1 = r0 + r1
            r1 = q0;
            group_dbl(&q0, r);      // r0 = r0 + r0
            *r = q0;
        }
        else {
            group_add(&q0, r, &r1); // r0 = r0 + r1
            *r = q0;
            group_dbl(&q0, &r1);    // r1 = r1 + r1
            r1 = q0;
        }
    }
}

void affine_scalar_mul(Affine *r, const Scalar k, const Affine *p)
{
    Group pp, pr;
    affine_to_projective(&pp, p);
    group_scalar_mul(&pr, k, &pp);
    projective_to_affine(r, &pr);
}

inline unsigned int is_odd(const Field y)
{
    return y[FIELD_BYTES - 1] & 0x01;
}

// Ledger uses:
// - BIP39 to generate and interpret the master seed, which produces
//   the 24 words shown on the device at startup.
// - BIP32 for HD key derivation (using the child key derivation
//   function)
// - BIP44 for HD account derivation (so e.g. btc and mina keys don't
//   clash)
void generate_keypair(Keypair *keypair, uint32_t account)
{
    if (!keypair) {
        THROW(INVALID_PARAMETER);
    }

    const uint32_t bip32_path[BIP32_PATH_LEN] = {
        44      | BIP32_HARDENED_OFFSET,
        12586   | BIP32_HARDENED_OFFSET, // 0x312a
        account | BIP32_HARDENED_OFFSET,
        0       | BIP32_HARDENED_OFFSET,
        0       | BIP32_HARDENED_OFFSET
    };

    os_perso_derive_node_bip32(CX_CURVE_256K1, bip32_path, BIP32_PATH_LEN, keypair->priv, NULL);

    // Make sure the private key is in [0, p)
    //
    // Note: Mina does rejection sampling to obtain a private key in
    // [0, p), where the field modulus
    //
    //     p = 28948022309329048855892746252171976963363056481941560715954676764349967630337
    //
    // Due to constraints, this implementation take a different
    // approach and just unsets the top two bits of the 256bit bip44
    // secret, so
    //
    //     max = 28948022309329048855892746252171976963317496166410141009864396001978282409983.
    //
    // If p < max then we could still generate invalid private keys
    // (although it's highly unlikely), but
    //
    //     p - max = 45560315531419706090280762371685220354
    //
    // Thus, we cannot generate invalid private keys and instead lose an
    // insignificant amount of entropy.

    keypair->priv[0] &= 0x3f; // drop first two bits

    affine_scalar_mul(&keypair->pub, keypair->priv, &AFFINE_ONE);

    return;
}

int get_address(char *address, size_t len, const Affine *pub_key)
{
    if (len != MINA_ADDRESS_LEN) {
        THROW(INVALID_PARAMETER);
    }

    struct bytes {
        uint8_t version;
        uint8_t payload[35];
        uint8_t checksum[4];
    } raw;

    raw.version    = 0xcb; // version for base58 check
    raw.payload[0] = 0x01; // non_zero_curve_point version
    raw.payload[1] = 0x01; // compressed_poly version
    // reversed x-coordinate
    for (size_t i = 0; i < sizeof(pub_key->x); i++) {
        raw.payload[i + 2] = pub_key->x[sizeof(pub_key->x) - i - 1];
    }
    // y-coordinate parity
    raw.payload[34] = is_odd(pub_key->y);

    uint8_t hash1[32];
    cx_hash_sha256((const unsigned char *)&raw, 36, hash1, 32);

    uint8_t hash2[32];
    cx_hash_sha256(hash1, 32, hash2, 32);
    os_memmove(raw.checksum, hash2, 4);

    // Encode as address
    int result = b58_encode((unsigned char *)&raw, sizeof(raw), (unsigned char *)address, len);
    if (result < 0) {
        address[0] = '\0';
    }
    else {
        address[MINA_ADDRESS_LEN - 1] = '\0';
    }

    return result;
}

void generate_pubkey(Affine *pub_key, const Scalar priv_key)
{
    affine_scalar_mul(pub_key, priv_key, &AFFINE_ONE);
}

void message_derive(Scalar out, const Keypair *kp, const ROInput *input)
{
    uint8_t derive_msg[267] = { };
    size_t derive_len = roinput_derive_message(derive_msg, sizeof(derive_msg), kp, input);

    // blake2b hash
    cx_blake2b_t ctx;
    cx_blake2b_init(&ctx, 256);
    cx_hash(&ctx.header, 0, derive_msg, derive_len, NULL, 0);
    Scalar bytes;
    cx_hash(&ctx.header, CX_LAST, NULL, 0, bytes, ctx.ctx.outlen);

    // Convert to scalar
    for (size_t i = sizeof(Scalar); i > 0; i--) {
        out[i - 1] = bytes[sizeof(Scalar) - i]; // Reverse bytes
    }
    // Unset top two bits
    out[0] &= 0x3f;
}

void message_hash(Scalar out, const Affine *pub, const Field rx, const ROInput *input)
{
    Field hash_msg[9];
    size_t hash_msg_len = roinput_hash_message(hash_msg, sizeof(hash_msg),
                                               pub, rx, input);

    // Initial sponge state
    State pos = {
        {
            0x3e, 0x32, 0x37, 0x18, 0xb3, 0xfe, 0x2a, 0x3d,
            0x6a, 0x27, 0x89, 0x07, 0xaf, 0xa2, 0xa9, 0xfc,
            0x7d, 0x17, 0x93, 0x3d, 0x1e, 0xf7, 0xc6, 0x65,
            0x50, 0x07, 0xb9, 0x62, 0xb8, 0xc8, 0x53, 0x92
        },
        {
            0x37, 0x0a, 0x1c, 0x8b, 0x48, 0x37, 0x40, 0xa5,
            0xcc, 0xe5, 0xc5, 0x0c, 0x92, 0xcb, 0x64, 0xb9,
            0x64, 0xaf, 0x7d, 0x48, 0xa0, 0x24, 0x8c, 0x54,
            0xd1, 0x03, 0x2c, 0x6a, 0x3d, 0xe4, 0x4e, 0x99
        },
        {
            0x07, 0x1c, 0x33, 0x03, 0xf0, 0x2d, 0x91, 0x1a,
            0xff, 0xae, 0xfb, 0x48, 0x9e, 0x71, 0x4b, 0x51,
            0xd7, 0xe1, 0x8e, 0x31, 0xb3, 0x2d, 0x83, 0x9c,
            0xfd, 0x3d, 0x55, 0x33, 0x46, 0xc1, 0x0d, 0x36
        }
    };

    poseidon_update(pos, hash_msg, hash_msg_len);
    poseidon_digest(out, pos);
}

void sign(Signature *sig, const Keypair *kp, const ROInput *input)
{
    // k = message_derive(input.fields + kp.pub + input.bits + kp.priv)
    Scalar k = { };
    message_derive(k, kp, input);

    // r = k*g
    Affine r;
    affine_scalar_mul(&r, k, &AFFINE_ONE);
    field_copy(sig->rx, r.x);

    if (is_odd(r.y)) {
        // k = -k
        Scalar tmp;
        scalar_copy(tmp, k);
        scalar_negate(k, tmp);
    }

    // e = message_hash(input + kp.pub + r.x)
    Scalar e;
    message_hash(e, &kp->pub, r.x, input);

    // s = k + e*sk
    Scalar e_priv;
    scalar_mul(e_priv, e, kp->priv);
    scalar_add(sig->s, k, e_priv);

    // Clear secrets from memory
    os_memset(&k, 0, sizeof(k));
    os_memset(&e_priv, 0, sizeof(e_priv));
}
