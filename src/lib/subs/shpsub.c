/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shpsub.c: Ship subroutine stuff
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1996-2000
 */

#include <config.h>

#include <math.h>
#include "misc.h"
#include "queue.h"
#include "player.h"
#include "sect.h"
#include "ship.h"
#include "plane.h"
#include "land.h"
#include "news.h"
#include "item.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "path.h"
#include "mission.h"
#include "optlist.h"
#include "damage.h"
#include "server.h"
#include "prototypes.h"

static int shp_check_one_mines(struct mlist *);
static int shp_hit_mine(struct shpstr *, struct mchrstr *);
static void shp_mess(s_char *, struct mlist *);

void
shp_sel(struct nstr_item *ni, struct emp_qelem *list)


    /*  int     wantflags;
       int      nowantflags;
     */
{
    struct shpstr ship;
    struct mchrstr *mcp;
    struct mlist *mlp;

    emp_initque(list);
    while (nxtitem(ni, &ship)) {
	if (!player->owner)
	    continue;
	mcp = &mchr[(int)ship.shp_type];
	/* if (wantflags && (mcp->m_flags & wantflags) != wantflags)
	   continue;
	   if (nowantflags && mcp->m_flags & nowantflags)
	   continue;
	 */
	if (opt_MARKET) {
	    if (ontradingblock(EF_SHIP, (int *)&ship)) {
		pr("ship #%d inelligible - it's for sale.\n",
		   ship.shp_uid);
		continue;
	    }
	}
	/* This abuse is better fixed by building a ship with the normal negative
	   mobility that everything else is built with */
/*
	if (opt_MOB_ACCESS) {
	  if (ship.shp_effic < 21 &&
	    ship.shp_mobil < etu_per_update) {
	    pr("%s needs at least %d mob to navigate.\n",
	       prship(&ship), etu_per_update);
	    continue;
	  }
	}
*/
	ship.shp_mission = 0;
	ship.shp_rflags = 0;
	memset(ship.shp_rpath, 0, sizeof(ship.shp_rpath));
	putship(ship.shp_uid, &ship);
	mlp = malloc(sizeof(struct mlist));
	mlp->mcp = mcp;
	mlp->ship = ship;
	mlp->mobil = (double)ship.shp_mobil;
	emp_insque(&mlp->queue, list);
    }
}

/* This function assumes that the list was created by shp_sel */
void
shp_nav(struct emp_qelem *list, double *minmobp, double *maxmobp,
	int *togetherp, natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    struct sctstr sect;
    struct shpstr ship;
    coord allx;
    coord ally;
    int first = 1;

    *minmobp = 9876.0;
    *maxmobp = -9876.0;
    *togetherp = 1;
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	getship(mlp->ship.shp_uid, &ship);
	if (ship.shp_own != actor) {
	    mpr(actor, "%s was sunk at %s\n",
		prship(&ship), xyas(ship.shp_x, ship.shp_y, actor));
	    emp_remque((struct emp_qelem *)mlp);
	    free(mlp);
	    continue;
	}
	if (opt_SAIL) {
	    if (*ship.shp_path && !update_pending) {
		shp_mess("has a sail path", mlp);
		mpr(actor, "Use `sail <#> -' to reset\n");
		continue;
	    }
	}
	/* check crew - uws don't count */
	if (ship.shp_item[I_MILIT] == 0 && ship.shp_item[I_CIVIL] == 0) {
	    shp_mess("is crewless", mlp);
	    continue;
	}
	if (!getsect(ship.shp_x, ship.shp_y, &sect)) {
	    shp_mess("was sucked into the sky by a strange looking spaceship", mlp);	/* heh -KHS */
	    continue;
	}
	switch (shp_check_nav(&sect)) {
	case CN_CONSTRUCTION:
	    shp_mess("is caught in a construction zone", mlp);
	    continue;
	case CN_LANDLOCKED:
	    shp_mess("is landlocked", mlp);
	    break;
	case CN_NAVIGABLE:
	    break;
	case CN_ERROR:
	default:
	    shp_mess("was just swallowed by a big green worm", mlp);
	    continue;
	}
	if (first) {
	    allx = ship.shp_x;
	    ally = ship.shp_y;
	    first = 0;
	}
	if (ship.shp_x != allx || ship.shp_y != ally)
	    *togetherp = 0;
	if (ship.shp_mobil + 1 < (int)mlp->mobil) {
	    mlp->mobil = (double)ship.shp_mobil;
	}
	if (mlp->mobil < *minmobp)
	    *minmobp = mlp->mobil;
	if (mlp->mobil > *maxmobp)
	    *maxmobp = mlp->mobil;
	mlp->ship = ship;
    }
}

