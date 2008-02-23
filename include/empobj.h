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
 *  empobj.h: General empire objects.
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2006
 *     Markus Armbruster, 2006
 */

#ifndef EMPOBJ_H
#define EMPOBJ_H

#include "commodity.h"
#include "game.h"
#include "land.h"
#include "loan.h"
#include "lost.h"
#include "nat.h"
#include "news.h"
#include "nuke.h"
#include "plane.h"
#include "sect.h"
#include "ship.h"
#include "trade.h"
#include "treaty.h"
#include "types.h"

struct empobj {
    short ef_type;	/* is always valid */
    natid own;		/* is valid if EFF_OWNER   is set in table def. */
    short uid;
    coord x;		/* is valid if EFF_XY      is set in table def. */
    coord y;		/* is valid if EFF_XY      is set in table def. */
    signed char type;	/* is valid for sectors and units */
    /* remaining are valid for units */
    signed char effic;
    signed char mobil;
    unsigned char off;
    short tech;	
    char group;		/* is valid if EFF_GROUP   is set in table def. */
    coord opx, opy;
    short mission;
    short radius;
};

union empobj_storage {
    short ef_type;
    struct empobj gen;
    struct comstr comm;
    struct gamestr game;
    struct lndstr land;
    struct lonstr loan;
    struct loststr lost;
    struct natstr nat; 
    struct nwsstr news;
    struct nukstr nuke;
    struct plnstr plane;
    struct realmstr realm;
    struct sctstr sect;
    struct shpstr ship;
    struct trdstr trade;
    struct trtstr treaty;
};

struct empobj_chr;

extern char *obj_nameof(struct empobj *gp);
extern struct empobj *get_empobjp(int type, int id);
extern int put_empobj(int, int, struct empobj *gp);
extern struct empobj_chr *get_empobj_chr(struct empobj *gp);
extern char *emp_obj_chr_name(struct empobj *gp);
extern int get_empobj_mob_max(int type);

#endif
