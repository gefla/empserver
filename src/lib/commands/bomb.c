/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  bomb.c: Fly bombing missions
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include <ctype.h>
#include <string.h>
#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "item.h"
#include "plane.h"
#include "nuke.h"
#include "retreat.h"
#include "xy.h"
#include "nsc.h"
#include "news.h"
#include "file.h"
#include "nat.h"
#include "path.h"
#include "optlist.h"
#include "damage.h"
#include "commands.h"

static void pin_bomb(struct emp_qelem *list, struct sctstr *target);
static void strat_bomb(struct emp_qelem *list, struct sctstr *target);
static void comm_bomb(struct emp_qelem *list, struct sctstr *target);
static void eff_bomb(struct emp_qelem *list, struct sctstr *target);
static int pinflak_planedamage(struct plnstr *pp, struct plchrstr *pcp,
			       natid from, int flak);
static void plane_bomb(struct emp_qelem *list, struct sctstr *target);
static void land_bomb(struct emp_qelem *list, struct sctstr *target);
static void ship_bomb(struct emp_qelem *list, struct sctstr *target);

static int bombcomm[] = {
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
static int nbomb = sizeof(bombcomm) / sizeof(int);

int
bomb(void)
{
    s_char *p;
    int mission_flags;
    int tech;
    coord tx, ty;
    coord ax, ay;
    int ap_to_target;
    struct ichrstr *ip;
    s_char flightpath[MAX_PATH_LEN];
    struct nstr_item ni_bomb;
    struct nstr_item ni_esc;
    coord x, y;
    struct sctstr target;
    struct emp_qelem bomb_list;
    struct emp_qelem esc_list;
    int wantflags;
    struct sctstr ap_sect;
    s_char mission;
    int rel;
    struct natstr *natp;
    s_char buf[1024];

    wantflags = 0;
    if (!snxtitem(&ni_bomb, EF_PLANE, player->argp[1]))
	return RET_SYN;
    if (!snxtitem
	(&ni_esc, EF_PLANE, getstarg(player->argp[2], "escort(s)? ", buf)))
	pr("No escorts...\n");
    if ((p =
	 getstarg(player->argp[3], "pinpoint, or strategic? ", buf)) == 0)
	return RET_SYN;
    mission = *p;
    if (strchr("ps", mission) == 0)
	return RET_SYN;
    if ((p = getstarg(player->argp[4], "assembly point? ", buf)) == 0
	|| *p == 0)
	return RET_SYN;
    if (!sarg_xy(p, &x, &y) || !getsect(x, y, &ap_sect))
	return RET_SYN;
    if (ap_sect.sct_own && ap_sect.sct_own != player->cnum &&
	getrel(getnatp(ap_sect.sct_own), player->cnum) != ALLIED) {
	pr("Assembly point not owned by you or an ally!\n");
	return RET_SYN;
    }
    ax = x;
    ay = y;
    if (getpath(flightpath, player->argp[5], ax, ay, 0, 0,
		0, P_FLYING) == 0 || *flightpath == 0)
	return RET_SYN;
    tx = ax;
    ty = ay;
    (void)pathtoxy(flightpath, &tx, &ty, fcost);
    pr("target sector is %s\n", xyas(tx, ty, player->cnum));
    getsect(tx, ty, &target);
    if (target.sct_type == SCT_SANCT) {
	pr("You can't bomb that sector!\n");
	return RET_SYN;
    }
    ip = 0;
    ap_to_target = strlen(flightpath);
    if (*(flightpath + strlen(flightpath) - 1) == 'h')
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
     * tech is stored in high 16 bits of mission_flags.
     * yuck.
     */
    tech = 0;
    mission_flags |= P_X;	/* stealth (shhh) */
    mission_flags |= P_H;	/* gets turned off if not all choppers */
    mission_flags =
	pln_arm(&bomb_list, 2 * ap_to_target, mission, ip, 0,
		mission_flags, &tech);
    if (QEMPTY(&bomb_list)) {
	pr("No planes could be equipped for the mission.\n");
	return RET_FAIL;
    }
    mission_flags =
	pln_arm(&esc_list, 2 * ap_to_target, mission, ip, P_F | P_ESC,
		mission_flags, &tech);
    ac_encounter(&bomb_list, &esc_list, ax, ay, flightpath, mission_flags,
		 0, 0, 0);
    if (QEMPTY(&bomb_list)) {
	pr("No planes got through fighter defenses\n");
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
	    pr("Bad mission %c\n", mission);
	    break;
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
    s_char *p;
    int vec[I_MAX + 1];
    struct plist *plp;
    struct emp_qelem *qp;
    int bestacc;
    int nsubs;
    int nunits;
    struct natstr *natp;
    int rel;
    s_char buf[1024];
    int i;

    bad = 0;
    type = target->sct_type;
    dcp = &dchr[type];
    pr("Target sector is a %s constructed %s\n",
       effadv((int)target->sct_effic), dcp->d_name);
    nsubs = 0;
    plp = (struct plist *)list->q_forw;
    if (plp->pcp->pl_flags & P_A) {
	bestacc = 0;
	for (qp = list->q_forw; qp != list; qp = qp->q_forw)
	    plp = (struct plist *)qp;
	if (plp->plane.pln_acc < bestacc)
	    bestacc = plp->plane.pln_acc;
	nships = num_shipsatxy(target->sct_x, target->sct_y, 0, 0);
	nsubs = nships - shipsatxy(target->sct_x, target->sct_y, 0, M_SUB);
	if (nsubs > 0)
	    pr("Some subs are present in the sector.\n");
    } else {
	nships = shipsatxy(target->sct_x, target->sct_y, 0, M_SUB);
    }
    nplanes = planesatxy(target->sct_x, target->sct_y, 0, 0, list);
    nunits = unitsatxy(target->sct_x, target->sct_y, 0, 0);
    getvec(VT_ITEM, vec, (s_char *)target, EF_SECTOR);
  retry:
    p = getstring
	("Bomb what? (ship, plane, land unit, efficiency, commodities) ",
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
	    if (!vec[bombcomm[i]])
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
    register struct plist *plp;
    struct emp_qelem *qp;
    struct sctstr sect;
    int oldeff, dam = 0;
    int nukedam;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	if (plp->bombs || plp->plane.pln_nuketype != -1)
	    dam +=
		pln_damage(&plp->plane, target->sct_x, target->sct_y, 'p',
			   &nukedam, 1);
    }
    if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	return;
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    oldeff = target->sct_effic;
    target->sct_effic = effdamage(target->sct_effic, dam);
    target->sct_road = effdamage(target->sct_road, dam);
    target->sct_rail = effdamage(target->sct_rail, dam);
    target->sct_defense = effdamage(target->sct_defense, dam);
    if (!opt_DEFENSE_INFRA)
	target->sct_defense = target->sct_effic;
    pr("did %d%% damage to efficiency in %s\n",
       oldeff - target->sct_effic,
       xyas(target->sct_x, target->sct_y, player->cnum));
    if (target->sct_own)
	wu(0, target->sct_own,
	   "%s bombing raid did %d%% damage in %s\n",
	   cname(player->cnum), oldeff - target->sct_effic,
	   xyas(target->sct_x, target->sct_y, target->sct_own));
    if (target->sct_effic < 20) {
	if (target->sct_type == SCT_BSPAN)
	    knockdown(target, list);
	else if (target->sct_type == SCT_BTOWER) {
	    knockdown(target, list);
	    bridgefall(target, list);
	}
    }
    putsect(&sect);
    collateral_damage(target->sct_x, target->sct_y, dam, list);
}

static void
comm_bomb(struct emp_qelem *list, struct sctstr *target)
{
    register struct plist *plp;
    float b;
    int i;
    int amt, before;
    struct ichrstr *ip;
    int vec[I_MAX + 1];
    struct emp_qelem *qp;
    struct sctstr sect;
    int dam = 0;
    int nukedam;

    getvec(VT_ITEM, vec, (s_char *)target, EF_SECTOR);
    for (i = 0; i < nbomb; i++) {
	if (vec[bombcomm[i]] == 0)
	    continue;
	if (opt_SUPER_BARS && bombcomm[i] == I_BAR)
	    continue;
	ip = &ichr[bombcomm[i]];
	pr("some %s\n", ip->i_name);
    }
    for (;;) {
	ip = whatitem((s_char *)0, "commodity to bomb? ");
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
		pr(i == 0 ? "Bombable: " : ", ");
		pr(ichr[bombcomm[i]].i_name);
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
	    dam +=
		pln_damage(&plp->plane, target->sct_x, target->sct_y, 'p',
			   &nukedam, 1);
    }
    if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	return;
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    before = amt = getvar(ip->i_vtype, (s_char *)target, EF_SECTOR);
    putvar(ip->i_vtype, commdamage(amt, dam, ip->i_vtype),
	   (s_char *)target, EF_SECTOR);
    amt = getvar(ip->i_vtype, (s_char *)target, EF_SECTOR);
    if (before > 0.0)
	b = 100.0 * (1.0 - ((float)amt / (float)before));
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
    collateral_damage(target->sct_x, target->sct_y, dam, list);
}

static void
ship_bomb(struct emp_qelem *list, struct sctstr *target)
{
    struct plist *plp;
    int onsea;
    struct mchrstr *mcp;
    int dam;
    s_char *q;
    int n;
    struct emp_qelem *qp;
    int shipno;
    int ignore;
    struct shpstr ship;
    int nships = 0;
    struct shiplook head, *s, *s2;
    s_char buf[1024];
    s_char prompt[128];
    s_char msg[128];
    int hitchance;
    int nukedam;
    int flak;
    int gun;
    int shell;

    memset(&head, 0, sizeof(struct shiplook));
    head.uid = -1;
    onsea = (target->sct_type == SCT_WATER) ? 1 : 0;
    for (qp = list->q_forw; qp != list && !player->aborted;
	 qp = qp->q_forw) {
	if (head.uid != -1) {
	    s = head.next;
	    while (s != (struct shiplook *)0) {
		s2 = s;
		s = s->next;
		free(s2);
	    }
	}
	memset(&head, 0, sizeof(struct shiplook));
	head.uid = -1;
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	if (plp->pcp->pl_flags & P_A)
	    nships = asw_shipsatxy(target->sct_x, target->sct_y, 0, 0,
				   &plp->plane, &head);
	else
	    nships = shipsatxy(target->sct_x, target->sct_y, 0, M_SUB);
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
		    print_found(&head);
		else
		    shipsatxy(target->sct_x, target->sct_y, 0, M_SUB);
		continue;
	    }
	    if (*q == 'd')
		goto next;
	    if (!isdigit(*q))
		continue;
	    n = atoi(q);
	    if (n < 0)
		continue;
	    if (getship(n, &ship) && ship.shp_x == target->sct_x &&
		ship.shp_y == target->sct_y)
		shipno = n;
	}
	if (shipno < 0)
	    continue;

	shell = gun = 0;
	gun = min(getvar(V_GUN, (s_char *)&ship, EF_SHIP), ship.shp_glim);
	if (gun > 0) {
	    shell = getvar(V_SHELL, (s_char *)&ship, EF_SHIP);
	    if (shell <= 0)
		shell = supply_commod(ship.shp_own, ship.shp_x,
				      ship.shp_y, I_SHELL, 1);
	}
	mcp = &mchr[(int)ship.shp_type];
	if (gun > 0 && shell > 0 && !(mcp->m_flags & M_SUB)) {
	    flak = (int)(techfact(ship.shp_tech, (double)gun) * 2.0);
	    putvar(V_SHELL, shell, (s_char *)&ship, EF_SHIP);
	    putship(ship.shp_uid, &ship);
	    sprintf(msg, "Flak! Firing %d guns from ship %s\n",
		    flak, prship(&ship));
	    PR(ship.shp_own, msg);
	    if (pinflak_planedamage
		(&plp->plane, plp->pcp, ship.shp_own, flak))
		continue;
	}

	dam = 0;
	if (plp->plane.pln_nuketype != -1)
	    hitchance = 100;
	else {
	    hitchance =
		pln_hitchance(&plp->plane, shp_hardtarget(&ship), EF_SHIP);
	    pr("%d%% hitchance...", hitchance);
	}
	if (roll(100) <= hitchance) {
	    /* pinbombing is more accurate than normal bombing */
	    dam =
		2 * pln_damage(&plp->plane, target->sct_x, target->sct_y,
			       'p', &nukedam, 1);
	} else {
	    pr("splash\n");
	    /* Bombs that miss have to land somewhere! */
	    dam =
		pln_damage(&plp->plane, target->sct_x, target->sct_y, 'p',
			   &nukedam, 0);
	    collateral_damage(target->sct_x, target->sct_y, dam, list);
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
	getship(n, &ship);
	if (!ship.shp_own) {
	    pr("%s at %s sunk!\n",
	       prship(&ship),
	       xyas(target->sct_x, target->sct_y, player->cnum));
	}
	collateral_damage(target->sct_x, target->sct_y, dam / 2, list);
      next:
	;
    }
    s = head.next;
    while (s != (struct shiplook *)0) {
	s2 = s;
	s = s->next;
	free(s2);
    }
}

static void
plane_bomb(struct emp_qelem *list, struct sctstr *target)
{
    int dam;
    s_char *q;
    int n;
    natid own;
    struct plnstr plane;
    struct emp_qelem *qp;
    int planeno;
    int ignore;
    struct plist *plp;
    s_char prompt[128];
    s_char buf[1024];
    int hitchance;
    int nukedam;
    int nplanes;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	nplanes = planesatxy(target->sct_x, target->sct_y, 0, 0, list);
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
		planesatxy(target->sct_x, target->sct_y, 0, 0, list);
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
		((plane.pln_flags & PLN_LAUNCHED) == 0) &&
		(!ac_isflying(&plane, list)))
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
	    dam =
		2 * pln_damage(&plp->plane, target->sct_x, target->sct_y,
			       'p', &nukedam, 1);
	} else {
	    pr("thud\n");
	    /* Bombs that miss have to land somewhere! */
	    dam =
		pln_damage(&plp->plane, target->sct_x, target->sct_y, 'p',
			   &nukedam, 0);
	    collateral_damage(target->sct_x, target->sct_y, dam, list);
	    dam = 0;
	}
	if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	    continue;
	if (dam > 100)
	    dam = 100;
	own = plane.pln_own;
	if (dam > plane.pln_effic) {
	    plane.pln_effic = 0;
	    makelost(EF_PLANE, plane.pln_own, plane.pln_uid, plane.pln_x,
		     plane.pln_y);
	    plane.pln_own = 0;
	} else
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
	collateral_damage(plane.pln_x, plane.pln_y, dam, list);
      next:
	;
    }
}

