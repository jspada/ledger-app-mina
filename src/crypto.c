/*******************************************************************************
 * All of the elliptic curve arithmetic is implemented in this file. That means:
 *  - field_add, field_sub, field_mul, field_sq, field_inv, field_negate, field_pow, field_eq
 *  - scalar_add, scalar_sub, scalar_mul, scalar_sq, scalar_pow, scalar_eq
 *  - group_add, group_dbl, group_scalar_mul (group elements use projective coordinates)
 *  - affine_scalar_mul
 *  - projective_to_affine
 *  - generate_pubkey, generate_keypair
 *  - sign. Produces a schnorr signature according to the specification here :
 *    https://github.com/CodaProtocol/coda/blob/develop/docs/specs/signatures/description.md
 *
 * E1/Fp : y^2 = x^3 + 5
 * Tweedle_p = 28948022309329048855892746252171976963322203655955319056773317069363642105857
 * Tweedle_q = 28948022309329048855892746252171976963322203655954433126947083963168578338817
 ********************************************************************************/

#include "os.h"
#include "cx.h"
#include "crypto.h"
#include "poseidon.h"

// scalar field p
 static const field field_modulus = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x8a, 0xa1, 0x27, 0x6c, 0x3f, 0x59, 0xb9,
    0xa1, 0x40, 0x64, 0xe2, 0x00, 0x00, 0x00, 0x01, };

// base field q
static const scalar group_order = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x8a, 0xa1, 0x27, 0x69, 0x62, 0x86, 0xc9,
    0x84, 0x2c, 0xaf, 0xd4, 0x00, 0x00, 0x00, 0x01, };

// a = 0, b = 5
static const field group_coeff_b = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, };

static const field field_one = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, };

static const field field_two = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, };

static const field field_three = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, };

static const field field_four = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, };

static const field field_eight = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, };

static const field field_zero = {0};
static const scalar scalar_zero = {0};
static const affine affine_zero = {{0}, {0}};

// (X : Y : Z) = (0 : 1 : 0)
static const group group_zero = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, },
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, }};

// g_generator = (1 :
// 14240188643175251183985684255458419213835105645119662786317263805424119994471
// : 1)
static const group group_one = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, },
    {0x1f, 0x7b, 0xa9, 0x41, 0x05, 0xab, 0x7f, 0x06,
     0xc5, 0x61, 0x1d, 0x99, 0xcd, 0xa6, 0x58, 0x97,
     0x21, 0x70, 0xf4, 0x01, 0xe4, 0x0b, 0x71, 0x3b,
     0xb7, 0x28, 0x86, 0x69, 0xee, 0x92, 0x58, 0x67, },
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, }};

static const affine affine_one = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, },
    {0x1f, 0x7b, 0xa9, 0x41, 0x05, 0xab, 0x7f, 0x06,
     0xc5, 0x61, 0x1d, 0x99, 0xcd, 0xa6, 0x58, 0x97,
     0x21, 0x70, 0xf4, 0x01, 0xe4, 0x0b, 0x71, 0x3b,
     0xb7, 0x28, 0x86, 0x69, 0xee, 0x92, 0x58, 0x67, }};

void field_add(field c, const field a, const field b) {
  cx_math_addm(c, a, b, field_modulus, field_bytes);
}

void field_sub(field c, const field a, const field b) {
  cx_math_subm(c, a, b, field_modulus, field_bytes);
}

void field_mul(field c, const field a, const field b) {
  cx_math_multm(c, a, b, field_modulus, field_bytes);
}

void field_sq(field c, const field a) {
  cx_math_multm(c, a, a, field_modulus, field_bytes);
}

void field_inv(field c, const field a) {
  cx_math_invprimem(c, a, field_modulus, field_bytes);
}

void field_negate(field c, const field a) {
  cx_math_subm(c, field_modulus, a, field_modulus, field_bytes);
}

// c = a^e mod m
// cx_math_powm(result_pointer, a, e, len_e, m, len(result)  (which is also
// len(a) and len(m)) )
void field_pow(field c, const field a, const field e) {
  cx_math_powm(c, a, e, 1, field_modulus, field_bytes);
}

