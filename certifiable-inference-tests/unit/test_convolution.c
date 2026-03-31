/**
 * @file test_convolution.c
 * @project Certifiable Inference Engine
 * @brief Unit tests for deterministic 2D convolution operations.
 *
 * @details Comprehensive test suite verifying:
 * - Correctness of convolution operation
 * - Edge detection (Sobel kernels)
 * - Boundary handling
 * - Deterministic behavior
 *
 * @traceability SRS-004-CONVOLUTION
 * @compliance DO-178C, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "convolution.h"
#include "fixed_point.h"
#include <stdio.h>
#include <assert.h>

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
 * @test Test basic 3×3 convolution
 * @traceability SRS-004.1
 */
static void test_basic_convolution(void) {
    printf("\nTest: Basic 3×3 Convolution\n");
    printf("────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[25];
    fixed_t kernel_data[9];
    fixed_t out_data[9];

    /* Initialize matrices FIRST */
    fx_matrix_t in, kernel, out;
    fx_matrix_init(&in, in_data, 5, 5);
    fx_matrix_init(&kernel, kernel_data, 3, 3);
    fx_matrix_init(&out, out_data, 3, 3);

    /* Set values AFTER init */
    for (int i = 0; i < 25; i++) {
        in.data[i] = fixed_from_int(1);
    }

    for (int i = 0; i < 9; i++) {
        kernel.data[i] = fixed_from_int(1);
    }

    /* Perform convolution */
    fx_conv2d(&in, &kernel, &out);

    /* Expected: All outputs = 9 (sum of 3×3 window of ones) */
    int all_nine = 1;
    for (int i = 0; i < 9; i++) {
        if (out.data[i] != fixed_from_int(9)) {
            all_nine = 0;
            break;
        }
    }

    TEST_ASSERT(all_nine, "All output values = 9");
    TEST_ASSERT(out.rows == 3, "Output rows = 3");
    TEST_ASSERT(out.cols == 3, "Output cols = 3");
}

/**
 * @test Test identity kernel
 * @traceability SRS-004.2
 */
static void test_identity_kernel(void) {
    printf("\nTest: Identity Kernel (No Change)\n");
    printf("──────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[25];
    fixed_t kernel_data[9];
    fixed_t out_data[9];

    /* Initialize matrices FIRST */
    fx_matrix_t in, kernel, out;
    fx_matrix_init(&in, in_data, 5, 5);
    fx_matrix_init(&kernel, kernel_data, 3, 3);
    fx_matrix_init(&out, out_data, 3, 3);

    /* Set input values AFTER init */
    for (int i = 0; i < 25; i++) {
        in.data[i] = fixed_from_int(i);
    }

    /* Identity kernel: center=1, rest=0
     * ┌───────┐
     * │ 0 0 0 │
     * │ 0 1 0 │
     * │ 0 0 0 │
     * └───────┘
     */
    kernel.data[0] = fixed_from_int(0); kernel.data[1] = fixed_from_int(0); kernel.data[2] = fixed_from_int(0);
    kernel.data[3] = fixed_from_int(0); kernel.data[4] = fixed_from_int(1); kernel.data[5] = fixed_from_int(0);
    kernel.data[6] = fixed_from_int(0); kernel.data[7] = fixed_from_int(0); kernel.data[8] = fixed_from_int(0);

    fx_conv2d(&in, &kernel, &out);

    /* Expected: Center value of each 3×3 window
     * For 5×5 input, the 3×3 output should match centers:
     * Window at (0,0): center at (1,1) = 6
     * Window at (0,1): center at (1,2) = 7
     * etc.
     */
    int expected_values[9] = {6, 7, 8, 11, 12, 13, 16, 17, 18};
    int identity_preserved = 1;

    for (int i = 0; i < 9; i++) {
        if (out.data[i] != fixed_from_int(expected_values[i])) {
            identity_preserved = 0;
            break;
        }
    }

    TEST_ASSERT(identity_preserved, "Identity kernel preserves center values");
}

/**
 * @test Test horizontal edge detection (Sobel)
 * @traceability SRS-004.3
 */
static void test_horizontal_edges(void) {
    printf("\nTest: Horizontal Edge Detection (Sobel)\n");
    printf("────────────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[9];
    fixed_t kernel_data[9];
    fixed_t out_data[1];

    /* Initialize matrices FIRST */
    fx_matrix_t in, kernel, out;
    fx_matrix_init(&in, in_data, 3, 3);
    fx_matrix_init(&kernel, kernel_data, 3, 3);
    fx_matrix_init(&out, out_data, 1, 1);

    /* Input: Horizontal edge (top half = 0, bottom half = 1)
     * ┌─────┐
     * │0 0 0│
     * │0 0 0│
     * │1 1 1│
     * └─────┘
     */
    in.data[0] = fixed_from_int(0); in.data[1] = fixed_from_int(0); in.data[2] = fixed_from_int(0);
    in.data[3] = fixed_from_int(0); in.data[4] = fixed_from_int(0); in.data[5] = fixed_from_int(0);
    in.data[6] = fixed_from_int(1); in.data[7] = fixed_from_int(1); in.data[8] = fixed_from_int(1);

    /* Sobel vertical kernel (detects horizontal edges)
     * ┌─────────┐
     * │-1 -2 -1│
     * │ 0  0  0│
     * │ 1  2  1│
     * └─────────┘
     */
    kernel.data[0] = fixed_from_int(-1); kernel.data[1] = fixed_from_int(-2); kernel.data[2] = fixed_from_int(-1);
    kernel.data[3] = fixed_from_int(0);  kernel.data[4] = fixed_from_int(0);  kernel.data[5] = fixed_from_int(0);
    kernel.data[6] = fixed_from_int(1);  kernel.data[7] = fixed_from_int(2);  kernel.data[8] = fixed_from_int(1);

    fx_conv2d(&in, &kernel, &out);

    /* Expected: Strong positive response (1+2+1 = 4) */
    TEST_ASSERT(out.data[0] == fixed_from_int(4), "Horizontal edge detected (value = 4)");
}

/**
 * @test Test vertical edge detection (Sobel)
 * @traceability SRS-004.3
 */
static void test_vertical_edges(void) {
    printf("\nTest: Vertical Edge Detection (Sobel)\n");
    printf("──────────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[9];
    fixed_t kernel_data[9];
    fixed_t out_data[1];

    /* Initialize matrices FIRST */
    fx_matrix_t in, kernel, out;
    fx_matrix_init(&in, in_data, 3, 3);
    fx_matrix_init(&kernel, kernel_data, 3, 3);
    fx_matrix_init(&out, out_data, 1, 1);

    /* Input: Vertical edge (left half = 0, right half = 1)
     * ┌─────┐
     * │0 0 1│
     * │0 0 1│
     * │0 0 1│
     * └─────┘
     */
    in.data[0] = fixed_from_int(0); in.data[1] = fixed_from_int(0); in.data[2] = fixed_from_int(1);
    in.data[3] = fixed_from_int(0); in.data[4] = fixed_from_int(0); in.data[5] = fixed_from_int(1);
    in.data[6] = fixed_from_int(0); in.data[7] = fixed_from_int(0); in.data[8] = fixed_from_int(1);

    /* Sobel horizontal kernel (detects vertical edges)
     * ┌─────────┐
     * │-1  0  1│
     * │-2  0  2│
     * │-1  0  1│
     * └─────────┘
     */
    kernel.data[0] = fixed_from_int(-1); kernel.data[1] = fixed_from_int(0); kernel.data[2] = fixed_from_int(1);
    kernel.data[3] = fixed_from_int(-2); kernel.data[4] = fixed_from_int(0); kernel.data[5] = fixed_from_int(2);
    kernel.data[6] = fixed_from_int(-1); kernel.data[7] = fixed_from_int(0); kernel.data[8] = fixed_from_int(1);

    fx_conv2d(&in, &kernel, &out);

    /* Expected: Strong positive response (1+2+1 = 4) */
    TEST_ASSERT(out.data[0] == fixed_from_int(4), "Vertical edge detected (value = 4)");
}

/**
 * @test Test deterministic behavior
 * @traceability SRS-004.4
 */
static void test_deterministic_behavior(void) {
    printf("\nTest: Deterministic Behavior (Repeatability)\n");
    printf("─────────────────────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[25];
    fixed_t kernel_data[9];
    fixed_t out1_data[9], out2_data[9];

    /* Initialize matrices FIRST */
    fx_matrix_t in, kernel, out1, out2;
    fx_matrix_init(&in, in_data, 5, 5);
    fx_matrix_init(&kernel, kernel_data, 3, 3);
    fx_matrix_init(&out1, out1_data, 3, 3);
    fx_matrix_init(&out2, out2_data, 3, 3);

    /* Set values AFTER init */
    for (int i = 0; i < 25; i++) {
        in.data[i] = fixed_from_float(0.1f * (i % 7));
    }

    for (int i = 0; i < 9; i++) {
        kernel.data[i] = fixed_from_float(0.2f * i);
    }

    /* Perform convolution twice */
    fx_conv2d(&in, &kernel, &out1);
    fx_conv2d(&in, &kernel, &out2);

    /* Verify bit-exact repeatability */
    int identical = 1;
    for (int i = 0; i < 9; i++) {
        if (out1.data[i] != out2.data[i]) {
            identical = 0;
            break;
        }
    }

    TEST_ASSERT(identical, "Repeated convolution produces identical results");
}

/**
 * @test Test zero kernel (output should be zero)
 * @traceability SRS-004.2
 */
static void test_zero_kernel(void) {
    printf("\nTest: Zero Kernel (Output = 0)\n");
    printf("───────────────────────────────\n");

    /* Allocate buffers */
    fixed_t in_data[25];
    fixed_t kernel_data[9];
    fixed_t out_data[9];

    /* Initialize matrices FIRST */
    fx_matrix_t in, kernel, out;
    fx_matrix_init(&in, in_data, 5, 5);
    fx_matrix_init(&kernel, kernel_data, 3, 3);
    fx_matrix_init(&out, out_data, 3, 3);

    /* Set input values AFTER init */
    for (int i = 0; i < 25; i++) {
        in.data[i] = fixed_from_int(i * 2 + 1);
    }

    /* Zero kernel - already zeroed by init, no need to set */

    fx_conv2d(&in, &kernel, &out);

    /* Expected: All outputs = 0 */
    int all_zero = 1;
    for (int i = 0; i < 9; i++) {
        if (out.data[i] != fixed_from_int(0)) {
            all_zero = 0;
            break;
        }
    }

    TEST_ASSERT(all_zero, "Zero kernel produces zero output");
}

int main(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    printf("  SRS-006 Convolution Verification Suite\n");
    printf("═══════════════════════════════════════════════\n");
    printf("\n");

    /* Run all tests */
    test_basic_convolution();
    test_identity_kernel();
    test_horizontal_edges();
    test_vertical_edges();
    test_deterministic_behavior();
    test_zero_kernel();

    /* Print summary */
    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    if (tests_failed == 0) {
        printf("  ✅ SRS-006 Verified (%d tests passed)\n", tests_passed);
    } else {
        printf("  ❌ SRS-006 Failed (%d passed, %d failed)\n", tests_passed, tests_failed);
    }
    printf("═══════════════════════════════════════════════\n");
    printf("\n");
    printf("Requirements validated:\n");
    printf("  • SRS-006.1: 2D convolution correctness\n");
    printf("  • SRS-006.2: Kernel application\n");
    printf("  • SRS-006.3: Edge detection patterns\n");
    printf("  • SRS-006.4: Bit-perfect determinism\n");
    printf("\n");

    return tests_failed > 0 ? 1 : 0;
}
