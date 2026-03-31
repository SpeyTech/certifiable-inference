/**
 * @file activations.h
 * @project Certifiable Inference Engine
 * @brief Deterministic activation functions for neural networks.
 *
 * @details Provides in-place, bit-perfect activation functions suitable for
 * safety-critical AI inference. All operations are deterministic and maintain
 * O(1) space complexity through in-place modification.
 *
 * @traceability SRS-004-ACTIVATIONS
 * @compliance MISRA-C:2012, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 *          For commercial licensing: william@fstopify.com
 */

#ifndef ACTIVATIONS_H
#define ACTIVATIONS_H

#include "matrix.h"

/**
 * @brief Rectified Linear Unit (ReLU) activation function.
 *
 * @details Implements f(x) = max(0, x) in-place on matrix.
 * Most common activation in safety-critical neural networks due to:
 * - Simple implementation (one comparison per element)
 * - Bit-perfect determinism across all platforms
 * - No transcendental functions required
 * - Non-saturating (avoids vanishing gradient)
 *
 * @param[in,out] mat Matrix to apply ReLU to (modified in-place)
 *
 * @pre mat is valid pointer with allocated data
 * @post mat->data[i] = max(0, mat->data[i]) for all i
 *
 * @complexity O(rows * cols)
 * @determinism Bit-perfect across all platforms
 *
 * @traceability SRS-004.1, SRS-004.2
 */
void fx_relu(fx_matrix_t* mat);

/**
 * @brief Leaky ReLU activation function.
 *
 * @details Implements f(x) = x if x > 0, else alpha * x
 * Variant of ReLU that allows small negative gradients, preventing
 * "dead neurons" that never activate.
 *
 * Typical alpha: 0.01 (allows 1% gradient for negative values)
 *
 * @param[in,out] mat Matrix to apply Leaky ReLU to (modified in-place)
 * @param[in] alpha Slope for negative values (typically 0.01)
 *
 * @pre mat is valid pointer with allocated data
 * @pre alpha is small positive value (e.g., 0.01)
 * @post mat->data[i] = original if > 0, else alpha * original
 *
 * @complexity O(rows * cols)
 * @determinism Bit-perfect (uses fixed_mul from SRS-002)
 *
 * @traceability SRS-004.1, SRS-004.2, SRS-004.4
 */
void fx_leaky_relu(fx_matrix_t* mat, fixed_t alpha);

/**
 * @brief Identity activation (no operation).
 *
 * @details Sometimes neural networks use linear (identity) activation
 * in output layers. This function is provided for API completeness
 * and clear intent in layer definitions.
 *
 * @param[in,out] mat Matrix (unchanged)
 *
 * @pre mat is valid pointer
 * @post mat unchanged
 *
 * @complexity O(1)
 * @determinism Trivially deterministic (does nothing)
 *
 * @traceability SRS-004.1
 */
static inline void fx_identity(fx_matrix_t* mat) {
    (void)mat;  /* Suppress unused parameter warning */
    /* Intentionally empty - identity function */
}

#endif /* ACTIVATIONS_H */
