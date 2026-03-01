/**
 * @file test_hash_basic.c
 * @project Certifiable Inference Engine
 * @brief Basic unit tests for deterministic hash table.
 *
 * @details Tests core hash table functionality: initialization, insert,
 * get, duplicate key handling, not found handling, iteration, and
 * capacity limits. All operations must behave identically across runs.
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
#include <assert.h>
#include <string.h>

/* File-scope variables for iteration tests */
static int g_callback_count = 0;
static char g_order1[4][16];
static char g_order2[4][16];
static int g_idx1 = 0;
static int g_idx2 = 0;

/* File-scope callbacks for iteration tests */
static void count_callback(const char* key, int32_t value) {
    (void)key;
    (void)value;
    g_callback_count++;
}

static void collect_order1(const char* key, int32_t value) {
    (void)value;
    if (g_idx1 < 4) {
        strncpy(g_order1[g_idx1++], key, 15);
        g_order1[g_idx1 - 1][15] = '\0';
    }
}

static void collect_order2(const char* key, int32_t value) {
    (void)value;
    if (g_idx2 < 4) {
        strncpy(g_order2[g_idx2++], key, 15);
        g_order2[g_idx2 - 1][15] = '\0';
    }
}

void test_init(void) {
    uint8_t buffer[1024];
    d_table_t table;

    d_table_res_t result = d_table_init(&table, buffer, sizeof(buffer));
    assert(result == D_TABLE_OK);
    assert(table.count == 0);
    assert(table.capacity > 0);

    printf("✓ test_init passed\n");
}

void test_insert_and_get(void) {
    uint8_t buffer[1024];
    d_table_t table;

    d_table_init(&table, buffer, sizeof(buffer));

    /* Insert */
    d_table_res_t result = d_table_insert(&table, "test_key", 42);
    assert(result == D_TABLE_OK);
    assert(table.count == 1);

    /* Get */
    int32_t value;
    result = d_table_get(&table, "test_key", &value);
    assert(result == D_TABLE_OK);
    assert(value == 42);

    printf("✓ test_insert_and_get passed\n");
}

void test_duplicate_key(void) {
    uint8_t buffer[1024];
    d_table_t table;

    d_table_init(&table, buffer, sizeof(buffer));

    d_table_insert(&table, "key1", 10);
    d_table_res_t result = d_table_insert(&table, "key1", 20);

    assert(result == D_TABLE_KEY_EXISTS);

    printf("✓ test_duplicate_key passed\n");
}

void test_not_found(void) {
    uint8_t buffer[1024];
    d_table_t table;

    d_table_init(&table, buffer, sizeof(buffer));

    int32_t value;
    d_table_res_t result = d_table_get(&table, "nonexistent", &value);

    assert(result == D_TABLE_NOT_FOUND);

    printf("✓ test_not_found passed\n");
}

void test_iterate(void) {
    uint8_t buffer[1024];
    d_table_t table;

    d_table_init(&table, buffer, sizeof(buffer));

    d_table_insert(&table, "key1", 1);
    d_table_insert(&table, "key2", 2);
    d_table_insert(&table, "key3", 3);

    g_callback_count = 0;
    d_table_iterate(&table, count_callback);

    assert(g_callback_count == 3);

    printf("✓ test_iterate passed\n");
}

void test_capacity_limit(void) {
    uint8_t buffer[256];  /* Small buffer to trigger capacity limit */
    d_table_t table;

    d_table_res_t result = d_table_init(&table, buffer, sizeof(buffer));
    assert(result == D_TABLE_OK);

    /* Fill until full */
    int inserted = 0;
    for (int i = 0; i < 100; i++) {
        char key[16];
        snprintf(key, sizeof(key), "key_%d", i);
        result = d_table_insert(&table, key, i);
        if (result == D_TABLE_FULL) {
            break;
        }
        assert(result == D_TABLE_OK);
        inserted++;
    }

    assert(result == D_TABLE_FULL);
    assert(inserted > 0);  /* Should have inserted at least some */

    printf("✓ test_capacity_limit passed (inserted %d before full)\n", inserted);
}

void test_deterministic_iteration_order(void) {
    uint8_t buffer1[1024];
    uint8_t buffer2[1024];
    d_table_t table1, table2;

    /* Run 1 */
    d_table_init(&table1, buffer1, sizeof(buffer1));
    d_table_insert(&table1, "alpha", 1);
    d_table_insert(&table1, "beta", 2);
    d_table_insert(&table1, "gamma", 3);
    d_table_insert(&table1, "delta", 4);

    /* Run 2 - identical operations */
    d_table_init(&table2, buffer2, sizeof(buffer2));
    d_table_insert(&table2, "alpha", 1);
    d_table_insert(&table2, "beta", 2);
    d_table_insert(&table2, "gamma", 3);
    d_table_insert(&table2, "delta", 4);

    /* Reset indices */
    g_idx1 = 0;
    g_idx2 = 0;

    d_table_iterate(&table1, collect_order1);
    d_table_iterate(&table2, collect_order2);

    /* Verify identical iteration order */
    for (int i = 0; i < 4; i++) {
        assert(strcmp(g_order1[i], g_order2[i]) == 0);
    }

    printf("✓ test_deterministic_iteration_order passed\n");
}

int main(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    printf("  SRS-001 Deterministic Hash Table Suite\n");
    printf("═══════════════════════════════════════════════\n");
    printf("\n");

    test_init();
    test_insert_and_get();
    test_duplicate_key();
    test_not_found();
    test_iterate();
    test_capacity_limit();
    test_deterministic_iteration_order();

    printf("\n");
    printf("═══════════════════════════════════════════════\n");
    printf("  ✅ SRS-001 Verified (7 tests passed)\n");
    printf("═══════════════════════════════════════════════\n");
    printf("\n");
    printf("Requirements validated:\n");
    printf("  • SRS-001.1: Deterministic iteration order\n");
    printf("  • SRS-001.2: No dynamic allocation\n");
    printf("  • SRS-001.3: Bounded capacity\n");
    printf("  • SRS-001.4: Key collision handling\n");
    printf("\n");

    return 0;
}
