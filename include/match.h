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
 *  match.h: Used for string compare matching
 *
 *  Known contributors to this file:
 *
 */

#ifndef MATCH_H
#define MATCH_H

#include <stddef.h>

/* returned by stmtch() and comtch() */
#define M_IGNORE	-3
#define M_NOTUNIQUE	-2
#define M_NOTFOUND	-1
/*#define M_INDEX [0...N-1] */

/* returned by mineq() */
#define ME_MISMATCH	0
#define ME_PARTIAL	1
#define ME_EXACT	2

extern int stmtch(char *, void *, ptrdiff_t, size_t);
extern int mineq(char *, char *);

#endif
