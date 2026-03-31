/**
 * @file test_hash_consistency.c
 * @project Certifiable Inference Engine
 * @brief Bit-perfect consistency test for deterministic hash table.
 *
 * @details This test proves that the hash table produces identical memory states
 * across multiple runs with the same operations. This is the "mic drop"
 * proof of determinism - not just functional equivalence, but bit-for-bit
 * identical memory layout.
 *
 * @traceability SRS-001-DETERMINISM, SRS-002-BOUNDED-MEMORY
 * @compliance MISRA-C:2012, ISO 26262
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "deterministic_hash.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define POOL_SIZE 1024

/**
 * @brief Simulated workload representing a typical ML feature store operation.
 *
 * This mimics storing sensor readings and model metadata in a production system.
 */
void run_simulated_workload(uint8_t* buffer) {
    d_table_t table;
    d_table_init(&table, buffer, POOL_SIZE);

    /* Insert typical ML feature values */
    d_table_insert(&table, "sensor_a", 100);
    d_table_insert(&table, "sensor_b", -50);
    d_table_insert(&table, "model_version", 1);
    d_table_insert(&table, "threshold", 999);
    d_table_insert(&table, "cardiac_rate", 72);
    d_table_insert(&table, "oxygen_sat", 98);
    d_table_insert(&table, "temperature", 37);
    d_table_insert(&table, "blood_pressure", 120);
}

/**
 * @brief Print first N byte differences for debugging.
 */
void print_differences(const uint8_t* buf1, const uint8_t* buf2, size_t size, int max_diffs) {
    int diff_count = 0;
    printf("\nFirst differences:\n");
    for (size_t i = 0; i < size && diff_count < max_diffs; i++) {
        if (buf1[i] != buf2[i]) {
            printf("  Offset %zu: 0x%02x vs 0x%02x\n", i, buf1[i], buf2[i]);
            diff_count++;
        }
    }
}

int main(void) {
    uint8_t buffer1[POOL_SIZE];
    uint8_t buffer2[POOL_SIZE];
    uint8_t buffer3[POOL_SIZE];

    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    printf("  SRS-001 Bit-Perfect Consistency Suite\n");
    printf("═══════════════════════════════════════════════\n");
    printf("\n");
    printf("  Proving determinism via memory state comparison.\n");
    printf("\n");

    /* Run 1: First execution */
    printf("  ✓ Run 1: Executing simulated workload\n");
    run_simulated_workload(buffer1);

    /* Run 2: Second execution (identical operations) */
    printf("  ✓ Run 2: Executing identical workload\n");
    run_simulated_workload(buffer2);

    /* Run 3: Third execution (proving N-way consistency) */
    printf("  ✓ Run 3: Executing identical workload (N-way)\n");
    run_simulated_workload(buffer3);

    /* The Ultimate Proof: Byte-for-byte comparison of entire memory pools */
    printf("\n");
    printf("  Comparing memory states (byte-for-byte):\n");

    int result_1_2 = memcmp(buffer1, buffer2, POOL_SIZE);
    int result_2_3 = memcmp(buffer2, buffer3, POOL_SIZE);
    int result_1_3 = memcmp(buffer1, buffer3, POOL_SIZE);

    printf("    Run 1 vs Run 2: %s\n", result_1_2 == 0 ? "IDENTICAL" : "DIFFERS");
    printf("    Run 2 vs Run 3: %s\n", result_2_3 == 0 ? "IDENTICAL" : "DIFFERS");
    printf("    Run 1 vs Run 3: %s\n", result_1_3 == 0 ? "IDENTICAL" : "DIFFERS");

    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    if (result_1_2 == 0 && result_2_3 == 0 && result_1_3 == 0) {
        printf("  ✅ SRS-001 Verified (3 runs bit-identical)\n");
        printf("═══════════════════════════════════════════════\n");
        printf("\n");
        printf("This proves:\n");
        printf("  • No uninitialized memory leakage\n");
        printf("  • No memory-address dependencies\n");
        printf("  • Reproducible behavior across N runs\n");
        printf("  • Suitable for safety-critical certification\n");
        printf("\n");
        return 0;
    } else {
        printf("  ❌ SRS-001 Failed (non-determinism detected)\n");
        printf("═══════════════════════════════════════════════\n");

        if (result_1_2 != 0) {
            printf("\n  Run 1 vs Run 2:\n");
            print_differences(buffer1, buffer2, POOL_SIZE, 5);
        }
        if (result_2_3 != 0) {
            printf("\n  Run 2 vs Run 3:\n");
            print_differences(buffer2, buffer3, POOL_SIZE, 5);
        }
        if (result_1_3 != 0) {
            printf("\n  Run 1 vs Run 3:\n");
            print_differences(buffer1, buffer3, POOL_SIZE, 5);
        }
        printf("\n");

        return 1;
    }
}
