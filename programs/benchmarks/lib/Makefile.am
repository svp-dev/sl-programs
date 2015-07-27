## Makefile.am: this file is part of the SL toolchain.
##
## Copyright (C) 2009-2015 The SL project
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## The complete GNU General Public Licence Notice can be found as the
## `COPYING' file in the root directory.
##
#

include $(top_srcdir)/build-aux/sl.mk

EXTRA_DIST = \
	benchmark.c \
	benchmark.h

CLEANFILES =
noinst_DATA =
noinst_LIBRARIES =

##
## Targets for `seqc'
##
noinst_LIBRARIES += seqc/libbench.a

seqc_libbench_a_SOURCES =
seqc_libbench_a_LIBADD = \
	seqc/benchmark.o

seqc/%.o: $(srcdir)/%.c
	$(AM_V_at)$(MKDIR_P) seqc
	$(slc_verbose)$(SLC_RUN) -g -b seqc -c -o $@ $<

CLEANFILES += seqc/benchmark.o

include targets.mk
