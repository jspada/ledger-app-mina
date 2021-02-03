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

void field_copy(Field b, const Field a)
{
    os_memmove(b, a, FIELD_BYTES);
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

void field_sq(Field b, const Field a)
{
    cx_math_multm(b, a, a, FIELD_MODULUS, FIELD_BYTES);
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

bool field_eq(const Field a, const Field b)
{
    return (os_memcmp(a, b, FIELD_BYTES) == 0);
}

bool field_is_odd(const Field y)
{
    return y[FIELD_BYTES - 1] & 0x01;
}

void scalar_from_bytes(Scalar a, const uint8_t *bytes, const size_t len)
{
    if (len != SCALAR_BYTES) {
        THROW(INVALID_PARAMETER);
    }

    os_memmove(a, bytes, SCALAR_BYTES);

    // Make sure the scalar is in [0, p)
    //
    // Note: Mina does rejection sampling to obtain a scalar in
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
    // If p < max then we could still generate invalid scalars
    // (although it's highly unlikely), but
    //
    //     p - max = 45560315531419706090280762371685220354
    //
    // Thus, we cannot generate invalid scalar and instead lose an
    // insignificant amount of entropy.

    a[0] &= 0x3f; // drop first two bits
}

void scalar_copy(Scalar b, const Scalar a)
{
    os_memmove(b, a, SCALAR_BYTES);
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

void scalar_sq(Scalar b, const Scalar a)
{
    cx_math_multm(b, a, a, GROUP_ORDER, SCALAR_BYTES);
}

void scalar_negate(Field b, const Field a)
{
    // Ledger API expects inputs to be in range [0, GROUP_ORDER)
    cx_math_subm(b, SCALAR_ZERO, a, GROUP_ORDER, SCALAR_BYTES);
}

// c = a^e mod m
void scalar_pow(Scalar c, const Scalar a, const Scalar e)
{
    cx_math_powm(c, a, e, SCALAR_BYTES, GROUP_ORDER, SCALAR_BYTES);
}

bool scalar_eq(const Scalar a, const Scalar b)
{
    return os_memcmp(a, b, SCALAR_BYTES) == 0;
}

bool scalar_is_zero(const Scalar a)
{
    return scalar_eq(a, SCALAR_ZERO);
}

void group_copy(Group *b, const Group *a)
{
    os_memmove(b, a, sizeof(Group));
}

// zero is the only point with Z = 0 in jacobian coordinates
bool group_is_zero(const Group *p)
{
    return field_eq(p->Z, FIELD_ZERO);
}

// https://www.hyperelliptic.org/EFD/g1p/auto-code/shortw/jacobian-0/doubling/dbl-1986-cc.op3
// cost 3M + 3S + 24 + 1*a + 4add + 2*2 + 1*3 + 1*4 + 1*8
void group_dbl(Group *r, const Group *p)
{
    if (group_is_zero(p)) {
        group_copy(r, p);
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
    if (group_is_zero(p)) {
        group_copy(r, q);
        return;
    }

    if (group_is_zero(q)) {
        group_copy(r, p);
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

    field_mul(U1, R, t2);      // U1 = R*t2
    field_sub(r->Y, U1, S2);   // Y3 = t3-S2
    field_mul(U2, q->Z, P);    // U2 = Z2*P
    field_mul(r->Z, p->Z, U2); // Z3 = Z1*t4
}

void group_negate(Group *q, const Group *p)
{
    field_copy(q->X, p->X);
    field_negate(q->Y, p->Y);
    field_copy(q->Z, p->Z);
}

// Double-and-add scalar multiplication
void group_scalar_mul(Group *q, const Scalar k, const Group *p)
{
    group_copy(q, &GROUP_ZERO);
    if (group_is_zero(p)) {
        return;
    }
    if (scalar_is_zero(k)) {
        return;
    }

    Group t0;
    for (size_t i = 0; i < SCALAR_BITS; i++) {
        uint8_t di = (k[i / 8] >> (7 - (i % 8))) & 0x01;

        // q = 2q
        group_dbl(&t0, q);
        group_copy(q, &t0);

        if (di) {
            // q = q + p
            group_add(&t0, q, p);
            group_copy(q, &t0);
        }
    }
}

bool group_is_on_curve(const Group *p)
{
    if (group_is_zero(p)) {
        return true;
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

bool affine_is_zero(const Affine *p)
{
    return field_eq(p->x, FIELD_ZERO) && field_eq(p->y, FIELD_ZERO);
}

void affine_to_group(Group *q, const Affine *p)
{
    if (field_eq(p->x, FIELD_ZERO) && field_eq(p->y, FIELD_ZERO)) {
        field_copy(q->X, FIELD_ZERO);
        field_copy(q->Y, FIELD_ONE);
        field_copy(q->Z, FIELD_ZERO);
        return;
    }

    field_copy(q->X, p->x);
    field_copy(q->Y, p->y);
    field_copy(q->Z, FIELD_ONE);
}

void affine_from_group(Affine *q, const Group *p)
{
    if (field_eq(p->Z, FIELD_ZERO)) {
        field_copy(q->x, FIELD_ZERO);
        field_copy(q->y, FIELD_ZERO);
        return;
    }

    Field zi, tmp;
    field_inv(zi, p->Z);         // 1/Z
    field_mul(q->y, zi, zi);     // 1/Z^2
    field_mul(tmp, q->y, zi);    // 1/Z^3
    field_mul(q->x, p->X, q->y); // X/Z^2
    field_mul(q->y, p->Y, tmp);  // Y/Z^3
}

void affine_scalar_mul(Affine *q, const Scalar k, const Affine *p)
{
    Group pp, pq;
    affine_to_group(&pp, p);
    group_scalar_mul(&pq, k, &pp);
    affine_from_group(q, &pq);
}

bool affine_eq(const Affine *p, const Affine *q)
{
    return field_eq(p->x, q->x) && field_eq(p->y, q->y);
}

void affine_add(Affine *r, const Affine *p, const Affine *q)
{
    Group gr, gp, gq;
    affine_to_group(&gp, p);
    affine_to_group(&gq, q);
    group_add(&gr, &gp, &gq);
    affine_from_group(r, &gr);
}

void affine_negate(Affine *q, const Affine *p)
{
    Group gq, gp;
    affine_to_group(&gp, p);
    group_negate(&gq, &gp);
    affine_from_group(q, &gq);
}

bool affine_is_on_curve(const Affine *p)
{
    Group gp;
    affine_to_group(&gp, p);
    return group_is_on_curve(&gp);
}

void generate_pubkey(Affine *pub_key, const Scalar priv_key)
{
    affine_scalar_mul(pub_key, priv_key, &AFFINE_ONE);
}

void generate_keypair(Keypair *keypair, const uint32_t account)
{
    if (!keypair) {
        THROW(INVALID_PARAMETER);
    }

    const uint32_t bip32_path[BIP32_PATH_LEN] = {
        44      | BIP32_HARDENED_OFFSET,
        12586   | BIP32_HARDENED_OFFSET, // 0x312a
        account | BIP32_HARDENED_OFFSET,
        0,
        0
    };

    // Generate private key
    os_perso_derive_node_bip32(CX_CURVE_256K1, bip32_path, BIP32_PATH_LEN, keypair->priv, NULL);
    scalar_from_bytes(keypair->priv, keypair->priv, sizeof(keypair->priv));

    // Generate public key
    generate_pubkey(&keypair->pub, keypair->priv);

    return;
}

bool generate_address(char *address, const size_t len, const Affine *pub_key)
{
    if (len != MINA_ADDRESS_LEN) {
        address[0] = '\0';
        return false;
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
    raw.payload[34] = field_is_odd(pub_key->y);

    uint8_t hash1[CX_SHA256_SIZE];
    cx_hash_sha256((const unsigned char *)&raw, 36, hash1, sizeof(hash1));

    uint8_t hash2[CX_SHA256_SIZE];
    cx_hash_sha256(hash1, sizeof(hash1), hash2, sizeof(hash2));
    os_memcpy(raw.checksum, hash2, 4);

    // Encode as address
    int result = b58_encode((unsigned char *)&raw, sizeof(raw), (unsigned char *)address, len);
    address[MINA_ADDRESS_LEN - 1] = '\0';
    if (result < 0) {
        address[0] = '\0';
        return false;
    }
    if (result != MINA_ADDRESS_LEN - 1) {
        address[0] = '\0';
        return false;
    }

    return true;
}

bool validate_address(const char *address)
{
    uint8_t bytes[40];
    size_t bytes_len = sizeof(bytes);

    if (strnlen(address, MINA_ADDRESS_LEN) != MINA_ADDRESS_LEN - 1) {
        return false;
    }

    b58_decode(bytes, &bytes_len, address, MINA_ADDRESS_LEN - 1);

    struct bytes {
        uint8_t version;
        uint8_t payload[35];
        uint8_t checksum[4];
    } *raw = (struct bytes *)bytes;

    uint8_t hash1[CX_SHA256_SIZE];
    cx_hash_sha256((const unsigned char *)raw, 36, hash1, sizeof(hash1));

    uint8_t hash2[CX_SHA256_SIZE];
    cx_hash_sha256(hash1, sizeof(hash1), hash2, sizeof(hash2));
    return os_memcmp(raw->checksum, hash2, 4) == 0;
}

void message_derive(Scalar out, const Keypair *kp, const ROInput *input, const uint8_t network_id)
{
    uint8_t derive_msg[268] = { };
    int derive_len = roinput_derive_message(derive_msg, sizeof(derive_msg), kp, input, network_id);
    if (derive_len < 0) {
        THROW(INVALID_PARAMETER);
    }

    // blake2b hash
    cx_blake2b_t ctx;
    cx_blake2b_init(&ctx, 256);
    cx_hash(&ctx.header, 0, derive_msg, derive_len, NULL, 0);
    cx_hash(&ctx.header, CX_LAST, NULL, 0, out, ctx.ctx.outlen);

    // Swap from little-endian to big-endian in place
    for (size_t i = SCALAR_BYTES; i > SCALAR_BYTES/2; i--) {
        uint8_t tmp;
        tmp = out[i - 1];
        out[i - 1] = out[SCALAR_BYTES - i];
        out[SCALAR_BYTES - i] = tmp;
    }

    // Convert to scalar
    scalar_from_bytes(out, out, SCALAR_BYTES);
}

void message_hash(Scalar out, const Affine *pub, const Field rx, const ROInput *input, const uint8_t network_id)
{
    Field hash_msg[9];
    int hash_msg_len = roinput_hash_message(hash_msg, sizeof(hash_msg), pub, rx, input);
    if (hash_msg_len < 0) {
        THROW(INVALID_PARAMETER);
    }

    // Initial sponge state
    State pos;
    poseidon_init(pos, network_id);
    poseidon_update(pos, hash_msg, hash_msg_len);
    poseidon_digest(out, pos);
}

void sign(Signature *sig, const Keypair *kp, const ROInput *input, const uint8_t network_id)
{
    Scalar k;
    Affine r;
    Scalar tmp;

    BEGIN_TRY {
        TRY {
            // k = message_derive(input.fields + kp.pub + input.bits + kp.priv)
            message_derive(k, kp, input, network_id);

            // r = k*g
            affine_scalar_mul(&r, k, &AFFINE_ONE);
            field_copy(sig->rx, r.x);

            if (field_is_odd(r.y)) {
                // k = -k
                scalar_copy(tmp, k);
                scalar_negate(k, tmp);
            }

            // e = message_hash(input + kp.pub + r.x)
            message_hash(sig->s, &kp->pub, r.x, input, network_id);

            // s = k + e*sk
            scalar_mul(tmp, sig->s, kp->priv);
            scalar_add(sig->s, k, tmp);
        }
        FINALLY {
            // Clear secrets from memory
            explicit_bzero(tmp, sizeof(tmp));
            explicit_bzero(k, sizeof(k));
        }
        END_TRY;
    }
}
