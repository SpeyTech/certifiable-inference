/**
 * @file convolution.h
 * @project Certifiable Inference Engine
 * @brief Bounded-resource, deterministic 2D Convolution for computer vision.
 *
 * @details Implements sliding-window convolution with bit-perfect results
 * across all platforms. Essential for CNNs in medical imaging, autonomous
 * vehicles, and safety-critical vision systems.
 *
 * @traceability SRS-006-CONVOLUTION
 * @compliance MISRA-C:2012, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 *          For commercial licensing: william@fstopify.com
 */

#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "matrix.h"

/**
 * @brief Deterministic 2D Convolution with valid padding.
 *
 * @details Performs sliding-window dot product of kernel over input.
 * Uses "valid" padding (no zero-padding), so output is smaller than input.
 *
 * Output dimensions:
 *   output_height = input_height - kernel_height + 1
 *   output_width = input_width - kernel_width + 1
 *
 * Mathematical operation:
 *   For each output position (r, c):
 *     output[r][c] = Σ(i,j) input[r+i][c+j] × kernel[i][j]
 *
 * Common applications:
 * - Edge detection (Sobel, Laplacian filters)
 * - Feature extraction (CNN layers)
 * - Image preprocessing (blur, sharpen)
 *
 * @param[in] in Input feature map (H×W matrix)
 * @param[in] kernel Convolution kernel (KH×KW matrix)
 * @param[out] out Output feature map ((H-KH+1)×(W-KW+1) matrix)
 *
 * @pre in, kernel, out are valid pointers with allocated data
 * @pre kernel dimensions ≤ input dimensions
 * @pre out dimensions = (in->rows - kernel->rows + 1) × (in->cols - kernel->cols + 1)
 * @post out contains convolution result
 *
 * @complexity O(OH × OW × KH × KW) where OH,OW=output dims, KH,KW=kernel dims
 * @determinism Bit-perfect across all platforms (uses 64-bit accumulator)
 *
 * @note Uses same quantization strategy as SRS-003 (matrix multiplication)
 * @note No dynamic memory allocation (caller provides buffers)
 * @note Safe for concurrent calls (no shared state)
 *
 * @traceability SRS-006.1, SRS-006.2, SRS-006.3, SRS-006.4
 *
 * @example
 * ```c
 * // 8×8 input, 3×3 kernel → 6×6 output
 * fixed_t in_buf[64], kernel_buf[9], out_buf[36];
 * fx_matrix_t input, kernel, output;
 * fx_matrix_init(&input, in_buf, 8, 8);
 * fx_matrix_init(&kernel, kernel_buf, 3, 3);
 * fx_matrix_init(&output, out_buf, 6, 6);
 *
 * // ... fill input and kernel
 *
 * fx_conv2d(&input, &kernel, &output);
 * // output now contains convolution result
 * ```
 */
void fx_conv2d(const fx_matrix_t* in, const fx_matrix_t* kernel, fx_matrix_t* out);

#endif /* CONVOLUTION_H */
