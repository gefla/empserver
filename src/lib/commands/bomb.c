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
 *  bomb.c: Fly bombing missions
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "damage.h"
#include "item.h"
#include "land.h"
#include "news.h"
#include "optlist.h"
#include "path.h"
#include "plane.h"
#include "retreat.h"
#include "ship.h"

static void pin_bomb(struct emp_qelem *list, struct sctstr *target);
static void strat_bomb(struct emp_qelem *list, struct sctstr *target);
static void comm_bomb(struct emp_qelem *list, struct sctstr *target);
static void eff_bomb(struct emp_qelem *list, struct sctstr *target);
static int pinflak_planedamage(struct plnstr *pp, struct plchrstr *pcp,
			       natid from, int flak);
static void plane_bomb(struct emp_qelem *list, struct sctstr *target);
static void land_bomb(struct emp_qelem *list, struct sctstr *target);
static void ship_bomb(struct emp_qelem *list, struct sctstr *target);

static i_type bombcomm[] = {
    I_CIVIL,
    I_MILIT,
    I_SHELL,
    I_GUN,
    I_PETROL,
    I_IRON,
    I_DUST,
    I_BAR,
    I_FOOD,
    I_OIL,
    I_LCM,
    I_HCM,
    I_UW,
    I_RAD
};
static int nbomb = sizeof(bombcomm) / sizeof(*bombcomm);

int
bomb(void)
{
    char *p;
    int mission_flags;
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    struct ichrstr *ip;
    char flightpath[MAX_PATH_LEN];
    struct nstr_item ni_bomb;
    struct nstr_item ni_esc;
    struct sctstr target;
    struct emp_qelem bomb_list;
    struct emp_qelem esc_list;
    int wantflags;
    struct sctstr ap_sect;
    char mission;
    int rel;
    struct natstr *natp;
    char buf[1024];

    wantflags = 0;
    if (!snxtitem(&ni_bomb, EF_PLANE, player->argp[1]))
	return RET_SYN;
    if (!snxtitem(&ni_esc, EF_PLANE,
		  getstarg(player->argp[2], "escort(s)? ", buf)))
	pr("No escorts...\n");
    p = getstarg(player->argp[3], "pinpoint, or strategic? ", buf);
    if (!p)
	return RET_SYN;
    mission = *p;
    if (strchr("ps", mission) == 0)
	return RET_SYN;
    if (!get_assembly_point(player->argp[4], &ap_sect, buf))
	return RET_SYN;
    ax = ap_sect.sct_x;
    ay = ap_sect.sct_y;
    if (getpath(flightpath, player->argp[5], ax, ay, 0, 0, P_FLYING) == 0
	|| *flightpath == 0)
	return RET_SYN;
    tx = ax;
    ty = ay;
    (void)pathtoxy(flightpath, &tx, &ty, fcost);
    pr("target sector is %s\n", xyas(tx, ty, player->cnum));
    getsect(tx, ty, &target);
    ip = 0;
    ap_to_target = strlen(flightpath);
    if (flightpath[ap_to_target - 1] == 'h')
	ap_to_target--;
    pr("range to target is %d\n", ap_to_target);
    /*
     * select planes within range
     */
    mission_flags = 0;
    pln_sel(&ni_bomb, &bomb_list, &ap_sect, ap_to_target,
	    2, wantflags, P_M | P_O);
    pln_sel(&ni_esc, &esc_list, &ap_sect, ap_to_target,
	    2, wantflags | P_F | P_ESC, P_M | P_O);
    /*
     * now arm and equip the bombers, transports, whatever.
     */
    mission_flags |= P_X;	/* stealth (shhh) */
    mission_flags |= P_H;	/* gets turned off if not all choppers */
    mission_flags = pln_arm(&bomb_list, 2 * ap_to_target, mission,
			    ip, 0, mission_flags);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    mission_flags = pln_arm(&esc_list, 2 * ap_to_target, mission,
			    ip, P_F | P_ESC, mission_flags);
    ac_encounter(&bomb_list, &esc_list, ax, ay,
		 flightpath, mission_flags, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
    } else if (target.sct_type == SCT_SANCT) {
	pr("You can't bomb that sector!\n");
    } else {
	switch (mission) {
	case 'p':
	    pin_bomb(&bomb_list, &target);
	    break;
	case 's':
	    if (opt_SLOW_WAR) {
		natp = getnatp(player->cnum);
		if (target.sct_own) {
		    rel = getrel(natp, target.sct_own);
		    if ((rel != AT_WAR) && (player->cnum != target.sct_own)
			&& (target.sct_own) &&
			(target.sct_oldown != player->cnum)) {
			pr("You're not at war with them!\n");
			pln_put(&bomb_list);
			pln_put(&esc_list);
			return RET_FAIL;
		    }
		}
	    }
	    nreport(player->cnum, N_SCT_BOMB, target.sct_own, 1);
	    strat_bomb(&bomb_list, &target);
	    break;
	default:
	    CANT_REACH();
	}
    }
    pln_put(&bomb_list);
    pln_put(&esc_list);
    return RET_OK;
}

