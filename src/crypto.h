#pragma once

#include <stdint.h>
#include <stddef.h>

#define BIP32_PATH_LEN 5
#define BIP32_HARDENED_OFFSET 0x80000000

#define FIELD_BYTES   32
#define SCALAR_BYTES  32
#define SCALAR_BITS   256
#define SCALAR_OFFSET 2   // Scalars have 254 used bits

#define MINA_ADDRESS_LEN 56 // includes null-byte

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

typedef struct signature {
    Field rx;
    Scalar s;
} Signature;

typedef struct keypair {
    Affine pub;
    Scalar priv;
} Keypair;

void field_add(Field c, const Field a, const Field b);
void field_mul(Field c, const Field a, const Field b);
void field_pow(Field c, const Field a, const Field e);
void group_add(Group *c, const Group *a, const Group *b);
void group_dbl(Group *c, const Group *a);
void group_scalar_mul(Group *r, const Scalar k, const Group *p);
void affine_scalar_mul(Affine *r, const Scalar k, const Affine *p);
void projective_to_affine(Affine *p, const Group *r);

void generate_pubkey(Affine *pub_key, const Scalar priv_key);
void generate_keypair(uint32_t account, Keypair *keypair);
int get_address(const Affine *pub_key, char *address, size_t len);

void sign(const Keypair *kp, const Scalar msgx, const Scalar msgm, Signature *sig);
