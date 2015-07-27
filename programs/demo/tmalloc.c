//
// tmalloc.c: this file is part of the SL program suite.
//
// Copyright (C) 2009-2015 The SL project.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// The complete GNU General Public Licence Notice can be found as the
// `COPYING' file in the root directory.
//

#include <svp/perf.h>
#include <svp/testoutput.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXP 200000

void* pointers[MAXP];

sl_def(doalloc, void)
{
    sl_index(i);
    int s = i & 511;
    pointers[i] = malloc((s >> 6) | (s & 0x28) | ((s & 0x7) << 6));
}
sl_enddef

sl_def(dofree, void)
{
    sl_index(i);
    free(pointers[i]);
}
sl_enddef

#ifdef __mt_freestanding__
extern void tls_malloc_stats(void);
#endif

// SLT_RUN: -- -n 100 -d
// XIGNORE: *:D

int main(int argc, char **argv)
{
    struct s_interval a[3];
    size_t N = 1000;
    long B = 0;
    int mstat = 0;
    int ch;

    while ((ch = getopt(argc, argv, "n:b:dh")) != -1)
        switch (ch) {
        case 'h':
            output_string("usage: ", 2);
            output_string(argv[0], 2);
            output_string(" [OPTIONS...]\n"
                          "Options:\n"
                          "  -h    Print this help.\n"
                          "  -n N  Run max N rounds.\n"
                          "  -d    Dump malloc state.\n"
                          "  -b N  Max N threads per core.\n", 2);
            return 0;
        case 'd':
            mstat = 1;
            break;
        case 'b':
            B = atoi(optarg);
            break;
        case 'n':
            N = atoi(optarg);
            break;
        }


    assert(N <= MAXP);


    mtperf_start_interval(a, 0, 0, "alloc");
    sl_create(,,,N,,B,,doalloc); sl_sync();
    mtperf_finish_interval(a, 0);

#ifdef __mt_freestanding__
    if (mstat) tls_malloc_stats();
#endif

    mtperf_start_interval(a, 1, 0, "free");
    sl_create(,,,N,,B,,dofree); sl_sync();
    mtperf_finish_interval(a, 1);

    mtperf_start_interval(a, 2, 0, "baseline");
    mtperf_finish_interval(a, 2);

    mtperf_report_intervals(a, 3, REPORT_CSV|CSV_SEP('\t')|CSV_INCLUDE_HEADER);

    return 0;
}