void
shp_put(struct emp_qelem *list, natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *newqp;
    struct mlist *mlp;

    qp = list->q_back;
    while (qp != list) {
	mlp = (struct mlist *)qp;
	mpr(actor, "%s stopped at %s\n", prship(&mlp->ship),
	    xyas(mlp->ship.shp_x, mlp->ship.shp_y, mlp->ship.shp_own));
	mlp->ship.shp_mobil = (int)mlp->mobil;
	putship(mlp->ship.shp_uid, &mlp->ship);
	newqp = qp->q_back;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }
}

int
shp_sweep(struct emp_qelem *ship_list, int verbose, natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    struct sctstr sect;
    int mines, m, max, shells;
    int changed = 0;
    int stopping = 0;
    double mobcost;

    for (qp = ship_list->q_back; qp != ship_list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	if (!(mlp->mcp->m_flags & M_SWEEP)) {
	    if (verbose)
		mpr(actor, "%s doesn't have minesweeping capability!\n",
		    prship(&mlp->ship));
	    continue;
	}
	if (mlp->mobil <= 0.0) {
	    if (verbose)
		mpr(actor, "%s is out of mobility!\n", prship(&mlp->ship));
	    continue;
	}
	getsect(mlp->ship.shp_x, mlp->ship.shp_y, &sect);
	if (sect.sct_type != SCT_WATER) {
	    if (verbose)
		mpr(actor, "%s is not at sea.  No mines there!\n",
		    prship(&mlp->ship));
	    continue;
	}
	mobcost = mlp->ship.shp_effic * 0.01 * mlp->ship.shp_speed;
	mobcost = 480.0 / (mobcost +
			   techfact(mlp->ship.shp_tech, mobcost));
	mlp->mobil -= mobcost;
	mlp->ship.shp_mobil = (int)mlp->mobil;
	putship(mlp->ship.shp_uid, &mlp->ship);
	if (!(mines = sect.sct_mines))
	    continue;
	max = mlp->mcp->m_item[I_SHELL];
	shells = mlp->ship.shp_item[I_SHELL];
	for (m = 0; mines > 0 && m < 5; m++) {
	    if (chance(0.66)) {
		mpr(actor, "Sweep...\n");
		mines--;
		shells = MIN(max, shells + 1);
		changed |= map_set(actor, sect.sct_x, sect.sct_y, 'X', 0);
	    }
	}
	sect.sct_mines = mines;
	mlp->ship.shp_item[I_SHELL] = shells;
	if (shp_check_one_mines(mlp)) {
	    stopping = 1;
	    emp_remque(qp);
	    free(qp);
	}
	putship(mlp->ship.shp_uid, &mlp->ship);
	putsect(&sect);
    }
    if (changed)
	writemap(actor);
    return stopping;
}

static int
shp_check_one_mines(struct mlist *mlp)
{
    struct sctstr sect;
    int changed = 0;
    int actor;

    getsect(mlp->ship.shp_x, mlp->ship.shp_y, &sect);
    if (sect.sct_type != SCT_WATER)
	return 0;
    if (!sect.sct_mines)
	return 0;
    if (chance(DMINE_HITCHANCE(sect.sct_mines))) {
	actor = mlp->ship.shp_own;
	shp_hit_mine(&mlp->ship, mlp->mcp);
	sect.sct_mines--;
	changed |= map_set(actor, sect.sct_x, sect.sct_y, 'X', 0);
	if (changed)
	    writemap(actor);
	putsect(&sect);
	putship(mlp->ship.shp_uid, &mlp->ship);
	if (!mlp->ship.shp_own)
	    return 1;
    }
    return 0;
}

