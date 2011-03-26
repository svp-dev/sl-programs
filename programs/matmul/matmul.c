#include <stddef.h>
#include <svp/delegate.h>

#ifndef TYPE
#define number long
#else
#define number TYPE
#endif

#ifdef FLOAT_TYPE
#define sl_shparm_type sl_shfparm
#define sl_sharg_type sl_shfarg
#else
#define sl_shparm_type sl_shparm
#define sl_sharg_type sl_sharg
#endif



sl_def(matmul_inner, void,
       sl_shparm_type(number, sum),
       sl_glparm(void*, a),
       sl_glparm(void*, b),
       sl_glparm(size_t, i),
       sl_glparm(size_t, j)
#ifndef N
       , sl_glparm(size_t, N)
#endif
    )
{
    sl_index(k);

#ifndef N
    size_t N = sl_getp(N);
#endif
    const number (* restrict a)[N][N] = (const number (*)[N][N])(void*) sl_getp(a);
    const number (* restrict b)[N][N] = (const number (*)[N][N])(void*) sl_getp(b);

    number v = (*a)[sl_getp(i)][k] * (*b)[k][sl_getp(j)];
    sl_setp(sum, v + sl_getp(sum));
}
sl_enddef

sl_def(matmul_middle, void,
       sl_glparm(void*, a),
       sl_glparm(void*, b),
       sl_glparm(void*, c),
       sl_glparm(size_t, i)
#ifndef N
       , sl_glparm(size_t, N)
#endif
    )
{
    sl_index(j);
#ifndef N
    size_t N = sl_getp(N);
#endif

    sl_create(,PLACE_LOCAL, 0, N, 1, ,, matmul_inner,
              sl_sharg_type(number, sum, 0),
              sl_glarg(void*, , sl_getp(a)),
              sl_glarg(void*, , sl_getp(b)),
              sl_glarg(size_t, , sl_getp(i)),
              sl_glarg(size_t, , j)
#ifndef N
              , sl_glarg(size_t, , N)
#endif
        );
    sl_sync();
              
    number (* restrict c)[N][N] = (number (*)[N][N])(void*) sl_getp(c);
    (*c)[sl_getp(i)][j] = sl_geta(sum);
}
sl_enddef       

sl_def(matmul_outer, void,
       sl_glparm(void*, a),
       sl_glparm(void*, b),
       sl_glparm(void*, c)
#ifndef N
       , sl_glparm(size_t, N)
#endif
    )
{
    sl_index(i);

#ifndef N
    size_t N = sl_getp(N);
#endif
    
    sl_create(, PLACE_LOCAL, 0, N, 1, ,, matmul_middle,
              sl_glarg(void*, , sl_getp(a)),
              sl_glarg(void*, , sl_getp(b)),
              sl_glarg(void*, , sl_getp(c)),
              sl_glarg(size_t, , i)
#ifndef N
              , sl_glarg(size_t, , N)
#endif
        );
    sl_sync();
}
sl_enddef

sl_def(matmul, void, 
       sl_glparm(void*, a),
       sl_glparm(void*, b),
       sl_glparm(void*, c)
#ifndef N
       , sl_glparm(size_t, N)
#endif
    )
{
#ifndef N
    size_t N = sl_getp(N);
#endif

    sl_create(,, 0, N, 1, ,, matmul_outer,
              sl_glarg(void*, , sl_getp(a)),
              sl_glarg(void*, , sl_getp(b)),
              sl_glarg(void*, , sl_getp(c))
#ifndef N
              , sl_glarg(size_t, , N)
#endif
        );
    sl_sync();
}
sl_enddef


