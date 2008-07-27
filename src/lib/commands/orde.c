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
 *  orde.c: Turn on/off autonavigation
 * 
 *  Known contributors to this file:
 *     Chad Zabel, 1994
 *     Steve McClure, 2000
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "optlist.h"
#include "path.h"
#include "ship.h"

/*
 *  Command syntax:
 *
 *  ORDER <ship>				  Show orders
 *  ORDER <ship> c[ancel]			  Cancel orders
 *  ORDER <ship> s[top]				  Suspend orders
 *  ORDER <ship> r[esume]			  Resume orders
 *  ORDER <ship> d[eclare] <dest1>		  Set destination
 *		 d[eclare] <dest1> <dest2>
 *  ORDER <ship> l[evel]   <field> <start/end> <comm> <level>
 *
 * New syntax:
 *  qorder <ship>    display cargo levels     
 *  sorder <ship>    display statistical info 
 */

int
orde(void)
{
    int diffeachship = 0;
    int orders, sub, level;
    int scuttling = 0;
    struct nstr_item nb;
    struct shpstr ship;
    struct shpstr start;	/* Used for checking database */
    struct ichrstr *i1;
    coord p0x, p0y, p1x, p1y;
    int i;
    char *p, *p1, *dest;
    char buf1[128];
    char buf[1024];
    char prompt[128];

    if (!snxtitem(&nb, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;
    while (!player->aborted && nxtitem(&nb, (&ship))) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (opt_SAIL) {
	    if (*ship.shp_path) {
		if (!diffeachship)
		    pr("Ship #%d has a \"sail\" path!\n", ship.shp_uid);
		continue;
	    }
	}
	memcpy(&start, &ship, sizeof(struct shpstr));
	sprintf(prompt,
		"Ship #%d, declare, cancel, suspend, resume, level? ",
		ship.shp_uid);
	p = getstarg(player->argp[2], prompt, buf);
	if (player->aborted || !p)
	    return RET_FAIL;
	if (!*p) {
	    if (!diffeachship)
		return RET_FAIL;
	    else
		continue;
	}
	switch (*p) {
	default:
	    pr("Bad order type!\n");
	    return RET_SYN;
	case 'c':		/* clear ship fields  */
	    ship.shp_mission = 0;
	    ship.shp_autonav &= ~(AN_AUTONAV + AN_STANDBY + AN_LOADING);
	    for (i = 0; i < TMAX; i++) {
		ship.shp_tstart[i] = I_NONE;
		ship.shp_tend[i] = I_NONE;
		ship.shp_lstart[i] = 0;
		ship.shp_lend[i] = 0;
	    }
	    break;
	case 's':		/* suspend ship movement  */
	    ship.shp_mission = 0;
	    ship.shp_autonav |= AN_STANDBY;
	    break;
	case 'r':		/* resume ship movement   */
	    ship.shp_mission = 0;
	    ship.shp_autonav &= ~AN_STANDBY;
	    break;
	case 'd':		/* declare path */
	    orders = 0;
	    scuttling = 0;
	    /* Need location */
	    if ((p = getstarg(player->argp[3], "Destination? ", buf)) == 0
		|| *p == 0)
		return RET_SYN;
	    if (!sarg_xy(p, &p0x, &p0y))
		return RET_SYN;
	    p1x = p0x;
	    p1y = p0y;



	    if (!orders) {
		p = getstarg(player->argp[4], "Second dest? ", buf);
		if (!p)
		    return RET_SYN;
		if (!*p || !strcmp(p, "-")) {
		    orders = 1;
		    pr("A one-way order has been accepted.\n");
		} else if (!strncmp(p, "s", 1)) {
		    if (opt_TRADESHIPS) {
			if (!(mchr[(int)ship.shp_type].m_flags & M_TRADE)) {
			    pr("You can't auto-scuttle that ship!\n");
			    return RET_SYN;
			}
		    } else {
			pr("You can't auto-scuttle that ship!\n");
			return RET_SYN;
		    }
		    pr("A scuttle order has been accepted.\n");
		    scuttling = 1;
		} else {
		    if (!sarg_xy(p, &p1x, &p1y))
			return RET_SYN;
		    pr("A circular order has been accepted.\n");
		}
	    }

	    /*
	     *  Set new destination and trade type fields.
	     */
	    ship.shp_mission = 0;
	    ship.shp_destx[1] = p1x;
	    ship.shp_desty[1] = p1y;
	    ship.shp_destx[0] = p0x;
	    ship.shp_desty[0] = p0y;

	    ship.shp_autonav &= ~(AN_STANDBY | AN_LOADING);
	    ship.shp_autonav |= AN_AUTONAV;

	    if (scuttling)
		ship.shp_autonav |= AN_SCUTTLE;
	    break;

	    /* set cargo levels on the ship */

	case 'l':
	    /* convert player->argp[3] to an integer */
	    if (player->argp[3])
		sub = atoi(player->argp[3]);
	    else {
		sprintf(buf1, "Field (1-%d) ", TMAX);
		if (getstarg(player->argp[3], buf1, buf) == 0)
		    return RET_SYN;
		sub = atoi(buf);
	    }
	    /* check to make sure value in within range. */
	    if (sub > TMAX || sub < 1) {
		pr("Value must range from 1 to %d\n", TMAX);
		break;
	    }

	    /* to keep sub in range of our arrays 
	       subtract 1 so the new range is 0-(TMAX-1)
	     */
	    sub = sub - 1;;

	    if (ship.shp_autonav & AN_AUTONAV) {
		orders = 1;
		dest = getstarg(player->argp[4], "Start or End? ", buf);
		if (orders) {	/* before dest check */
		    if (!dest)
			break;
		    switch (*dest) {
		    default:
			pr("You must enter 'start' or 'end'\n");
			return RET_SYN;
		    case 'e':
		    case 'E':
			i1 = whatitem(player->argp[5], "Commodity? ");
			if (!i1)
			    break;
			else {
			    p1 = getstarg(player->argp[6], "Amount? ",
					  buf);
			    if (!p1)
				return RET_SYN;
			    level = atoi(p1);
			}
			if (level < 0) {
			    level = 0;	/* prevent negatives. */
			    pr("You must use positive number! Level set to 0.\n");
			}
			ship.shp_tstart[sub] = i1->i_uid;
			ship.shp_lstart[sub] = level;
			pr("Order Set \n");
			break;
		    case 's':
		    case 'S':
			i1 = whatitem(player->argp[5], "Commodity? ");
			if (!i1)
			    break;
			else {
			    p1 = getstarg(player->argp[6], "Amount? ",
					  buf);
			    if (!p1)
				return RET_SYN;
			    level = atoi(p1);
			}
			if (level < 0) {
			    level = 0;
			    pr("You must use positive number! Level set to 0.\n");
			}
			ship.shp_tend[sub] = i1->i_uid;
			ship.shp_lend[sub] = level;
			pr("Order Set \n");
			break;
		    }
		}
	    } else
		pr("You need to 'declare' a ship path first, see 'info order'\n");

	    break;
	}			/* end of switch (*p) */



	/*
	 *  Set loading flag if ship is already in one
	 *  of the specified harbors and a cargo has been
	 *  specified.
	 */

	if (((ship.shp_x == ship.shp_destx[0])
	     && (ship.shp_y == ship.shp_desty[0])
	     && (ship.shp_lstart[0] != ' '))
	    || ((ship.shp_x == ship.shp_desty[1])
		&& (ship.shp_y == ship.shp_desty[1])
		&& (ship.shp_lstart[1] != ' '))) {

	    coord tcord;
	    i_type tcomm;
	    short lev[TMAX];
	    int i;

	    ship.shp_autonav |= AN_LOADING;

	    /*  swap variables, this keeps 
	       the load_it() procedure happy. CZ
	     */
	    tcord = ship.shp_destx[0];
	    ship.shp_destx[0] = ship.shp_destx[1];
	    ship.shp_destx[1] = tcord;
	    tcord = ship.shp_desty[0];
	    ship.shp_desty[0] = ship.shp_desty[1];
	    ship.shp_desty[1] = tcord;

	    for (i = 0; i < TMAX; i++) {
		lev[i] = ship.shp_lstart[i];
		ship.shp_lstart[i] = ship.shp_lend[i];
		ship.shp_lend[i] = lev[i];
		tcomm = ship.shp_tstart[i];
		ship.shp_tstart[i] = ship.shp_tend[i];
		ship.shp_tend[i] = tcomm;
	    }
	}
	/*
	   **  Write ship back to database, then give it
	   **  a kick down the autonav route if necessary.
	 */


	/* Now do a sanity check. */
	if (!check_ship_ok(&start))
	    return RET_SYN;

	putship(ship.shp_uid, &ship);
    }
    return RET_OK;
}

static void
eta_calc(struct shpstr *sp, char *path, int *len, int *nupdates)
{
    double mobcost, mobil;
    int i;

    i = strlen(path);
    *len = i;
    *nupdates = 1;

    mobcost = shp_mobcost(sp);
    mobil = sp->shp_mobil;
    while (i) {
	if (mobil > 0) {
	    mobil -= mobcost;
	    i--;
	} else {
	    mobil += (ship_mob_scale * (float)etu_per_update);
	    (*nupdates)++;
	}
    }
}

static void
prhold(int hold, i_type itype, int amt)
{
    if (itype != I_NONE && amt != 0) {
	if (CANT_HAPPEN(itype <= I_NONE || itype > I_MAX))
	    return;
	pr("%d-", hold + 1);
	pr("%c", ichr[itype].i_mnem);
	pr(":");
	pr("%d ", amt);
    }
}

int
qorde(void)
{
    int nships = 0;
    int i;
    struct nstr_item nb;
    struct shpstr ship;

    if (!snxtitem(&nb, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&nb, (&ship))) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (!(ship.shp_autonav & AN_AUTONAV)
	    && (!opt_SAIL || !ship.shp_path[0]))
	    continue;

	if (!nships) {		/* 1st ship, print banner */
	    if (player->god)
		pr("own ");
	    pr("shp#     ship type    ");
	    pr("[Starting]       (Ending)    \n");
	}
	nships++;
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%4d", nb.cur);
	pr(" %-16.16s", mchr[(int)ship.shp_type].m_name);

	if (ship.shp_autonav & AN_AUTONAV) {
	    pr(" [");
	    for (i = 0; i < TMAX; i++)
		prhold(i, ship.shp_tend[i], ship.shp_lend[i]);
	    pr("] , (");
	    for (i = 0; i < TMAX; i++)
		prhold(i, ship.shp_tstart[i], ship.shp_lstart[i]);
	    pr(")");
	    if (ship.shp_autonav & AN_SCUTTLE)
		pr(" scuttling");
	    pr("\n");
	} else
	    pr(" has a sail path\n");

	if (ship.shp_name[0] != 0) {
	    if (player->god)
		pr("    ");
	    pr("       %s\n", ship.shp_name);
	}
    }
    if (!nships) {
	if (player->argp[1])
	    pr("%s: No ship(s)\n", player->argp[1]);
	else
	    pr("%s: No ship(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d ship%s\n", nships, splur(nships));
    return RET_OK;
}

int
sorde(void)
{
    int nships = 0;
    int len, updates;
    char *c;
    struct nstr_item nb;
    struct shpstr ship;
    char buf[1024];

    if (!snxtitem(&nb, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&nb, (&ship))) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (!(ship.shp_autonav & AN_AUTONAV)
	    && (!opt_SAIL || !ship.shp_path[0]))
	    continue;

	if (!nships) {		/* 1st ship, print banner */
	    if (player->god)
		pr("own ");
	    pr("shp#     ship type      x,y    ");
	    pr("start    end   ");
	    pr("len  eta\n");
	}
	nships++;
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%4d", nb.cur);
	pr(" %-16.16s", mchr[(int)ship.shp_type].m_name);
	prxy(" %3d,%-3d", ship.shp_x, ship.shp_y, player->cnum);

	if (ship.shp_autonav & AN_AUTONAV) {
	    /* Destination 1 */
	    prxy(" %3d,%-3d",
		 ship.shp_destx[1], ship.shp_desty[1], player->cnum);

	    /* Destination 2 */
	    if ((ship.shp_destx[1] != ship.shp_destx[0])
		|| (ship.shp_desty[1] != ship.shp_desty[0])) {
		prxy(" %3d,%-3d",
		     ship.shp_destx[0], ship.shp_desty[0], player->cnum);
	    } else
		pr("        ");

	    if (ship.shp_autonav & AN_STANDBY)
		pr(" suspended");
	    else if (ship.shp_autonav & AN_LOADING)
		pr(" loading");
	    else {
		/* ETA calculation */

		c = BestShipPath(buf, ship.shp_x, ship.shp_y,
				 ship.shp_destx[0], ship.shp_desty[0],
				 ship.shp_own);
		if (!c)
		    pr(" no route possible");
		else if (*c == 'h')
		    pr(" has arrived");
		else {
		    /* distance to destination */
		    eta_calc(&ship, c, &len, &updates);
		    pr(" %3d %4d", len, updates);
		}
	    }
	    if (ship.shp_autonav & AN_SCUTTLE)
		pr(" (scuttling)");
	    pr("\n");
	} else
	    pr(" has a sail path\n");

	if (ship.shp_name[0] != 0) {
	    if (player->god)
		pr("    ");
	    pr("       %s\n", ship.shp_name);
	}
    }
    if (!nships) {
	if (player->argp[1])
	    pr("%s: No ship(s)\n", player->argp[1]);
	else
	    pr("%s: No ship(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d ship%s\n", nships, splur(nships));
    return RET_OK;
}
