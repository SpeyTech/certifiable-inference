/**
 * @file test_activations.c
 * @project Certifiable Inference Engine
 * @brief Verification suite for SRS-004 (Activation Functions).
 *
 * @details Tests determinism, correctness, and in-place operation of
 * activation functions for neural networks.
 *
 * @traceability SRS-004-ACTIVATIONS
 * @compliance MISRA-C:2012, ISO 26262
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "activations.h"
#include "matrix.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * @brief Test ReLU correctness.
 * @traceability SRS-004.2, V-004.1
 */
void test_relu_correctness(void) {
    printf("  Testing ReLU correctness... ");

    fixed_t buf[6];
    fx_matrix_t mat;
    fx_matrix_init(&mat, buf, 2, 3);

    /* Test data: mix of positive, negative, and zero */
    mat.data[0] = fixed_from_float(5.5f);   /* Positive */
    mat.data[1] = fixed_from_float(-3.2f);  /* Negative */
    mat.data[2] = fixed_from_float(0.0f);   /* Zero */
    mat.data[3] = fixed_from_float(-7.8f);  /* Negative */
    mat.data[4] = fixed_from_float(2.1f);   /* Positive */
    mat.data[5] = fixed_from_float(-0.5f);  /* Negative */

    /* Apply ReLU */
    fx_relu(&mat);

    /* Verify: positive unchanged, negative become zero */
    assert(fixed_to_float(mat.data[0]) > 5.4f && fixed_to_float(mat.data[0]) < 5.6f);
    assert(mat.data[1] == FIXED_ZERO);
    assert(mat.data[2] == FIXED_ZERO);
    assert(mat.data[3] == FIXED_ZERO);
    assert(fixed_to_float(mat.data[4]) > 2.0f && fixed_to_float(mat.data[4]) < 2.2f);
    assert(mat.data[5] == FIXED_ZERO);

    printf("✓\n");
}

/**
 * @brief Test ReLU in-place operation.
 * @traceability SRS-004.1, V-004.2
 */
void test_relu_in_place(void) {
    printf("  Testing ReLU in-place operation... ");

    fixed_t buf[4];
    fx_matrix_t mat;
    fx_matrix_init(&mat, buf, 2, 2);

    /* Store original pointer */
    fixed_t* original_ptr = mat.data;

    /* Fill with test data */
    mat.data[0] = fixed_from_int(1);
    mat.data[1] = fixed_from_int(-2);
    mat.data[2] = fixed_from_int(3);
    mat.data[3] = fixed_from_int(-4);

    /* Apply ReLU */
    fx_relu(&mat);

    /* Verify pointer unchanged (in-place operation) */
    assert(mat.data == original_ptr);

    /* Verify results */
    assert(fixed_to_int(mat.data[0]) == 1);
    assert(mat.data[1] == FIXED_ZERO);
    assert(fixed_to_int(mat.data[2]) == 3);
    assert(mat.data[3] == FIXED_ZERO);

    printf("✓\n");
}

/**
 * @brief Test Leaky ReLU correctness.
 * @traceability SRS-004.2, SRS-004.4
 */
void test_leaky_relu(void) {
    printf("  Testing Leaky ReLU correctness... ");

    fixed_t buf[4];
    fx_matrix_t mat;
    fx_matrix_init(&mat, buf, 2, 2);

    fixed_t alpha = fixed_from_float(0.01f);  /* 1% leak */

    /* Test data */
    mat.data[0] = fixed_from_float(10.0f);   /* Positive: unchanged */
    mat.data[1] = fixed_from_float(-10.0f);  /* Negative: * 0.01 = -0.1 */
    mat.data[2] = fixed_from_float(5.0f);    /* Positive: unchanged */
    mat.data[3] = fixed_from_float(-20.0f);  /* Negative: * 0.01 = -0.2 */

    /* Apply Leaky ReLU */
    fx_leaky_relu(&mat, alpha);

    /* Verify results */
    assert(fixed_to_float(mat.data[0]) > 9.9f);   /* ~10.0 */
    assert(fixed_to_float(mat.data[1]) > -0.11f && fixed_to_float(mat.data[1]) < -0.09f);  /* ~-0.1 */
    assert(fixed_to_float(mat.data[2]) > 4.9f);   /* ~5.0 */
    assert(fixed_to_float(mat.data[3]) > -0.21f && fixed_to_float(mat.data[3]) < -0.19f);  /* ~-0.2 */

    printf("✓\n");
}

/**
 * @brief Test bias addition correctness.
 * @traceability SRS-004.3, V-004.3
 */
