//
// bench-xasum.c: this file is part of the SL program suite.
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

#include "blasbench.h"

BEGIN_VARS
DEF_COUNTER(n)
DEF_ARRAY_IN(sx)
DEF_COUNTER(incx)
DEF_SCALAR(sum)
END_VARS

BEGIN_READ
READ_COUNTER(n)
READ_ARRAY_IN(sx, n)
READ_COUNTER(incx)
END_READ

BEGIN_PREPARE
END_PREPARE

BEGIN_WORK
     sl_create(,,,,,,, FUNCTION,
	       sl_shfarg(FLOAT, ret, 0),
	       sl_glarg(long, , USE_VAR(n)),
	       sl_glarg(const FLOAT*, , USE_VAR(sx)),
	       sl_glarg(long, , USE_VAR(incx)));
     sl_sync();
USE_VAR(sum) = sl_geta(ret);
END_WORK

BEGIN_OUTPUT
PRINT_SCALAR(sum);
END_OUTPUT

BEGIN_TEARDOWN
FREE_ARRAY_IN(sx)
END_TEARDOWN

BEGIN_DESC
BENCH_TITLE("BLAS: _ASUM")
BENCH_AUTHOR("kena")
BENCH_DESC("Compute S := S + ABS(X[i])")
END_DESC
