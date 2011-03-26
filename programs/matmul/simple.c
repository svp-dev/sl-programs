#include <svp/fibre.h>
#include <svp/perf.h>
#include <assert.h>
#include <stdio.h>

#include "matmul.c"

sl_def(t_main, void)
{
    size_t f = 0;

    size_t L = 1;

    if (fibre_rank(1) == 0)
    {
        assert(fibre_tag(0) == 0);
        assert(fibre_rank(0) == 0);
        L = *(unsigned long*)fibre_data(0);
        ++f;
    }

    assert(fibre_tag(f) == 0);
    assert(fibre_rank(f) == 0);
    size_t input_n = *(unsigned long*)fibre_data(f);
#ifdef N
    assert(N == input_n);
#else
    size_t N = input_n;
#endif
    
    ++f;
    assert(fibre_rank(f) == 2);
    assert(fibre_shape(f)[0] == N);
    assert(fibre_shape(f)[1] == N);
    ++f;
    assert(fibre_rank(f) == 2);
    assert(fibre_shape(f)[0] == N);
    assert(fibre_shape(f)[1] == N);
    ++f;
    assert(fibre_rank(f) == 2);
    assert(fibre_shape(f)[0] == N);
    assert(fibre_shape(f)[1] == N);
    ++f;
    assert(fibre_rank(f) == 2);
    assert(fibre_shape(f)[0] == N);
    assert(fibre_shape(f)[1] == N);

    number (*a)[N][N] = (number (*)[N][N])fibre_data(2);
    number (*b)[N][N] = (number (*)[N][N])fibre_data(3);
    number (*expected)[N][N] = (number (*)[N][N])fibre_data(4);
    number (*c)[N][N] = (number (*)[N][N])fibre_data(5);


    struct s_interval ivs[L + 1];
    size_t i, j;

    mtperf_start_interval(ivs, 0, 0, "work");
    for (i = 1; i <= L; ++i)
    {
        mtperf_start_interval(ivs, i, i, "compute");
        sl_proccall(matmul,
                    sl_glarg(void*, , a),
                    sl_glarg(void*, , b),
                    sl_glarg(void*, , c)
#ifndef N
                    ,       sl_glarg(size_t, , N)
#endif
            );
        mtperf_finish_interval(ivs, i);
    }
    mtperf_finish_interval(ivs, 0);
    mtperf_report_intervals(ivs, L, REPORT_FIBRE|REPORT_STREAM(2));

    number max = 0;
    for (i = 0; i < N; ++i)
        for (j = 0; j < N; ++j)
        {
            number v = (*c)[i][j] - (*expected)[i][j];
            if (v < 0) v = -v;
            if (v > max) max = v;
        }
    printf("%ld\n", ((long)(max * 1000)));
}
sl_enddef
