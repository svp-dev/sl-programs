//
// fft-bench.c: this file is part of the SL program suite.
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

#include <svp/testoutput.h>
#include <svp/fibre.h>
#include <svp/perf.h>
#include <assert.h>
#include <stdlib.h>

#include "benchmark.h"

#define TABLE_SIZE 8
#include "fft.h"

struct bdata {
  const cpx_t * restrict x_orig;
  cpx_t * restrict y_fft;
  cpx_t * restrict z_inv;
  size_t M;
  size_t N;
};

void initialize(struct benchmark_state* st)
{
  int i;
  struct bdata *bdata = (struct bdata*) malloc(sizeof(struct bdata));
  assert(bdata != 0);

  assert(2*sizeof(double) == sizeof(cpx_t));

  assert(fibre_tag(0) == 0);
  assert(fibre_rank(0) == 0);
  bdata->M = *(unsigned long*)fibre_data(0);
  assert(bdata->M <= TABLE_SIZE);
  bdata->N = 1 << bdata->M;

  assert(fibre_tag(1) == 2);
  assert(fibre_rank(1) == 2);
  assert(fibre_shape(1)[0] >= bdata->N);
  assert(fibre_shape(1)[1] == 2);
  bdata->x_orig = (const cpx_t*)fibre_data(1);

  bdata->y_fft = (cpx_t*)malloc(sizeof(cpx_t) * bdata->N);
  assert(bdata->y_fft != 0);

  bdata->z_inv = (cpx_t*)malloc(sizeof(cpx_t) * bdata->N);
  assert(bdata->z_inv != 0);

  st->data = bdata;
}

void prepare(struct benchmark_state* st)
{
  struct bdata *bdata = (struct bdata*)st->data;
  int i;
  for (i = 0; i < bdata->N; ++i)
    bdata->y_fft[i] = bdata->x_orig[i];
}

void work(struct benchmark_state* st)
{
  struct bdata *bdata = (struct bdata*)st->data;

#ifdef FFT_BENCH_SMALL
  extern const void *sc_table_ptr;

  FFT_1(bdata->M, bdata->y_fft, bdata->N/2, sc_table_ptr);

#else

  struct work_lapses *wl = st->wl;
  int i;

  FFT(bdata->y_fft, bdata->M, wl, "work1");

  start_interval(wl, "copy");
  for (i = 0; i < bdata->N; ++i)
    bdata->z_inv[i] = bdata->y_fft[i];
  finish_interval(wl);

  FFT_Inv(bdata->z_inv, bdata->M, wl);

#endif
}


sl_def(print_fl, void,
       sl_shparm(long, guard),
       sl_glparm(cpx_t*restrict, array))
{
  sl_index(i);
  cpx_t A = sl_getp(array)[i];

  long g = sl_getp(guard);
  output_int(i, 1);
  output_char('\t', 1);
  output_float(A.re, 1, 6);
  output_char('\t', 1);
  output_float(A.im, 1, 6);
  output_char('\n', 1);
  sl_setp(guard, g);
}
sl_enddef

void output(struct benchmark_state* st)
{
  struct bdata *bdata = (struct bdata*)st->data;
  output_string("# Forward FFT:\n", 1);
  sl_create(,,, bdata->N,,,, print_fl,
	    sl_sharg(long, , 0), sl_glarg(cpx_t*restrict, , bdata->y_fft));
  sl_sync();
  output_string("# Reverse FFT:\n", 1);
  sl_create(,,, bdata->N,,,, print_fl,
	    sl_sharg(long, , 0), sl_glarg(cpx_t*restrict, , bdata->z_inv));
  sl_sync();
}

void teardown(struct benchmark_state* st)
{
  struct bdata *bdata = (struct bdata*)st->data;
  free(bdata->y_fft);
  free(bdata->z_inv);
  free(bdata);
}

int main(void)
{
  struct benchmark b = {
    "FFT 1D",
    "kena",
    "Perform FFT and inverse FFT over a 1D vector " EXTRA_COMMENT,
    &initialize, &prepare, &work, &output, &teardown
  };
  run_benchmark(&b);
  return 0;
}
