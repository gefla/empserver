/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  optlist.h: Definitions of option listings
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#ifndef OPTLIST_H
#define OPTLIST_H

#include "nsc.h"

extern char dflt_econfig[];

/* Game parameters, can be set in econfig */
#define	EMP_CONFIG_H_OUTPUT
#include "econfig-spec.h"
#undef	EMP_CONFIG_H_OUTPUT

extern char *configdir;
extern char *schedulefil;

extern char motdfil[];
extern char downfil[];
extern char annfil[];
extern char teldir[];

enum {
    KM_ALLOC = 1,		/* memory allocated */
    KM_INTERNAL = 2,		/* not to be disclosed to players */
    KM_OPTION = 4		/* historically an option */
};

enum {
    UPD_DEMAND_NONE,		/* no demand updates */
    UPD_DEMAND_SCHED,		/* scheduled updates are demand updates */
    UPD_DEMAND_ASYNC,		/* zdone triggers unscheduled update */
};

struct keymatch {
    char *km_key;		/* the key */
    nsc_type km_type;		/* type of associated data */
    void *km_data;		/* pointer to associated data */
    int km_flags;		/* useful flags */
    char *km_comment;		/* Comment (hopefully useful) */
};

extern struct keymatch configkeys[];

#endif