static int
shp_check_mines(struct emp_qelem *ship_list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    int stopping = 0;

    for (qp = ship_list->q_back; qp != ship_list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	if (shp_check_one_mines(mlp)) {
	    stopping = 1;
	    emp_remque(qp);
	    free(qp);
	}
    }
    return stopping;
}

void
shp_list(struct emp_qelem *ship_list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    struct shpstr *shp;

    pr("shp#     ship type       x,y   fl  eff mil  sh gun pn he xl ln mob tech\n");

    for (qp = ship_list->q_back; qp != ship_list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	shp = &mlp->ship;
	pr("%4d ", shp->shp_uid);
	pr("%-16.16s ", mlp->mcp->m_name);
	prxy("%4d,%-4d ", shp->shp_x, shp->shp_y, mlp->ship.shp_own);
	pr("%c", shp->shp_fleet);
	pr("%4d%%", shp->shp_effic);
	pr("%4d", shp->shp_item[I_MILIT]);
	pr("%4d", shp->shp_item[I_SHELL]);
	pr("%4d", shp->shp_item[I_GUN]);
	count_planes(shp);
	pr("%3d", shp->shp_nplane);
	pr("%3d", shp->shp_nchoppers);
	pr("%3d", shp->shp_nxlight);
	count_units(shp);
	pr("%3d", shp->shp_nland);
	pr("%4d", shp->shp_mobil);
	pr("%4d\n", shp->shp_tech);
    }
}

static void
shp_mess(s_char *str, struct mlist *mlp)
{
    mpr(mlp->ship.shp_own, "%s %s & stays in %s\n",
	prship(&mlp->ship),
	str, xyas(mlp->ship.shp_x, mlp->ship.shp_y, mlp->ship.shp_own));
    mlp->ship.shp_mobil = (int)mlp->mobil;
    putship(mlp->ship.shp_uid, &mlp->ship);
    emp_remque((struct emp_qelem *)mlp);
    free(mlp);
}

int
shp_check_nav(struct sctstr *sect)
{
    switch (dchr[sect->sct_type].d_nav) {
    case NAVOK:
	break;

    case NAV_02:
	if (sect->sct_effic < 2)
	    return CN_CONSTRUCTION;
	break;
    case NAV_60:
	if (sect->sct_effic < 60)
	    return CN_CONSTRUCTION;
	break;
    default:
	return CN_LANDLOCKED;
    }
    return CN_NAVIGABLE;
}

int
sect_has_dock(struct sctstr *sect)
{
    switch (dchr[sect->sct_type].d_nav) {
    case NAV_02:
	return 1;
    default:
	return 0;
    }
}

static int
shp_count(struct emp_qelem *list, int wantflags, int nowantflags, int x,
	  int y)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    int count = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	if (mlp->ship.shp_x != x || mlp->ship.shp_y != y)
	    continue;
	if (wantflags && (mlp->mcp->m_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags && mlp->mcp->m_flags & nowantflags)
	    continue;
	++count;
    }
    return count;
}

static void
shp_damage_one(struct mlist *mlp, int dam)
{
    shipdamage(&mlp->ship, dam);
    putship(mlp->ship.shp_uid, &mlp->ship);
    if (!mlp->ship.shp_own) {
	emp_remque((struct emp_qelem *)mlp);
	free(mlp);
    }
}

static int
shp_damage(struct emp_qelem *list, int totdam, int wantflags,
	   int nowantflags, int x, int y)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    int dam;
    int count;

    if (!totdam
	|| !(count = shp_count(list, wantflags, nowantflags, x, y)))
	return 0;
    dam = ldround(((double)totdam / (double)count), 1);
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	if (mlp->ship.shp_x != x || mlp->ship.shp_y != y)
	    continue;
	if (wantflags && (mlp->mcp->m_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags && mlp->mcp->m_flags & nowantflags)
	    continue;
	shp_damage_one(mlp, dam);
    }
    return dam;
}

