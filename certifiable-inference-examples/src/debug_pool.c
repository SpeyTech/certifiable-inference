/**
 * @file debug_pool.c
 * @project Certifiable Inference Engine
 * @brief Debug utility for pooling testing.
 *
 * @traceability SRS-008-POOLING
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "pooling.h"
#include "fixed_point.h"
#include <stdio.h>

int main(void) {
    printf("Debug: Basic 4×4 Max Pooling\n");
    printf("════════════════════════════\n\n");

    /* Allocate buffers */
    fixed_t in_data[16];
    fixed_t out_data[4];

    /* Initialize matrices FIRST (this zeros the buffers) */
    fx_matrix_t in, out;
    fx_matrix_init(&in, in_data, 4, 4);
    fx_matrix_init(&out, out_data, 2, 2);

    /* Set values AFTER init: 4×4 matrix with values 1-16
     * Layout:
     *   [  1  2  3  4 ]
     *   [  5  6  7  8 ]
     *   [  9 10 11 12 ]
     *   [ 13 14 15 16 ]
     */
    for (int i = 0; i < 16; i++) {
        in.data[i] = fixed_from_int(i + 1);
    }

    /* Perform pooling */
    fx_maxpool_2x2(&in, &out);

    /* Print results
     * 2×2 max pooling with stride 2:
     *   Window [1,2,5,6]   → max = 6
     *   Window [3,4,7,8]   → max = 8
     *   Window [9,10,13,14] → max = 14
     *   Window [11,12,15,16] → max = 16
     */
    printf("Expected: [6, 8, 14, 16]\n");
    printf("Actual output:\n");
    for (int i = 0; i < 4; i++) {
        int val = fixed_to_int(out.data[i]);
        printf("  out[%d] = %d (raw: 0x%08x)\n", i, val, out.data[i]);
    }

    printf("\nExpected values as Q16.16:\n");
    printf("  6:  0x%08x\n", fixed_from_int(6));
    printf("  8:  0x%08x\n", fixed_from_int(8));
    printf("  14: 0x%08x\n", fixed_from_int(14));
    printf("  16: 0x%08x\n", fixed_from_int(16));

    return 0;
}
