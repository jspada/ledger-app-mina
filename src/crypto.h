#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BIP32_PATH_LEN 5
#define BIP32_HARDENED_OFFSET 0x80000000

#define FIELD_BYTES   32
#define FIELD_SIZE_IN_BITS 255
#define SCALAR_BYTES  32
#define SCALAR_BITS   256
#define SCALAR_OFFSET 2     // Scalars have 254 used bits

#define MINA_ADDRESS_LEN 56 // includes null-byte

#define COIN 1000000000ULL

typedef uint64_t Currency;
#define FEE_BITS 64
#define AMOUNT_BITS 64
typedef uint32_t GlobalSlot;
#define GLOBAL_SLOT_BITS 32
typedef uint32_t Nonce;
#define NONCE_BITS 32
typedef uint64_t TokenId;
#define TOKEN_ID_BITS 64
#define MEMO_BYTES 34
typedef uint8_t Memo[MEMO_BYTES];
#define MEMO_BITS (MEMO_BYTES * 8)
typedef bool Tag[3];
#define TAG_BITS 3
#define TX_BITS_LEN ((FEE_BITS + TOKEN_ID_BITS + 1 + NONCE_BITS + \
                      GLOBAL_SLOT_BITS + MEMO_BITS + TAG_BITS + 1 + 1 + \
                      TOKEN_ID_BITS + AMOUNT_BITS + 1 + 7)/8)

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

typedef struct transaction {
    // common
    Currency fee;
    TokenId fee_token;
    Compressed fee_payer_pk;
    Nonce nonce;
    GlobalSlot valid_until;
    Memo memo;
    // body
    Tag tag;
    Compressed source_pk;
    Compressed receiver_pk;
    TokenId token_id;
    Currency amount;
    bool token_locked;
} Transaction;

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
void field_pow_orig(Field c, const Field a, const Field e);
void group_add(Group *c, const Group *a, const Group *b);
void group_dbl(Group *c, const Group *a);
void group_scalar_mul(Group *r, const Scalar k, const Group *p);
void affine_scalar_mul(Affine *r, const Scalar k, const Affine *p);
void projective_to_affine(Affine *p, const Group *r);

void generate_keypair(Keypair *keypair, uint32_t account);
void generate_pubkey(Affine *pub_key, const Scalar priv_key);
int get_address(char *address, size_t len, const Affine *pub_key);

void sign(Signature *sig, const Keypair *kp, const ROInput *input);