static void
pin_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct dchrstr *dcp;
    int nplanes;
    int nships;
    int type;
    int bad;
    char *p;
    struct plist *plp;
    int nsubs;
    int nunits;
    struct natstr *natp;
    int rel;
    char buf[1024];
    int i;

    bad = 0;
    type = target->sct_type;
    dcp = &dchr[type];
    pr("Target sector is a %s constructed %s\n",
       effadv((int)target->sct_effic), dcp->d_name);
    nsubs = 0;
    plp = (struct plist *)list->q_forw;
    nships = shipsatxy(target->sct_x, target->sct_y, 0, M_SUB, 0);
    if (plp->pcp->pl_flags & P_A) {
	nsubs = shipsatxy(target->sct_x, target->sct_y, M_SUB, 0, 1);
	if (nsubs > 0)
	    pr("Some subs are present in the sector.\n");
    }
    nplanes = planesatxy(target->sct_x, target->sct_y, 0, 0);
    nunits = unitsatxy(target->sct_x, target->sct_y, 0, 0);
  retry:
    p = getstring("Bomb what? (ship, plane, land unit, efficiency, commodities) ",
		  buf);
    if (p == 0 || *p == 0) {
	if (player->aborted)
	    return;
	bad++;
	if (bad > 2)
	    return;
	goto retry;
    }
    switch (*p) {
    case 'l':
	if (opt_SLOW_WAR) {
	    natp = getnatp(player->cnum);
	    if (target->sct_own) {
		rel = getrel(natp, target->sct_own);
		if ((rel != AT_WAR) && (player->cnum != target->sct_own)
		    && (target->sct_own) &&
		    (target->sct_oldown != player->cnum)) {
		    pr("You're not at war with them!\n");
		    goto retry;
		}
	    }
	}
	if (nunits == 0) {
	    pr("no units there\n");
	    goto retry;
	}
	land_bomb(list, target);
	break;
    case 'p':
	if (opt_SLOW_WAR) {
	    natp = getnatp(player->cnum);
	    if (target->sct_own) {
		rel = getrel(natp, target->sct_own);
		if ((rel != AT_WAR) && (player->cnum != target->sct_own)
		    && (target->sct_own) &&
		    (target->sct_oldown != player->cnum)) {
		    pr("You're not at war with them!\n");
		    goto retry;
		}
	    }
	}
	if (nplanes == 0) {
	    pr("no planes there\n");
	    goto retry;
	}
	plane_bomb(list, target);
	break;
    case 's':
	if (nships == 0) {
	    if (((struct plist *)list->q_forw)->pcp->pl_flags & P_A) {
		if (nsubs == 0) {
		    pr("no ships there\n");
		    goto retry;
		}
	    } else {
		pr("no ships there\n");
		goto retry;
	    }
	}
	ship_bomb(list, target);
	break;
    case 'c':
	if (opt_SLOW_WAR) {
	    natp = getnatp(player->cnum);
	    if (target->sct_own) {
		rel = getrel(natp, target->sct_own);
		if ((rel != AT_WAR) && (player->cnum != target->sct_own)
		    && (target->sct_own) &&
		    (target->sct_oldown != player->cnum)) {
		    pr("You're not at war with them!\n");
		    goto retry;
		}
	    }
	}

	for (i = 0; i < nbomb; i++) {
	    if (!target->sct_item[bombcomm[i]])
		continue;
	    break;
	}
	if (i >= nbomb) {
	    pr("No bombable commodities in %s\n",
	       xyas(target->sct_x, target->sct_y, player->cnum));
	    goto retry;
	}
	comm_bomb(list, target);
	break;
    case 'e':
	if (opt_SLOW_WAR) {
	    natp = getnatp(player->cnum);
	    if (target->sct_own) {
		rel = getrel(natp, target->sct_own);
		if ((rel != AT_WAR) && (player->cnum != target->sct_own)
		    && (target->sct_own) &&
		    (target->sct_oldown != player->cnum)) {
		    pr("You're not at war with them!\n");
		    goto retry;
		}
	    }
	}
	eff_bomb(list, target);
	break;
    case 'q':
	pr("Aborting mission.\n");
	return;
    default:
	pr("Bad target type.\n");
	goto retry;
    }
}

