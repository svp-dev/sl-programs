//
// benchmark.c: this file is part of the SL program suite.
//
// Copyright (C) 2009,2010,2011 The SL project.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// The complete GNU General Public Licence Notice can be found as the
// `COPYING' file in the root directory.
//

#include <svp/delegate.h>
#include <svp/sep.h>
#include <svp/perf.h>
#include <svp/slr.h>
#include <svp/testoutput.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "benchmark.h"

#define pr(S) output_string((S), 1)
#define prnl() output_char('\n', 1)

slr_decl(slr_var(unsigned, L, "number of outer iterations (default 3)"),
         slr_var(unsigned, ncores, "(unused)"),
	 slr_var(int, format, "format for benchmark results (default=0=fibre, 1=raw)"),
	 slr_var(int, results, "output computation results (default 0=no)"),
	 slr_var(int, sep_dump, "output initial place configuration (default 0=no)"));

extern sl_place_t __main_place_id;
union placeinfo __main_placeinfo;

sl_def(do_work, ,
       sl_glparm(size_t, p),
       sl_glparm(int, i),
       sl_glparm(struct benchmark *, b),
       sl_glparm(struct s_interval*, intervals),
       sl_glparm(struct benchmark_state*, bs))
{
    mtperf_start_interval(sl_getp(intervals), sl_getp(p), sl_getp(i), "work");
    sl_getp(b)->work(sl_getp(bs));
    mtperf_finish_interval(sl_getp(intervals), sl_getp(p));
}
sl_enddef

void run_benchmark(struct benchmark* b)
{
  /* configuration from environment */
  unsigned L = 3;
  int results = 0;
  int format = 0;
  int sep_dump = 0;

  if (slr_len(L)) L = slr_get(L)[0];
  if (slr_len(results)) results = slr_get(results)[0];
  if (slr_len(format)) format = slr_get(format)[0];
  if (slr_len(sep_dump)) sep_dump = slr_get(sep_dump)[0];

  /* some introduction */
  if (!b->title) b->title = "(unnamed)";
  if (!b->author) b->author = "(anonymous)";
  printf("####\n#### %s\n####\n## By: %s\n", b->title, b->author);
  if (b->description)
    printf("## %s\n", b->description);
  prnl();

  /* prepare intervals and lapses */
  struct s_interval *intervals;
  intervals = (struct s_interval*)calloc(4 + L + 10 * L, sizeof (struct s_interval));

  struct work_lapses wl = { 0, 0, intervals };

  /* prepare benchmark state and place */

  struct benchmark_state bs;
  bs.wl = &wl;
  bs.data = 0;

  int qr = sep_query(root_sep, &__main_place_id, &__main_placeinfo);
  bs.place = (qr == -1) ? 0 : &__main_placeinfo;

  size_t p = 0;

  pr("# 1. initialize...");
  if (b->initialize) {
    mtperf_start_interval(intervals, p, -1, "initialize");
    b->initialize(&bs);
    mtperf_finish_interval(intervals, p++);
    pr("ok\n");
  } else {
    pr("(nothing to do)\n");
    mtperf_empty_interval(intervals, p++, -1, "initialize");
  }

  int i;
  for (i = 0; i < L; ++i) {
    printf("# 2.%u prepare...", i+1);
    if (b->prepare) {
      mtperf_start_interval(intervals, p, i, "prepare");
      b->prepare(&bs);
      mtperf_finish_interval(intervals, p++);
      pr("ok\n");
    } else {
      pr("(nothing to do)\n");
      mtperf_empty_interval(intervals, p++, i, "prepare");
    }

    printf("# 3.%u work...", i+1);
    wl.current_interval = p+1;
    wl.current_iter = i;
    sl_create(, __main_place_id,,,,,, do_work, 
              sl_glarg(size_t, , p),
              sl_glarg(int, , i),
              sl_glarg(struct benchmark*, , b),
              sl_glarg(struct s_interval*, , intervals),
              sl_glarg(struct benchmark_state*, , &bs));
    sl_sync();
    p = wl.current_interval;
    pr("ok\n");
  }

  pr("# 4. results...");
  if (results && b->output) {
    prnl();
    mtperf_start_interval(intervals, p, -1, "output");
    b->output(&bs);
    mtperf_finish_interval(intervals, p++);
    prnl();
  } else {
    pr("(nothing to do)\n");
    mtperf_empty_interval(intervals, p++, -1, "output");
  }

  pr("# 5. teardown...");
  if (b->teardown) {
    mtperf_start_interval(intervals, p, -1, "teardown");
    b->teardown(&bs);
    mtperf_finish_interval(intervals, p++);
    pr("ok\n");
  } else {
    pr("(nothing to do)\n");
    mtperf_empty_interval(intervals, p++, -1, "teardown");
  }
  pr("# done.\n\n");

  output_string("### begin benchmark results\n", 2);
  long report_flags = (format ? (REPORT_CSV|CSV_SEP(' ')|CSV_INCLUDE_HEADER) \
		       : REPORT_FIBRE) | REPORT_STREAM(2);

  mtperf_report_intervals(intervals, p, report_flags);
  output_string("### end benchmark results\n", 2);
}

