//
// fft_extra.c: this file is part of the SL program suite.
//
// Copyright (C) 2009 The SL project.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// The complete GNU General Public Licence Notice can be found as the
// `COPYING' file in the root directory.
//

sl_def(FFT_Swap, sl__static,
       sl_glparm(cpx_t*restrict, X),
       sl_glparm(unsigned long, i),
       sl_glparm(unsigned long, j))
{
  if (sl_getp(i) < sl_getp(j)) {
    cpx_t *restrict Xi = sl_getp(X) + sl_getp(i);
    cpx_t *restrict Xj = sl_getp(X) + sl_getp(j);
    cpx_t T1 = *Xi, T2 = *Xj;
    *Xi = T2; *Xj = T1;
  }
}
sl_enddef

sl_def(FFT_Reverse, sl__static,
       sl_glparm(cpx_t*restrict, X),
       sl_glparm(unsigned long, N),
       sl_shparm(unsigned long, j))
{
  sl_index(i);
  unsigned long t = sl_getp(j);
  unsigned long k;
  sl_create(,,,,,,, FFT_Swap,
	    sl_glarg(cpx_t*restrict, gX, sl_getp(X)),
	    sl_glarg(unsigned long, gi, i),
	    sl_glarg(unsigned long, gt, t));

  k = sl_getp(N) ;
  while (k <= t)
    {
      t = t - k;
      k = k / 2;
    }
  sl_sync();

  sl_setp(j, t + k);
}
sl_enddef

void FFT(cpx_t*restrict X, unsigned long M, struct work_lapses* wl, const char *wname)
{
  unsigned long N = 1 << M;

  start_interval(wl, "reversal");
  sl_create(,,,N-1,,2,, FFT_Reverse,
	    sl_glarg(cpx_t*restrict, , X),
	    sl_glarg(unsigned long, , N/2),
	    sl_sharg(unsigned long, , 0));
  sl_sync();
  finish_interval(wl);

  start_interval(wl, wname);
  FFT_1(M, X, N/2, sc_table);
  finish_interval(wl);
}

sl_def(Conjugate, sl__static, sl_glparm(cpx_t*restrict, X))
{
  sl_index(i);
  cpx_t *X = sl_getp(X);

  X[i].im = -X[i].im;
}
sl_enddef

sl_def(Scale, sl__static,
       sl_glparm(cpx_t*restrict, X),
       sl_glparm(unsigned long, N))
{
  sl_index(i);
  cpx_t *X = sl_getp(X);

  X[i].re = X[i].re / sl_getp(N);
  X[i].im = -X[i].im / sl_getp(N);
}
sl_enddef

void FFT_Inv(cpx_t*restrict X, unsigned long M, struct work_lapses* wl)
{
  unsigned long N = 1 << M;

  start_interval(wl, "conj");
  sl_create(,,,N,,,, Conjugate,
	    sl_glarg(cpx_t*restrict, , X));
  sl_sync();
  finish_interval(wl);


  FFT(X, M, wl, "work2");

  start_interval(wl, "scale");
  sl_create(,,,N,,,, Scale,
	    sl_glarg(cpx_t*restrict, , X),
	    sl_glarg(unsigned long, , N));
  sl_sync();
  finish_interval(wl);

}

