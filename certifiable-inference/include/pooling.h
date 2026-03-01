/**
 * @file pooling.h
 * @project Certifiable Inference Engine
 * @brief Bounded-resource, deterministic Max Pooling operations for CNNs.
 *
 * @details Implements max pooling layers for spatial dimension reduction
 * in convolutional neural networks. All operations are deterministic,
 * use pre-allocated memory, and have provable time complexity.
 *
 * @traceability SRS-008-POOLING
 * @compliance DO-178C, ISO 26262, IEC 62304, IEC 61508
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#ifndef POOLING_H
#define POOLING_H

#include "matrix.h"

/**
 * @brief Deterministic 2×2 Max Pooling with stride 2.
 *
 * @details Reduces spatial dimensions by factor of 2 using non-overlapping
 * 2×2 windows. Selects maximum value from each window using deterministic
 * fixed-point comparison.
 *
 * Mathematical operation:
 * For each 2×2 window starting at (i,j):
 *   out[i/2][j/2] = max(in[i][j], in[i][j+1], in[i+1][j], in[i+1][j+1])
 *
 * @param in Input feature map (must have even dimensions)
 * @param out Output feature map (dimensions = in dimensions / 2)
 *
 * @precondition in->rows % 2 == 0 (even number of rows)
 * @precondition in->cols % 2 == 0 (even number of columns)
 * @precondition out->rows == in->rows / 2
 * @precondition out->cols == in->cols / 2
 * @precondition Both matrices pre-allocated
 *
 * @postcondition out contains maximum values from each 2×2 window
 * @postcondition min(in) ≤ all(out) ≤ max(in)
 *
 * @complexity Time: O(M×N) where M×N is input size
 * @complexity Space: O(1) stack usage
 *
 * @determinism Execution time independent of input values
 * @determinism Fixed iteration count based on dimensions only
 * @determinism Bit-perfect reproducibility across all platforms
 *
 * @safety No dynamic memory allocation
 * @safety No floating-point operations
 * @safety Bounded stack usage (<10 bytes)
 * @safety No recursion
 *
 * @example
 * @code
 * // 14×14 feature map after convolution
 * fixed_t conv_buf[196];
 * fixed_t pool_buf[49];  // 7×7 after pooling
 *
 * fx_matrix_t conv_out, pool_out;
 * fx_matrix_init(&conv_out, conv_buf, 14, 14);
 * fx_matrix_init(&pool_out, pool_buf, 7, 7);
 *
 * // Perform max pooling
 * fx_maxpool_2x2(&conv_out, &pool_out);
 *
 * // Result: pool_out contains 7×7 downsampled features
 * @endcode
 *
 * @traceability SRS-008.1, SRS-008.2, SRS-008.7
 */
void fx_maxpool_2x2(const fx_matrix_t* in, fx_matrix_t* out);

#endif /* POOLING_H */
