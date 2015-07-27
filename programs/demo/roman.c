//
// roman.c: this file is part of the SL program suite.
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

#include <svp/testoutput.h>
#include <stdlib.h>

static struct roman_table_t {
  long base;
  const char* repr;
} roman_table[] = {
  {   50000, "(L)" },
  {   10000, "(X)" },
  {    5000, "(V)" },
  {    1000,   "M" },
  {     500,   "D" },
  {     100,   "C" },
  {      50,   "L" },
  {      10,   "X" },
  {       5,   "V" },
  {       1,   "I" },
  {       0,     0 }
};

sl_def(roman, sl__static, sl_glparm(short, x))
{
  long num = sl_getp(x);
  if (num < 0) {
    output_char('-', 1);
    num = -num;
  }

  struct roman_table_t *p = roman_table;
  const char *s;

  for (p = roman_table; p->base; ++p)
    while(num >= p->base) {
      for (s = p->repr; *s; ++s) output_char(*s, 1);
      num = num - p->base;
    };
}
sl_enddef

// SLT_RUN: 42

int main(int argc, char **argv)
{
  if (argc < 2)
      output_string("no number specified!\n", 2);
  else {
      unsigned N = atoi(argv[1]);
      sl_proccall(roman, sl_glarg(short, x, N));
      output_char('\n', 1);
  }
}
