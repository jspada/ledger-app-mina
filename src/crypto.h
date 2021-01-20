#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BIP32_PATH_LEN 5
#define BIP32_HARDENED_OFFSET 0x80000000

#define FIELD_BYTES    32
#define SCALAR_BYTES   32
#define SCALAR_BITS    256
#define SCALAR_OFFSET  2     // Scalars have 254 used bits

#define SIGNATURE_LEN    129 // as strings,
#define MINA_ADDRESS_LEN 56  // includes null-bytes

#define COIN 1000000000ULL

typedef uint8_t Field[FIELD_BYTES];
typedef uint8_t Scalar[SCALAR_BYTES];

typedef struct group {
    Field X;
    Field Y;
    Field Z;
} Group;

typedef struct affine {
    Field x;
    Field y;
} Affine;

typedef struct compressed {
    Field x;
    bool is_odd;
} Compressed;

typedef struct signature {
    Field rx;
    Scalar s;
} Signature;

typedef struct keypair {
    Affine pub;
    Scalar priv;
} Keypair;

typedef struct roinput ROInput; // Forward declaration

void field_copy(Field a, const Field b);
void field_add(Field c, const Field a, const Field b);
void field_mul(Field c, const Field a, const Field b);
void field_sq(Field c, const Field a);
void field_pow(Field c, const Field a, const Field e);
void group_add(Group *c, const Group *a, const Group *b);
void group_dbl(Group *c, const Group *a);
void group_scalar_mul(Group *r, const Scalar k, const Group *p);
void affine_scalar_mul(Affine *r, const Scalar k, const Affine *p);
void projective_to_affine(Affine *p, const Group *r);

void generate_keypair(Keypair *keypair, uint32_t account);
void generate_pubkey(Affine *pub_key, const Scalar priv_key);
int get_address(char *address, const size_t len, const Affine *pub_key);

void sign(Signature *sig, const Keypair *kp, const ROInput *input);
