/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  ---
 *
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  termlib.c: Various termlib stuff
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"

#if !defined(_WIN32)
#include <unistd.h>
#endif

s_char *SO = 0;
s_char *SE = 0;

int tgetent(char *, char *);

void
parsedelay(s_char *r)
{
    s_char *s, *t;

    s = r;
    while (isdigit(*s) || (*s == '*') || (*s == '.')) {
	s++;
    }
    for (t = r; *s != 0; (s++, t++)) {
	*t = *s;
    }
    *t = 0;
}

void
getsose(void)
{
#ifndef _WIN32
    extern s_char *tgetstr(char *, char **);
    s_char *cp;
    s_char *term;
    static s_char tbuf[1024];
    static s_char cbuf[20];

    memset((s_char *)&cbuf[0], 0, 20);
    term = getenv("TERM");
    if (term == 0) {
	fprintf(stderr, "warning: no TERM environment variable\n");
	return;
    }
    tgetent(tbuf, term);
    cp = cbuf;
    SO = tgetstr("so", &cp);
    SE = tgetstr("se", &cp);
    if (SO == 0) {
	SO = tgetstr("us", &cp);
	SE = tgetstr("ue", &cp);
    }
    if (SO != 0) {
	parsedelay(SO);
	parsedelay(SE);
    }
#endif
}
