/**
 * @file matrix.h
 * @project Certifiable Inference Engine
 * @brief Bounded-resource, deterministic matrix operations.
 *
 * @details Implements fixed-point matrix multiplication (GEMM) and
 * vector operations. Designed for O(1) space complexity relative to
 * pre-allocated buffers. All operations guarantee bit-perfect reproducibility
 * across platforms.
 *
 * @traceability SRS-003-LINEAR-ALGEBRA
 * @compliance MISRA-C:2012, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 *          For commercial licensing: william@fstopify.com
 */

#ifndef MATRIX_H
#define MATRIX_H

#include "fixed_point.h"
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Matrix structure for fixed-point data.
 *
 * @details Uses row-major layout for cache efficiency.
 * Element [i][j] stored at data[i * cols + j].
 *
 * @note Memory managed by caller - no dynamic allocation.
 */
typedef struct {
    fixed_t* data;               /**< Pointer to pre-allocated buffer */
    uint16_t rows;               /**< Number of rows */
    uint16_t cols;               /**< Number of columns */
} fx_matrix_t;

/**
 * @brief Initialize a matrix using a provided buffer (zeros buffer).
 *
 * @details Zeros the buffer to ensure deterministic initial state.
 * No dynamic allocation - buffer provided by caller.
 *
 * @warning This function zeros the buffer. If your buffer already contains
 *          data, use fx_matrix_attach() instead to preserve contents.
 *
 * @param[out] mat Matrix structure to initialize
 * @param[in] buffer Pre-allocated memory buffer
 * @param[in] rows Number of rows
 * @param[in] cols Number of columns
 *
 * @pre mat and buffer are valid pointers
 * @pre buffer size >= rows * cols * sizeof(fixed_t)
 * @post Matrix initialized and zeroed
 *
 * @complexity O(rows * cols) for zeroing
 * @determinism Always produces same initial state
 *
 * @traceability SRS-003.1, SRS-003.2
 */
void fx_matrix_init(fx_matrix_t* mat, fixed_t* buffer, uint16_t rows, uint16_t cols);

/**
 * @brief Attach a pre-populated buffer to a matrix (no zeroing).
 *
 * @details Binds an existing buffer to a matrix structure without
 * modifying buffer contents. Use this when the buffer already contains
 * data that must be preserved.
 *
 * Use cases:
 * - Wrapping statically initialized arrays
 * - Attaching buffers populated by external sources
 * - Reinterpreting raw data as matrices
 *
 * @param[out] mat Matrix structure to initialize
 * @param[in] buffer Pre-allocated memory buffer (contents preserved)
 * @param[in] rows Number of rows
 * @param[in] cols Number of columns
 *
 * @pre mat and buffer are valid pointers
 * @pre buffer size >= rows * cols * sizeof(fixed_t)
 * @pre buffer contains valid fixed_t data
 * @post Matrix attached to buffer, contents unchanged
 *
 * @complexity O(1)
 * @determinism Bit-perfect (no data modification)
 *
 * @traceability SRS-003.1
 */
static inline void fx_matrix_attach(fx_matrix_t* mat, fixed_t* buffer,
                                    uint16_t rows, uint16_t cols) {
    if (!mat || !buffer) {
        return;
    }
    mat->data = buffer;
    mat->rows = rows;
    mat->cols = cols;
}

/**
 * @brief Deterministic matrix multiplication: C = A × B
 *
 * @details Implements GEMM (General Matrix Multiply) with:
 * - 64-bit intermediate accumulators (prevents overflow)
 * - Proper rounding (minimizes quantization error)
 * - Sequential operations only (no SIMD, bit-perfect)
 * - Row-major access pattern (cache-friendly)
 *
 * Dimension requirements: A(N×M) × B(M×P) = C(N×P)
 * Must have: A.cols == B.rows
 *
 * @param[in] A First matrix (N×M)
 * @param[in] B Second matrix (M×P)
 * @param[out] C Result matrix (N×P), must be pre-allocated
 *
 * @pre A, B, C are valid pointers
 * @pre A.cols == B.rows (compatible dimensions)
 * @pre C dimensions are A.rows × B.cols
 * @post C contains A × B if dimensions compatible, unchanged otherwise
 *
 * @complexity O(N * M * P) where N=A.rows, M=A.cols, P=B.cols
 * @determinism Bit-perfect across all platforms and compilers
 *
 * @note Returns early without modifying C if dimensions incompatible
 *
 * @traceability SRS-003.3, SRS-003.4, SRS-003.5, SRS-003.6
 */
