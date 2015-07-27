###
### Build settings
###
include $(top_srcdir)/build-aux/sl.mk
include $(top_srcdir)/benchmarks/lib/flags.mk
SLFLAGS = -I$(top_srcdir)/benchmarks/lib -lbench
SLFLAGS_seqc = -L$(top_builddir)/benchmarks/lib/seqc -lm
CLEANFILES = $(BENCHMARKS:.c=.x) $(BENCHMARKS:.c=.*.bin)
DISTCLEANFILES =
BUILT_SOURCES =

data_verbose = $(data_verbose_$(V))
data_verbose_ = $(data_verbose_$(AM_DEFAULT_VERBOSITY))
data_verbose_0 = @echo '  DATA   $@';
bench_verbose = $(bench_verbose_$(V))
bench_verbose_ = $(bench_verbose_$(AM_DEFAULT_VERBOSITY))
bench_verbose_0 = @echo '  BENCH  $@';

SUFFIXES += .ilist .blist .tlist
ILIST_FILES = $(BENCHMARKS:.c=.ilist)
BLIST_FILES = $(BENCHMARKS:.c=.blist)
TLIST_FILES = $(BENCHMARKS:.c=.tlist)

%.ilist: %.c Makefile
	$(AM_V_at)rm -f $@ $@.tmp1 $@.tmp2 $@.tmp3 
	$(AM_V_GEN)set -e;							\
	        ifile=`test -r "$*".inputs || echo "$(srcdir)/"`$*.inputs; \
		if test -r "$$ifile"; then					\
		  { while read pat; do						\
		    for f in "$(srcdir)"/$$pat.d*[0-9] ./$$pat.d*[0-9]; do	\
		      if test -r "$$f"; then echo "$$f"; fi;			\
		    done;							\
		  done; } <"$$ifile";						\
		else								\
		  for f in "$(srcdir)/$*".d*[0-9] ./"$*".d*[0-9]; do		\
		     if test -r "$$f"; then echo "$$f"; fi;			\
		  done;								\
		fi >$@.tmp1 && \
		for i in `cat $@.tmp1`; do \
	           suff=; if grep -q 'USE IN MAKE CHECK' "$$i" >/dev/null 2>&1; then suff=.check; fi; \
	           echo "$$i$$suff"; \
	        done >$@.tmp2
	$(AM_V_at)sed -e 's|^$(srcdir)/||g' <$@.tmp2 | sort | uniq >$@.tmp3
	$(AM_V_at)mv -f $@.tmp3 $@
	$(AM_V_at)rm -f $@.tmp1 $@.tmp2

%.tlist: %.ilist
	$(AM_V_GEN)grep .check <$< | sed -e 's/.check//g' >$@.tmp
	$(AM_V_at)mv -f $@.tmp $@

%.blist: %.ilist
	$(AM_V_GEN)sed -e 's/.check//g' <$< >$@.tmp
	$(AM_V_at)mv -f $@.tmp $@

.PRECIOUS: $(ILIST_FILES) $(BLIST_FILES) $(TLIST_FILES)
DISTCLEANFILES += $(ILIST_FILES) $(BLIST_FILES) $(TLIST_FILES)

.SECONDEXPANSION:

##
## Fibre data files
##

SUFFIXES += .fdata

# Format of data paths: benchdata/ARCH/inputs/DATANAME.fdata
GENDATA_DEF = gen_fdata() { \
	  set -e; \
	  target=$$1; \
	  binfmt=`echo $$1|cut -d/ -f2`; \
	  dataname=`echo $$1|cut -d/ -f4`; \
	  dataname=`basename $$dataname .fdata`; \
	  pname=`echo $(word 1,$(BENCHMARKS)) | cut -d. -f1`; \
	  data=`test -r "$$dataname" || echo "$(srcdir)"`/"$$dataname"; \
	  rm -f "$$target"; $(MKDIR_P) `dirname "$$target"`; \
	  TIMEOUT=$${TIMEOUT:-7200} $(TMO) $(SLR) $$pname.$$binfmt.bin -f "$$data" -wf "$$target".tmp -wo && \
	  mv -f "$$target".tmp "$$target"; \
	}

