#ifndef CODA_CRYPTO
#define CODA_CRYPTO

#define field_bytes   48
#define scalar_bytes  48
#define group_bytes   144
#define affine_bytes  96
#define scalar_bits   384   // scalar_bytes * 8
#define scalar_offset 2     // scalars have 382 ( = 384 - 2 ) used bits

typedef unsigned char field[field_bytes];
typedef unsigned char scalar[scalar_bytes];

typedef struct group {
  field X;
  field Y;
  field Z;
} group;

typedef struct affine {
  field x;
  field y;
} affine;

typedef struct signature {
  field rx;
  scalar s;
} signature;

void field_add(field c, const field a, const field b);
void field_mul(field c, const field a, const field b);
void field_pow(field c, const field a, const field e);
void group_add(group *c, const group *a, const group *b);
void group_dbl(group *c, const group *a);
void group_scalar_mul(group *r, const scalar k, const group *p);
void affine_scalar_mul(affine *r, const scalar k, const affine *p);
void projective_to_affine(affine *p, const group *r);

void generate_pubkey(affine *pub_key, const scalar priv_key);
void generate_keypair(unsigned int index, affine *pub_key, scalar priv_key);

void sign(field rx, scalar s, const affine *public_key, const scalar private_key,
    const scalar msgx, const scalar msgm);

#endif // CODA_CRYPTO