void fx_matrix_mul(const fx_matrix_t* A, const fx_matrix_t* B, fx_matrix_t* C);

/**
 * @brief Dot product of two fixed-point vectors.
 *
 * @details Computes sum(a[i] * b[i]) for i in [0, len).
 * Uses 64-bit accumulator to prevent overflow.
 * Common operation in dense neural network layers.
 *
 * @param[in] a First vector
 * @param[in] b Second vector
 * @param[in] len Length of both vectors
 *
 * @return Dot product a·b in fixed-point format
 *
 * @pre a and b are valid pointers
 * @pre Both vectors have length >= len
 * @post Returns sum(a[i] * b[i])
 *
 * @complexity O(len)
 * @determinism Bit-perfect across all platforms
 *
 * @traceability SRS-003.5, SRS-003.6
 */
fixed_t fx_vector_dot(const fixed_t* a, const fixed_t* b, uint16_t len);

/**
 * @brief Element-wise matrix addition: C = A + B
 *
 * @details Adds corresponding elements of A and B.
 * Matrices must have identical dimensions.
 *
 * @param[in] A First matrix
 * @param[in] B Second matrix
 * @param[out] C Result matrix
 *
 * @pre A, B, C are valid pointers
 * @pre A.rows == B.rows == C.rows
 * @pre A.cols == B.cols == C.cols
 * @post C[i][j] = A[i][j] + B[i][j] for all i, j
 *
 * @complexity O(rows * cols)
 * @determinism Bit-perfect
 *
 * @traceability SRS-003.3
 */
void fx_matrix_add(const fx_matrix_t* A, const fx_matrix_t* B, fx_matrix_t* C);

/**
 * @brief Apply function element-wise to matrix.
 *
 * @details Applies activation function to each element.
 * Used for ReLU, sigmoid, etc. in neural network layers.
 *
 * @param[in,out] mat Matrix to modify in-place
 * @param[in] fn Function to apply to each element
 *
 * @pre mat and fn are valid pointers
 * @post mat->data[i] = fn(mat->data[i]) for all i
 *
 * @complexity O(rows * cols)
 * @determinism Depends on fn determinism
 *
 * @traceability SRS-003.3
 */
void fx_matrix_apply(fx_matrix_t* mat, fixed_t (*fn)(fixed_t));

/**
 * @brief Add bias vector to each row of matrix.
 *
 * @details Broadcasts 1×N bias vector to each row of M×N matrix.
 * Required for standard dense layer: y = activation(Wx + b)
 *
 * Example:
 *   Matrix (2×3):     Bias (1×3):      Result (2×3):
 *   [1  2  3]         [10 20 30]       [11 22 33]
 *   [4  5  6]                          [14 25 36]
 *
 * @param[in,out] mat Matrix to add bias to (M×N)
 * @param[in] bias Bias vector (1×N)
 *
 * @pre mat and bias are valid pointers
 * @pre mat->cols == bias->cols
 * @pre bias->rows == 1
 * @post mat[i][j] += bias[j] for all i, j
 *
 * @complexity O(rows * cols)
 * @determinism Bit-perfect (uses fixed-point addition)
 *
 * @traceability SRS-004.3, SRS-004.4
 */
void fx_matrix_add_bias(fx_matrix_t* mat, const fx_matrix_t* bias);

#endif /* MATRIX_H */