.PRECIOUS: %.fdata
%.fdata: $$(notdir $$*) $$(word 1,$$(subst ., ,$$(word 1,$$(BENCHMARKS)))).x
	$(data_verbose)$(GENDATA_DEF); gen_fdata $@

##
## Benchmarks
##

# Format of output files: benchdata/ARCH/PROG/INPUT/PROFILE.out

SUFFIXES += .out

DOBENCH_DEF = do_bench() { \
	  set -e; \
	  target=$$1; \
	  binfmt=`echo "$$1"|cut -d/ -f2`; \
	  progbase=`echo "$$1"|cut -d/ -f3`; \
	  input=`echo "$$1"|cut -d/ -f4`; \
	  profile=`basename "$$1" .out`; \
	  prog=$$progbase.$$binfmt.bin; \
	  fdata=benchdata/$$binfmt/inputs/$$input.fdata; \
	  dores=`if test $$2 = 1; then echo 1; fi`; \
	  rm -f "$$target" "$$target".err; \
	  $(MKDIR_P) `dirname "$$target"`; \
	  set +e; TIMEOUT=$${TIMEOUT:-10800} $(TMO) $(SLR) "$$prog" -rf "$$fdata" \
	    L= sep_dump= results=$$dores format=1 -m "$$profile" \
	    -t -p "$$target".work >>"$$target".err 2>&1; \
	  ecode=$$?; set -e; if test $$ecode != 0; then \
	    if test -n "$$dores"; then \
	      { echo "Exit status: $$ecode"; echo; echo "Error log::"; echo; sed -e 's/^/  /g' <"$$target".err; } >&2; \
	    fi; \
	    exit $$ecode; \
	  fi; \
	  mv -f "$$target".err "$$target" && rm -rf "$$target".work; \
	}

%.out: \
	$$(word 3,$$(subst /, ,$$@)).x \
	benchdata/$$(word 2,$$(subst /, ,$$@))/inputs/$$(word 4,$$(subst /, ,$$@)).fdata
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(bench_verbose)$(DOBENCH_DEF); do_bench $@ 0

###
### Global clean rule
###
clean-local:
	-rm -rf benchdata

##
## Unit testing
##

check-local: $(BENCHMARKS:.c=.x) $(TLIST_FILES)
	$(MAKE) $(AM_MAKEFLAGS) $(BENCHMARKS:.c=.check)

%.check: %.x \
	$$(foreach I,$$(shell cat $$*.tlist), \
	  $$(foreach T,$$(wildcard $$*.*.bin),\
            benchdata/$$(word 2, $$(subst ., ,$$(T)))/inputs/$$(I).fdata))
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) \
	   $(foreach T,$(wildcard $*.*.bin), \
	     $$(for I in `cat $*.tlist`; do \
		   bn="benchdata/$(word 2,$(subst ., ,$(T)))/$*/$$I/default.out"; \
		   echo "$$bn"; \
                done))

noinst_DATA = $(BENCHMARKS:.c=.x) $(BLIST_FILES)

##
## Automated benchmarking
##

BENCH_TARGETS ?= mta mta_n
BENCH_PROFILES ?= coma128 rbm128

%.bench: %.x \
	$$(foreach I,$$(shell cat $$*.blist), \
	  $$(foreach T,$$(subst ., ,$$(suffix $$(wildcard $$*.bin.*))),\
            benchdata/$$(T)/inputs/$$(I).fdata))
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) \
	   $(foreach T,$(BENCH_TARGETS), \
	     $$(for I in `cat $*.blist`; do \
		   $(foreach P,$(BENCH_PROFILES), echo "benchdata/$(T)/$*/$$I/$(P).out";) \
                done))
