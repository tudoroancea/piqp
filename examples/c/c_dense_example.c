// This file is part of PIQP.
//
// Copyright (c) 2023 EPFL
//
// This source code is licensed under the BSD 2-Clause License found in the
// LICENSE file in the root directory of this source tree.

#include "piqp.h"
#include "stdio.h"
#include "stdlib.h"

int main()
{
    piqp_int n = 2;
    piqp_int p = 1;
    piqp_int m = 2;

    piqp_float P[4] = {6, 0, 0, 4};
    piqp_float c[2] = {-1, -4};

    piqp_float A[2] = {1, -2};
    piqp_float b[1] = {1};

    piqp_float G[4] = {1, -1, 2, 0};
    piqp_float h[2] = {0.2, -1};

    piqp_float x_lb[2] = {-1, -PIQP_INF};
    piqp_float x_ub[2] = {1, PIQP_INF};

    piqp_workspace* work;
    piqp_settings* settings = (piqp_settings*) malloc(sizeof(piqp_settings));
    piqp_data_dense* data = (piqp_data_dense*) malloc(sizeof(piqp_data_dense));

    piqp_set_default_settings(settings);
    settings->verbose = 1;
    settings->compute_timings = 1;

    data->n = n;
    data->p = p;
    data->m = m;
    data->P = P;
    data->c = c;
    data->A = A;
    data->b = b;
    data->G = G;
    data->h = h;
    data->x_lb = x_lb;
    data->x_ub = x_ub;

    piqp_setup_dense(&work, data, settings);
    piqp_status status = piqp_solve(work);

    printf("status = %d\n", status);
    printf("x = %f %f\n", work->result->x[0], work->result->x[1]);

    piqp_cleanup(work);
    free(settings);
    free(data);

    return 0;
}
