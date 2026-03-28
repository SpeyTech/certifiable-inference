/**
 * @file types.h
 * @brief DVM Core Types — Q16.16 Fixed-Point and Fault Flags
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * DVEC: v1.3
 * DETERMINISM: D1 — Strict Deterministic
 * MEMORY: Zero Dynamic Allocation
 * SRS: SRS-005
 * Patent: UK GB2521625.0
 *
 * This is the CANONICAL source for Q16.16 types and fault flags.
 * All other Axioma layers (L2–L7) SHALL depend on this header.
 *
 * @traceability SRS-005-SHALL-001, SRS-005-SHALL-002, SRS-005-SHALL-003,
 *               SRS-005-SHALL-004, SRS-005-SHALL-026, SRS-005-SHALL-029,
 *               SRS-005-SHALL-030, SRS-005-SHALL-064, SRS-005-SHALL-066
 */

#ifndef AXILOG_TYPES_H
#define AXILOG_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================
 * Q16.16 Fixed-Point Type
 * ======================================================================== */

/**
 * @brief Q16.16 fixed-point type.
 *
 * Provides 16 bits of integer range [-32768, 32767] and 16 bits of
 * fractional precision (1/65536 ≈ 0.0000153).
 *
 * SRS-005-SHALL-001: Q16.16 representation in int32_t
 * SRS-005-SHALL-004: Two's-complement requirement
 */
typedef int32_t q16_16_t;

/**
 * @brief Q16.16 constants.
 *
 * SRS-005-SHALL-002: Unity constant = 65536
 * SRS-005-SHALL-003: Representation bounds
 */
#define Q16_SHIFT  16
#define Q16_ONE    65536                  /**< 1.0 = 0x00010000 */
#define Q16_HALF   32768                  /**< 0.5 = 0x00008000 */
#define Q16_MAX    INT32_MAX              /**< Maximum value = 0x7FFFFFFF */
#define Q16_MIN    INT32_MIN              /**< Minimum value = 0x80000000 */
#define Q16_EPS    1                      /**< Smallest positive = 0x00000001 */

/* ========================================================================
 * Fault Flags
 * ======================================================================== */

/**
 * @brief Fault flags for arithmetic operations.
 *
 * Every L1 primitive accepts a fault context pointer. Once a flag is set,
 * it remains set until explicitly cleared by the caller.
 *
 * SRS-005-SHALL-029: Fault context acceptance
 * SRS-005-SHALL-030: Arithmetic fault types
 * SRS-005-SHALL-035: Fault propagation
 *
 * Note: The 'protocol' field is reserved for higher-layer integration
 * (L5 agent state machines, L4 policy violations). L1 primitives SHALL
 * NOT set this flag.
 */
typedef struct {
    uint8_t overflow;    /**< Result exceeded Q16_MAX */
    uint8_t underflow;   /**< Result exceeded Q16_MIN (negative overflow) */
    uint8_t div_zero;    /**< Division by zero attempted */
    uint8_t domain;      /**< Input outside valid domain */
    uint8_t precision;   /**< Precision loss occurred */
    uint8_t encoding;    /**< Reserved: L3 encoding validation */
    uint8_t schema;      /**< Reserved: L3 schema validation */
    uint8_t ordering;    /**< Reserved: L3 ordering violation */
    uint8_t size;        /**< Reserved: L3 size limit */
    uint8_t protocol;    /**< Reserved: L5+ integration */
} ct_fault_flags_t;

/**
 * @brief Clear all fault flags.
 *
 * SRS-005-SHALL-035: Fault propagation (caller clears)
 */
static inline void ct_fault_clear(ct_fault_flags_t *f)
{
    f->overflow = 0;
    f->underflow = 0;
    f->div_zero = 0;
    f->domain = 0;
    f->precision = 0;
    f->encoding = 0;
    f->schema = 0;
    f->ordering = 0;
    f->size = 0;
    f->protocol = 0;
}

/**
 * @brief Check if any fault flag is set.
 *
 * SRS-005-SHALL-034: No silent failure
 */
static inline bool ct_fault_any(const ct_fault_flags_t *f)
{
    return f->overflow || f->underflow || f->div_zero ||
           f->domain || f->precision || f->encoding ||
           f->schema || f->ordering || f->size || f->protocol;
}

/**
 * @brief Check if any arithmetic fault is set.
 *
 * Checks only L1-level faults (overflow, underflow, div_zero, domain).
 */
static inline bool ct_fault_arithmetic(const ct_fault_flags_t *f)
{
    return f->overflow || f->underflow || f->div_zero || f->domain;
}

/* ========================================================================
 * Sign Classification
 * ======================================================================== */

/**
 * @brief Sign classification enumeration.
 *
 * SRS-005-SHALL-026: Sign classification
 */
typedef enum {
    AX_SIGN_NEGATIVE = -1,
    AX_SIGN_ZERO     =  0,
    AX_SIGN_POSITIVE =  1
} ax_sign_t;

/* ========================================================================
 * Return Codes
 * ======================================================================== */

/**
 * @brief Standard return codes for L1 functions.
 */
#define AX_OK           0
#define AX_ERR_OVERFLOW 1
#define AX_ERR_UNDERFLOW 2
#define AX_ERR_DIV_ZERO 3
#define AX_ERR_DOMAIN   4

#endif /* AXILOG_TYPES_H */
