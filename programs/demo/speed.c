//
// speed.c: this file is part of the SL program suite.
//
// Copyright (C) 2012-2015 The SL project.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// The complete GNU General Public Licence Notice can be found as the
// `COPYING' file in the root directory.
//
#include <time.h>
#include <stdio.h>
#include <svp/perf.h>

// SLT_RUN: 1
// XIGNORE: *:D

int main(int argc, char **argv)
{
    time_t start_t, expected_t, end_t;
    clock_t start_c, end_c;


    struct s_interval iv;

    start_t = time(0);
    expected_t = start_t + atoi(argv[1]);

    printf("starting at %lu, waiting to %lu...\n",
           (unsigned long)start_t, (unsigned long)expected_t);

    start_c = clock();

    mtperf_start_interval(&iv, 0, 0, "loop");
    while((end_t = time(0)) < expected_t)
        ;
    mtperf_finish_interval(&iv, 0);

    end_c = clock();

    clock_t c = end_c - start_c;
    time_t t = end_t - start_t;

    printf("%lu clocks in %lu seconds: %lu clocks/s\n\n",
           (unsigned long)c, (unsigned long)t, (unsigned long)(c/t));

    mtperf_report_intervals(&iv, 1, REPORT_FIBRE|REPORT_STREAM(1));

    return 0;
}
