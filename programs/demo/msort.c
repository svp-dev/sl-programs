//
// msort.c: this file is part of the SL program suite.
//
// Copyright (C) 2011-2015 The SL project.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// The complete GNU General Public Licence Notice can be found as the
// `COPYING' file in the root directory.
//
#include <svp/testoutput.h>
#include <svp/fibre.h>
#include <svp/perf.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define T unsigned long

sl_def(printdata,sl__static, sl_glparm(T*, data), sl_shparm(long, tok))
{
    sl_index(i);
    T d = sl_getp(data)[i];
    long t = sl_getp(tok);
    output_int(d, 1);
    output_char('\n', 1);
    sl_setp(tok, t);
}
sl_enddef

sl_def(copy,sl__static,sl_glparm(T*restrict,dst), sl_glparm(T*restrict,src))
{
    sl_index(i);
    sl_getp(dst)[i] = sl_getp(src)[i];
}
sl_enddef

sl_def(merge, ,
       sl_glparm(T*restrict, dst),
       sl_glparm(T*restrict, left),
       sl_glparm(T*restrict, right),
       sl_glparm(size_t, lrmax),
       sl_shparm(size_t, ileft),
       sl_shparm(size_t, iright));
{
    sl_index(idst);
    size_t il = sl_getp(ileft);
    size_t ir = sl_getp(iright);
    if (il < sl_getp(lrmax) && ir < sl_getp(lrmax))
    {
        T l = sl_getp(left)[il];
        T r = sl_getp(right)[ir];
        if (l <= r) {
            sl_setp(ileft, il+1);
            sl_setp(iright, ir);
            sl_getp(dst)[idst] = l;
        } else {
            sl_setp(ileft, il);
            sl_setp(iright, ir+1);
            sl_getp(dst)[idst] = r;
        }
    }
    else if (il < sl_getp(lrmax))
    {
        sl_setp(ileft, il+1);
        sl_setp(iright, ir);
        sl_getp(dst)[idst] = sl_getp(left)[il];
    }
    else
    {
        sl_setp(ileft, il);
        sl_setp(iright, ir+1);
        sl_getp(dst)[idst] = sl_getp(right)[ir];
    }
}
sl_enddef


sl_def(merge_sort, ,sl_glparm(T*restrict, data), sl_glparm(size_t, N))
{
    sl_index(where);
    size_t N = sl_getp(N);
    if (N <= 1)
        sl_end_thread;

    size_t N2 = N/2;
    T dcopy[N];
    T *restrict left = &dcopy[0];
    T *restrict right = &dcopy[N2];

    sl_create(,,0,N,,,, copy, sl_glarg(T*, ,dcopy), sl_glarg(T*,,sl_getp(data)+where));
    sl_sync();

#if 0
    output_string("-- left, before --\n", 1);
    sl_create(,,,N2,,,, printdata,
              sl_glarg(T*, ,left), sl_sharg(long, , 0));
    sl_sync();
    output_string("-- right, before --\n", 1);
    sl_create(,,,N2,,,, printdata,
              sl_glarg(T*, ,right), sl_sharg(long, , 0));
    sl_sync();
#endif

    sl_create(,,0,N,N2,,, merge_sort, sl_glarg(T*,,dcopy), sl_glarg(size_t,,N2));
//    sl_create(,,,,,,, merge_sort, sl_glarg(T*,,right), sl_glarg(size_t,,N2));
//    sl_sync();
    sl_sync();

#if 0
    output_string("-- left, after --\n", 1);
    sl_create(,,,N2,,,, printdata,
              sl_glarg(T*, ,left), sl_sharg(long, , 0));
    sl_sync();
    output_string("-- right, after --\n", 1);
    sl_create(,,,N2,,,, printdata,
              sl_glarg(T*, ,right), sl_sharg(long, , 0));
    sl_sync();
#endif

    sl_create(,,0,N,,,, merge,
              sl_glarg(T*,, sl_getp(data)+where),
              sl_glarg(T*, ,left),
              sl_glarg(T*, ,right),
              sl_glarg(size_t, , N2),
              sl_sharg(size_t, , 0),
              sl_sharg(size_t, , 0));
    sl_sync();

#if 0
    output_string("-- data, merged --\n", 1);
    sl_create(,,,N,,,, printdata,
              sl_glarg(T*, ,sl_getp(data)+where), sl_sharg(long, , 0));
    sl_sync();
#endif
}
sl_enddef

sl_def(t_main,,)
{
    bool doprint = false;

    assert(fibre_tag(0) == 0);
    assert(fibre_rank(0) == 1);

    size_t N = fibre_shape(0)[0];
    T *data = (T*)fibre_data(0);

    if (getenv("MSORT_SHOW_INPUT")) doprint = true;

    if (doprint) {
        output_string("-- input --\n", 1);
        sl_create(,,,N,,,, printdata,
                  sl_glarg(T*, ,data), sl_sharg(long, , 0));
        sl_sync();
    }

    counter_t a[MTPERF_NCOUNTERS];
    counter_t b[MTPERF_NCOUNTERS];
    mtperf_sample(b);

    sl_create(,,,,,,, merge_sort, sl_glarg(T*, ,data), sl_glarg(size_t, , N));
    sl_sync();

    mtperf_sample(a);

    if (doprint) {
        output_string("-- output --\n", 1);
        sl_create(,,,N,,,, printdata,
                  sl_glarg(T*, ,data), sl_sharg(long, , 0));
        sl_sync();
    }

    output_string("-- performance --\n", 1);

    mtperf_report_diffs(b, a, REPORT_CSV | CSV_INCLUDE_HEADER | CSV_SEP(' '));

}
sl_enddef