static void
eff_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    struct emp_qelem *qp;
    struct sctstr sect;
    int oldeff, dam = 0;
    int nukedam;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	if (plp->bombs || plp->plane.pln_nuketype != -1)
	    dam += pln_damage(&plp->plane, target->sct_x, target->sct_y,
			      'p', &nukedam, 1);
    }
    if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	return;
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    oldeff = target->sct_effic;
    target->sct_effic = effdamage(target->sct_effic, dam);
    target->sct_avail = effdamage(target->sct_avail, dam);
    target->sct_road = effdamage(target->sct_road, dam);
    target->sct_rail = effdamage(target->sct_rail, dam);
    target->sct_defense = effdamage(target->sct_defense, dam);
    pr("did %d%% damage to efficiency in %s\n",
       oldeff - target->sct_effic,
       xyas(target->sct_x, target->sct_y, player->cnum));
    if (target->sct_own)
	wu(0, target->sct_own,
	   "%s bombing raid did %d%% damage in %s\n",
	   cname(player->cnum), oldeff - target->sct_effic,
	   xyas(target->sct_x, target->sct_y, target->sct_own));
    bridge_damaged(target);
    putsect(&sect);
    collateral_damage(target->sct_x, target->sct_y, dam);
}

static void
comm_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    double b;
    int i;
    int amt, before;
    struct ichrstr *ip;
    struct emp_qelem *qp;
    struct sctstr sect;
    int dam = 0;
    int nukedam;

    for (i = 0; i < nbomb; i++) {
	if (target->sct_item[bombcomm[i]] == 0)
	    continue;
	if (opt_SUPER_BARS && bombcomm[i] == I_BAR)
	    continue;
	ip = &ichr[bombcomm[i]];
	pr("some %s\n", ip->i_name);
    }
    for (;;) {
	ip = whatitem(NULL, "commodity to bomb? ");
	if (player->aborted)
	    return;
	if (!ip)
	    continue;

	for (i = 0; i < nbomb; i++) {
	    if (opt_SUPER_BARS && bombcomm[i] == I_BAR)
		continue;
	    if (&ichr[bombcomm[i]] == ip)
		break;
	}
	if (i == nbomb) {
	    pr("You can't bomb %s!\n", ip->i_name);
	    for (i = 0; i < nbomb; i++) {
		if (opt_SUPER_BARS && bombcomm[i] == I_BAR)
		    continue;
		pr("%s%s", i == 0 ? "Bombable: " : ", ",
		   ichr[bombcomm[i]].i_name);
	    }
	    pr("\n");
	} else
	    break;
    }
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	if (plp->bombs || plp->plane.pln_nuketype != -1)
	    dam += pln_damage(&plp->plane, target->sct_x, target->sct_y,
			      'p', &nukedam, 1);
    }
    if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	return;
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    before = target->sct_item[ip->i_uid];
    target->sct_item[ip->i_uid] = amt = commdamage(before, dam, ip->i_uid);
    if (before > 0.0)
	b = 100.0 * (1.0 - (double)amt / (double)before);
    else
	b = 0.0;
    pr("did %.2f%% damage to %s in %s\n",
       b, ip->i_name, xyas(target->sct_x, target->sct_y, player->cnum));
    nreport(player->cnum, N_SCT_BOMB, target->sct_own, 1);
    if (target->sct_own != 0)
	wu(0, target->sct_own,
	   "%s precision bombing raid did %.2f%% damage to %s in %s\n",
	   cname(player->cnum), b, ip->i_name,
	   xyas(target->sct_x, target->sct_y, target->sct_own));
    putsect(&sect);
    collateral_damage(target->sct_x, target->sct_y, dam);
}

