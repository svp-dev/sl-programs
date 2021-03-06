//
// blasbench.h: this file is part of the SL program suite.
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

#ifndef SL_BENCHMARKS_BLAS_BLASBENCH_H
# define SL_BENCHMARKS_BLAS_BLASBENCH_H

#include <svp/testoutput.h>
#include <stdlib.h>
#include <assert.h>
#include <svp/fibre.h>
#include "benchmark.h"

#define BEGIN_VARS struct bdata {
#define DEF_COUNTER(Var)  long Var;
#define DEF_SCALAR(Var)   FLOAT Var;
#define DEF_ARRAY_IN(Var) \
  const FLOAT * restrict Var;
#define DEF_ARRAY_INOUT(Var) \
  const FLOAT * restrict Var ## _orig; \
  FLOAT * restrict Var;
#define END_VARS };

#define BEGIN_READ \
  void initialize(struct benchmark_state* st)     \
  { \
    int i, f = 0;							\
    struct bdata *bdata = (struct bdata*) malloc(sizeof(struct bdata)); \
    assert(bdata != NULL);

#define READ_COUNTER(Var)				\
  assert(fibre_tag(f) == 0 || fibre_tag(f) == 1);	\
  assert(fibre_rank(f) == 0);			\
  bdata->Var = *(long*)fibre_data(f); ++f;

#define READ_SCALAR(Var)			\
  assert(fibre_tag(f) == 2);		\
  assert(fibre_rank(f) == 0);		\
  bdata->Var = *(FLOAT*)fibre_data(f); ++f;

#define READ_ARRAY_IN(Var, N)				 \
  assert(fibre_tag(f) == 2);			 \
  assert(fibre_rank(f) == 1);			 \
  assert(fibre_shape(f)[0] >= bdata->N);		 \
  FLOAT *Var = (FLOAT*)malloc(bdata->N * sizeof(FLOAT)); \
  assert(Var != 0); \
  for (i = 0; i < bdata->N; ++i) Var[i] = ((double*)fibre_data(f))[i]; \
  ++f;								       \
  bdata->Var = Var;

#define READ_ARRAY_INOUT(Var, N)				       \
  assert(fibre_tag(f) == 2);			 \
  assert(fibre_rank(f) == 1);			 \
  assert(fibre_shape(f)[0] >= bdata->N);		 \
  FLOAT *Var = (FLOAT*)malloc(bdata->N * sizeof(FLOAT));	       \
  assert(Var != 0);						       \
  for (i = 0; i < bdata->N; ++i) Var[i] = ((double*)fibre_data(f))[i]; \
  ++f;								       \
  bdata->Var ## _orig = Var;					       \
  bdata->Var = (FLOAT*)malloc(bdata->N * sizeof(FLOAT));	       \
  assert(bdata->Var != 0);

#define END_READ            \
  st->data = (void*) bdata; \
	     }

#define BEGIN_PREPARE \
  void prepare(struct benchmark_state* st) { \
  struct bdata *bdata = (struct bdata*)st->data;        \
    long i;

#define RESET_ARRAY_INOUT(Var, N)		\
  for (i = 0; i < bdata->N; ++i)		\
    bdata->Var[i] = bdata->Var ## _orig[i];

#define END_PREPARE }

#define BEGIN_WORK \
    void work(struct benchmark_state* st) { \
    struct bdata *bdata = (struct bdata*)st->data;

#define USE_VAR(Var) bdata->Var

#define END_WORK }


#define BEGIN_OUTPUT \
    void output(struct benchmark_state* st) {   \
    struct bdata *bdata = (struct bdata*)st->data; \
    long i;

#define PRINT_ARRAY(Var, N)					\
  for (i = 0; i < bdata->N; ++i)	{			\
    output_float(bdata->Var[i], 1, 4);				\
    output_char('\n', 1);					\
  }

#define PRINT_SCALAR(Var) \
  output_float(bdata->Var, 1, 4); \
  output_char('\n', 1);

#define PRINT_COUNTER(Var) \
  output_int(bdata->Var, 1); \
  output_char('\n', 1);

#define END_OUTPUT }

#define BEGIN_TEARDOWN \
  void teardown(struct benchmark_state* st) { \
    struct bdata *bdata = (struct bdata*)st->data;

#define FREE_ARRAY_INOUT(Var)			\
  free((void*)bdata->Var ## _orig);		\
  free((void*)bdata->Var);

#define FREE_ARRAY_IN(Var)			\
  free((void*)bdata->Var);

#define END_TEARDOWN \
  free(bdata);	     \
  }

#define BEGIN_DESC \
    sl_def(t_main,,) {                          \
    struct benchmark b = {

#define BENCH_TITLE(Title) Title,
#define BENCH_AUTHOR(Author) Author,

#define _STRFY(Var) # Var
#define STRFY(Var) _STRFY(Var)
#define BENCH_DESC(Desc) Desc " (using type " STRFY(FLOAT) ")",

#define END_DESC							\
    &initialize, &prepare, &work, &output, &teardown };			\
    run_benchmark(&b);                                                  \
    } sl_enddef

#endif // ! SL_BENCHMARKS_BLAS_BLASBENCH_H
