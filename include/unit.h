/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  unit.h: Generalize unit data structures and functions.
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2006-2007
 *     Markus Armbruster, 2006-2015
 */

#ifndef UNIT_H
#define UNIT_H

#include "empobj.h"

struct ulist {
    struct emp_qelem queue;	/* list of units */
    double mobil;		/* moving: how much the unit has left
				   ground combat: how much to charge */
    union empobj_storage unit;	/* unit */
    /* Rest used only by ground combat (attsub.c) */
    coord x, y;			/* x,y it came from */
    int eff;			/* initial efficiency */
    int supplied;
};

extern void unit_cargo_init(void);
extern void unit_carrier_change(struct empobj *, int, int, int);
extern int unit_cargo_first(int, int, int);
extern int unit_cargo_next(int, int);
extern int unit_cargo_count(int, int, int);
extern int unit_nplane(int, int, int *, int *, int *);
extern void unit_onresize(int);

extern char *unit_nameof(struct empobj *);
extern void unit_rad_map_set(struct emp_qelem *);
extern int unit_move(struct emp_qelem *);
extern void unit_teleport(struct empobj *, coord, coord);
extern int unit_update_cargo(struct empobj *);
extern void unit_drop_cargo(struct empobj *, natid);
extern void unit_give_away(struct empobj *, natid, natid);
extern void unit_wipe_orders(struct empobj *);

#endif