static void
ship_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    struct mchrstr *mcp;
    int dam;
    char *q;
    int n;
    struct emp_qelem *qp;
    int shipno;
    int ignore;
    struct shpstr ship;
    int nships = 0;
    struct shiplist *head = NULL;
    char buf[1024];
    char prompt[128];
    int hitchance;
    int nukedam;
    int flak;
    int gun;
    int shell;

    for (qp = list->q_forw; qp != list && !player->aborted;
	 qp = qp->q_forw) {
	free_shiplist(&head);
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	if (plp->pcp->pl_flags & P_A)
	    nships = asw_shipsatxy(target->sct_x, target->sct_y, 0, 0,
				   &plp->plane, &head);
	else
	    nships = shipsatxy(target->sct_x, target->sct_y, 0, M_SUB, 0);
	if (nships == 0) {
	    pr("%s could not find any ships!\n", prplane(&plp->plane));
	    continue;
	}
	(void)sprintf(prompt, "%s, %d bombs.  Target ('~' to skip)? ",
		      prplane(&plp->plane), plp->bombs);
	ignore = 0;
	shipno = -1;
	while (shipno < 0 && !player->aborted && !ignore) {
	    if ((q = getstring(prompt, buf)) == 0 || *q == 0)
		continue;
	    if (*q == '~') {
		ignore = 1;
		continue;
	    }
	    if (*q == '?') {
		if (plp->pcp->pl_flags & P_A)
		    print_shiplist(head);
		else
		    shipsatxy(target->sct_x, target->sct_y, 0, M_SUB, 0);
		continue;
	    }
	    if (*q == 'd')
		goto next;
	    if (!isdigit(*q))
		continue;
	    n = atoi(q);
	    if (n < 0)
		continue;
	    if ((plp->pcp->pl_flags & P_A) && !on_shiplist(n, head))
		continue;
	    if (getship(n, &ship) && ship.shp_x == target->sct_x &&
		ship.shp_y == target->sct_y)
		shipno = n;
	}
	if (shipno < 0)
	    continue;
	if ((plp->pcp->pl_flags & P_A) && !on_shiplist(shipno, head))
	    continue;

	shell = 0;
	gun = shp_usable_guns(&ship);
	if (gun > 0) {
	    shell = ship.shp_item[I_SHELL];
	    if (shell <= 0)
		shell = supply_commod(ship.shp_own,
				      ship.shp_x, ship.shp_y, I_SHELL, 1);
	}
	mcp = &mchr[(int)ship.shp_type];
	if (gun > 0 && shell > 0 && !(mcp->m_flags & M_SUB)) {
	    flak = (int)(techfact(ship.shp_tech, gun) * 2.0);
	    ship.shp_item[I_SHELL] = shell;
	    putship(ship.shp_uid, &ship);
	    PR(ship.shp_own, "Flak! Firing %d guns from ship %s\n",
	       flak, prship(&ship));
	    if (pinflak_planedamage(&plp->plane, plp->pcp, ship.shp_own, flak))
		continue;
	}

	dam = 0;
	if (plp->plane.pln_nuketype != -1)
	    hitchance = 100;
	else {
	    hitchance = pln_hitchance(&plp->plane,
				      shp_hardtarget(&ship), EF_SHIP);
	    pr("%d%% hitchance...", hitchance);
	}
	if (roll(100) <= hitchance) {
	    /* pinbombing is more accurate than normal bombing */
	    dam = 2 * pln_damage(&plp->plane, target->sct_x, target->sct_y,
				 'p', &nukedam, 1);
	} else {
	    pr("splash\n");
	    /* Bombs that miss have to land somewhere! */
	    dam = pln_damage(&plp->plane, target->sct_x, target->sct_y,
			     'p', &nukedam, 0);
	    collateral_damage(target->sct_x, target->sct_y, dam);
	    dam = 0;
	}
	if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	    continue;
	if (mcp->m_flags & M_SUB)
	    nreport(player->cnum, N_SUB_BOMB, ship.shp_own, 1);
	else
	    nreport(player->cnum, N_SHP_BOMB, ship.shp_own, 1);
	if (ship.shp_own) {
	    wu(0, ship.shp_own, "%s bombs did %d damage to %s at %s\n",
	       cname(player->cnum), dam,
	       prship(&ship),
	       xyas(target->sct_x, target->sct_y, ship.shp_own));
	}
	pr("\n");
	check_retreat_and_do_shipdamage(&ship, dam);
	if (ship.shp_rflags & RET_BOMBED)
	    if (((ship.shp_rflags & RET_INJURED) == 0) || !dam)
		retreat_ship(&ship, 'b');
	putship(ship.shp_uid, &ship);
	getship(ship.shp_uid, &ship);
	if (!ship.shp_own) {
	    pr("%s at %s sunk!\n",
	       prship(&ship),
	       xyas(target->sct_x, target->sct_y, player->cnum));
	}
	collateral_damage(target->sct_x, target->sct_y, dam / 2);
      next:
	;
    }
    free_shiplist(&head);
}

