//
// pi.c: this file is part of the SL program suite.
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// 2015-07-25: MIPSel does not support FP yet.
// XIGNORE: mipsel*:R

#if 0
extern char* __dtoa  (double d, int mode, int ndigits, int *decpt,
			int *sign, char **rve);
#endif

int main(int argc, char **argv)
{
    double cst = M_PI;
    if (argc > 1)
        cst = strtod(argv[1], 0);

    printf("%g\n", cst);
    printf("%.0g\n", cst);
    printf("%.1g\n", cst);
    printf("%.2g\n", cst);

#if 0
#ifdef __mt_freestanding__
    int expt, signflag;
    char *dtoaend;
    char *s;
    s = __dtoa(cst, 0, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 0: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 1, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 1: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 2, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 2: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 3, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 3: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 4, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 4: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 5, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 5: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 6, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 6: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 7, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 7: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 8, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 8: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
    s = __dtoa(cst, 9, 6 /*prec*/, &expt, &signflag, &dtoaend);
    fprintf(stderr, "dtoa 9: '%s', exp %d, sign %d len %d\n", s, expt, signflag, dtoaend - s);
#endif
#endif

    return 0;
}
