/*******************************************************************************
 * Poseidon is a hash function explained in https://eprint.iacr.org/2019/458
 * It requires the following parameters, with p a prime defining a prime field.
 * alpha = smallest prime st gcd(p, alpha) = 1
 * m = number of field elements in the state of the hash function.
 * N = number of rounds the hash function performs on each digest.
 * For m = r + c, the sponge absorbs (via field addition) and squeezes r field
 * elements per iteration, and offers log2(c) bits of security.
 * For our p (definied in crypto.c), we have alpha = 11, m = 3, r = 1, s = 2.
 *
 * Poseidon splits the full rounds into two, putting half before the parital
 * rounds are run, and the other half after. We have :
 * full rounds = 8
 * partial = 30, 
 * meaning that the rounds total 38. 
 * poseidon.c handles splitting the partial rounds in half and execution order.
 ********************************************************************************/

#ifndef POSEIDON
#define POSEIDON

#include "crypto.h"

#define rounds 38
#define full_rounds 8
#define partial_rounds 30
#define sponge_size 3

typedef scalar state[sponge_size];

void poseidon_1in(state s, const scalar in);
void poseidon_2in(state s, const scalar in0, const scalar in1);
void poseidon_digest(scalar out, const state s);

#endif // POSEIDON