static int
shp_contains(struct emp_qelem *list, int newx, int newy, int wantflags,
	     int nowantflags)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
/* If the ship isn't in the requested sector, then continue */
	if (newx != mlp->ship.shp_x || newy != mlp->ship.shp_y)
	    continue;
	if (wantflags && (mlp->mcp->m_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags && mlp->mcp->m_flags & nowantflags)
	    continue;
	return 1;
    }
    return 0;
}

static struct mlist *
most_valuable_ship(struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    struct mlist *mvs = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	if (mlp->mcp->m_flags & M_SUB)
	    continue;
	if (!mlp->mcp->m_nxlight &&
	    !mlp->mcp->m_nchoppers &&
	    mlp->mcp->m_cost < 1000 &&
	    !mlp->mcp->m_nplanes && !mlp->mcp->m_nland)
	    continue;
	if (!mvs) {
	    mvs = mlp;
	    continue;
	}
	if (mlp->mcp->m_cost * mlp->ship.shp_effic >
	    mvs->mcp->m_cost * mvs->ship.shp_effic)
	    mvs = mlp;
    }
    return mvs;
}

static int
shp_easiest_target(struct emp_qelem *list, int wantflags, int nowantflags)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    int hard;
    int easiest = 9876;		/* things start great for victim */
    int count = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	if (wantflags && (mlp->mcp->m_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags && mlp->mcp->m_flags & nowantflags)
	    continue;
	hard = shp_hardtarget(&mlp->ship);
	if (hard < easiest)
	    easiest = hard;	/* things get worse for victim */
	++count;
    }
    return easiest - count;
}

static int
shp_missile_interdiction(struct emp_qelem *list, coord newx, coord newy,
			 natid victim)
{
    int dam;
    int twotries;
    int stopping = 0;
    struct emp_qelem msl_list, *qp, *newqp;
    struct mlist *mvs;
    s_char what[512];

    msl_sel(&msl_list, newx, newy, victim, P_T | P_MAR, 0, MI_INTERDICT);

    twotries = 0;
    while (!QEMPTY(&msl_list) && (mvs = most_valuable_ship(list))) {
	sprintf(what, "%s", prship(&mvs->ship));
	dam = msl_launch_mindam(&msl_list, newx, newy,
				shp_hardtarget(&mvs->ship),
				EF_SHIP, 1, what, victim, MI_INTERDICT);
	if (dam) {
	    mpr(victim,
		"missile interdiction mission does %d damage to %s!\n",
		dam, what);
	    shp_damage_one(mvs, dam);
	    twotries = 0;
	    stopping |= 1;
	} else if (++twotries >= 2) {
	    break;
	}
    }
    qp = msl_list.q_forw;
    while (qp != msl_list.q_forw) {
	newqp = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = newqp;
    }

    return stopping;
}

/* Note that this function has a side effect - it uses coastwatch
 * ranges to see if it should fire upon a ship.  So, this function
 * is expected to return positive if a ship is in range, and 0 if a
 * ship is not in range. */
static int
notify_coastguard(struct emp_qelem *list, int trange, struct sctstr *sectp)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    struct natstr *natp;
    int vrange;

    natp = getnatp(sectp->sct_own);

    vrange = sectp->sct_type == SCT_RADAR ? 14 : 4;
    vrange *= tfact(sectp->sct_own, 1.0) * sectp->sct_effic / 100.0;

    if (vrange < 1)
	vrange = 1;

    if (vrange < trange)
	return 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	if (mlp->mcp->m_flags & M_SUB)
	    continue;
	if (natp->nat_flags & NF_COASTWATCH)
	    wu(0, sectp->sct_own,
	       "%s %s sighted at %s\n",
	       cname(mlp->ship.shp_own),
	       prship(&mlp->ship),
	       xyas(mlp->ship.shp_x, mlp->ship.shp_y, sectp->sct_own));
	if (opt_HIDDEN)
	    setcont(sectp->sct_own, mlp->ship.shp_own, FOUND_COAST);
    }

    return 1;
}

