/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  optlist.h: Definitions of option listings
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#ifndef _OPTLIST_H_
#define _OPTLIST_H_

struct option_list {
    char *opt_key;
    int *opt_valuep;
};

extern struct option_list Options[];

/* Options, can be switched in econfig */
extern int opt_ALL_BLEED;
extern int opt_BIG_CITY;
extern int opt_BLITZ;
extern int opt_BRIDGETOWERS;
extern int opt_DEFENSE_INFRA;
extern int opt_DEMANDUPDATE;
extern int opt_DRNUKE;
extern int opt_EASY_BRIDGES;
extern int opt_FALLOUT;
extern int opt_FUEL;
extern int opt_GODNEWS;
extern int opt_GO_RENEW;
extern int opt_GUINEA_PIGS;
extern int opt_HIDDEN;
extern int opt_INTERDICT_ATT;
extern int opt_LANDSPIES;
extern int opt_LOANS;
extern int opt_LOSE_CONTACT;
extern int opt_MARKET;
extern int opt_MOB_ACCESS;
extern int opt_NEUTRON;
extern int opt_NOFOOD;
extern int opt_NOMOBCOST;
extern int opt_NONUKES;
extern int opt_NO_FORT_FIRE;
extern int opt_NO_HCMS;
extern int opt_NO_LCMS;
extern int opt_NO_OIL;
extern int opt_NO_PLAGUE;
extern int opt_PINPOINTMISSILE;
extern int opt_RES_POP;
extern int opt_ROLLOVER_AVAIL;
extern int opt_SAIL;
extern int opt_SHIPNAMES;
extern int opt_SHOWPLANE;
extern int opt_SLOW_WAR;
extern int opt_SNEAK_ATTACK;
extern int opt_SUPER_BARS;
extern int opt_TECH_POP;
extern int opt_TRADESHIPS;
extern int opt_TREATIES;
extern int opt_UPDATESCHED;

/* Game parameters, can be set in econfig */
#define	EMP_CONFIG_H_OUTPUT
#include "econfig-spec.h"
#undef	EMP_CONFIG_H_OUTPUT

/* Game parameters that cannot be set in econfig (yet) */
extern int melt_item_denom[];
#ifdef START_UNITS
int start_unit_type[];
#endif

/* Variables that get values derived from econfig */
extern char *motdfil;
extern char *downfil;
extern char *disablefil;
extern char *banfil;
extern char *authfil;
extern char *annfil;
extern char *timestampfil;
extern char *teldir;
extern char *telfil;

enum {
    KM_ALLOC = 0x01,		/* memory allocated */
    KM_INTERNAL = 0x02		/* not to be disclosed to players */
};

struct keymatch {
    char *km_key;		/* the key */
    nsc_type km_type;		/* type of associated data */
    void *km_data;		/* pointer to associated data */
    int km_flags;		/* useful flags */
    char *km_comment;		/* Comment (hopefully useful) */
};

extern struct keymatch configkeys[];

#endif /* _OPTLIST_H_ */
