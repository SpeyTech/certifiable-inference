/**
 * @file debug_conv.c
 * @project Certifiable Inference Engine
 * @brief Debug utility for convolution testing.
 *
 * @traceability SRS-006-CONVOLUTION
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "convolution.h"
#include "fixed_point.h"
#include <stdio.h>

int main(void) {
    printf("Debug: Basic 3×3 Convolution\n");
    printf("════════════════════════════\n\n");

    /* Allocate buffers */
    fixed_t in_data[25];
    fixed_t kernel_data[9];
    fixed_t out_data[9];

    /* Initialize matrices FIRST (this zeros the buffers) */
    fx_matrix_t in, kernel, out;
    fx_matrix_init(&in, in_data, 5, 5);
    fx_matrix_init(&kernel, kernel_data, 3, 3);
    fx_matrix_init(&out, out_data, 3, 3);

    /* Set values AFTER init */
    for (int i = 0; i < 25; i++) {
        in.data[i] = fixed_from_int(1);
    }

    for (int i = 0; i < 9; i++) {
        kernel.data[i] = fixed_from_int(1);
    }

    /* Perform convolution */
    fx_conv2d(&in, &kernel, &out);

    /* Print results */
    printf("Expected: All values = 9\n");
    printf("Actual output:\n");
    for (int i = 0; i < 9; i++) {
        int val = fixed_to_int(out.data[i]);
        printf("  out[%d] = %d (raw: 0x%08x)\n", i, val, out.data[i]);
    }

    printf("\n");
    printf("Expected fixed-point: 0x%08x (9 as Q16.16)\n", fixed_from_int(9));

    return 0;
}
