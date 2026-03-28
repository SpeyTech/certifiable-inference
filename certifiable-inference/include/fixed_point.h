/**
 * @file fixed_point.h
 * @brief L2 Fixed-Point Types (L1 Delegation)
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * DVEC: v1.3
 * DETERMINISM: D1 — Strict Deterministic
 * SRS: SRS-005-CR-001 v1.1-Frozen
 *
 * This header delegates to L1 (libaxilog) for all arithmetic
 * type definitions. L2 SHALL NOT define independent constants.
 *
 * @traceability SRS-005-SHALL-001, SRS-005-SHALL-002
 */

#ifndef CERTIFIABLE_INFERENCE_FIXED_POINT_H
#define CERTIFIABLE_INFERENCE_FIXED_POINT_H

#include <axilog/types.h>

/* ========================================================================
 * L2 Legacy Type Aliases — Delegate to L1
 * ======================================================================== */

typedef q16_16_t fixed_t;

#define FIXED_SHIFT   Q16_SHIFT
#define FIXED_ONE     Q16_ONE
#define FIXED_HALF    Q16_HALF
#define FIXED_MAX     Q16_MAX
#define FIXED_MIN     Q16_MIN
#define FIXED_ZERO    0

/* ========================================================================
 * Conversion Functions
 * ======================================================================== */

/**
 * @brief Convert integer to Q16.16 fixed-point.
 *
 * SRS-005-SHALL-005: Integer-to-Q16.16 conversion
 * SRS-005-SHALL-068: No signed shift — uses multiplication
 */
static inline fixed_t fixed_from_int(int32_t i)
{
    return (fixed_t)(i * FIXED_ONE);
}

/**
 * @brief Convert Q16.16 fixed-point to integer (truncation toward zero).
 *
 * SRS-005-SHALL-006: Q16.16-to-integer conversion
 * SRS-005-SHALL-068: No signed shift — uses division
 */
static inline int32_t fixed_to_int(fixed_t f)
{
    return f / FIXED_ONE;
}

/**
 * @brief Convert float to fixed-point.
 *
 * @warning Only use during initialization or model loading, not in
 *          inference runtime. Floating-point operations are non-deterministic.
 */
static inline fixed_t fixed_from_float(float fl)
{
    return (fixed_t)(fl * FIXED_ONE);
}

/**
 * @brief Convert fixed-point to float.
 *
 * @warning Only use for debugging or logging, not in inference runtime.
 */
static inline float fixed_to_float(fixed_t f)
{
    return (float)f / FIXED_ONE;
}

/* ========================================================================
 * Basic Arithmetic
 * ======================================================================== */

/**
 * @brief Fixed-point addition.
 *
 * SRS-005-SHALL-010: Saturated addition (saturation deferred to Phase 3)
 */
static inline fixed_t fixed_add(fixed_t a, fixed_t b)
{
    return a + b;
}

/**
 * @brief Fixed-point subtraction.
 *
 * SRS-005-SHALL-011: Saturated subtraction (saturation deferred to Phase 3)
 */
static inline fixed_t fixed_sub(fixed_t a, fixed_t b)
{
    return a - b;
}

/**
 * @brief Fixed-point absolute value with INT32_MIN saturation.
 *
 * SRS-005-SHALL-015: Absolute value
 * SRS-005-SHALL-069: INT32_MIN overflow handled
 */
static inline fixed_t fixed_abs(fixed_t f)
{
    if (f == INT32_MIN) {
        return INT32_MAX;  /* Saturate — fault signalling deferred to Phase 3 */
    }
    return (f < 0) ? -f : f;
}

/**
 * @brief Fixed-point negation with INT32_MIN saturation.
 *
 * SRS-005-SHALL-014: Negation
 * SRS-005-SHALL-069: INT32_MIN overflow handled
 */
static inline fixed_t fixed_neg(fixed_t f)
{
    if (f == INT32_MIN) {
        return INT32_MAX;  /* Saturate — fault signalling deferred to Phase 3 */
    }
    return -f;
}

/* ========================================================================
 * Complex Arithmetic (implemented in fixed_point.c)
 * ======================================================================== */

/**
 * @brief Saturated Q16.16 multiplication (truncation toward zero).
 *
 * SRS-005-SHALL-012: Saturated multiplication
 * SRS-005-SHALL-067: Truncation mandated (no rounding)
 * SRS-005-SHALL-068: No signed shift
 */
fixed_t fixed_mul(fixed_t a, fixed_t b);

/**
 * @brief Saturated Q16.16 division.
 *
 * SRS-005-SHALL-013: Saturated division
 * SRS-005-SHALL-068: No signed shift
 * SRS-005-SHALL-069: INT32_MIN/-1 handled
 */
fixed_t fixed_div(fixed_t a, fixed_t b);

#endif /* CERTIFIABLE_INFERENCE_FIXED_POINT_H */