static int
shp_fort_interdiction(struct emp_qelem *list, coord newx, coord newy,
		      natid victim)
{
    struct nstr_sect ns;
    struct sctstr fsect;
    int trange;
    double range, range2, guneff;
    int shell, gun;
    int dam;
    int totdam = 0;
    s_char notified[MAXNOC];
    int i;

    /* Inform neutral and worse */
    for (i = 0; i < MAXNOC; ++i) {
	if (getrel(getnatp(i), victim) <= NEUTRAL)
	    notified[i] = 0;
	else
	    notified[i] = 1;
    }

    snxtsct_dist(&ns, newx, newy, fort_max_interdiction_range);
    while (nxtsct(&ns, &fsect)) {
	if (!fsect.sct_own)
	    continue;
	if (fsect.sct_own == victim)
	    continue;
	if (notified[fsect.sct_own])
	    continue;
	trange = mapdist(newx, newy, fsect.sct_x, fsect.sct_y);
	if (notify_coastguard(list, trange, &fsect))
	    notified[fsect.sct_own] = 1;
    }
    if (opt_NO_FORT_FIRE)
	return 0;		/* Only coastwatch notify in nofortfire */
    /* Only fire at Hostile ships */
    for (i = 0; i < MAXNOC; ++i) {
	if (getrel(getnatp(i), victim) >= NEUTRAL)
	    notified[i] = 0;
    }
    snxtsct_dist(&ns, newx, newy, fort_max_interdiction_range);
    while (nxtsct(&ns, &fsect)) {
	if (!notified[fsect.sct_own])
	    continue;
	if (fsect.sct_type != SCT_FORTR)
	    continue;
	gun = fsect.sct_item[I_GUN];
	if (gun < 1)
	    continue;
	range = tfactfire(fsect.sct_own, (double)MIN(gun, 7));
	if (fsect.sct_effic > 59)
	    range++;
	range2 = roundrange(range);
	trange = mapdist(newx, newy, fsect.sct_x, fsect.sct_y);
	if (trange > range2)
	    continue;
	if (fsect.sct_item[I_MILIT] < 5)
	    continue;
	shell = fsect.sct_item[I_SHELL];
	if (shell < 1)
	    shell += supply_commod(fsect.sct_own,
				   fsect.sct_x, fsect.sct_y, I_SHELL, 1);
	if (shell < 1)
	    continue;
	shell--;
	fsect.sct_item[I_SHELL] = shell;
	putsect(&fsect);
	if (gun > 7)
	    gun = 7;
	guneff = landgun((int)fsect.sct_effic, gun);
	dam = (int)guneff;
	totdam += dam;
	mpr(victim, "Incoming fire does %d damage!\n", dam);
/*
  mpr(victim, "%s fires at you for %d!\n",
  xyas(fsect.sct_x,fsect.sct_y,victim),
  dam);
*/
	wu(0, fsect.sct_own,
	   "%s fires at %s ships in %s for %d!\n",
	   xyas(fsect.sct_x, fsect.sct_y,
		fsect.sct_own),
	   cname(victim), xyas(newx, newy, fsect.sct_own), dam);
	nreport(fsect.sct_own, N_SHP_SHELL, victim, 1);
    }
    if (totdam > 0)
	return shp_damage(list, totdam, 0, M_SUB, newx, newy);
    return 0;
}

static int
shp_interdict(struct emp_qelem *list, coord newx, coord newy, natid victim)
{
    int stopping = 0;

    if (shp_contains(list, newx, newy, 0, M_SUB)) {
	stopping |= shp_fort_interdiction(list, newx, newy, victim);

	if (shp_contains(list, newx, newy, 0, M_SUB)) {
	    stopping |=
		shp_damage(list,
			   unit_interdict(newx, newy, victim, "ships",
					  shp_easiest_target(list, 0, M_SUB),
					  MI_INTERDICT),
			   0, M_SUB, newx, newy);
	    if (most_valuable_ship(list)) {
		stopping |=
		    shp_missile_interdiction(list, newx, newy, victim);
	    }
	}
    }
    if (shp_contains(list, newx, newy, M_SUB, 0)) {
	stopping |=
	    shp_damage(list,
		       unit_interdict(newx, newy, victim, "subs",
				      shp_easiest_target(list, M_SUB, 0),
				      MI_SINTERDICT),
		       M_SUB, 0, newx, newy);
    }
    return stopping;
}