static void
land_bomb(struct emp_qelem *list, struct sctstr *target)
{
    int dam;
    s_char *q;
    int n;
    natid own;
    s_char prompt[128];
    s_char buf[1024];
    s_char msg[128];
    struct lndstr land;
    struct lchrstr *lcp;
    struct emp_qelem *qp;
    int unitno;
    int ignore, flak, hitchance;
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

	lcp = &lchr[(int)land.lnd_type];

	flak = (int)(techfact(land.lnd_tech, (double)land.lnd_aaf) * 3.0);
	if (flak) {
	    sprintf(msg,
		    "Flak! Firing flak guns from unit %s (aa rating %d)\n",
		    prland(&land), land.lnd_aaf);
	    PR(land.lnd_own, msg);
	    if (pinflak_planedamage
		(&plp->plane, plp->pcp, land.lnd_own, flak))
		continue;
	}

	dam = 0;
	if (plp->plane.pln_nuketype != -1)
	    hitchance = 100;
	else {
	    hitchance =
		pln_hitchance(&plp->plane, lnd_hardtarget(&land), EF_LAND);
	    pr("%d%% hitchance...", hitchance);
	}
	if (roll(100) <= hitchance) {
	    dam =
		2 * pln_damage(&plp->plane, target->sct_x, target->sct_y,
			       'p', &nukedam, 1);
	} else {
	    pr("thud\n");
	    /* Bombs that miss have to land somewhere! */
	    dam =
		pln_damage(&plp->plane, target->sct_x, target->sct_y, 'p',
			   &nukedam, 0);
	    collateral_damage(target->sct_x, target->sct_y, dam, list);
	    dam = 0;
	}
	if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	    continue;
	if (dam > 100)
	    dam = 100;
	own = land.lnd_own;
	if (own != 0)
	    mpr(own,
		"%s pinpoint bombing raid did %d damage to %s\n",
		cname(player->cnum), dam, prland(&land));
	check_retreat_and_do_landdamage(&land, dam);

	if (land.lnd_rflags & RET_BOMBED)
	    if (((land.lnd_rflags & RET_INJURED) == 0) || !dam)
		retreat_land(&land, 'b');
	nreport(player->cnum, N_UNIT_BOMB, own, 1);
	putland(land.lnd_uid, &land);
	collateral_damage(land.lnd_x, land.lnd_y, dam, list);
      next:
	;
    }
}