unsigned int field_eq(const field a, const field b) {
  return (os_memcmp(a, b, field_bytes) == 0);
}

void scalar_add(scalar c, const scalar a, const scalar b) {
  cx_math_addm(c, a, b, group_order, scalar_bytes);
}

void scalar_sub(scalar c, const scalar a, const scalar b) {
  cx_math_subm(c, a, b, group_order, scalar_bytes);
}

void scalar_mul(scalar c, const scalar a, const scalar b) {
  cx_math_multm(c, a, b, group_order, scalar_bytes);
}

void scalar_sq(scalar c, const scalar a) {
  cx_math_multm(c, a, a, group_order, scalar_bytes);
}

// c = a^e mod m
// cx_math_powm(result_pointer, a, e, len_e, m, len(result)  (which is also
// len(a) and len(m)) )
void scalar_pow(scalar c, const scalar a, const scalar e) {
  cx_math_powm(c, a, e, 1, group_order, scalar_bytes);
}

unsigned int scalar_eq(const scalar a, const scalar b) {
  return (os_memcmp(a, b, scalar_bytes) == 0);
}

// zero is the only point with Z = 0 in jacobian coordinates
unsigned int is_zero(const group *p) { return field_eq(p->Z, field_zero); }

unsigned int affine_is_zero(const affine *p) {
  return (field_eq(p->x, field_zero) && field_eq(p->y, field_zero));
}

unsigned int is_on_curve(const group *p) {
  if (is_zero(p)) {
    return 1;
  }
  field lhs, rhs;

  if (field_eq(p->Z, field_one)) {
    // we can check y^2 == x^3 + ax + b
    field_sq(lhs, p->Y);                // y^2
    field_sq(rhs, p->X);                // x^2
    field_mul(rhs, rhs, p->X);          // x^3
    field_add(rhs, rhs, group_coeff_b); // x^3 + b
  } else {
    // we check (y/z^3)^2 == (x/z^2)^3 + b
    // => y^2 == x^3 + bz^6
    field x3, z6;
    field_sq(x3, p->X);                 // x^2
    field_mul(x3, x3, p->X);            // x^3
    field_sq(lhs, p->Y);                // y^2
    field_sq(z6, p->Z);                 // z^2
    field_sq(z6, z6);                   // z^4
    field_mul(z6, z6, p->Z);            // z^5
    field_mul(z6, z6, p->Z);            // z^6

    field_mul(rhs, z6, group_coeff_b);  // bz^6
    field_add(rhs, x3, rhs);            // x^3 + bz^6
  }
  return field_eq(lhs, rhs);
}

void affine_to_projective(group *r, const affine *p) {
  if (field_eq(p->x, field_zero) && field_eq(p->y, field_zero)) {
    os_memcpy(r->X, field_zero, field_bytes);
    os_memcpy(r->Y, field_one, field_bytes);
    os_memcpy(r->Z, field_zero, field_bytes);
    return;
  }
  os_memcpy(r->X, p->x, field_bytes);
  os_memcpy(r->Y, p->y, field_bytes);
  os_memcpy(r->Z, field_one, field_bytes);
  return;
}

void projective_to_affine(affine *r, const group *p) {
  if (field_eq(p->Z, field_zero)) {
    os_memcpy(r->x, field_zero, field_bytes);
    os_memcpy(r->y, field_zero, field_bytes);
    return;
  }

  field zi, zi2, zi3;
  field_inv(zi, p->Z);        // 1/Z
  field_mul(zi2, zi, zi);     // 1/Z^2
  field_mul(zi3, zi2, zi);    // 1/Z^3
  field_mul(r->x, p->X, zi2); // X/Z^2
  field_mul(r->y, p->Y, zi3); // Y/Z^3
  return;
}


