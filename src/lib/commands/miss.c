/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  miss.c: set missions for ships/planes/units
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Steve McClure, 2000
 *     Markus Armbruster, 2005-2010
 */

#include <config.h>

#include "commands.h"
#include "mission.h"
#include "optlist.h"
#include "path.h"
#include "unit.h"

static int clear_mission(struct nstr_item *);
static int show_mission(struct nstr_item *);

/*
 *  mission <type> <planes/ships/units> <mission type> <op sector> [<radius>]
 */
int
mission(void)
{
    static int ef_with_missions[] = { EF_SHIP, EF_LAND, EF_PLANE, EF_BAD };
    char *p;
    int type;
    int mission;
    coord x, y;
    int radius, range;
    union empobj_storage item;
    struct empobj *gp;
    int num = 0;
    struct nstr_item ni;
    char buf[1024];

    p = getstarg(player->argp[1], "Ship, plane or land unit (p,sh,la)? ",
		 buf);
    if (!p)
	return RET_SYN;
    type = ef_byname_from(p, ef_with_missions);
    if (type < 0) {
	pr("Ships, land units or planes only! (s, l, p)\n");
	return RET_SYN;
    }
    if (!snxtitem(&ni, type, player->argp[2], NULL))
	return RET_SYN;

    p = getstarg(player->argp[3],
		 "Mission (int, sup, osup, dsup, esc, res, air, query, clear)? ",
		 buf);
    if (!p)
	return RET_SYN;

/*
 * 'i'     interdiction
 * 's'     support
 * 'o'     support attacks
 * 'd'     support defenders
 * 'e'     escort
 * 'r'     defensive reserve
 * 'a'     air defense (intercepts)
 */
    switch (*p) {
    case 'I':
    case 'i':
	mission = MI_INTERDICT;
	break;
    case 'O':
    case 'o':
	mission = MI_OSUPPORT;
	break;
    case 'D':
    case 'd':
	mission = MI_DSUPPORT;
	break;
    case 'S':
    case 's':
	mission = MI_SUPPORT;
	break;
    case 'C':
    case 'c':
	return clear_mission(&ni);
    case 'E':
    case 'e':
	mission = MI_ESCORT;
	break;
    case 'R':
    case 'r':
	mission = MI_RESERVE;
	break;
    case 'A':
    case 'a':
	mission = MI_AIR_DEFENSE;
	break;
    case 'q':
	return show_mission(&ni);
    default:
	pr("bad condition\n");
	pr("i\tinterdiction (any)\n");
	pr("s\tsupport (tactical planes only)\n");
	pr("o\toffensive support (tactical planes only)\n");
	pr("d\tdefensive support (tactical planes only)\n");
	pr("r\treserve (land units only)\n");
	pr("e\tescort (tactical or escort planes only)\n");
	pr("a\tair defense (intercept planes only)\n");
	pr("c\tclear mission\n");
	pr("q\tquery\n");
	return RET_SYN;
    }

    if (!cando(mission, type)) {
	pr("A %s cannot do that mission!\n", ef_nameof(type));
	pr("i\tinterdiction (any)\n");
	pr("s\tsupport (planes only)\n");
	pr("o\toffensive support (planes only)\n");
	pr("d\tdefensive support (planes only)\n");
	pr("r\treserve (land units only)\n");
	pr("e\tescort (planes only)\n");
	pr("a\tair defense (planes only)\n");
	return RET_FAIL;
    }

    p = getstarg(player->argp[4], "operations point? ", buf);
    if (!p || !*p)
	return RET_SYN;

    if (*p != '.') {
	if (!sarg_xy(p, &x, &y))
	    return RET_SYN;
    }

    if (player->argp[5] != NULL) {
	radius = atoi(player->argp[5]);
	if (radius < 0) {
	    pr("Radius can't be negative!\n");
	    return RET_FAIL;
	}
    } else
	radius = 9999;

    while (nxtitem(&ni, &item)) {
	gp = (struct empobj *)&item;

	if (!player->owner || gp->own == 0)
	    continue;

	if (mission == MI_RESERVE) {
	    if (!lnd_can_attack((struct lndstr *)gp)) {
		pr("%s is not designed to fight ground troops\n",
		   unit_nameof(gp));
		continue;
	    }
	    if (!lchr[gp->type].l_rad) {
		pr("%s cannot react anywhere!\n", unit_nameof(gp));
		continue;
	    }
	}

	if ((mission == MI_INTERDICT) && (type == EF_SHIP))
	    if (mchr[(int)gp->type].m_glim == 0) {
		pr("%s: cannot fire at range!\n", unit_nameof(gp));
		continue;
	    }

	if ((mission == MI_INTERDICT) && (type == EF_LAND))
	    if (lchr[(int)gp->type].l_dam == 0) {
		pr("%s: cannot fire at range!\n", unit_nameof(gp));
		continue;
	    }

	if ((mission == MI_INTERDICT) && (type == EF_PLANE)) {
	    struct plchrstr *pcp;

	    pcp = &plchr[(int)gp->type];
	    if (!(pcp->pl_flags & P_T)) {
		pr("Only planes with the tactical ability can interdict.\n"
		   "%s #%d is ineligible\n",
		   pcp->pl_name, gp->uid);
		continue;
	    }
	}

	if ((mission == MI_AIR_DEFENSE) && (type == EF_PLANE)) {
	    struct plchrstr *pcp;

	    pcp = &plchr[(int)gp->type];
	    if (!(pcp->pl_flags & P_F)) {
		pr("Only planes with the intercept abilities can perform air defense.\n"
		   "%s #%d is ineligible\n",
		   pcp->pl_name, gp->uid);
		continue;
	    }
	}

	if ((mission == MI_ESCORT) && (type == EF_PLANE)) {
	    struct plchrstr *pcp;

	    pcp = &plchr[(int)gp->type];
	    if (!(pcp->pl_flags & P_ESC) && !(pcp->pl_flags & P_F)) {
		pr("Only planes with the escort or intercept abilities can escort.\n"
		   "%s #%d is ineligible\n",
		   pcp->pl_name, gp->uid);
		continue;
	    }
	}

	if ((mission == MI_SUPPORT || mission == MI_OSUPPORT ||
	     mission == MI_DSUPPORT) && (type == EF_PLANE)) {
	    struct plchrstr *pcp;

	    pcp = &plchr[(int)gp->type];
	    if (!(pcp->pl_flags & P_T)) {
		pr("Only planes with the tactical ability can support.\n"
		   "%s #%d is ineligible\n",
		   pcp->pl_name, gp->uid);
		continue;
	    }
	}

	if (type == EF_PLANE && nuk_on_plane((struct plnstr *)gp) >= 0) {
	    pr("%s can't perform a mission while it carries a nuclear weapon",
	       unit_nameof(gp));
	    continue;
	}

	if (*p == '.') {
	    x = gp->x;
	    y = gp->y;
	}

	gp->mission = mission;
	range = oprange(gp);
	if (range < mapdist(gp->x, gp->y, x, y)) {
	    pr("%s: out of range! (range %d)\n",
	       unit_nameof(gp), range);
	    continue;
	}
	gp->opx = x;
	gp->opy = y;
	gp->radius = MIN(range, radius);
	put_empobj(type, gp->uid, gp);
	num++;

	pr("%s on %s mission, centered on %s, radius %d\n",
	   unit_nameof(gp), mission_name(mission),
	   xyas(x, y, player->cnum), gp->radius);
    }
    if (num == 0) {
	pr("No %s%s\n", ef_nameof(type), splur(num));
	return RET_FAIL;
    }
    pr("%d %s%s\n", num, ef_nameof(type), splur(num));
    return RET_OK;
}

