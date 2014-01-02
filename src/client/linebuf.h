/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  linebuf.h: Simple line buffer
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#ifndef LINEBUF_H
#define LINEBUF_H

#define LBUF_LEN_MAX 4096

struct lbuf {
    /* All members are private! */
    unsigned len;		/* strlen(line) */
    int full;			/* got a complete line, with newline? */
    char line[LBUF_LEN_MAX];	/* buffered line, zero-terminated */
};

extern void lbuf_init(struct lbuf *);
extern int lbuf_len(struct lbuf *);
extern int lbuf_full(struct lbuf *);
extern char *lbuf_line(struct lbuf *);
extern int lbuf_putc(struct lbuf *, char);

#endif
