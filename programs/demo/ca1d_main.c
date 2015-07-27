//
// ca1d_main.c: this file is part of the SL program suite.
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
#include <svp/testoutput.h>
#include <svp/perf.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "ca1d.h"
#ifdef DRAW
#include <svp/gfx.h>
#endif

int main(int argc, char **argv)
{
    size_t w = 98;
    size_t iters = 99;
    rule1D rule = 60;
    bool print = false;
    size_t block = 0;
    char *init = 0;
    int ch;

    while ((ch = getopt(argc, argv, "r:w:i:n:db:h")) != -1)
        switch (ch) {
        case 'h':
            output_string("usage: ", 2);
            output_string(argv[0], 2);
            output_string(" [OPTIONS...]\n"
                          "Options:\n"
                          "  -h    Print this help.\n"
                          "  -n N  Run N iterations.\n"
                          "  -d    Print state between iterations.\n"
                          "  -b N  Use max N threads per core.\n"
                          "  -i VV Use VV as initial state.\n"
                          "  -r N  Use Wolfram rule N.\n"
                          "  -w N  Set space width to W items.\n", 2);
            return 0;
        case 'd':
            print = true;
            break;
        case 'w':
            w = atoi(optarg);
            break;
        case 'n':
            iters = atoi(optarg);
            break;
        case 'r':
            rule = atoi(optarg);
            break;
        case 'b':
            block = atoi(optarg);
            break;
        case 'i':
            init = optarg;
            break;
        }

    size_t row_width = w + 2;
    ++iters;

#ifdef DRAW
    gfx_init();
    gfx_resize(row_width, iters);
#endif

    /* init space */
    cell (*rows)[iters][row_width] = (cell (*)[iters][row_width])calloc(iters, row_width);

    /* copy initial state */
    if (init)
    {
        size_t init_len = strlen(init);
        for (size_t i = 0; i < init_len; ++i)
        {
            bool v = (init[i] == '1');
            (*rows)[0][1+i] = v;
#ifdef DRAW
            gfx_fb_set(1+i, v ? -1 : 0);
#endif
        }
    }
    else
    {
        (*rows)[0][1] = true;
#ifdef DRAW
        gfx_fb_set(1, -1);
#endif
    }

    /* maybe print init */
    if (print)
    {
        for (size_t j = 1; j < w+1; ++j)
            output_char((*rows)[0][j] ? 'O' : '.', 1);
        output_char(' ', 1);
        output_uint(0, 1);
        output_char('\n', 1);
    }        

    /* iterate */

    struct s_interval iv[iters];
    mtperf_start_interval(iv, 0, 0, "all");
    for (size_t i = 1; i < iters; ++i)
    {
        mtperf_start_interval(iv, i, i, "compute");
        sl_create(,,,,,,, cycle1D,
                  sl_glarg(size_t,,w),
                  sl_glarg(cell*,, (*rows)[i-1]),
                  sl_glarg(cell*,, (*rows)[i]),
                  sl_glarg(rule1D,, rule),
                  sl_glarg(size_t,, block)
#ifdef DRAW
                  , sl_glarg(size_t,, i * row_width)
#endif
);
        sl_sync();
        mtperf_finish_interval(iv, i);

        if (print)
        {
            for (size_t j = 1; j < w+1; ++j)
                output_char((*rows)[i][j] ? 'O' : '.', 1);
            output_char(' ', 1);
            output_uint(i, 1);
            output_char('\n', 1);
        }        
    }
    mtperf_finish_interval(iv, 0);

#ifdef DRAW
    gfx_dump(0, 1, 0, 0);
    gfx_close();
#endif

    mtperf_report_intervals(iv, iters, REPORT_FIBRE|REPORT_STREAM(2));

    return 0;
}
