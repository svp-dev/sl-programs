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
