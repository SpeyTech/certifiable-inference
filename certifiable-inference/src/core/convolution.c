/**
 * @file convolution.c
 * @project Certifiable Inference Engine
 * @brief Implementation of deterministic 2D convolution.
 *
 * @details Provides bit-perfect sliding-window convolution for computer vision
 * applications. Uses explicit loops and 64-bit accumulation for reproducibility.
 *
 * @traceability SRS-006-CONVOLUTION
 * @compliance MISRA-C:2012, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 *          For commercial licensing: william@fstopify.com
 */

#include "convolution.h"

void fx_conv2d(const fx_matrix_t* in, const fx_matrix_t* kernel, fx_matrix_t* out) {
    /* SRS-006.1: Dimension validation */
    if (!in || !kernel || !out || !in->data || !kernel->data || !out->data) {
        return;
    }

    /* Verify kernel fits within input */
    if (kernel->rows > in->rows || kernel->cols > in->cols) {
        return;
    }

    /* Calculate expected output dimensions (valid padding) */
    uint16_t expected_out_rows = in->rows - kernel->rows + 1;
    uint16_t expected_out_cols = in->cols - kernel->cols + 1;

    /* Verify output buffer has correct dimensions */
    if (out->rows != expected_out_rows || out->cols != expected_out_cols) {
        return;
    }

    /* SRS-006.2: Sliding window implementation with explicit loops
     * SRS-006.5: Bounded execution time (depends only on dimensions) */

    /* Iterate over each output position */
    for (uint16_t out_row = 0; out_row < out->rows; out_row++) {
        for (uint16_t out_col = 0; out_col < out->cols; out_col++) {

            /* SRS-006.3: 64-bit accumulator to prevent overflow */
            int64_t accumulator = 0;

            /* Sliding window: compute dot product of kernel with input patch */
            for (uint16_t ker_row = 0; ker_row < kernel->rows; ker_row++) {
                for (uint16_t ker_col = 0; ker_col < kernel->cols; ker_col++) {

                    /* Input position for this kernel element */
                    uint16_t in_row = out_row + ker_row;
                    uint16_t in_col = out_col + ker_col;

                    /* Get values (row-major layout) */
                    fixed_t input_val = in->data[in_row * in->cols + in_col];
                    fixed_t kernel_val = kernel->data[ker_row * kernel->cols + ker_col];

                    /* Multiply and accumulate (Q16.16 × Q16.16 = Q32.32) */
                    int64_t product = (int64_t)input_val * kernel_val;
                    accumulator += product;
                }
            }

            /* SRS-006.4: Quantize back to Q16.16 with round-to-nearest
             * Add FIXED_HALF (0.5 in Q16.16) before shifting for proper rounding */
            accumulator += FIXED_HALF;
            out->data[out_row * out->cols + out_col] = (fixed_t)(accumulator >> FIXED_SHIFT);
        }
    }
}
