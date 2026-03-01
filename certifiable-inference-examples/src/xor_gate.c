/**
 * @file xor_gate.c
 * @project Certifiable Inference Engine
 * @brief Demonstration of complete neural network forward pass solving XOR.
 *
 * @details XOR is the classic AI benchmark because it cannot be solved with
 * a simple linear classifier - it requires a hidden layer and non-linear
 * activation. This demo proves our deterministic engine works end-to-end.
 *
 * @traceability SRS-001, SRS-002, SRS-003, SRS-004
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "matrix.h"
#include "activations.h"
#include "fixed_point.h"
#include <stdio.h>

/**
 * @brief XOR truth table demonstration
 *
 * XOR Truth Table:
 *   Input A | Input B | Output
 *   --------|---------|--------
 *      0    |    0    |   0
 *      0    |    1    |   1
 *      1    |    0    |   1
 *      1    |    1    |   0
 *
 * Network Architecture:
 *   Input Layer: 2 neurons
 *   Hidden Layer: 2 neurons with ReLU
 *   Output Layer: 1 neuron with ReLU
 *
 * This demonstrates:
 * - Matrix multiplication (Wx)
 * - Bias addition (+ b)
 * - Non-linear activation (ReLU)
 * - Multi-layer forward pass
 */

static void print_banner(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   SpeyTech Certifiable Inference Engine       ║\n");
    printf("║   XOR Gate Neural Network Demonstration       ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");
}

static int test_xor(fixed_t input_a, fixed_t input_b, int expected, const char* label) {
    printf("Testing: %s\n", label);
    printf("  Input: [%.1f, %.1f]\n",
           fixed_to_float(input_a),
           fixed_to_float(input_b));

    /*
     * Layer 1: Input → Hidden (2 → 2)
     *
     * IMPORTANT: fx_matrix_init() zeros the buffer, so we must set
     * values AFTER calling init, not before.
     */
    fixed_t input_buf[2];
    fx_matrix_t input;
    fx_matrix_init(&input, input_buf, 1, 2);
    input.data[0] = input_a;
    input.data[1] = input_b;

    /*
     * Hidden layer weights (classic XOR solution)
     *
     * Strategy:
     *   H1 = ReLU(A + B + 0)      -- "OR" detector, activates if either input
     *   H2 = ReLU(A + B - 0.9)    -- "AND" detector, activates only if both
     *   Output = ReLU(H1 - 2*H2)  -- H1 AND NOT H2
     *
     * Weight matrix W (2×2):
     *   W[0][0]=1  W[0][1]=1   (input A contributes to both H1 and H2)
     *   W[1][0]=1  W[1][1]=1   (input B contributes to both H1 and H2)
     */
    fixed_t hidden_weights_buf[4];
    fx_matrix_t hidden_weights;
    fx_matrix_init(&hidden_weights, hidden_weights_buf, 2, 2);
    hidden_weights.data[0] = fixed_from_float(1.0f);  /* W[0][0]: A → H1 */
    hidden_weights.data[1] = fixed_from_float(1.0f);  /* W[0][1]: A → H2 */
    hidden_weights.data[2] = fixed_from_float(1.0f);  /* W[1][0]: B → H1 */
    hidden_weights.data[3] = fixed_from_float(1.0f);  /* W[1][1]: B → H2 */

    /*
     * Hidden layer biases
     *   b1 = 0.0   (H1 activates with any input)
     *   b2 = -0.9  (H2 needs sum > 0.9, i.e. both inputs)
     */
    fixed_t hidden_bias_buf[2];
    fx_matrix_t hidden_bias;
    fx_matrix_init(&hidden_bias, hidden_bias_buf, 1, 2);
    hidden_bias.data[0] = fixed_from_float(0.0f);
    hidden_bias.data[1] = fixed_from_float(-0.9f);

    /* Hidden layer output buffer */
    fixed_t hidden_buf[2];
    fx_matrix_t hidden;
    fx_matrix_init(&hidden, hidden_buf, 1, 2);

    /* Forward pass: Hidden = ReLU(Input × W + b) */
    fx_matrix_mul(&input, &hidden_weights, &hidden);
    fx_matrix_add_bias(&hidden, &hidden_bias);
    fx_relu(&hidden);

    printf("  Hidden: [%.2f, %.2f]\n",
           fixed_to_float(hidden.data[0]),
           fixed_to_float(hidden.data[1]));

    /*
     * Layer 2: Hidden → Output (2 → 1)
     *
     * XOR logic: output should be high when exactly one input is active.
     *   - H1 detects "at least one" (OR)
     *   - H2 detects "both" (AND)
     *   - Output = H1 - 2*H2 implements XOR
     *
     * When only one input:  H1≈1, H2≈0 → 1 - 0 = 1 ✓
     * When both inputs:     H1≈2, H2≈1 → 2 - 2 = 0 ✓
     * When no inputs:       H1=0, H2=0 → 0 - 0 = 0 ✓
     */
    fixed_t output_weights_buf[2];
    fx_matrix_t output_weights;
    fx_matrix_init(&output_weights, output_weights_buf, 2, 1);
    output_weights.data[0] = fixed_from_float(1.0f);   /* H1 → Output (+) */
    output_weights.data[1] = fixed_from_float(-2.0f);  /* H2 → Output (-) */

    fixed_t output_bias_buf[1];
    fx_matrix_t output_bias;
    fx_matrix_init(&output_bias, output_bias_buf, 1, 1);
    output_bias.data[0] = fixed_from_float(0.0f);

    fixed_t output_buf[1];
    fx_matrix_t output;
    fx_matrix_init(&output, output_buf, 1, 1);

    /* Forward pass: Output = ReLU(Hidden × W + b) */
    fx_matrix_mul(&hidden, &output_weights, &output);
    fx_matrix_add_bias(&output, &output_bias);
    fx_relu(&output);

    printf("  Output: %.2f\n", fixed_to_float(output.data[0]));

    /* Interpret result (threshold at 0.5) */
    float result = fixed_to_float(output.data[0]);
    int predicted = (result > 0.5f) ? 1 : 0;
    int passed = (predicted == expected);

    printf("  Predicted: %d  %s\n\n", predicted, passed ? "✓" : "✗ FAILED");

    return passed;
}

int main(void) {
    print_banner();

    printf("XOR Truth Table Test:\n");
    printf("═══════════════════════════════════════════════\n\n");

    /* Test all four XOR cases */
    int passed = 0;
    passed += test_xor(FIXED_ZERO, FIXED_ZERO, 0, "0 XOR 0 = 0");
    passed += test_xor(FIXED_ZERO, FIXED_ONE,  1, "0 XOR 1 = 1");
    passed += test_xor(FIXED_ONE,  FIXED_ZERO, 1, "1 XOR 0 = 1");
    passed += test_xor(FIXED_ONE,  FIXED_ONE,  0, "1 XOR 1 = 0");

    printf("═══════════════════════════════════════════════\n");

    if (passed == 4) {
        printf("✅ XOR Neural Network: All %d/4 tests passed!\n\n", passed);
        printf("Key Achievements:\n");
        printf("  • Zero floating-point operations in inference\n");
        printf("  • Zero dynamic memory allocation\n");
        printf("  • Bit-perfect determinism\n");
        printf("  • Complete multi-layer forward pass\n");
        printf("  • Real neural network solving real problem\n\n");
        printf("This is certifiable AI inference.\n");
        return 0;
    } else {
        printf("❌ XOR Neural Network: %d/4 tests passed\n", passed);
        return 1;
    }
}