static int
clear_mission(struct nstr_item *np)
{
    union empobj_storage item;

    while (nxtitem(np, &item)) {
	item.gen.mission = 0;
	put_empobj(item.gen.ef_type, item.gen.uid, &item);
    }

    return RET_OK;
}

static int
show_mission(struct nstr_item *np)
{
    int first = 1;
    union empobj_storage item;
    struct empobj *gp;

    while (nxtitem(np, &item)) {
	gp = (struct empobj *)&item;
	if (!player->owner || gp->own == 0)
	    continue;

	if (first) {
	    pr("Thing                         x,y   op-sect rad mission\n");
	    first = 0;
	}
	pr("%-25s", unit_nameof(gp));
	prxy(" %3d,%-3d", gp->x, gp->y);
	switch (gp->mission) {
	case MI_INTERDICT:
	case MI_SUPPORT:
	case MI_RESERVE:
	case MI_ESCORT:
	case MI_AIR_DEFENSE:
	case MI_DSUPPORT:
	case MI_OSUPPORT:
	    prxy(" %3d,%-3d", gp->opx, gp->opy);
	    pr("  %4d", gp->radius);
	    break;
	default:
	    CANT_REACH();
	    /* fall through */
	case MI_NONE:
	    pr("              ");
	}
	if (gp->mission)
	    pr(" is on %s mission\n", mission_name(gp->mission));
	else
	    pr(" has no mission.\n");
    }

    return RET_OK;
}
