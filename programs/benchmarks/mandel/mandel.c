//
// mandel.c: this file is part of the SL program suite.
//
// Copyright (C) 2009,2010,2011,2012 The SL project.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// The complete GNU General Public Licence Notice can be found as the
// `COPYING' file in the root directory.
//

#include <svp/compiler.h>
#include <svp/testoutput.h>
#include <svp/fibre.h>
#include <svp/gfx.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <svp/sep.h>

#include "benchmark.h"

#if defined(__mt_freestanding__) && !defined(BASE_DIST)
#define OTHER_DIST
#endif

struct point
{ short x, y; uint32_t data; };

struct bdata {
  /* input parameters */
  uint16_t xN, yN;
  uint16_t icount;
  uint16_t blocksize;
  double xmin, xmax, ymin, ymax;
  /* computed at initialization */
  double xstep, ystep;
  size_t N;
  sl_place_t par_place;
  sl_place_t excl_place;
#ifdef MANY_COLORS
  uint32_t * restrict colors;
#endif
#ifndef SKIP_MEM
  struct point * restrict pixeldata;
#endif
};

sl_def(prepare_colors, void,
       sl_glparm(uint32_t*restrict, colors),
       sl_glfparm(double, licount))
{
  sl_index(i);

#define RGB(G) ((G << 16) | (G << 8) | 128+(G/2))
  uint32_t v = 255*log(1+i)/sl_getp(licount);
  sl_getp(colors)[i] = RGB(v);
}
sl_enddef

void initialize(struct benchmark_state* st)
{
  struct bdata *bdata = (struct bdata*) malloc(sizeof(struct bdata));

  /* fibre input:
     4 x ulong (xres, yres, icount, blocksize)
     1 x double array (box)
  */
  assert(fibre_tag(0) == 0);
  assert(fibre_rank(0) == 0);
  bdata->xN = *(unsigned long*)fibre_data(0);

  assert(fibre_tag(1) == 0);
  assert(fibre_rank(1) == 0);
  bdata->yN = *(unsigned long*)fibre_data(1);

  assert(fibre_tag(2) == 0);
  assert(fibre_rank(2) == 0);
  bdata->icount = *(unsigned long*)fibre_data(2);
  assert(bdata->icount > 0);

  assert(fibre_tag(3) == 0);
  assert(fibre_rank(3) == 0);
  bdata->blocksize = *(unsigned long*)fibre_data(3);

  assert(fibre_tag(4) == 2);
  assert(fibre_rank(4) == 1);
  assert(fibre_shape(4)[0] == 4);
  double *box = (double*)fibre_data(4);
  bdata->xmin = box[0];
  bdata->xmax = box[1];
  bdata->ymin = box[2];
  bdata->ymax = box[3];

  bdata->xstep = (bdata->xmax - bdata->xmin) / bdata->xN;
  bdata->ystep = (bdata->ymax - bdata->ymin) / bdata->yN;

  bdata->N = bdata->xN * bdata->yN;
  assert(bdata->N > 0);

#ifndef SKIP_MEM
  bdata->pixeldata = (struct point*)malloc(sizeof(struct point) * bdata->N);
#endif

#ifdef MANY_COLORS
  /* initialize colors */
  bdata->colors = (uint32_t*)malloc(sizeof(uint32_t) * (bdata->icount + 1));
  double licount = log(bdata->icount+1);
  sl_create(,,,bdata->icount+2,,4,, prepare_colors,
	    sl_glarg(uint32_t*, , bdata->colors),
	    sl_glfarg(double, , licount));
  sl_sync();
#endif

#if defined(DISPLAY_DURING_COMPUTE) && !defined(PARALLEL_DISPLAY)
  int sep_alloc_result = sep_alloc(root_sep, SAL_DONTCARE, &bdata->excl_place, 0);
  assert(sep_alloc_result != -1);
  assert(sl_geta(pex) != 0);
#else
  bdata->excl_place = PLACE_DEFAULT;
#endif

  /* initialize graphics output */
  gfx_init();
  gfx_resize(bdata->xN, bdata->yN);

  st->data = bdata;
}

alwaysinline
void do_display(uint16_t dx, uint16_t dy, uint32_t v)
{
#ifdef TEXT
  output_uint(dx, 1);
  output_char(' ', 1);
  output_uint(dy, 1);
  output_char(' ', 1);
  output_hex(v, 1);
  output_char('\n', 1);
#endif
  gfx_putpixel(dx, dy, v);
}


sl_def(displayPoint, void,
       sl_glparm(uint16_t, dx),
       sl_glparm(uint16_t, dy),
       sl_glparm(uint32_t, v))
{
  do_display(sl_getp(dx), sl_getp(dy), sl_getp(v));
}
sl_enddef

sl_def(displayAfter, void,
       sl_glparm(struct point*restrict, v)
#ifndef PARALLEL_DISPLAY
       , sl_shparm(int, token)
#endif
)
{
  sl_index(i);
  uint16_t x, y; uint32_t v;
  x = sl_getp(v)[i].x;
  y = sl_getp(v)[i].y;
  v = sl_getp(v)[i].data;
#ifndef PARALLEL_DISPLAY
  int tok = sl_getp(token);
#endif
  do_display(x, y, v);
#ifndef PARALLEL_DISPLAY
  sl_setp(token, tok);
#endif
}
sl_enddef

