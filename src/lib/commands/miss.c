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
 *  miss.c: set missions for ships/planes/units
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Steve McClure, 2000
 */

#include <config.h>

#include "commands.h"
#include "empobj.h"
#include "mission.h"
#include "optlist.h"
#include "path.h"

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
    int desired_radius, radius;
    struct sctstr opsect;
    union empobj_storage item;
    struct empobj *gp;
    int num = 0, mobmax, mobused, dist;
    struct nstr_item ni;
    char prompt[128];
    char buf[1024];

    if ((p =
	 getstarg(player->argp[1], "Ship, plane or land unit (p,sh,la)? ",
		  buf)) == 0)
	return RET_SYN;
    type = ef_byname_from(p, ef_with_missions);
    if (type < 0) {
	pr("Ships, land units or planes only! (s, l, p)\n");
	return RET_SYN;
    }
    sprintf(prompt, "%s(s)? ", ef_nameof(type));
    p = getstarg(player->argp[2], prompt, buf);
    if (!snxtitem(&ni, type, p))
	return RET_SYN;

    if ((p =
	 getstarg(player->argp[3],
		  "Mission (int, sup, osup, dsup, esc, res, air, query, clear)? ",
		  buf)) == 0)
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
	mission = 0;
	break;
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
	show_mission(type, &ni);
	return RET_OK;
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

    if (mission && !cando(mission, type)) {
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

    if (mission && ((mission != MI_RESERVE) && (mission != MI_ESCORT))) {
	if ((p = getstarg(player->argp[4], "operations point? ", buf)) == 0
	    || *p == 0)
	    return RET_SYN;

	if (*p != '.') {
	    if (!sarg_xy(p, &x, &y))
		return RET_SYN;

	    if (!getsect(x, y, &opsect))
		return RET_FAIL;
	}
    } else {
	x = 0;
	y = 0;
    }

    if (player->argp[5] != NULL) {
	desired_radius = atoi(player->argp[5]);
	if (desired_radius < 0) {
	    pr("Radius must be greater than zero!\n");
	    return RET_FAIL;
	}
    } else {
	desired_radius = 9999;
    }

    if ((mobmax = get_empobj_mob_max(type)) == -1)
	return RET_FAIL;

    mobused = ldround(mission_mob_cost * (double)mobmax, 1);

    while (nxtitem(&ni, &item)) {
	gp = (struct empobj *)&item;

	if (!player->owner || gp->own == 0)
	    continue;

	if ((mission && (gp->mobil < mobused)) && mission_mob_cost) {
	    pr("%s: not enough mobility! (needs %d)\n",
	       obj_nameof(gp), mobused);
	    continue;
	}
	if (mission == MI_RESERVE && !lnd_can_attack((struct lndstr *)gp)) {
	    pr("%s is not designed to fight ground troops\n",
	       obj_nameof(gp));
	    continue;
	}
	if (*p == '.') {
	    x = gp->x;
	    y = gp->y;
	    if (!getsect(x, y, &opsect))
		return RET_FAIL;
	}

	dist = mapdist(gp->x, gp->y, x, y);
	radius = 999;
	if ((mission == MI_INTERDICT || mission == MI_SUPPORT ||
	     mission == MI_OSUPPORT || mission == MI_DSUPPORT ||
	     mission == MI_AIR_DEFENSE) &&
	    (oprange(gp, &radius) < dist)) {
	    pr("%s: out of range! (range %d)\n",
	       obj_nameof(gp), oprange(gp, &radius));
	    continue;
	}

	if (radius > desired_radius)
	    radius = desired_radius;

	if ((mission == MI_INTERDICT) && (type == EF_SHIP))
	    if (mchr[(int)gp->type].m_glim == 0) {
		pr("%s: cannot fire at range!\n", obj_nameof(gp));
		continue;
	    }

	if ((mission == MI_INTERDICT) && (type == EF_LAND)) {
	    if (lchr[(int)gp->type].l_dam == 0) {
		pr("%s: cannot fire at range!\n", obj_nameof(gp));
		continue;
	    }
	    if (lchr[(int)gp->type].l_flags & L_HEAVY) {
		pr("%s: too ponderous to interdict!\n", obj_nameof(gp));
		continue;
	    }
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

	num++;			/* good one.. go with it */

	if (mission == MI_INTERDICT || mission == MI_SUPPORT ||
	    mission == MI_OSUPPORT || mission == MI_DSUPPORT ||
	    mission == MI_AIR_DEFENSE)
	    gp->radius = radius;
	else
	    gp->radius = 0;

	if (mission == MI_SUPPORT || mission == MI_OSUPPORT ||
	    mission == MI_DSUPPORT || mission == MI_INTERDICT ||
	    mission == MI_AIR_DEFENSE) {
	    pr("%s on %s mission, centered on %s, radius %d\n",
	       obj_nameof(gp), mission_name(mission),
	       xyas(x, y, player->cnum), gp->radius);
	} else if (mission == MI_RESERVE) {
	    int plus = 2;

	    if (((struct lndstr *)gp)->lnd_rad_max == 0) {
		plus = 0;
	    } else {
		getsect(gp->x, gp->y, &opsect);
		if ((opsect.sct_type == SCT_HEADQ)
		    && (opsect.sct_effic >= 60))
		    plus++;
		plus += ((struct lndstr *)gp)->lnd_rad_max;
	    }

	    pr("%s on %s mission with maximum reaction radius %d\n",
	       obj_nameof(gp), mission_name(mission), plus);
	} else if (mission) {
	    pr("%s on %s mission\n", obj_nameof(gp),
	       mission_name(mission));
	}

	if (mission)
	    gp->mobil -= mobused;
	gp->mission = mission;
	gp->opx = x;
	gp->opy = y;
	put_empobj(type, gp->uid, gp);
    }
    if (num == 0) {
	pr("No %s%s\n", ef_nameof(type), splur(num));
	return RET_FAIL;
    }
    pr("%d %s%s\n", num, ef_nameof(type), splur(num));
    return RET_OK;
}
