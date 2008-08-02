/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  journal.h: Log a journal of events to a file
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2008
 */

#ifndef JOURNAL_H
#define JOURNAL_H

struct player;			/* FIXME temporary hack */

int journal_startup(void);
void journal_shutdown(void);
int journal_reopen(void);
void journal_login(void);
void journal_logout(void);
void journal_prng(unsigned);
void journal_output(struct player *, int, char *);
void journal_input(char *);
void journal_command(char *);
void journal_update(int);

#endif