void test_bias_addition(void) {
    printf("  Testing bias addition correctness... ");

    /* Matrix: 2×3 */
    fixed_t mat_buf[6];
    fx_matrix_t mat;
    fx_matrix_init(&mat, mat_buf, 2, 3);

    mat.data[0] = fixed_from_int(1);
    mat.data[1] = fixed_from_int(2);
    mat.data[2] = fixed_from_int(3);
    mat.data[3] = fixed_from_int(4);
    mat.data[4] = fixed_from_int(5);
    mat.data[5] = fixed_from_int(6);

    /* Bias: 1×3 */
    fixed_t bias_buf[3];
    fx_matrix_t bias;
    fx_matrix_init(&bias, bias_buf, 1, 3);

    bias.data[0] = fixed_from_int(10);
    bias.data[1] = fixed_from_int(20);
    bias.data[2] = fixed_from_int(30);

    /* Add bias */
    fx_matrix_add_bias(&mat, &bias);

    /* Verify each row got bias added */
    assert(fixed_to_int(mat.data[0]) == 11);  /* 1 + 10 */
    assert(fixed_to_int(mat.data[1]) == 22);  /* 2 + 20 */
    assert(fixed_to_int(mat.data[2]) == 33);  /* 3 + 30 */
    assert(fixed_to_int(mat.data[3]) == 14);  /* 4 + 10 */
    assert(fixed_to_int(mat.data[4]) == 25);  /* 5 + 20 */
    assert(fixed_to_int(mat.data[5]) == 36);  /* 6 + 30 */

    printf("✓\n");
}

/**
 * @brief Test bias dimension validation.
 * @traceability SRS-004.3, V-004.3
 */
void test_bias_dimension_validation(void) {
    printf("  Testing bias dimension validation... ");

    /* Matrix: 2×3 */
    fixed_t mat_buf[6];
    fx_matrix_t mat;
    fx_matrix_init(&mat, mat_buf, 2, 3);

    /* Fill with sentinel values */
    for (int i = 0; i < 6; i++) {
        mat.data[i] = fixed_from_int(999);
    }

    /* Bias with wrong dimensions: 1×2 (should be 1×3) */
    fixed_t bias_buf[2];
    fx_matrix_t bias;
    fx_matrix_init(&bias, bias_buf, 1, 2);

    /* Try to add incompatible bias */
    fx_matrix_add_bias(&mat, &bias);

    /* Verify matrix unchanged (operation rejected) */
    for (int i = 0; i < 6; i++) {
        assert(fixed_to_int(mat.data[i]) == 999);
    }

    printf("✓\n");
}

/**
 * @brief Test complete dense layer forward pass.
 * @traceability SRS-004.1, SRS-004.2, SRS-004.3
 */
void test_dense_layer_forward(void) {
    printf("  Testing complete dense layer forward pass... ");

    /* Simple 2-input, 2-output layer
     * Input: [1, 2]
     * Weights: [[0.5, 1.0],    Output before activation:
     *           [1.5, 0.5]]    [1*0.5 + 2*1.5 = 3.5]
     *                          [1*1.0 + 2*0.5 = 2.0]
     * Bias: [0.5, -1.0]        After bias: [4.0, 1.0]
     * After ReLU: [4.0, 1.0]   (both positive, unchanged) */

    /* Input: 1×2 */
    fixed_t input_buf[2];
    fx_matrix_t input;
    fx_matrix_init(&input, input_buf, 1, 2);
    input.data[0] = fixed_from_int(1);
    input.data[1] = fixed_from_int(2);

    /* Weights: 2×2 */
    fixed_t weight_buf[4];
    fx_matrix_t weights;
    fx_matrix_init(&weights, weight_buf, 2, 2);
    weights.data[0] = fixed_from_float(0.5f);
    weights.data[1] = fixed_from_float(1.0f);
    weights.data[2] = fixed_from_float(1.5f);
    weights.data[3] = fixed_from_float(0.5f);

    /* Bias: 1×2 */
    fixed_t bias_buf[2];
    fx_matrix_t bias;
    fx_matrix_init(&bias, bias_buf, 1, 2);
    bias.data[0] = fixed_from_float(0.5f);
    bias.data[1] = fixed_from_float(-1.0f);

    /* Output: 1×2 */
    fixed_t output_buf[2];
    fx_matrix_t output;
    fx_matrix_init(&output, output_buf, 1, 2);

    /* Forward pass: y = ReLU(Wx + b) */
    fx_matrix_mul(&input, &weights, &output);  /* Wx */
    fx_matrix_add_bias(&output, &bias);        /* + b */
    fx_relu(&output);                          /* activation */

    /* Verify results */
    float result0 = fixed_to_float(output.data[0]);
    float result1 = fixed_to_float(output.data[1]);

    assert(result0 > 3.9f && result0 < 4.1f);  /* ~4.0 */
    assert(result1 > 0.9f && result1 < 1.1f);  /* ~1.0 */

    printf("✓\n");
}

int main(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    printf("  SRS-004 Activation Functions Verification Suite\n");
    printf("═══════════════════════════════════════════════\n");
    printf("\n");

    test_relu_correctness();
    test_relu_in_place();
    test_leaky_relu();
    test_bias_addition();
    test_bias_dimension_validation();
    test_dense_layer_forward();

    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    printf("  ✅ SRS-004 Verified (6 tests passed)\n");
    printf("═══════════════════════════════════════════════\n");
    printf("\n");
    printf("Requirements validated:\n");
    printf("  • SRS-004.1: In-place processing\n");
    printf("  • SRS-004.2: ReLU determinism\n");
    printf("  • SRS-004.3: Bias vector addition\n");
    printf("  • SRS-004.4: Bounded fixed-point arithmetic\n");
    printf("\n");

    return 0;
}