static void
strat_bomb(struct emp_qelem *list, struct sctstr *target)
{
    register struct plist *plp;
    int dam = 0;
    struct emp_qelem *qp;
    struct sctstr sect;
    int nukedam;

    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	plp = (struct plist *)qp;
	if ((plp->pcp->pl_flags & P_C) && (!(plp->pcp->pl_flags & P_T)))
	    continue;
	if (plp->bombs || plp->plane.pln_nuketype != -1)
	    dam +=
		pln_damage(&plp->plane, target->sct_x, target->sct_y, 's',
			   &nukedam, 1);
    }
    if (dam <= 0)		/* dam == 0 if only nukes were delivered */
	return;
    getsect(target->sct_x, target->sct_y, &sect);
    target = &sect;
    if (target->sct_own)
	wu(0, target->sct_own, "%s bombing raid did %d damage in %s\n",
	   cname(player->cnum), PERCENT_DAMAGE(dam),
	   xyas(target->sct_x, target->sct_y, target->sct_own));

    sectdamage(target, dam, list);

    pr("did %d damage in %s\n", PERCENT_DAMAGE(dam),
       xyas(target->sct_x, target->sct_y, player->cnum));
    putsect(&sect);
}

#define FLAK_MAX 15
float lflaktable[16] = { 0.20, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50,
    0.50, 0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85,
};