static void
plane_bomb(struct emp_qelem *list, struct sctstr *target)
{
    int dam;
    char *q;
    int n;
    natid own;
    struct plnstr plane;
    struct emp_qelem *qp;
    int planeno;
    int ignore;
    struct plist *plp;
    char prompt[128];
    char buf[1024];
    int hitchance;
    int nukedam;
    int nplanes;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	nplanes = planesatxy(target->sct_x, target->sct_y, 0, 0);
	if (nplanes == 0) {
	    pr("%s could not find any planes!\n", prplane(&plp->plane));
	    continue;
	}
	(void)sprintf(prompt, "%s, %d bombs.  Target ('~' to skip)? ",
		      prplane(&plp->plane), plp->bombs);
	planeno = -1;
	ignore = 0;
	while (planeno < 0 && !player->aborted && !ignore) {
	    if ((q = getstring(prompt, buf)) == 0 || *q == 0)
		continue;
	    if (*q == '~') {
		ignore = 1;
		continue;
	    }
	    if (*q == '?') {
		planesatxy(target->sct_x, target->sct_y, 0, 0);
		continue;
	    }
	    if (*q == 'd')
		goto next;
	    n = atoi(q);
	    if (n < 0)
		continue;
	    if (getplane(n, &plane) &&
		plane.pln_x == target->sct_x &&
		plane.pln_y == target->sct_y &&
		!(plane.pln_flags & PLN_LAUNCHED))
		planeno = n;
	    else
		pr("Plane #%d not spotted\n", n);
	}
	if (planeno < 0)
	    continue;
	dam = 0;
	if (plp->plane.pln_nuketype != -1)
	    hitchance = 100;
	else {
	    hitchance = pln_hitchance(&plp->plane, 0, EF_PLANE);
	    pr("%d%% hitchance...", hitchance);
	}
	if (roll(100) <= hitchance) {
	    /* pinbombing is more accurate than normal bombing */
	    dam = 2 * pln_damage(&plp->plane, target->sct_x, target->sct_y,
				 'p', &nukedam, 1);
	} else {
	    pr("thud\n");
	    /* Bombs that miss have to land somewhere! */
	    dam = pln_damage(&plp->plane, target->sct_x, target->sct_y,
			     'p', &nukedam, 0);
	    collateral_damage(target->sct_x, target->sct_y, dam);
	    dam = 0;
	}
	if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	    continue;
	if (dam > 100)
	    dam = 100;
	own = plane.pln_own;
	if (dam > plane.pln_effic)
	    plane.pln_effic = 0;
	else
	    plane.pln_effic -= dam;
	plane.pln_mobil = (dam * plane.pln_mobil / 100.0);
	if (own == player->cnum) {
	    pr("%s reports %d%% damage\n", prplane(&plane), dam);
	} else {
	    if (own != 0)
		wu(0, own,
		   "%s pinpoint bombing raid did %d%% damage to %s\n",
		   cname(player->cnum), dam, prplane(&plane));
	}
	nreport(player->cnum, N_DOWN_PLANE, own, 1);
	if (own != 0)
	    wu(0, own, "%s bombs did %d%% damage to %s at %s\n",
	       cname(player->cnum), dam, prplane(&plane),
	       xyas(target->sct_x, target->sct_y, own));
	putplane(plane.pln_uid, &plane);
	collateral_damage(target->sct_x, target->sct_y, dam);
      next:
	;
    }
}

