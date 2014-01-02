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
 *  com.h: Definitions used to parse Empire commands
 *
 *  Known contributors to this file:
 *
 */

#ifndef COM_H
#define COM_H

struct cmndstr {
    char *c_form;		/* prototype of command */
    int c_cost;			/* btu cost of command */
    int (*c_addr)(void);	/* core addr of appropriate routine */
    int c_flags;		/* command flags */
    int c_permit;		/* command capabilities required */
};

/* Command flag bits */
enum {
    /*
     * Command modifies database.
     * This also prevents print functions from blocking.  In fact,
     * that's the effective meaning of this flag.  Nothing stops a
     * command without C_MOD modifying whatever it wants.
     */
    C_MOD = 1
};

/* variables associated with this stuff */

extern struct cmndstr player_coms[];

#endif
