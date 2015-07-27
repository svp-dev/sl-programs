//
// uart.c: this file is part of the SL program suite.
//
// Copyright (C) 2011-2015 The SL project.
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
#include <svp/delegate.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "mtconf.h"

// XIGNORE: *:R

sl_def(uart_driver,,sl_glparm(char*,ctl), sl_glparm(long*,chans), sl_glparm(void*,cb))
{
    volatile unsigned char* ctl = sl_getp(ctl);
    volatile long* chans = sl_getp(chans);

    void (*cb)(char*, size_t) = sl_getp(cb);

    // buffer for incoming bytes
    char *buf;
    size_t bufsz, curp;
    if (!(buf = malloc(bufsz = 10))) { output_string("uart: malloc\n", 2); abort(); }
    curp = 0;

    output_string("draining interrupt channel 0...\n", 2);
    chans[0] = 0; // disable channel
    (void)chans[0]; // drain
    (void)chans[0];
    (void)chans[0];
    chans[0] = 1; // enable channel again

    output_string("enabling UART with input interrupt 0...\n", 2);
    ctl[1] = 1; // enable received data interrupt
    ctl[8] = 0; // interrupt channel 0
    ctl[10] = 1; // enable UART

    while(1)
    {
        (void)chans[0];
        int data_ready = ctl[5] & 1;
        int maybe_data = ctl[0];

        if (data_ready)
        {
            if (curp >= bufsz)
                if (!(buf = realloc(buf, bufsz += 10))) { output_string("uart: malloc\n", 2); abort(); }

            buf[curp++] = maybe_data;

            if (maybe_data == '\n')
            {
                // special character
                // first flush existing data, if any
                (*cb)(buf, curp);

                // new buffer
                if (!(buf = malloc(bufsz = 10))) { output_string("uart: malloc\n", 2); abort(); }
                curp = 0;
            }
        }

    }
}
sl_enddef

void input(char* buf, size_t len)
{
    output_string("world: ", 2);
    for (int i = 0; i < len; ++i)
        output_char(buf[i], 2);
    output_char('\n', 2);
    free(buf);
}


int main(void)
{
    if (mg_uart_devid == -1)
    {
        output_string("no UART available\n", 2);
        return 1;
    }
    sl_create(,1,,,,,,uart_driver,
              sl_glarg(char*,, mg_devinfo.base_addrs[mg_uart_devid]),
              sl_glarg(long*,, mg_devinfo.channels),
              sl_glarg(void*,, input));
    sl_sync();

    return 0;
}