// https://www.hyperelliptic.org/EFD/g1p/auto-code/shortw/jacobian-0/doubling/dbl-2009-l.op3
// cost 2M + 5S + 6add + 3*2 + 1*3 + 1*8
void group_dbl(group *r, const group *p) {
  if (is_zero(p)) {
    *r = *p;
    return;
  }

  field a, b, c;
  field_sq(a, p->X);                // a = X1^2
  field_sq(b, p->Y);                // b = Y1^2
  field_sq(c, b);                   // c = b^2

  field d, e, f;
  field_add(r->X, p->X, b);         // t0 = X1 + b
  field_sq(r->Y, r->X);             // t1 = t0^2
  field_sub(r->Z, r->Y, a);         // t2 = t1 - a
  field_sub(r->X, r->Z, c);         // t3 = t2 - c
  field_add(d, r->X, r->X);         // d = 2 * t3
  field_mul(e, field_three, a);     // e = 3 * a
  field_sq(f, e);                   // f = e^2

  field_add(r->Y, d, d);            // t4 = 2 * d
  field_sub(r->X, f, r->Y);         // X = f - t4

  field_sub(r->Y, d, r->X);         // t5 = d - X
  field_mul(f, field_eight, c);     // t6 = 8 * c
  field_mul(r->Z, e, r->Y);         // t7 = e * t5
  field_sub(r->Y, r->Z, f);         // Y = t7 - t6

  field_mul(f, p->Y, p->Z);         // t8 = Y1 * Z1
  field_add(r->Z, f, f);            // Z = 2 * t8
}

// https://www.hyperelliptic.org/EFD/g1p/auto-code/shortw/jacobian-0/addition/add-2007-bl.op3
// cost 11M + 5S + 9add + 4*2
void group_add(group *r, const group *p, const group *q) {

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

  field z1z1, z2z2;
  field_sq(z1z1, p->Z);         // Z1Z1 = Z1^2
  field_sq(z2z2, q->Z);         // Z2Z2 = Z2^2

  field u1, u2, s1, s2;
  field_mul(u1, p->X, z2z2);    // u1 = x1 * z2z2
  field_mul(u2, q->X, z1z1);    // u2 = x2 * z1z1
  field_mul(r->X, q->Z, z2z2);  // t0 = z2 * z2z2
  field_mul(s1, p->Y, r->X);    // s1 = y1 * t0
  field_mul(r->Y, p->Z, z1z1);  // t1 = z1 * z1z1
  field_mul(s2, q->Y, r->Y);    // s2 = y2 * t1

  field h, i, j, w, v;
  field_sub(h, u2, u1);         // h = u2 - u1
  field_add(r->Z, h, h);        // t2 = 2 * h
  field_sq(i, r->Z);            // i = t2^2
  field_mul(j, h, i);           // j = h * i
  field_sub(r->X, s2, s1);      // t3 = s2 - s1
  field_add(w, r->X, r->X);     // w = 2 * t3
  field_mul(v, u1, i);          // v = u1 * i

  // X3 = w^2 - j - 2*v
  field_sq(r->X, w);            // t4 = w^2
  field_add(r->Y, v, v);        // t5 = 2 * v
  field_sub(r->Z, r->X, j);     // t6 = t4 - j
  field_sub(r->X, r->Z, r->Y);  // t6 - t5

  // Y3 = w * (v - X3) - 2*s1*j
  field_sub(r->Y, v, r->X);     // t7 = v - X3
  field_mul(r->Z, s1, j);       // t8 = s1 * j
  field_add(s1, r->Z, r->Z);    // t9 = 2 * t8
  field_mul(r->Z, w, r->Y);     // t10 = w * t7
  field_sub(r->Y, r->Z, s1);    // w * (v - X3) - 2*s1*j

  // Z3 = ((Z1 + Z2)^2 - Z1Z1 - Z2Z2) * h
  field_add(r->Z, p->Z, q->Z);  // t11 = z1 + z2
  field_sq(s1, r->Z);           // t12 = (z1 + z2)^2
  field_sub(r->Z, s1, z1z1);    // t13 = (z1 + z2)^2 - z1z1
  field_sub(j, r->Z, z2z2);     // t14 = (z1 + z2)^2 - z1z1 - z2z2
  field_mul(r->Z, j, h);        // ((z1 + z2)^2 - z1z1 - z2z2) * h
}

