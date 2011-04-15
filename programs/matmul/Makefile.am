## Makefile.am: this file is part of the SL toolchain.
## 
## Copyright (C) 2011 The SL project
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
##
## The complete GNU General Public Licence Notice can be found as the
## `COPYING' file in the root directory.
##

DEMOS = \
	simple_l_10.c \
	simple_f_10.c \
	simple_d_10.c \
	simple_l_var.c \
	simple_f_var.c \
	simple_d_var.c

TESTS = $(DEMOS)

include $(top_srcdir)/build-aux/slctests.mk

EXTRA_DIST = $(DEMOS)
EXTRA_DIST += \
	simple.c \
	matmul.c \
	simple_l_30.c \
	simple_f_30.c \
	simple_d_30.c \
	input.d10 \
	input.d30

if ENABLE_DEMOS
noinst_DATA = $(DEMOS:.c=.x)
CLEANFILES = $(DEMOS:.c=.x) $(DEMOS:.c=.bin.*)
endif

.c.tar: $(EXTRA_DIST) $(DEMOS)
	$(AM_V_at)rm -rf $@ $@.tmp
	$(AM_V_GEN)b="$<" && \
	  bn=$$(basename $$b .c) && \
	  rm -rf "$$bn" && \
	  $(MKDIR_P) "$$bn" && \
	  for f in $(EXTRA_DIST) $(DEMOS); do \
	    case $$f in *.d*[0-9]|*.inputs|*.bmk|*fdatas.mk) continue ;; esac && \
	    dn=$$(dirname $$(echo $$f|$(SED) -e 's|^'$(abs_top_srcdir).*/'||g')) && \
	    $(MKDIR_P) $$bn/$$dn && \
	    cp `test -r $$f || echo $(srcdir)/`$$f $$bn/$$dn/; \
	  done && \
	  echo "a.out: $$bn.c; "'$$'"(COMPILER) "'$$'"(FLAGS) -I. -o "'$$'"@ "'$$'"^ -lm" \
	     >$$bn/Makefile && \
	  tardir=$$bn && $(am__tar) >$@.tmp && \
	  rm -rf $$bn
	$(AM_V_at)chmod -w $@.tmp
	$(AM_V_at)mv -f $@.tmp $@
