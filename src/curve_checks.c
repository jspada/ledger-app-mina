// Mina elliptic cruve unit tests
//
//    Imported from https://github.com/MinaProtocol/c-reference-signer/blob/master/curve_checks.c

#include <string.h>

#include "curve_checks.h"

bool curve_checks(void)
{
    Affine a3;
    Affine a4;
    union {
        // Fit in stackspace!
        Affine a5;
        Scalar s2;
    } u;

    for (size_t i = 0; i < EPOCHS; i++) {
        // Test1: On curve after scaling
        if (!affine_is_on_curve(&A[i][0])) {
            return false;
        }
        if (!affine_is_on_curve(&A[i][1])) {
            return false;
        }
        if (!affine_is_on_curve(&A[i][2])) {
            return false;
        }

        // Test2: Addition is commutative
        //     A0 + A1 == A1 + A0
        affine_add(&a3, &A[i][0], &A[i][1]); // a3 = A0 + A1
        affine_add(&a4, &A[i][1], &A[i][0]); // a4 = A1 + A0
        if (!affine_eq(&a3, &a4)) {
            return false;
        }
        if (!affine_is_on_curve(&a3)) {
            return false;
        }
        // Test target check: a3 == T0
        if (memcmp(&a3, &T[i][0], sizeof(a3)) != 0) {
            return false;
        }

        // Test3: Scaling commutes with adding scalars
        //     G*(S0 + S1) == G*S0 + G*S1
        scalar_add(u.s2, S[i][0], S[i][1]);
        generate_pubkey(&a3, u.s2);          // a3 = G*(S0 + S1)
        affine_add(&a4, &A[i][0], &A[i][1]); // a4 = G*S0 + G*S1
        if (!affine_eq(&a3, &a4)) {
            return false;
        }
        if (!affine_is_on_curve(&a3)) {
            return false;
        }
        // Test target check: a3 == T1
        if (memcmp(&a3, &T[i][1], sizeof(a3)) != 0) {
            return false;
        }

        // Test4: Scaling commutes with multiplying scalars
        //    G*(S0*S1) == S0*(G*S1)
        scalar_mul(u.s2, S[i][0], S[i][1]);
        generate_pubkey(&a3, u.s2);                // a3 = G*(S0*S1)
        affine_scalar_mul(&a4, S[i][0], &A[i][1]); // a4 = S0*(G*S1)
        if (!affine_eq(&a3, &a4)) {
            return false;
        }
        if (!affine_is_on_curve(&a3)) {
            return false;
        }
        // Test target check: a3 == T2
        if (memcmp(&a3, &T[i][2], sizeof(a3)) != 0) {
            return false;
        }

        // Test5: Scaling commutes with negation
        //    G*(-S0) == -(G*S0)
        scalar_negate(u.s2, S[i][0]);
        generate_pubkey(&a3, u.s2);   // a3 = G*(-S0)
        affine_negate(&a4, &A[i][0]); // a4 = -(G*S0)
        if (!affine_eq(&a3, &a4)) {
            return false;
        }
        if (!affine_is_on_curve(&a3)) {
            return false;
        }
        // Test target check: a3 == T3
        if (memcmp(&a3, &T[i][3], sizeof(a3)) != 0) {
            return false;
        }

        // Test6: Addition is associative
        //     (A0 + A1) + A2 == A0 + (A1 + A2)
        affine_add(&a3, &A[i][0], &A[i][1]);
        affine_add(&a4, &a3, &A[i][2]);      // a4 = (A0 + A1) + A2
        affine_add(&a3, &A[i][1], &A[i][2]);
        affine_add(&u.a5, &A[i][0], &a3);    // a5 = A0 + (A1 + A2)
        if (!affine_eq(&a4, &u.a5)) {
            return false;
        }
        if (!affine_is_on_curve(&a4)) {
            return false;
        }
        // Test target check: a4 == T4
        if (memcmp(&a4, &T[i][4], sizeof(a4)) != 0) {
            return false;
        }
    }

    return true;
}