static void
land_bomb(struct emp_qelem *list, struct sctstr *target)
{
    int dam;
    char *q;
    int n;
    natid own;
    char prompt[128];
    char buf[1024];
    struct lndstr land;
    struct emp_qelem *qp;
    int unitno;
    int ignore, aaf, flak, hitchance;
    struct plist *plp;
    int nukedam;
    int nunits;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	nunits = unitsatxy(target->sct_x, target->sct_y, 0, 0);
	if (nunits == 0) {
	    pr("%s could not find any units!\n", prplane(&plp->plane));
	    continue;
	}
	(void)sprintf(prompt, "%s, %d bombs.  Target ('~' to skip)? ",
		      prplane(&plp->plane), plp->bombs);
	unitno = -1;
	ignore = 0;
	while (unitno < 0 && !player->aborted && !ignore) {
	    if ((q = getstring(prompt, buf)) == 0 || *q == 0)
		continue;
	    if (*q == '~') {
		ignore = 1;
		continue;
	    }
	    if (*q == '?') {
		unitsatxy(target->sct_x, target->sct_y, 0, 0);
		continue;
	    }
	    if (*q == 'd')
		goto next;
	    n = atoi(q);
	    if (n < 0)
		continue;
	    if (getland(n, &land) &&
		land.lnd_x == target->sct_x && land.lnd_y == target->sct_y)
		unitno = n;
	    else
		pr("Unit #%d not spotted\n", n);
	}
	if (unitno < 0)
	    continue;

	aaf = lnd_aaf(&land);
	flak = (int)(techfact(land.lnd_tech, aaf) * 3.0);
	if (flak) {
	    PR(land.lnd_own,
	       "Flak! Firing flak guns from unit %s (aa rating %d)\n",
	       prland(&land), aaf);
	    if (pinflak_planedamage(&plp->plane, plp->pcp, land.lnd_own, flak))
		continue;
	}

	dam = 0;
	if (plp->plane.pln_nuketype != -1)
	    hitchance = 100;
	else {
	    hitchance = pln_hitchance(&plp->plane,
				      lnd_hardtarget(&land), EF_LAND);
	    pr("%d%% hitchance...", hitchance);
	}
	if (roll(100) <= hitchance) {
	    dam = 2 * pln_damage(&plp->plane, target->sct_x, target->sct_y,
				 'p', &nukedam, 1);
	} else {
	    pr("thud\n");
	    /* Bombs that miss have to land somewhere! */
	    dam = pln_damage(&plp->plane, target->sct_x, target->sct_y,
			     'p', &nukedam, 0);
	    collateral_damage(target->sct_x, target->sct_y, dam);
	    dam = 0;
	}
	if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	    continue;
	if (dam > 100)
	    dam = 100;
	own = land.lnd_own;
	mpr(own, "%s pinpoint bombing raid did %d damage to %s\n",
	    cname(player->cnum), dam, prland(&land));
	check_retreat_and_do_landdamage(&land, dam);

	if (land.lnd_rflags & RET_BOMBED)
	    if (((land.lnd_rflags & RET_INJURED) == 0) || !dam)
		retreat_land(&land, 'b');
	nreport(player->cnum, N_UNIT_BOMB, own, 1);
	putland(land.lnd_uid, &land);
	collateral_damage(target->sct_x, target->sct_y, dam);
      next:
	;
    }
}

static void
strat_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    int dam = 0;
    struct emp_qelem *qp;
    struct sctstr sect;
    int nukedam;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	if (plp->bombs || plp->plane.pln_nuketype != -1)
	    dam += pln_damage(&plp->plane, target->sct_x, target->sct_y,
			      's', &nukedam, 1);
    }
    if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	return;
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    if (target->sct_own)
	wu(0, target->sct_own, "%s bombing raid did %d damage in %s\n",
	   cname(player->cnum), PERCENT_DAMAGE(dam),
	   xyas(target->sct_x, target->sct_y, target->sct_own));

    sectdamage(target, dam);

    pr("did %d damage in %s\n", PERCENT_DAMAGE(dam),
       xyas(target->sct_x, target->sct_y, player->cnum));
    putsect(&sect);
}

static int
pinflak_planedamage(struct plnstr *pp, struct plchrstr *pcp, natid from,
		    int flak)
{
    int disp;
    char dmess[255];
    int eff;
    natid plane_owner;
    int dam;

    dam = ac_flak_dam(flak, pln_def(pp), pcp->pl_flags);
    disp = 0;
    plane_owner = pp->pln_own;
    eff = pp->pln_effic;
    if (dam <= 0)
	return 0;
    memset(dmess, 0, sizeof(dmess));
    eff -= dam;
    if (eff < 0)
	eff = 0;
    if (eff < PLANE_MINEFF) {
	sprintf(dmess, " -- shot down");
	disp = 1;
    } else if (chance((100 - eff) / 100.0)) {
	sprintf(dmess, " -- aborted with %d%% damage", 100 - eff);
	disp = 2;
    }
    PR(plane_owner, "    Flak! %s %s takes %d%s.\n",
       cname(pp->pln_own), prplane(pp), dam, dmess);

    pp->pln_effic = eff;
    if (disp == 1) {
	if (from != 0)
	    nreport(from, N_DOWN_PLANE, pp->pln_own, 1);
    }
    putplane(pp->pln_uid, pp);

    if (disp > 0)
	return 1;
    return 0;
}
