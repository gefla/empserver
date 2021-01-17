/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  xdump.h: Extended dumps
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2008-2016
 */

#ifndef XDUMP_H
#define XDUMP_H

#include <stdio.h>
#include "nsc.h"
#include "types.h"

/* xdump descriptor */
struct xdstr {
    natid cnum;			/* dump for this country */
    int divine;			/* is this a deity dump? */
    int human;			/* dump human-readable format */
    int sloppy;			/* try to cope with invalid data */
    void (*pr)(char *fmt, ...)	/* callback for printing dump */
	ATTRIBUTE((format (printf, 1, 2)));
};

struct xdstr *xdinit(struct xdstr *, natid, int, int,
		     void (*)(char *, ...));
extern void xdhdr(struct xdstr *, char *, int);
extern void xdcolhdr(struct xdstr *, struct castr[]);
extern void xdflds(struct xdstr *, struct castr[], void *);
extern void xdftr(struct xdstr *, int);
extern int xundump(FILE *, char *, int *, int);
extern int xundumpable(int);
extern int xundump_redundant(int, int, void *);

#endif
