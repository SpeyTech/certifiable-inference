/**
 * @file edge_detection.c
 * @project Certifiable Inference Engine
 * @brief Demonstration of bit-perfect edge detection using Conv2D.
 *
 * @details This demo proves the engine "sees" correctly by applying a Sobel
 * filter to detect vertical edges. Sobel filters are foundational to:
 * - Autonomous vehicle lane detection
 * - Medical image segmentation
 * - Industrial defect detection
 *
 * @traceability SRS-006-CONVOLUTION
 * @compliance DO-178C, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "matrix.h"
#include "convolution.h"
#include "fixed_point.h"
#include <stdio.h>

/**
 * @brief Print matrix as ASCII art for visualization.
 *
 * @param mat Matrix to visualize
 * @param label Description label
 * @param show_values If true, show actual values; if false, show edge map
 */
static void print_matrix(const fx_matrix_t* mat, const char* label, int show_values) {
    printf("%s (%d×%d):\n", label, mat->rows, mat->cols);

    for (uint16_t r = 0; r < mat->rows; r++) {
        printf("  ");
        for (uint16_t c = 0; c < mat->cols; c++) {
            fixed_t val = mat->data[r * mat->cols + c];

            if (show_values) {
                int ival = fixed_to_int(val);
                if (ival == 0) {
                    printf("  . ");
                } else {
                    printf("%3d ", ival);
                }
            } else {
                /* Edge map: # for edges, . for flat regions */
                if (val > 0) {
                    printf(" + ");  /* Positive edge (light→dark transition) */
                } else if (val < 0) {
                    printf(" - ");  /* Negative edge (dark→light transition) */
                } else {
                    printf(" . ");  /* No edge */
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}

/**
 * @brief Print the Sobel kernel for documentation.
 */
static void print_sobel_kernel(void) {
    printf("Sobel Vertical Kernel (detects vertical edges):\n");
    printf("  ┌─────────────┐\n");
    printf("  │ -1   0   1  │\n");
    printf("  │ -2   0   2  │\n");
    printf("  │ -1   0   1  │\n");
    printf("  └─────────────┘\n\n");
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   SpeyTech Certifiable Inference Engine       ║\n");
    printf("║   Sobel Edge Detection Demonstration          ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");

    /*
     * Step 1: Create 8×8 test image with vertical bar
     *
     * Pattern:
     *   . . . # # . . .
     *   . . . # # . . .
     *   . . . # # . . .
     *   . . . # # . . .
     *   . . . # # . . .
     *   . . . # # . . .
     *   . . . # # . . .
     *   . . . # # . . .
     */
    fixed_t img_buf[64];
    fx_matrix_t input;
    fx_matrix_init(&input, img_buf, 8, 8);

    /* Set vertical bar AFTER init (columns 3 and 4) */
    for (int r = 0; r < 8; r++) {
        input.data[r * 8 + 3] = FIXED_ONE;
        input.data[r * 8 + 4] = FIXED_ONE;
    }

    print_matrix(&input, "Input Image: Vertical Bar", 1);

    /*
     * Step 2: Define Sobel vertical kernel
     *
     * This kernel detects vertical edges by computing horizontal gradient:
     *   [-1  0  1]
     *   [-2  0  2]
     *   [-1  0  1]
     *
     * Positive output = transition from dark (left) to light (right)
     * Negative output = transition from light (left) to dark (right)
     */
    fixed_t sobel_buf[9];
    fx_matrix_t kernel;
    fx_matrix_init(&kernel, sobel_buf, 3, 3);

    /* Set kernel values AFTER init */
    kernel.data[0] = fixed_from_int(-1); kernel.data[1] = FIXED_ZERO; kernel.data[2] = fixed_from_int(1);
    kernel.data[3] = fixed_from_int(-2); kernel.data[4] = FIXED_ZERO; kernel.data[5] = fixed_from_int(2);
    kernel.data[6] = fixed_from_int(-1); kernel.data[7] = FIXED_ZERO; kernel.data[8] = fixed_from_int(1);

    print_sobel_kernel();

    /*
     * Step 3: Prepare output buffer
     *
     * Valid convolution: output_size = input_size - kernel_size + 1
     * 8 - 3 + 1 = 6, so output is 6×6
     */
    fixed_t out_buf[36];
    fx_matrix_t output;
    fx_matrix_init(&output, out_buf, 6, 6);

    /*
     * Step 4: Execute convolution
     */
    printf("Applying Sobel filter...\n\n");
    fx_conv2d(&input, &kernel, &output);

    /*
     * Step 5: Visualize results
     *
     * Expected output:
     *   - Left edge of bar: positive values (dark→light transition)
     *   - Right edge of bar: negative values (light→dark transition)
     *   - Interior and exterior: zero (no gradient)
     */
    print_matrix(&output, "Edge Detection Result (values)", 1);
    print_matrix(&output, "Edge Map (+ = rising edge, - = falling edge)", 0);

    /*
     * Step 6: Verify correctness
     */
    printf("═══════════════════════════════════════════════\n");
    printf("Analysis:\n");
    printf("───────────────────────────────────────────────\n");

    /* Check for expected edge pattern */
    int left_edges = 0;
    int right_edges = 0;
    int interior = 0;

    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 6; c++) {
            fixed_t val = output.data[r * 6 + c];
            if (val > 0) left_edges++;
            else if (val < 0) right_edges++;
            else interior++;
        }
    }

    printf("  • Rising edges (dark→light):  %d pixels\n", left_edges);
    printf("  • Falling edges (light→dark): %d pixels\n", right_edges);
    printf("  • Flat regions (no gradient): %d pixels\n", interior);
    printf("\n");

    /*
     * Expected for 2-pixel wide bar in 6×6 output:
     * - 2 columns of rising edges × 6 rows = 12
     * - 2 columns of falling edges × 6 rows = 12
     * - 2 columns of flat regions × 6 rows = 12
     */
    if (left_edges == 12 && right_edges == 12 && interior == 12) {
        printf("✅ Edge Detection Verified!\n\n");
        printf("This demonstrates:\n");
        printf("  • Bit-perfect convolution operation\n");
        printf("  • Correct Sobel gradient computation\n");
        printf("  • Deterministic edge detection\n");
        printf("  • Foundation for autonomous perception\n");
    } else {
        printf("❌ Unexpected edge pattern - review implementation\n");
        return 1;
    }

    return 0;
}