static int
pinflak_planedamage(struct plnstr *pp, struct plchrstr *pcp, natid from,
		    int flak)
{
    extern double flakscale;
    int disp;
    s_char dmess[255];
    int eff;
    s_char mesg[128];
    struct shpstr ship;
    struct lndstr land;
    natid plane_owner;
    int dam;
    float mult;

    flak -= (pp->pln_def + 1);
    if (pcp->pl_flags & P_X)
	flak -= 2;
    if (pcp->pl_flags & P_H)
	flak -= 1;
    if (flak > 8)
	mult = lflaktable[FLAK_MAX] * 1.33;
    else if (flak < -7)
	mult = lflaktable[0] * 0.66;
    else {
	flak += 7;
	mult = lflaktable[flak];
    }
    mult *= flakscale;
    dam = (int)((roll(8) + 2) * mult);
    if (dam > 100)
	dam = 100;

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
	sprintf(dmess, " -- aborted with %d%%%% damage", 100 - eff);
	disp = 2;
    }
    sprintf(mesg, "    Flak! %s %s takes %d%s.\n",
	    cname(pp->pln_own), prplane(pp), dam, dmess);
    PR(plane_owner, mesg);

    pp->pln_effic = eff;
    if (disp == 1) {
	if (from != 0)
	    nreport(from, N_DOWN_PLANE, pp->pln_own, 1);
	if (pp->pln_ship >= 0) {
	    getship(pp->pln_ship, &ship);
	    take_plane_off_ship(pp, &ship);
	}
	if (pp->pln_land >= 0) {
	    getland(pp->pln_land, &land);
	    take_plane_off_land(pp, &land);
	}
	makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
	pp->pln_own = 0;
	putplane(pp->pln_uid, pp);
    } else
	putplane(pp->pln_uid, pp);

    if (disp > 0)
	return 1;
    return 0;
}
