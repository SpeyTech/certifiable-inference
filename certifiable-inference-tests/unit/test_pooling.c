/**
 * @file test_pooling.c
 * @project Certifiable Inference Engine
 * @brief Unit tests for deterministic max pooling operations.
 *
 * @details Comprehensive test suite verifying:
 * - Correctness of max selection
 * - Dimension reduction
 * - Boundary value handling
 * - Deterministic behavior
 *
 * @traceability SRS-008-POOLING
 * @compliance DO-178C, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "pooling.h"
#include "fixed_point.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Test counter */
static int tests_passed = 0;
static int tests_failed = 0;

/* Test result macro */
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  ✗ FAILED: %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

/**
 * @test Test basic 4×4 → 2×2 max pooling
 * @traceability SRS-008.1
 */
static void test_basic_maxpool(void) {
    printf("\nTest: Basic 4×4 Max Pooling\n");
    printf("──────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[16];
    fixed_t out_data[4];

    /* Initialize matrices FIRST */
    fx_matrix_t in, out;
    fx_matrix_init(&in, in_data, 4, 4);
    fx_matrix_init(&out, out_data, 2, 2);

    /* Set values AFTER init
     * ┌──────────┐
     * │ 1  2│ 3  4│
     * │ 5  6│ 7  8│
     * ├──────────┤
     * │ 9 10│11 12│
     * │13 14│15 16│
     * └──────────┘
     */
    for (int i = 0; i < 16; i++) {
        in.data[i] = fixed_from_int(i + 1);
    }

    /* Perform pooling */
    fx_maxpool_2x2(&in, &out);

    /* Expected output: 2×2 matrix
     * ┌─────┐
     * │ 6  8│  (max of each 2×2 quadrant)
     * │14 16│
     * └─────┘
     */
    TEST_ASSERT(out.data[0] == fixed_from_int(6),  "Top-left max = 6");
    TEST_ASSERT(out.data[1] == fixed_from_int(8),  "Top-right max = 8");
    TEST_ASSERT(out.data[2] == fixed_from_int(14), "Bottom-left max = 14");
    TEST_ASSERT(out.data[3] == fixed_from_int(16), "Bottom-right max = 16");
}

/**
 * @test Test uniform input (all same values)
 * @traceability SRS-008.2
 */
static void test_uniform_input(void) {
    printf("\nTest: Uniform Input (Identity Property)\n");
    printf("────────────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[16];
    fixed_t out_data[4];

    /* Initialize matrices FIRST */
    fx_matrix_t in, out;
    fx_matrix_init(&in, in_data, 4, 4);
    fx_matrix_init(&out, out_data, 2, 2);

    /* Set all elements = 5 AFTER init */
    for (int i = 0; i < 16; i++) {
        in.data[i] = fixed_from_int(5);
    }

    fx_maxpool_2x2(&in, &out);

    /* Expected: All outputs = 5 */
    TEST_ASSERT(out.data[0] == fixed_from_int(5), "Output[0,0] = 5");
    TEST_ASSERT(out.data[1] == fixed_from_int(5), "Output[0,1] = 5");
    TEST_ASSERT(out.data[2] == fixed_from_int(5), "Output[1,0] = 5");
    TEST_ASSERT(out.data[3] == fixed_from_int(5), "Output[1,1] = 5");
}

/**
 * @test Test with negative values
 * @traceability SRS-008.2
 */
static void test_negative_values(void) {
    printf("\nTest: Negative Values Handling\n");
    printf("───────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[16];
    fixed_t out_data[4];

    /* Initialize matrices FIRST */
    fx_matrix_t in, out;
    fx_matrix_init(&in, in_data, 4, 4);
    fx_matrix_init(&out, out_data, 2, 2);

    /* Set values AFTER init: Mix of positive and negative
     * ┌──────────┐
     * │-1 -2│ 1  2│
     * │-3 -4│ 3  4│
     * ├──────────┤
     * │-5 -6│-7 -8│
     * │-9 -10│-11 -12│
     * └──────────┘
     */
    in.data[0]  = fixed_from_int(-1);  in.data[1]  = fixed_from_int(-2);
    in.data[2]  = fixed_from_int(1);   in.data[3]  = fixed_from_int(2);
    in.data[4]  = fixed_from_int(-3);  in.data[5]  = fixed_from_int(-4);
    in.data[6]  = fixed_from_int(3);   in.data[7]  = fixed_from_int(4);
    in.data[8]  = fixed_from_int(-5);  in.data[9]  = fixed_from_int(-6);
    in.data[10] = fixed_from_int(-7);  in.data[11] = fixed_from_int(-8);
    in.data[12] = fixed_from_int(-9);  in.data[13] = fixed_from_int(-10);
    in.data[14] = fixed_from_int(-11); in.data[15] = fixed_from_int(-12);

    fx_maxpool_2x2(&in, &out);

    /* Expected: Max of each quadrant
     * ┌──────┐
     * │-1  4│
     * │-5 -7│
     * └──────┘
     */
    TEST_ASSERT(out.data[0] == fixed_from_int(-1), "Top-left max = -1");
    TEST_ASSERT(out.data[1] == fixed_from_int(4),  "Top-right max = 4");
    TEST_ASSERT(out.data[2] == fixed_from_int(-5), "Bottom-left max = -5");
    TEST_ASSERT(out.data[3] == fixed_from_int(-7), "Bottom-right max = -7");
}

/**
 * @test Test boundary values (min/max fixed-point)
 * @traceability SRS-008.2
 */
static void test_boundary_values(void) {
    printf("\nTest: Boundary Values (Fixed-Point Min/Max)\n");
    printf("───────────────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[16];
    fixed_t out_data[4];

    /* Initialize matrices FIRST */
    fx_matrix_t in, out;
    fx_matrix_init(&in, in_data, 4, 4);
    fx_matrix_init(&out, out_data, 2, 2);

    /* Set values AFTER init: Mix of min and max fixed-point values */
    in.data[0]  = FIXED_MIN; in.data[1]  = FIXED_MIN; in.data[2]  = FIXED_MAX; in.data[3]  = FIXED_MAX;
    in.data[4]  = FIXED_MIN; in.data[5]  = FIXED_MIN; in.data[6]  = FIXED_MAX; in.data[7]  = FIXED_MAX;
    in.data[8]  = FIXED_MIN; in.data[9]  = FIXED_MIN; in.data[10] = FIXED_MIN; in.data[11] = FIXED_MIN;
    in.data[12] = FIXED_MIN; in.data[13] = FIXED_MIN; in.data[14] = FIXED_MIN; in.data[15] = FIXED_MIN;

    fx_maxpool_2x2(&in, &out);

    /* Expected: Max of each quadrant */
    TEST_ASSERT(out.data[0] == FIXED_MIN, "Top-left: all MIN → MIN");
    TEST_ASSERT(out.data[1] == FIXED_MAX, "Top-right: all MAX → MAX");
    TEST_ASSERT(out.data[2] == FIXED_MIN, "Bottom-left: all MIN → MIN");
    TEST_ASSERT(out.data[3] == FIXED_MIN, "Bottom-right: all MIN → MIN");
}

/**
 * @test Test larger dimensions (14×14 → 7×7)
 * @traceability SRS-008.6
 */
static void test_larger_dimensions(void) {
    printf("\nTest: Larger Dimensions (14×14 → 7×7)\n");
    printf("──────────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[196];  /* 14×14 */
    fixed_t out_data[49];  /* 7×7 */

    /* Initialize matrices FIRST */
    fx_matrix_t in, out;
    fx_matrix_init(&in, in_data, 14, 14);
    fx_matrix_init(&out, out_data, 7, 7);

    /* Set values AFTER init: value = row * 14 + col */
    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 14; j++) {
            in.data[i * 14 + j] = fixed_from_int(i * 14 + j);
        }
    }

    fx_maxpool_2x2(&in, &out);

    /* Verify output dimensions */
    TEST_ASSERT(out.rows == 7, "Output rows = 7");
    TEST_ASSERT(out.cols == 7, "Output cols = 7");

    /* Verify corner values */
    /* Top-left 2×2 window: [0,1,14,15] → max = 15 */
    TEST_ASSERT(out.data[0] == fixed_from_int(15), "Top-left corner correct");

    /* Top-right 2×2 window: [12,13,26,27] → max = 27 */
    TEST_ASSERT(out.data[6] == fixed_from_int(27), "Top-right corner correct");

    /* Bottom-right 2×2 window: [180,181,194,195] → max = 195 */
    TEST_ASSERT(out.data[48] == fixed_from_int(195), "Bottom-right corner correct");
}

/**
 * @test Test deterministic behavior (repeated operations)
 * @traceability SRS-008.7
 */
static void test_deterministic_behavior(void) {
    printf("\nTest: Deterministic Behavior (Repeatability)\n");
    printf("─────────────────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[64];   /* 8×8 */
    fixed_t out1_data[16]; /* 4×4 */
    fixed_t out2_data[16];

    /* Initialize matrices FIRST */
    fx_matrix_t in, out1, out2;
    fx_matrix_init(&in, in_data, 8, 8);
    fx_matrix_init(&out1, out1_data, 4, 4);
    fx_matrix_init(&out2, out2_data, 4, 4);

    /* Set values AFTER init */
    for (int i = 0; i < 64; i++) {
        in.data[i] = fixed_from_float(0.1f * (i % 7));
    }

    /* Perform pooling twice */
    fx_maxpool_2x2(&in, &out1);
    fx_maxpool_2x2(&in, &out2);

    /* Verify bit-exact repeatability */
    int identical = 1;
    for (int i = 0; i < 16; i++) {
        if (out1.data[i] != out2.data[i]) {
            identical = 0;
            break;
        }
    }

    TEST_ASSERT(identical, "Repeated pooling produces identical results");
}

/**
 * @test Test range preservation property
 * @traceability SRS-008.2
 */
static void test_range_preservation(void) {
    printf("\nTest: Range Preservation Property\n");
    printf("──────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[16];
    fixed_t out_data[4];

    /* Initialize matrices FIRST */
    fx_matrix_t in, out;
    fx_matrix_init(&in, in_data, 4, 4);
    fx_matrix_init(&out, out_data, 2, 2);

    /* Set values AFTER init */
    int values[16] = {5, 10, 2, 8, 1, 15, 3, 7, 12, 4, 9, 6, 11, 13, 14, 0};
    for (int i = 0; i < 16; i++) {
        in.data[i] = fixed_from_int(values[i]);
    }

    /* Find input min/max */
    fixed_t in_min = in.data[0];
    fixed_t in_max = in.data[0];
    for (int i = 1; i < 16; i++) {
        if (in.data[i] < in_min) in_min = in.data[i];
        if (in.data[i] > in_max) in_max = in.data[i];
    }

    fx_maxpool_2x2(&in, &out);

    /* Verify all outputs within input range */
    int range_preserved = 1;
    for (int i = 0; i < 4; i++) {
        if (out.data[i] < in_min || out.data[i] > in_max) {
            range_preserved = 0;
            break;
        }
    }

    TEST_ASSERT(range_preserved, "All outputs within input range");
    TEST_ASSERT(in_min == fixed_from_int(0), "Input min = 0");
    TEST_ASSERT(in_max == fixed_from_int(15), "Input max = 15");
}

int main(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    printf("  SRS-008 Max Pooling Verification Suite\n");
    printf("═══════════════════════════════════════════════\n");
    printf("\n");

    /* Run all tests */
    test_basic_maxpool();
    test_uniform_input();
    test_negative_values();
    test_boundary_values();
    test_larger_dimensions();
    test_deterministic_behavior();
    test_range_preservation();

    /* Print summary */
    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    if (tests_failed == 0) {
        printf("  ✅ SRS-008 Verified (%d tests passed)\n", tests_passed);
    } else {
        printf("  ❌ SRS-008 Failed (%d passed, %d failed)\n", tests_passed, tests_failed);
    }
    printf("═══════════════════════════════════════════════\n");
    printf("\n");
    printf("Requirements validated:\n");
    printf("  • SRS-008.1: Max pooling correctness\n");
    printf("  • SRS-008.2: Negative value handling\n");
    printf("  • SRS-008.3: Boundary conditions\n");
    printf("  • SRS-008.4: Bit-perfect determinism\n");
    printf("  • SRS-008.5: Range preservation\n");
    printf("\n");

    return tests_failed > 0 ? 1 : 0;
}
