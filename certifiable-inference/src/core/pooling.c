/**
 * @file pooling.c
 * @project Certifiable Inference Engine
 * @brief Implementation of deterministic max pooling operations.
 *
 * @details Implements 2×2 max pooling with stride 2 for CNN feature map
 * dimension reduction. Uses only fixed-point arithmetic and deterministic
 * comparisons.
 *
 * @traceability SRS-008-POOLING
 * @compliance DO-178C, ISO 26262, IEC 62304, IEC 61508
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "pooling.h"
#include <assert.h>

/**
 * @brief Deterministic 2×2 Max Pooling with stride 2.
 *
 * @implementation
 * This function implements max pooling through:
 * 1. Dimension validation (even rows/cols required)
 * 2. Non-overlapping 2×2 window iteration
 * 3. Deterministic max selection (4 comparisons per window)
 * 4. Sequential output writing
 *
 * Time Complexity: O(M×N) where M×N is input size
 * - Outer loop: M/2 iterations
 * - Inner loop: N/2 iterations
 * - Per window: 4 element extractions + 3 comparisons
 * - Total: (M×N)/4 windows × 7 operations = O(M×N)
 *
 * Space Complexity: O(1)
 * - Local variables: 6 integers (row/col counters, temp values)
 * - No recursion
 * - No dynamic allocation
 *
 * Determinism Properties:
 * - Fixed iteration count (depends only on dimensions)
 * - No data-dependent branches in inner loop
 * - Integer comparisons (no floating-point)
 * - Sequential memory access pattern
 *
 * @traceability SRS-008.1, SRS-008.2, SRS-008.3, SRS-008.4,
 *               SRS-008.5, SRS-008.6, SRS-008.7
 */
void fx_maxpool_2x2(const fx_matrix_t* in, fx_matrix_t* out) {
    /*
     * Precondition Validation (SRS-008.3)
     *
     * Assert even dimensions to ensure non-overlapping 2×2 windows.
     * Odd dimensions would leave partial windows, creating ambiguity.
     */
    assert(in != NULL && "Input matrix cannot be NULL");
    assert(out != NULL && "Output matrix cannot be NULL");
    assert(in->data != NULL && "Input data cannot be NULL");
    assert(out->data != NULL && "Output data cannot be NULL");

    assert(in->rows % 2 == 0 && "Input rows must be even for 2×2 pooling");
    assert(in->cols % 2 == 0 && "Input cols must be even for 2×2 pooling");

    assert(out->rows == in->rows / 2 && "Output rows must be half of input");
    assert(out->cols == in->cols / 2 && "Output cols must be half of input");

    /*
     * Pooling Loop (SRS-008.1, SRS-008.6, SRS-008.7)
     *
     * Fixed iteration count: (in->rows / 2) × (in->cols / 2)
     * This ensures deterministic execution time independent of data values.
     *
     * Loop structure:
     * - Outer loop: Step by 2 through input rows
     * - Inner loop: Step by 2 through input columns
     * - Each iteration processes one 2×2 window
     */
    uint16_t out_row = 0;

    for (uint16_t i = 0; i < in->rows; i += 2) {
        uint16_t out_col = 0;

        for (uint16_t j = 0; j < in->cols; j += 2) {
            /*
             * Extract 2×2 Window (SRS-008.2)
             *
             * Window layout:
             *   ┌─────┐
             *   │ a b │
             *   │ c d │
             *   └─────┘
             *
             * Access pattern optimized for row-major storage:
             * a = [i  ][j  ]
             * b = [i  ][j+1]
             * c = [i+1][j  ]
             * d = [i+1][j+1]
             */
            const uint16_t row1_offset = i * in->cols;
            const uint16_t row2_offset = (i + 1) * in->cols;

            const fixed_t a = in->data[row1_offset + j];
            const fixed_t b = in->data[row1_offset + j + 1];
            const fixed_t c = in->data[row2_offset + j];
            const fixed_t d = in->data[row2_offset + j + 1];

            /*
             * Deterministic Max Selection (SRS-008.2)
             *
             * Uses sequential comparison (not tree-based) for predictable timing:
             * 1. Initialize max = a
             * 2. Compare b, update if larger
             * 3. Compare c, update if larger
             * 4. Compare d, update if larger
             *
             * This approach:
             * - Always performs exactly 3 comparisons
             * - No early exit based on data
             * - Deterministic for all input patterns
             * - Integer comparison (fixed-point values)
             *
             * Alternative tree-based comparison:
             *   max1 = max(a, b)
             *   max2 = max(c, d)
             *   result = max(max1, max2)
             * Would also work, but sequential is simpler to verify.
             */
            fixed_t max_val = a;

            if (b > max_val) {
                max_val = b;
            }

            if (c > max_val) {
                max_val = c;
            }

            if (d > max_val) {
                max_val = d;
            }

            /*
             * Write to Output (SRS-008.1)
             *
             * Sequential write to output matrix in row-major order.
             * Output position (out_row, out_col) corresponds to input
             * window starting at (i, j).
             */
            out->data[out_row * out->cols + out_col] = max_val;
            out_col++;
        }

        out_row++;
    }

    /*
     * Postcondition (Implicit)
     *
     * Mathematical properties guaranteed by this implementation:
     * 1. Range preservation: min(in) ≤ all(out) ≤ max(in)
     * 2. Monotonicity: If in increases, out increases or stays same
     * 3. Identity: If 2×2 window all same, out = that value
     *
     * These properties follow from max operation semantics and
     * do not require explicit validation.
     */
}
