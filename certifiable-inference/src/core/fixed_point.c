/**
 * @file fixed_point.c
 * @brief SRS-005 Conformant Q16.16 Arithmetic Implementation
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * DVEC: v1.3
 * DETERMINISM: D1 — Strict Deterministic
 * SRS: SRS-005-CR-001 v1.1-Frozen
 *
 * All operations:
 * - Use multiplication/division instead of shifts (no UB/impl-defined)
 * - Truncate toward zero (no rounding)
 * - Saturate on overflow
 *
 * @traceability SRS-005-SHALL-012, SRS-005-SHALL-013, SRS-005-SHALL-067,
 *               SRS-005-SHALL-068, SRS-005-SHALL-069
 */

#include "fixed_point.h"
#include <stdint.h>

/**
 * @brief Saturated Q16.16 multiplication (truncation toward zero).
 *
 * SRS-005-SHALL-012: Saturated multiplication
 * SRS-005-SHALL-067: Truncation mandated (no rounding)
 * SRS-005-SHALL-068: No signed shift — uses division
 */
fixed_t fixed_mul(fixed_t a, fixed_t b)
{
    int64_t result = (int64_t)a * (int64_t)b;

    /* Truncation toward zero — C99 division semantics
     * NO rounding (removed FIXED_HALF per SRS-005-SHALL-067) */
    result = result / FIXED_ONE;

    /* Saturation */
    if (result > INT32_MAX) {
        return INT32_MAX;
    }
    if (result < INT32_MIN) {
        return INT32_MIN;
    }

    return (fixed_t)result;
}

/**
 * @brief Saturated Q16.16 division.
 *
 * SRS-005-SHALL-013: Saturated division
 * SRS-005-SHALL-068: No signed shift — uses multiplication
 * SRS-005-SHALL-069: INT32_MIN/-1 handled via saturation
 */
fixed_t fixed_div(fixed_t a, fixed_t b)
{
    if (b == 0) {
        return FIXED_ZERO;  /* Fault signalling deferred to Phase 3 */
    }

    /* Use multiplication instead of shift (SRS-005-SHALL-068) */
    int64_t numerator = (int64_t)a * FIXED_ONE;
    int64_t result = numerator / (int64_t)b;

    /* Saturation — also handles INT32_MIN / -1 overflow */
    if (result > INT32_MAX) {
        return INT32_MAX;
    }
    if (result < INT32_MIN) {
        return INT32_MIN;
    }

    return (fixed_t)result;
}
