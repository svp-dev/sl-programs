//
// sep.c: this file is part of the SL program suite.
//
// Copyright (C) 2009,2010 The SL project.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// The complete GNU General Public Licence Notice can be found as the
// `COPYING' file in the root directory.
//

#include <stdio.h>
#include <svp/sep.h>
#include <svp/perf.h>
#include <svp/slr.h>

#include <svp/testoutput.h>

// SLT_RUN: L=1 nmax=15
// XIGNORE: *:D

slr_decl(slr_var(size_t, L, "number of rounds"),
	 slr_var(size_t, nmax, "max nr policies used"),
         slr_var(int, dump, "dump status between reqs"));

static
int lrand(void)
{
    static unsigned long long next = 0;
    next = next * 0x5deece66dLL + 11;
    return (int)((next >> 16) & 0x7fffffff);
}

#define MAKEREQ(Val1) { #Val1, SEP_ALLOC|(Val1)}

struct req {
    const char *desc;
    unsigned long request;
    void  *arg;
};

static
struct req sizes[] = {
    MAKEREQ(SAL_DONTCARE),
    MAKEREQ(SAL_DONTCARE),
    MAKEREQ(SAL_DONTCARE),
    MAKEREQ(SAL_MIN),
    MAKEREQ(SAL_MIN),
    MAKEREQ(SAL_MAX),
    MAKEREQ(SAL_MAX),
    MAKEREQ(SAL_EXACT)
};

struct results {
    const char *sizedesc;
    sl_place_t result;
};

#define MAXTRIES 50
static
struct results res[MAXTRIES];

extern void sep_dump_info(struct SEP*);

sl_def(t_main, void)
{

  struct SEP* restrict sep = root_sep;
  size_t N = sizeof(res) / sizeof(res[0]);
  const size_t nsizes = sizeof(sizes) / sizeof(sizes[0]);
  int dump = 0;

  size_t n;
  size_t L = 4;
  if (slr_len(L)) L = slr_get(L)[0];
  if (slr_len(nmax) && slr_get(nmax)[0] < N) N = slr_get(nmax)[0];
  if (slr_len(dump)) dump = 1;

  struct s_interval ct[2*N*L];
  int cti = 0;

  for (n = 0; n < L; ++n) {
    size_t i;
    
    if (dump) {
        printf("Status dump (before alloc %zu):\n", n);
        sep_dump_info(sep);
    }
    
    for (i = 0; i < N; ++i) {
        int r = lrand();
        r = (r & 0xff) ^ ((r >> 8) & 0xff) ^ ((r >> 16) & 0xff);
        struct req *s = &sizes[r % nsizes];
        r = (r & 0x1f) ^ ((r >> 8) & 0x1f) ^ ((r >> 16) & 0x1f);
        size_t ncores = r;

        res[i].sizedesc = s->desc;

        mtperf_start_interval(ct, cti, i, "alloc");
        int code = sep_alloc(sep, &(res[i].result), s->request, ncores);
        mtperf_finish_interval(ct, cti++);

        const char *success = (code != -1) ? "yes" : "no";

        printf("Round: %zu %zu\tPolicy: %s|%zu\tSucceeded: %s (%#lx)\n",
               n, i, res[i].sizedesc, ncores, success, (unsigned long)res[i].result);

        if (dump) {
            printf("Status dump (after alloc %zu %zu):\n", n, i);
            sep_dump_info(sep);
        }
    }

    // dealloc

    for (i = 0; i < N; ++i) {
        if (res[i].result) {
            mtperf_start_interval(ct, cti, i, "dealloc");
            sep_free(sep, &res[i].result);
            mtperf_finish_interval(ct, cti++);

            if (dump) {
                printf("\nStatus dump (after free %zu %zu):\n", n, i);
                sep_dump_info(sep);
            }
        }
        else
            mtperf_empty_interval(ct, cti++, i, "dealloc");
    }


  }

  printf("\nPerformance report (%d):\n", cti);
  mtperf_report_intervals(ct, cti, REPORT_CSV|CSV_INCLUDE_HEADER|CSV_SEP('\t'));

}
sl_enddef
