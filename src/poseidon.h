// Poseidon - cryptographic hash function for zero-knowledge proof systems
//
//     Details: https://eprint.iacr.org/2019/458
//
//     Poseidon requires the following parameters, with p a prime defining
//     a prime field.
//
//         alpha = smallest prime s.t. gcd(p, alpha) = 1
//         m = number of field elements in the state of the hash function
//         N = number of rounds the hash function performs on each digest
//
//     For m = r + c, the sponge absorbs (via field addition) and
//     squeezes r field elements per iteration, and offers log2(c)
//     bits of security.

#pragma once

#include "crypto.h"

#define ROUNDS 64
#define FULL_ROUNDS 63
#define SPONGE_SIZE 3

typedef Field State[SPONGE_SIZE];

void poseidon_init(State s, const uint8_t network_id);
void poseidon_update(State s, const Scalar *input, const size_t len);
void poseidon_digest(Scalar out, const State s);
