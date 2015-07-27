#! /bin/sh

gen_mk() {
    cat >>flags.mk <<EOF
SLFLAGS_$2 = -L\$(top_builddir)/benchmarks/lib/$2
EOF
    cat >>targets.mk <<EOF
if ENABLE_SLC_$1

CLEANFILES += $2/benchmark.o $2/libbench.a
noinst_DATA += $2/libbench.a

$2_libbench_a_CONTENTS = $2/benchmark.o
$2/%.o: \$(srcdir)/%.c
	\$(AM_V_at)\$(MKDIR_P) $2
	\$(slc_verbose)\$(SLC_RUN) -b $2 -c -o \$@ \$<
$2/libbench.a: \$($2_libbench_a_CONTENTS)
	\$(AM_V_at)rm -f \$@
	\$(AM_V_AR)\$(AR_$1) cru \$@ \$^
	\$(AM_V_at)\$(RANLIB_$1) \$@

endif
EOF
}

rm -f flags.mk targets.mk
for m in mta mta_s mta_n mta_on; do gen_mk MTALPHA $m; done
for m in mtsn mtsn_s mtsn_n; do gen_mk MTSPARC $m; done
for m in mipsel_s; do gen_mk MIPSEL $m; done
for m in ptl_n ptl_s ptld_n ptld_s; do gen_mk PTL $m; done
for m in hls hls_n hls_s hlsd hlsd_n hlsd_s; do gen_mk HLSIM $m; done