#ifdef INNER_THREAD
sl_def(mandel_loop, void,
       sl_shparm(size_t, v),
       sl_shfparm(double, zx),
       sl_shfparm(double, zy),
       sl_glfparm(double, cx),
       sl_glfparm(double, cy),
       sl_glfparm(double, four))
{
    double q1 = sl_getp(zx) * sl_getp(zx);
    double q2 = sl_getp(zy) * sl_getp(zy);
    if (unlikely((q1 + q2) >= sl_getp(four)))
    {
        sl_setp(zx, sl_getp(zx));
        sl_setp(zy, sl_getp(zy));
        sl_setp(v, sl_getp(v));
        sl_break;
    }
    else 
    {
        double t = q1 - q2 + sl_getp(cx);
        double q3 = sl_getp(zx) * sl_getp(zy);
        sl_setp(zy, q3 + q3 + sl_getp(cy));
        sl_setp(zx, t);
        sl_setp(v, 1+sl_getp(v));
    }
}
sl_enddef
#endif


sl_def(mandel, void,
       sl_glfparm(double, four),
       sl_glfparm(double, xstart),
       sl_glfparm(double, ystart),
       sl_glfparm(double, xstep),
       sl_glfparm(double, ystep),
       sl_glparm(uint16_t, xres),
       sl_glparm(size_t, icount)
#ifdef MANY_COLORS
       , sl_glparm(uint32_t*restrict, colors)
#endif
#ifndef SKIP_MEM
       , sl_glparm(struct point*restrict, mem)
#endif
#if defined(DISPLAY_DURING_COMPUTE) && !defined(PARALLEL_DISPLAY)
       , sl_glparm(sl_place_t, excl_place)
#endif
)
{
  sl_index(i);
#ifdef ADVERTISE
  //output_uint(i, 0);
  counter_t c1 = mtperf_sample1(MTPERF_EXECUTED_INSNS);
#endif

  uint16_t  xb = i % sl_getp(xres);
  double cx = sl_getp(xstart) + xb * sl_getp(xstep);
  uint16_t  yb = i / sl_getp(xres);
  double cy = sl_getp(ystart) + yb * sl_getp(ystep);

  uint16_t dx = xb;
  uint16_t dy = yb;
#ifdef TRACE_COMPUTE
  gfx_putpixel(dx, dy, 0xff0000);
#endif

#ifndef INNER_THREAD
  double zx = cx, zy = cy;
  size_t v;
  // size_t ic = sl_getp(icount);
  for (v = 0; likely(v < sl_getp(icount)); ++v)
    {
        double q1 = zx * zx;
        double q2 = zy * zy;
        if (unlikely((q1 + q2) >= sl_getp(four)))
            break;
        double t = q1 - q2 + cx;
        double q3 = zx * zy;
        zy = q3 + q3 + cy;
        zx = t;
    }
#else
  sl_create(,,, sl_getp(icount),,,,
            mandel_loop,
            sl_sharg(size_t, va, 0),
            sl_shfarg(double, , cx),
            sl_shfarg(double, , cy),
            sl_glfarg(double, , cx),
            sl_glfarg(double, , cy),
            sl_glfarg(double, , sl_getp(four)));
  sl_sync();
  size_t v = sl_geta(va);
#endif
  __asm__ __volatile__("" : : "r"(v));

#ifdef MANY_COLORS
  v = sl_getp(colors)[v];
#endif
#ifndef SKIP_MEM
  sl_getp(mem)[i].x = dx;
  sl_getp(mem)[i].y = dy;
  sl_getp(mem)[i].data = v;
#endif
#ifdef DISPLAY_DURING_COMPUTE
#ifndef PARALLEL_DISPLAY
  sl_create(,sl_getp(excl_place),,,,,sl__exclusive,
	    displayPoint,
	    sl_glarg(uint16_t, , dx),
	    sl_glarg(uint16_t, , dy),
	    sl_glarg(uint32_t, , v));
  sl_detach();
#else
  do_display(dx, dy, v);
#endif
#endif

#ifdef ADVERTISE
  output_uint(mtperf_sample1(MTPERF_EXECUTED_INSNS)-c1, 0);
#endif
}
sl_enddef