// https://www.hyperelliptic.org/EFD/g1p/auto-code/shortw/jacobian-0/addition/madd-2007-bl.op3
// for p = (X1, Y1, Z1), q = (X2, Y2, Z2); assumes Z2 = 1
// cost 7M + 4S + 9add + 3*2 + 1*4 ?
void group_madd(group *r, const group *p, const group *q) {

  if (is_zero(p)) {
    *r = *q;
    return;
  }
  if (is_zero(q)) {
    *r = *p;
    return;
  }

  field z1z1, u2;
  field_sq(z1z1, p->Z);             // z1z1 = Z1^2
  field_mul(u2, q->X, z1z1);        // u2 = X2 * z1z1

  field s2;
  field_mul(r->X, p->Z, z1z1);      // t0 = Z1 * z1z1
  field_mul(s2, q->Y, r->X);        // s2 = Y2 * t0

  field h, hh;
  field_sub(h, u2, p->X);           // h = u2 - X1
  field_sq(hh, h);                  // hh = h^2

  field j, w, v;
  field_mul(r->X, field_four, hh);  // i = 4 * hh
  field_mul(j, h, r->X);            // j = h * i
  field_sub(r->Y, s2, p->Y);        // t1 = s2 - Y1
  field_add(w, r->Y, r->Y);         // w = 2 * t1
  field_mul(v, p->X, r->X);         // v = X1 * i

  // X3 = w^2 - J - 2*V
  field_sq(r->X, w);                // t2 = w^2
  field_add(r->Y, v, v);            // t3 = 2*v
  field_sub(r->Z, r->X, j);         // t4 = t2 - j
  field_sub(r->X, r->Z, r->Y);      // X3 = w^2 - j - 2*v = t4 - t3

  // Y3 = w * (V - X3) - 2*Y1*J
  field_sub(r->Y, v, r->X);         // t5 = v - X3
  field_mul(v, p->Y, j);            // t6 = Y1 * j
  field_add(r->Z, v, v);            // t7 = 2 * t6
  field_mul(s2, w, r->Y);           // t8 = w * t5
  field_sub(r->Y, s2, r->Z);        // w * (v - X3) - 2*Y1*j = t8 - t7

  // Z3 = (Z1 + H)^2 - Z1Z1 - HH
  field_add(w, p->Z, h);            // t9 = Z1 + h
  field_sq(v, w);                   // t10 = t9^2
  field_sub(w, v, z1z1);            // t11 = t10 - z1z1
  field_sub(r->Z, w, hh);           // (Z1 + h)^2 - Z1Z1 - hh = t11 - hh
}

void group_scalar_mul(group *r, const scalar k, const group *p) {

  *r = group_zero;
  if (is_zero(p)) {
    return;
  }
  if (scalar_eq(k, scalar_zero)) {
    return;
  }
  group r1 = *p;
  for (unsigned int i = scalar_offset; i < scalar_bits; i++) {
    unsigned int di = k[i / 8] & (1 << (7 - (i % 8)));
    group q0;
    if (di == 0) {
      group_add(&q0, r, &r1); // r1 = r0 + r1
      r1 = q0;
      group_dbl(&q0, r);      // r0 = r0 + r0
      *r = q0;
    } else {
      group_add(&q0, r, &r1); // r0 = r0 + r1
      *r = q0;
      group_dbl(&q0, &r1);    // r1 = r1 + r1
      r1 = q0;
    }
  }
  return;
}

void affine_scalar_mul(affine *r, const scalar k, const affine *p) {
  group pp, pr;
  affine_to_projective(&pp, p);
  group_scalar_mul(&pr, k, &pp);
  projective_to_affine(r, &pr);
  return;
}

// Ledger uses:
// - BIP 39 to generate and interpret the master seed, which
//   produces the 24 words shown on the device at startup.
// - BIP 32 for HD key derivation (using the child key derivation function)
// - BIP 44 for HD account derivation (so e.g. btc and coda keys don't clash)

