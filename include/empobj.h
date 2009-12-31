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
 *  empobj.h: General empire objects.
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2006
 *     Markus Armbruster, 2006-2008
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
    /*
     * initial part must match struct emptypedstr
     * valid if EFF_TYPED is set in table's flags
     */
    signed ef_type: 8;
    unsigned seqno: 12;
    unsigned generation: 12;
    int uid;
    time_t timestamp;
    /* end of part matching struct emptypedstr */
    natid own;		/* valid if EFF_OWNER is in table's flags */
    coord x;		/* valid if EFF_XY    is in table's flags */
    coord y;		/* valid if EFF_XY    is in table's flags */
    signed char type;	/* valid for sectors and units */
    /* remaining are valid for units */
    signed char effic;
    signed char mobil;
    unsigned char off;
    short tech;
    char group;		/* valid if EFF_GROUP is in table's flags */
    coord opx, opy;
    short mission;
    short radius;
};

union empobj_storage {
    struct empobj gen;
    struct comstr comm;
    struct gamestr game;
    struct lndstr land;
    struct lonstr loan;
    struct loststr lost;
    struct natstr nat;
    struct nukstr nuke;
    struct plnstr plane;
    struct realmstr realm;
    struct sctstr sect;
    struct shpstr ship;
    struct trdstr trade;
    struct trtstr treaty;
};

struct empobj_chr;

#define get_empobj(type, n, p) ef_read((type), (n), (p))
#define put_empobj(type, n, p) ef_write((type), (n), (p))
#define get_empobjp(type, n) ((struct empobj *)ef_ptr((type), (n)))

extern char *obj_nameof(struct empobj *gp);
extern struct empobj_chr *get_empobj_chr(struct empobj *gp);
extern char *empobj_chr_name(struct empobj *gp);
extern int get_empobj_mob_max(int type);

#endif