/* high value of hardtarget is harder to hit */
int
shp_hardtarget(struct shpstr *sp)
{
    struct sctstr sect;
    int vis, onsea;
    struct mchrstr *mcp = mchr + sp->shp_type;

    vis = sp->shp_visib;
    getsect(sp->shp_x, sp->shp_y, &sect);
    onsea = (sect.sct_type == SCT_WATER);
    if (mcp->m_flags & M_SUB)
	vis *= 4;
    return (int)(((double)sp->shp_effic / 100.0) *
		 (20 + (double)sp->shp_speed * onsea / 2.0 - vis));
}

static int
shp_hit_mine(struct shpstr *sp, struct mchrstr *mcp)
{
    double m;

    mpr(sp->shp_own, "Kawhomp! Mine detected in %s!\n",
	xyas(sp->shp_x, sp->shp_y, sp->shp_own));

    nreport(sp->shp_own, N_HIT_MINE, 0, 1);

    m = 22.0 + (double)(random() % 21);
    if (mcp->m_flags & M_SWEEP)
	m /= 2.0;

    shipdamage(sp, ldround(m, 1));

    return (int)m;
}

void
shp_view(struct emp_qelem *list)
{
    struct sctstr sect;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	getsect(mlp->ship.shp_x, mlp->ship.shp_y, &sect);
	if (mlp->mcp->m_flags & M_FOOD)
	    mpr(mlp->ship.shp_own, "[fert:%d] ", sect.sct_fertil);
	if (mlp->mcp->m_flags & M_OIL)
	    mpr(mlp->ship.shp_own, "[oil:%d] ", sect.sct_oil);
	mpr(mlp->ship.shp_own, "%s @ %s %d%% %s\n",
	    prship(&mlp->ship),
	    xyas(mlp->ship.shp_x, mlp->ship.shp_y, player->cnum),
	    sect.sct_effic, dchr[sect.sct_type].d_name);
    }
}