void generate_keypair(unsigned int index, affine *pub_key, scalar priv_key) {

  unsigned int bip32_path[5];
  unsigned char chain[32];

  // bip32_path[0] = 44' because we use BIP/SLIP44, [1] = 49370' = 0xc0da
  // these must match those found in Makefile
  bip32_path[0] = 44 | 0x80000000;
  bip32_path[1] = 49370 | 0x80000000;
  bip32_path[2] = index | 0x80000000;
  bip32_path[3] = 0;
  bip32_path[4] = 0;

  os_perso_derive_node_bip32(CX_CURVE_256K1, bip32_path,
                             sizeof(bip32_path) / sizeof(bip32_path[0]),
                             priv_key, chain);
  os_memcpy(priv_key + 32, chain, 16);
  affine_scalar_mul(pub_key, priv_key, &affine_one);
  return;
}

void generate_pubkey(affine *pub_key, const scalar priv_key) {
  affine_scalar_mul(pub_key, priv_key, &affine_one);
  return;
}

inline unsigned int is_odd(const field y) { return (y[field_bytes - 1] & 1); }

void schnorr_hash(scalar out, const scalar in0, const scalar in1,
                  const scalar in2, const scalar in3, const scalar in4) {

  /*
  // state after applying {{0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x65,
  0x72, 0x75, 0x74, 0x61, 0x6e, 0x67, 0x69, 0x53, 0x61, 0x64, 0x6f, 0x43},
  {0}};
  */

  state pos = {
      {0x10, 0x3b, 0x9c, 0x65, 0x52, 0x8d, 0x48, 0xea,
       0xc5, 0x1d, 0x8f, 0xda, 0x9a, 0xb0, 0xf6, 0x24,
       0x75, 0x2b, 0x80, 0x22, 0xf9, 0x1a, 0x35, 0x23,
       0x40, 0xf0, 0xb4, 0x06, 0x6e, 0xbe, 0x4d, 0x56, },
      {0x09, 0xd3, 0xb2, 0x79, 0x8b, 0x0b, 0xdd, 0x9f,
       0x81, 0xfb, 0x4c, 0x7a, 0xaa, 0xd1, 0x2d, 0x82,
       0x8e, 0x78, 0xb2, 0x71, 0x6d, 0xc2, 0x5b, 0x4f,
       0x6c, 0x36, 0x42, 0x0b, 0xac, 0x75, 0xc6, 0xef, },
      {0x08, 0x03, 0x68, 0xc9, 0xb0, 0x6f, 0x76, 0xe2,
       0xb1, 0x6b, 0xb5, 0x7c, 0x24, 0x43, 0xef, 0x5b,
       0x5b, 0xf1, 0x62, 0x39, 0x47, 0x95, 0x2b, 0xca,
       0xbb, 0xc1, 0x73, 0x82, 0xa7, 0x5e, 0x44, 0x47, }};
  poseidon_2in(pos, in0, in1);
  poseidon_2in(pos, in2, in3);
  poseidon_1in(pos, in4);
  poseidon_digest(out, pos);
  return;
}

void sign(field rx, scalar s, const affine *public_key,
          const scalar private_key, const scalar msgx, const scalar msgm) {
  scalar k_prime;
  /* rx is G_io_apdu_buffer so we can take 192 bytes from it */
  {
    affine *r;
    r = (affine *)rx;
    schnorr_hash(k_prime, msgx, msgm, public_key->x, public_key->y,
                 private_key);                    // k = hash(m || pkx || pky || sk)
    affine_scalar_mul(r, k_prime, &affine_one);   // r = k*g

    if (is_odd(r->y)) {
      scalar_sub(k_prime, group_order, k_prime);  // if ry is odd, k = - k'
    }
    /* store so we don't need affine *r anymore */
    os_memcpy(rx, r->x, field_bytes);
  }
  schnorr_hash(s, msgx, public_key->x, public_key->y, rx,
               msgm);                             // e = hash(x || pkx || pky || xr || m)
  os_memcpy(s, scalar_zero, (scalar_bytes - 16)); // use 128 LSB (128/8 = 16) as challenge
  scalar_mul(s, s, private_key);                  // e*sk
  scalar_add(s, k_prime, s);                      // k + e*sk
  return;
}
