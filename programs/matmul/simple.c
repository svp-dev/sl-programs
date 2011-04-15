#include <svp/fibre.h>
#include <svp/perf.h>
#include <svp/testoutput.h>
#include <assert.h>


#include "matmul.c"

/*
void printm(size_t n, number (*m)[n][n])
{
    for (size_t j = 0; j < n; ++j)
    {
        for (size_t i = 0; i < n; ++i)
        {
            output_int((long)((*m)[j][i]), 1);
            output_char(' ',1);
        }
        output_char('\n', 1);
    }         
}
*/

sl_def(t_main, void)
{
    size_t f = 0;

    size_t L = 1;

    assert(fibre_tag(f) == 0);
    assert(fibre_rank(f) == 0);
    L = *(unsigned long*)fibre_data(f);

    ++f;
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
    number (*a)[N][N] = (number (*)[N][N])fibre_data(f);

    ++f;
    assert(fibre_rank(f) == 2);
    assert(fibre_shape(f)[0] == N);
    assert(fibre_shape(f)[1] == N);
    number (*b)[N][N] = (number (*)[N][N])fibre_data(f);

    ++f;
    assert(fibre_rank(f) == 2);
    assert(fibre_shape(f)[0] == N);
    assert(fibre_shape(f)[1] == N);
    number (*expected)[N][N] = (number (*)[N][N])fibre_data(f);

    ++f;
    assert(fibre_rank(f) == 2);
    assert(fibre_shape(f)[0] == N);
    assert(fibre_shape(f)[1] == N);
    number (*c)[N][N] = (number (*)[N][N])fibre_data(f);

    number (*c_other)[N][N] = malloc(sizeof(number)*N*N);
    assert(c_other != 0);
    number (*tmp)[N][N];


    struct s_interval ivs[L + 1];
    size_t i, j;

    mtperf_start_interval(ivs, 0, 0, "work");
    mtperf_start_interval(ivs, 1, 1, "compute");
    sl_proccall(matmul,
                sl_glarg(void*, , a),
                sl_glarg(void*, , b),
                sl_glarg(void*, , c)
#ifndef N
                , sl_glarg(size_t, , N)
#endif
        );
    mtperf_finish_interval(ivs, 1);

    // printm(N, c); output_string("----\n", 1);
    
    for (i = 2; i <= L; ++i)
    {
        mtperf_start_interval(ivs, i, i, "compute");
        sl_proccall(matmul,
                    sl_glarg(void*, , a),
                    sl_glarg(void*, , c),
                    sl_glarg(void*, , c_other)
#ifndef N
                    , sl_glarg(size_t, , N)
#endif
            );
        mtperf_finish_interval(ivs, i);
        tmp = c;
        c = c_other;
        c_other = tmp;

        // printm(N, c); output_string("----\n", 1);
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
    output_int(((long)(max * 1000)), 1);
    output_char('\n', 1);
}
sl_enddef
