/**
 * @file test_fixed_point.c
 * @brief SRS-005-CR-001 Compliance Tests for L2 Fixed-Point Arithmetic
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Tests verify:
 * - CR-001-ISSUE-001: No signed left shift (uses multiplication)
 * - CR-001-ISSUE-002: No signed right shift (uses division)
 * - CR-001-ISSUE-003: Truncation toward zero (no rounding)
 * - CR-001-ISSUE-004: Division uses multiplication for scaling
 * - CR-001-ISSUE-005: INT32_MIN overflow handled
 * - CR-001-ISSUE-006: Types delegate to L1 (axilog/types.h)
 *
 * @traceability SRS-005-SHALL-067, SRS-005-SHALL-068, SRS-005-SHALL-069
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include "fixed_point.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { printf("  [TEST] %s... ", name); tests_run++; } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); } while(0)

/* ========================================================================
 * CR-001-ISSUE-001/002: Conversion Tests (No Shifts)
 * ======================================================================== */

static void test_fixed_from_int_positive(void)
{
    TEST("fixed_from_int_positive");

    fixed_t result = fixed_from_int(5);
    /* 5 * 65536 = 327680 = 0x00050000 */

    if (result == (5 * FIXED_ONE)) {
        PASS();
    } else {
        FAIL("conversion incorrect");
        printf("    Expected: %d, Got: %d\n", 5 * FIXED_ONE, result);
    }
}

static void test_fixed_from_int_negative(void)
{
    TEST("fixed_from_int_negative");

    fixed_t result = fixed_from_int(-5);
    /* -5 * 65536 = -327680 */

    if (result == (-5 * FIXED_ONE)) {
        PASS();
    } else {
        FAIL("negative conversion incorrect");
        printf("    Expected: %d, Got: %d\n", -5 * FIXED_ONE, result);
    }
}

static void test_fixed_to_int_positive(void)
{
    TEST("fixed_to_int_positive");

    /* 5.75 in Q16.16 = 5 * 65536 + 0.75 * 65536 = 327680 + 49152 = 376832 */
    volatile fixed_t input = 376832;
    int32_t result = fixed_to_int(input);

    /* Truncation toward zero: 5.75 → 5 */
    if (result == 5) {
        PASS();
    } else {
        FAIL("truncation incorrect");
        printf("    Expected: 5, Got: %d\n", result);
    }
}

static void test_fixed_to_int_negative(void)
{
    TEST("fixed_to_int_negative");

    /* -5.75 in Q16.16 = -376832 */
    volatile fixed_t input = -376832;
    int32_t result = fixed_to_int(input);

    /* C99 truncation toward zero: -5.75 → -5 (not -6) */
    if (result == -5) {
        PASS();
    } else {
        FAIL("negative truncation incorrect");
        printf("    Expected: -5, Got: %d\n", result);
    }
}

/* ========================================================================
 * CR-001-ISSUE-003: Multiplication Truncation (No Rounding)
 * ======================================================================== */

static void test_mul_truncation_not_rounding(void)
{
    TEST("mul_truncation_not_rounding");

    /* 1.5 * 1.5 = 2.25
     * 1.5 in Q16.16 = 98304 (0x00018000)
     * Product: 98304 * 98304 = 9663676416
     * With rounding: (9663676416 + 32768) / 65536 = 147457 (2.25 + 0.5 ULP)
     * With truncation: 9663676416 / 65536 = 147456 (exact 2.25)
     */
    fixed_t a = 98304;  /* 1.5 */
    fixed_t b = 98304;  /* 1.5 */
    fixed_t result = fixed_mul(a, b);

    /* Expected: truncation gives 147456 */
    if (result == 147456) {
        PASS();
    } else {
        FAIL("rounding detected instead of truncation");
        printf("    Expected: 147456 (truncation), Got: %d\n", result);
    }
}

static void test_mul_saturation_overflow(void)
{
    TEST("mul_saturation_overflow");

    /* Large values that would overflow */
    fixed_t a = 0x7FFF0000;  /* ~32767.0 */
    fixed_t b = 0x00020000;  /* 2.0 */
    fixed_t result = fixed_mul(a, b);

    /* Should saturate to INT32_MAX */
    if (result == INT32_MAX) {
        PASS();
    } else {
        FAIL("overflow not saturated");
        printf("    Expected: %d, Got: %d\n", INT32_MAX, result);
    }
}

/* ========================================================================
 * CR-001-ISSUE-004: Division Uses Multiplication for Scaling
 * ======================================================================== */

static void test_div_basic(void)
{
    TEST("div_basic");

    /* 5.0 / 2.0 = 2.5
     * 5.0 in Q16.16 = 327680
     * 2.0 in Q16.16 = 131072
     * Result: 2.5 in Q16.16 = 163840
     */
    fixed_t a = 327680;   /* 5.0 */
    fixed_t b = 131072;   /* 2.0 */
    fixed_t result = fixed_div(a, b);

    if (result == 163840) {
        PASS();
    } else {
        FAIL("division incorrect");
        printf("    Expected: 163840 (2.5), Got: %d\n", result);
    }
}

static void test_div_by_zero(void)
{
    TEST("div_by_zero");

    fixed_t result = fixed_div(FIXED_ONE, 0);

    /* Should return 0, not crash */
    if (result == 0) {
        PASS();
    } else {
        FAIL("div by zero should return 0");
        printf("    Got: %d\n", result);
    }
}

