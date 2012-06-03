//
// fft.h: this file is part of the SL program suite.
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

#ifndef SL_BENCHMARKS_FFT_FFT_H
# define SL_BENCHMARKS_FFT_FFT_H

#include "benchmark.h"

#define FT double

typedef struct { FT re; FT im; } cpx_t;

/* low-level FFT (for benchmarks) */

static
void FFT_1(unsigned long M, cpx_t*restrict, unsigned long, const void*);
static
void FFT(cpx_t*restrict X, unsigned long M, struct work_lapses* wl, const char *wname);
static
void FFT_Inv(cpx_t*restrict X, unsigned long M, struct work_lapses* wl);

#define STRINGIFY_(N) # N
#define STRINGIFY(N) STRINGIFY_(N)
#define MAKENAME_(N, SZ) fft_table ## N ## _ ## SZ ## _data.h
#define MAKENAME(N, SZ) MAKENAME_(N, SZ)


#endif // ! SL_BENCHMARKS_FFT_FFT_H