#ifdef OTHER_DIST
sl_def(mandelouter, void,
       sl_glfparm(double, xstart),
       sl_glfparm(double, ystart),
       sl_glfparm(double, xstep),
       sl_glfparm(double, ystep),
       sl_glparm(size_t, npoints),
       sl_glparm(size_t, blocksize),
       sl_glparm(uint16_t, xres),
       sl_glparm(size_t, icount)
#ifdef MANY_COLORS
       , sl_glparm(uint32_t*restrict, colors)
#endif
#ifndef SKIP_MEM
       , sl_glparm(struct point*restrict, mem)
#endif
#if defined(DISPLAY_DURING_COMPUTE) && !defined(PARALLEL_DISPLAY)
       , sl_glparm(sl_place_t, excl_place)
#endif
)
{
    sl_index(i);
    sl_create(,PLACE_LOCAL, i, sl_getp(npoints)+i, sl_placement_size(sl_default_placement()), sl_getp(blocksize), ,
              mandel,
              sl_glfarg(double, , 4.0),
              sl_glfarg(double, , sl_getp(xstart)),
              sl_glfarg(double, , sl_getp(ystart)),
              sl_glfarg(double, , sl_getp(xstep)),
              sl_glfarg(double, , sl_getp(ystep)),
              sl_glarg(uint16_t, , sl_getp(xres)),
              sl_glarg(size_t, , sl_getp(icount))
#ifdef MANY_COLORS
              , sl_glarg(uint32_t*restrict, , sl_getp(colors))
#endif
#ifndef SKIP_MEM
              , sl_glarg(struct point*restrict, , sl_getp(mem))
#endif
#ifdef DISPLAY_DURING_COMPUTE
#ifndef PARALLEL_DISPLAY
              , sl_glarg(sl_place_t, , sl_getp(excl_place))
#endif
#endif
	    );
  sl_sync();


}
sl_enddef

#endif

void work(struct benchmark_state* st)
{
  struct work_lapses * wl = st->wl;
  struct bdata *bdata = (struct bdata*)st->data;

  start_interval(wl, "compute");

#ifndef OTHER_DIST
  sl_create(,,,bdata->N,,bdata->blocksize,,
	    mandel,
            sl_glfarg(double, , 4.0),
	    sl_glfarg(double, , bdata->xmin),
	    sl_glfarg(double, , bdata->ymin),
	    sl_glfarg(double, , bdata->xstep),
	    sl_glfarg(double, , bdata->ystep),
	    sl_glarg(uint16_t, , bdata->xN),
	    sl_glarg(size_t, , bdata->icount)
#ifdef MANY_COLORS
	    , sl_glarg(uint32_t*restrict, , bdata->colors)
#endif
#ifndef SKIP_MEM
	    , sl_glarg(struct point*restrict, , bdata->pixeldata)
#endif
#ifdef DISPLAY_DURING_COMPUTE
#ifndef PARALLEL_DISPLAY
            , sl_glarg(sl_place_t, , bdata->excl_place)
#endif
#endif
	    );
  sl_sync();
#else

  sl_create(,,,sl_placement_size(sl_default_placement()),,1,,
	    mandelouter,
	    sl_glfarg(double, , bdata->xmin),
	    sl_glfarg(double, , bdata->ymin),
	    sl_glfarg(double, , bdata->xstep),
	    sl_glfarg(double, , bdata->ystep),
            sl_glarg(size_t, , bdata->N),
            sl_glarg(size_t, , bdata->blocksize),
	    sl_glarg(uint16_t, , bdata->xN),
	    sl_glarg(size_t, , bdata->icount)
#ifdef MANY_COLORS
	    , sl_glarg(uint32_t*restrict, , bdata->colors)
#endif
#ifndef SKIP_MEM
	    , sl_glarg(struct point*restrict, , bdata->pixeldata)
#endif
#ifdef DISPLAY_DURING_COMPUTE
#ifndef PARALLEL_DISPLAY
            , sl_glarg(sl_place_t, , bdata->excl_place)
#endif
#endif
	    );
  sl_sync();


#endif


  finish_interval(wl);

#if !defined(DISPLAY_DURING_COMPUTE) && !defined(SKIP_MEM)
  start_interval(wl, "display");
#ifdef PARALLEL_DISPLAY
  sl_create(,,,bdata->N,,bdata->blocksize,,
	    displayAfter,
	    sl_glarg(struct point*restrict, , bdata->pixeldata));
  sl_sync();
#else
  sl_create(,,,bdata->N,,bdata->blocksize,,
	    displayAfter,
	    sl_glarg(struct point*restrict, , bdata->pixeldata),
	    sl_sharg(int, tok, 0));
  sl_sync();
#endif
  finish_interval(wl);

#endif
} 


void output(struct benchmark_state* st)
{
  /* dump screenshot */
  gfx_dump(0, 1, 0, 0);
}

void teardown(struct benchmark_state* st)
{
  gfx_close();

  struct bdata *bdata = (struct bdata*)st->data;
#ifndef SKIP_MEM
  free(bdata->pixeldata);
#endif
#ifdef MANY_COLORS
  free(bdata->colors);
#endif
#if defined(DISPLAY_DURING_COMPUTE) && !defined(PARALLEL_DISPLAY)
  sep_free(root_sep, &bdata->excl_place);
#endif
  free(bdata);
}

sl_def(t_main,,)
{
  struct benchmark b = {
    "Mandelbrot set approximation",
    "kena",
    "Iterate z_{n-1} = z_n^2 + c over the complex plane",
    &initialize, 0, &work, &output, &teardown
  };
  run_benchmark(&b);
}
sl_enddef