int
shp_nav_one_sector(struct emp_qelem *list, int dir, natid actor,
		   int together)
{
    struct sctstr sect;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct mlist *mlp;
    struct emp_qelem done;
    coord dx;
    coord dy;
    coord newx;
    coord newy;
    int stopping = 0;
    double mobcost;
    double tech;		/* for mapping */
    double tf;			/* for mapping */
    s_char dp[80];

    if (dir <= DIR_STOP || dir >= DIR_VIEW) {
	shp_put(list, actor);
	return 1;
    }
    dx = diroff[dir][0];
    dy = diroff[dir][1];
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct mlist *)qp;
	newx = xnorm(mlp->ship.shp_x + dx);
	newy = ynorm(mlp->ship.shp_y + dy);
	getsect(newx, newy, &sect);
	if (shp_check_nav(&sect) != CN_NAVIGABLE ||
	    (sect.sct_own && actor != sect.sct_own &&
	     getrel(getnatp(sect.sct_own), actor) < FRIENDLY)) {
	    if (together) {
		mpr(actor, "can't go to %s\n", xyas(newx, newy, actor));
		return 2;
	    } else {
		sprintf(dp, "can't go to %s", xyas(newx, newy, actor));
		shp_mess(dp, mlp);
		continue;
	    }
	}
	if (IS_BIG_CITY(sect.sct_type)) {
	    if (!(mlp->mcp->m_flags & M_CANAL)) {
		sprintf(dp,
			"is too large to fit into the canal system at %s",
			xyas(newx, newy, actor));
		shp_mess(dp, mlp);
		continue;
	    }
	}
	if (mlp->mobil <= 0.0) {
	    shp_mess("is out of mobility", mlp);
	    continue;
	}
	mobcost = mlp->ship.shp_effic * 0.01 * mlp->ship.shp_speed;
	mobcost = 480.0 /
	    (mobcost + techfact(mlp->ship.shp_tech, mobcost));
	mlp->ship.shp_x = newx;
	mlp->ship.shp_y = newy;
	if (mlp->mobil - mobcost < -127) {
	    mlp->mobil = -127;
	} else {
	    mlp->mobil -= mobcost;
	}
	mlp->ship.shp_mobil = (int)mlp->mobil;
	putship(mlp->ship.shp_uid, &mlp->ship);

	/* Now update the map for this ship */
	tech = techfact(mlp->ship.shp_tech, (double)mlp->mcp->m_vrnge);
	if (mlp->mcp->m_flags & M_SONAR)
	    tf = techfact(mlp->ship.shp_tech, 1.0);
	else
	    tf = 0.0;
	radmapupd(mlp->ship.shp_own, mlp->ship.shp_x, mlp->ship.shp_y,
		  (int)mlp->ship.shp_effic, (int)tech, tf);
    }
    if (QEMPTY(list))
	return stopping;
    stopping |= shp_sweep(list, 0, actor);
    if (QEMPTY(list))
	return stopping;
    stopping |= shp_check_mines(list);
    if (QEMPTY(list))
	return stopping;

    /* interdict ships sector by sector */
    emp_initque(&done);
    while (!QEMPTY(list)) {
	mlp = (struct mlist *)list->q_back;
	newx = mlp->ship.shp_x;
	newy = mlp->ship.shp_y;
	stopping |= shp_interdict(list, newx, newy, actor);
	/* move survivors in this sector to done */
	for (qp = list->q_back; qp != list; qp = next) {
	    next = qp->q_back;
	    mlp = (struct mlist *)qp;
	    if (mlp->ship.shp_x == newx && mlp->ship.shp_y == newy) {
		emp_remque(qp);
		emp_insque(qp, &done);
	    }
	}
    }
    /* assign surviving ships back to list */
    emp_insque(list, &done);
    emp_remque(&done);

    return stopping;
}

/*
 * shp_miss_defence 
 * Check for incoming missiles with a P_MAR flag. 
 * Return True=1 if the missile was shotdown.
 * Or False=0
 * 
 * Chad Zabel, July 95
 */

int
shp_missile_defense(coord dx, coord dy, natid bombown, int hardtarget)
{
    struct nstr_item ni;
    struct shpstr ship;
    int hitchance;
    int shell;
    double gun, eff, teff;

    snxtitem_dist(&ni, EF_SHIP, dx, dy, 1);

    while (nxtitem(&ni, &ship)) {
	if (!ship.shp_own)
	    continue;

	if (!(mchr[(int)ship.shp_type].m_flags & M_ANTIMISSILE))
	    continue;

	if (getrel(getnatp(ship.shp_own), bombown) >= NEUTRAL)
	    continue;

	if (ship.shp_effic < 60)
	    continue;

	shell = ship.shp_item[I_SHELL];
	if (ship.shp_item[I_MILIT] < 1)	/* do we have mil? */
	    continue;
	if (shell < 2) {	/* do we need shells */
	    shell += supply_commod(ship.shp_own, ship.shp_x, ship.shp_y,
				   I_SHELL, 2);
	    if (shell < 2)
		continue;
	}
	if (ship.shp_item[I_GUN] < 1)	/* we need at least 1 gun */
	    continue;

	/* now calculate the odds */
	gun = MIN(ship.shp_item[I_GUN], ship.shp_glim);
	eff = (double)ship.shp_effic / 100.0;
	teff =
	    (((double)ship.shp_tech) / (((double)ship.shp_tech) + 200.0));
	/* raise 4.5 for better interception -KHS */
	hitchance = (int)(gun * eff * teff * 4.5) - hardtarget;
	if (hitchance < 0)
	    hitchance = 0;
	if (hitchance > 100)
	    hitchance = 100;

	mpr(bombown, "%s anti-missile system activated...",
	    cname(ship.shp_own));
	mpr(ship.shp_own, "Ship #%i anti-missile system activated!\n",
	    ship.shp_uid);
	mpr(ship.shp_own, "%d%% hitchance...", hitchance);
	/* use ammo */
	ship.shp_item[I_SHELL] = shell - 2;
	putship(ship.shp_uid, &ship);

	if (roll(100) <= hitchance) {
	    mpr(bombown, "KABOOOM!! Missile destroyed\n\n");
	    mpr(ship.shp_own,
		"KABOOOM!!  Incoming missile destroyed!\n\n");
	    return 1;
	} else {
	    mpr(bombown, "SWOOSH!!  anti-missile system failed!!\n");
	    mpr(ship.shp_own,
		"SWOOSH!!  Missile evades anti-missile systems\n\n");
	}
    }
    return 0;			/* all attempts failed */
}