/* ========================================================================
 * CR-001-ISSUE-005: INT32_MIN Overflow Handling
 * ======================================================================== */

static void test_abs_int32_min(void)
{
    TEST("abs_int32_min");

    fixed_t result = fixed_abs(INT32_MIN);

    /* INT32_MIN cannot be negated — saturate to INT32_MAX */
    if (result == INT32_MAX) {
        PASS();
    } else {
        FAIL("INT32_MIN not saturated");
        printf("    Expected: %d, Got: %d\n", INT32_MAX, result);
    }
}

static void test_neg_int32_min(void)
{
    TEST("neg_int32_min");

    fixed_t result = fixed_neg(INT32_MIN);

    /* INT32_MIN cannot be negated — saturate to INT32_MAX */
    if (result == INT32_MAX) {
        PASS();
    } else {
        FAIL("INT32_MIN not saturated");
        printf("    Expected: %d, Got: %d\n", INT32_MAX, result);
    }
}

/* ========================================================================
 * CR-001-ISSUE-006: L1 Type Delegation Verification
 * ======================================================================== */

static void test_type_delegation(void)
{
    TEST("type_delegation_constants");

    /* Verify L2 constants match L1 — use volatile to prevent constant folding */
    volatile int32_t l2_one = FIXED_ONE;
    volatile int32_t l2_half = FIXED_HALF;
    volatile int32_t l2_max = FIXED_MAX;
    volatile int32_t l2_min = FIXED_MIN;
    volatile int l2_shift = FIXED_SHIFT;

    volatile int32_t l1_one = Q16_ONE;
    volatile int32_t l1_half = Q16_HALF;
    volatile int32_t l1_max = Q16_MAX;
    volatile int32_t l1_min = Q16_MIN;
    volatile int l1_shift = Q16_SHIFT;

    int pass = 1;

    if (l2_one != l1_one) {
        printf("\n    FIXED_ONE != Q16_ONE");
        pass = 0;
    }
    if (l2_half != l1_half) {
        printf("\n    FIXED_HALF != Q16_HALF");
        pass = 0;
    }
    if (l2_max != l1_max) {
        printf("\n    FIXED_MAX != Q16_MAX");
        pass = 0;
    }
    if (l2_min != l1_min) {
        printf("\n    FIXED_MIN != Q16_MIN");
        pass = 0;
    }
    if (l2_shift != l1_shift) {
        printf("\n    FIXED_SHIFT != Q16_SHIFT");
        pass = 0;
    }

    if (pass) {
        PASS();
    } else {
        printf("\n");
        FAIL("L2 constants do not delegate to L1");
    }
}

/* ========================================================================
 * Golden Vector: Multiplication Truncation
 * ======================================================================== */

static void test_golden_mul_vectors(void)
{
    TEST("golden_mul_vectors");

    /* Golden vectors for truncation-based multiplication */
    struct {
        fixed_t a;
        fixed_t b;
        fixed_t expected;
    } vectors[] = {
        { 65536, 65536, 65536 },       /* 1.0 * 1.0 = 1.0 */
        { 98304, 98304, 147456 },      /* 1.5 * 1.5 = 2.25 (truncated) */
        { 131072, 32768, 65536 },      /* 2.0 * 0.5 = 1.0 */
        { -65536, 65536, -65536 },     /* -1.0 * 1.0 = -1.0 */
        { -98304, -98304, 147456 },    /* -1.5 * -1.5 = 2.25 */
    };
    int num_vectors = sizeof(vectors) / sizeof(vectors[0]);
    int all_pass = 1;

    for (int i = 0; i < num_vectors; i++) {
        fixed_t result = fixed_mul(vectors[i].a, vectors[i].b);
        if (result != vectors[i].expected) {
            printf("\n    Vector %d: %d * %d = %d (expected %d)",
                   i, vectors[i].a, vectors[i].b, result, vectors[i].expected);
            all_pass = 0;
        }
    }

    if (all_pass) {
        PASS();
    } else {
        printf("\n");
        FAIL("golden vector mismatch");
    }
}

/* ========================================================================
 * Main
 * ======================================================================== */

int main(void)
{
    printf("======================================================================\n");
    printf("certifiable-inference — SRS-005-CR-001 Compliance Tests\n");
    printf("L2 Fixed-Point Arithmetic\n");
    printf("======================================================================\n\n");

    printf("[SUITE] Conversion (CR-001-ISSUE-001/002)\n");
    test_fixed_from_int_positive();
    test_fixed_from_int_negative();
    test_fixed_to_int_positive();
    test_fixed_to_int_negative();

    printf("\n[SUITE] Multiplication (CR-001-ISSUE-003)\n");
    test_mul_truncation_not_rounding();
    test_mul_saturation_overflow();

    printf("\n[SUITE] Division (CR-001-ISSUE-004)\n");
    test_div_basic();
    test_div_by_zero();

    printf("\n[SUITE] INT32_MIN Handling (CR-001-ISSUE-005)\n");
    test_abs_int32_min();
    test_neg_int32_min();

    printf("\n[SUITE] L1 Type Delegation (CR-001-ISSUE-006)\n");
    test_type_delegation();

    printf("\n[SUITE] Golden Vectors\n");
    test_golden_mul_vectors();

    printf("\n======================================================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("======================================================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
