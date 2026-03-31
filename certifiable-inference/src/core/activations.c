/**
 * @file activations.c
 * @project Certifiable Inference Engine
 * @brief Implementation of deterministic activation functions.
 *
 * @details Provides bit-perfect, in-place activation functions for neural
 * network inference. All operations maintain O(1) space complexity.
 *
 * @traceability SRS-004-ACTIVATIONS
 * @compliance MISRA-C:2012, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 *          For commercial licensing: william@fstopify.com
 */

#include "activations.h"

void fx_relu(fx_matrix_t* mat) {
    /* SRS-004.1: Operate in-place to minimize memory footprint */
    if (!mat || !mat->data) {
        return;
    }

    size_t total_elements = (size_t)mat->rows * mat->cols;

    /* SRS-004.2: Deterministic max(0, x) implementation
     * Sequential iteration ensures consistent behavior across platforms */
    for (size_t i = 0; i < total_elements; i++) {
        /* Simple comparison - deterministic on all architectures */
        if (mat->data[i] < 0) {
            mat->data[i] = FIXED_ZERO;
        }
        /* Positive values remain unchanged */
    }
}

void fx_leaky_relu(fx_matrix_t* mat, fixed_t alpha) {
    /* SRS-004.1: Operate in-place */
    if (!mat || !mat->data) {
        return;
    }

    size_t total_elements = (size_t)mat->rows * mat->cols;

    /* SRS-004.2 & SRS-004.4: Deterministic leaky ReLU with fixed-point multiply */
    for (size_t i = 0; i < total_elements; i++) {
        if (mat->data[i] < 0) {
            /* Use fixed_mul from SRS-002 for deterministic multiplication */
            mat->data[i] = fixed_mul(mat->data[i], alpha);
        }
        /* Positive values remain unchanged */
    }
}