s_char *
shp_path(int together, struct shpstr *shp, s_char *buf)
{
    coord destx;
    coord desty;
    struct sctstr d_sect;
    s_char *cp;

    if (!sarg_xy(buf, &destx, &desty))
	return 0;
    if (!together) {
	mpr(shp->shp_own,
	    "Cannot go to a destination sector if not all starting in the same sector\n");
	return 0;
    }
    if (!getsect(destx, desty, &d_sect)) {
	mpr(shp->shp_own, "%d,%d is not a sector\n", destx, desty);
	return 0;
    }

    cp = BestShipPath(buf, shp->shp_x, shp->shp_y,
		      d_sect.sct_x, d_sect.sct_y, player->cnum);
    if (!cp || shp->shp_mobil <= 0) {
	mpr(shp->shp_own, "Can't get to '%s' right now.\n",
	    xyas(d_sect.sct_x, d_sect.sct_y, player->cnum));
	return 0;
    }
    return cp;
}

/* Fire missiles at a ship which has fired shells */
void
shp_missdef(struct shpstr *sp, natid victim)
{
    struct emp_qelem list;
    struct mlist *mlp;
    int eff;
    s_char buf[512];

    emp_initque(&list);

    mlp = malloc(sizeof(struct mlist));
    mlp->mcp = &mchr[(int)sp->shp_type];
    mlp->ship = *sp;
    mlp->mobil = (double)sp->shp_mobil;
    emp_insque(&mlp->queue, &list);
    sprintf(buf, "%s", prship(&mlp->ship));

    eff = sp->shp_effic;
    if (most_valuable_ship(&list)) {
	shp_missile_interdiction(&list, sp->shp_x, sp->shp_y, sp->shp_own);
	getship(sp->shp_uid, sp);

	if (!sp->shp_own) {
	    wu(0, victim,
	       "missiles launched in defense did 100%% damage to %s\n",
	       buf);
	    wu(0, victim, "%s sunk!\n", buf);
	} else if (eff > 0 && sp->shp_effic < eff) {
	    wu(0, victim,
	       "missiles launched in defense did %d%% damage to %s\n",
	       100 * (eff - sp->shp_effic) / eff, buf);
	}
    }
    if (!QEMPTY(&list))
	free(mlp);
}

/*
 * Set SP's tech to TLEV along with everything else that depends on it.
 */
void
shp_set_tech(struct shpstr *sp, int tlev)
{
    struct mchrstr *mcp = mchr + sp->shp_type;
    int tech_diff = tlev - mcp->m_tech;

    if (CANT_HAPPEN(tech_diff < 0)) {
      tlev -= tech_diff;
      tech_diff = 0;
    }

    sp->shp_tech = tlev;
    sp->shp_armor = (short)SHP_DEF(mcp->m_armor, tech_diff);
    sp->shp_speed = (short)SHP_SPD(mcp->m_speed, tech_diff);
    sp->shp_visib = (short)SHP_VIS(mcp->m_visib, tech_diff);
    sp->shp_frnge = (short)SHP_RNG(mcp->m_frnge, tech_diff);
    sp->shp_glim  = (short)SHP_FIR(mcp->m_glim, tech_diff);
}
