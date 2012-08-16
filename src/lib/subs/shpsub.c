/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shpsub.c: Ship subroutine stuff
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1996-2000
 *     Markus Armbruster, 2006-2012
 */

#include <config.h>

#include <stdlib.h>
#include "chance.h"
#include "damage.h"
#include "empobj.h"
#include "file.h"
#include "map.h"
#include "misc.h"
#include "mission.h"
#include "news.h"
#include "nsc.h"
#include "optlist.h"
#include "path.h"
#include "player.h"
#include "prototypes.h"
#include "queue.h"
#include "server.h"
#include "unit.h"
#include "xy.h"

static int shp_check_one_mines(struct ulist *);
static int shp_hit_mine(struct shpstr *);
static void shp_stays(natid, char *, struct ulist *);

void
shp_sel(struct nstr_item *ni, struct emp_qelem *list)
{
    struct shpstr ship;
    struct mchrstr *mcp;
    struct ulist *mlp;

    emp_initque(list);
    while (nxtitem(ni, &ship)) {
	/*
	 * It would be nice to let deities navigate foreign ships, but
	 * much of the code assumes that only the ship's owner can
	 * navigate it.
	 */
	if (!ship.shp_own || ship.shp_own != player->cnum)
	    continue;
	mcp = &mchr[(int)ship.shp_type];
	if (opt_MARKET) {
	    if (ontradingblock(EF_SHIP, &ship)) {
		pr("ship #%d inelligible - it's for sale.\n",
		   ship.shp_uid);
		continue;
	    }
	}
	ship.shp_mission = 0;
	ship.shp_rflags = 0;
	memset(ship.shp_rpath, 0, sizeof(ship.shp_rpath));
	putship(ship.shp_uid, &ship);
	mlp = malloc(sizeof(struct ulist));
	mlp->chrp = (struct empobj_chr *)mcp;
	mlp->unit.ship = ship;
	mlp->mobil = ship.shp_mobil;
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
    struct ulist *mlp;
    struct shpstr *sp;
    struct sctstr sect;
    coord allx;
    coord ally;
    int first = 1;

    *minmobp = 9876.0;
    *maxmobp = -9876.0;
    *togetherp = 1;
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
	sp = &mlp->unit.ship;
	getship(sp->shp_uid, sp);
	if (sp->shp_own != actor) {
	    mpr(actor, "%s was sunk at %s\n",
		prship(sp), xyas(sp->shp_x, sp->shp_y, actor));
	    emp_remque((struct emp_qelem *)mlp);
	    free(mlp);
	    continue;
	}
	if (opt_SAIL) {
	    if (*sp->shp_path && !update_running) {
		shp_stays(actor, "has a sail path", mlp);
		mpr(actor, "Use `sail <#> -' to reset\n");
		continue;
	    }
	}
	/* check crew - uws don't count */
	if (sp->shp_item[I_MILIT] == 0 && sp->shp_item[I_CIVIL] == 0) {
	    shp_stays(actor, "is crewless", mlp);
	    continue;
	}
	if (!getsect(sp->shp_x, sp->shp_y, &sect)) {
	    shp_stays(actor, "was sucked into the sky by a strange looking spaceship", mlp);	/* heh -KHS */
	    continue;
	}
	switch (shp_check_nav(&sect, sp)) {
	case CN_CONSTRUCTION:
	    shp_stays(actor, "is caught in a construction zone", mlp);
	    continue;
	case CN_LANDLOCKED:
	    shp_stays(actor, "is landlocked", mlp);
	    continue;
	case CN_NAVIGABLE:
	    break;
	case CN_ERROR:
	default:
	    shp_stays(actor, "was just swallowed by a big green worm", mlp);
	    continue;
	}
	if (first) {
	    allx = sp->shp_x;
	    ally = sp->shp_y;
	    first = 0;
	}
	if (sp->shp_x != allx || sp->shp_y != ally)
	    *togetherp = 0;
	if (sp->shp_mobil + 1 < (int)mlp->mobil) {
	    mlp->mobil = sp->shp_mobil;
	}
	if (mlp->mobil < *minmobp)
	    *minmobp = mlp->mobil;
	if (mlp->mobil > *maxmobp)
	    *maxmobp = mlp->mobil;
    }
}

int
shp_sweep(struct emp_qelem *ship_list, int verbose, int takemob, natid actor)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *mlp;
    struct sctstr sect;
    int mines, m, max, shells;
    int changed = 0;
    int stopping = 0;

    for (qp = ship_list->q_back; qp != ship_list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
	if (!(((struct mchrstr *)mlp->chrp)->m_flags & M_SWEEP)) {
	    if (verbose)
		mpr(actor, "%s doesn't have minesweeping capability!\n",
		    prship(&mlp->unit.ship));
	    continue;
	}
	if (takemob && mlp->mobil <= 0.0) {
	    if (verbose)
		mpr(actor, "%s is out of mobility!\n",
		    prship(&mlp->unit.ship));
	    continue;
	}
	getsect(mlp->unit.ship.shp_x, mlp->unit.ship.shp_y, &sect);
	if (sect.sct_type != SCT_WATER) {
	    if (verbose)
		mpr(actor, "%s is not at sea.  No mines there!\n",
		    prship(&mlp->unit.ship));
	    continue;
	}
	if (takemob) {
	    mlp->mobil -= shp_mobcost(&mlp->unit.ship);
	    mlp->unit.ship.shp_mobil = (int)mlp->mobil;
	}
	putship(mlp->unit.ship.shp_uid, &mlp->unit.ship);
	if (!(mines = sect.sct_mines))
	    continue;
	max = ((struct mchrstr *)mlp->chrp)->m_item[I_SHELL];
	shells = mlp->unit.ship.shp_item[I_SHELL];
	for (m = 0; mines > 0 && m < 5; m++) {
	    if (chance(0.66)) {
		mpr(actor, "Sweep...\n");
		mines--;
		shells = MIN(max, shells + 1);
		changed |= map_set(actor, sect.sct_x, sect.sct_y, 'X', 0);
	    }
	}
	sect.sct_mines = mines;
	mlp->unit.ship.shp_item[I_SHELL] = shells;
	putship(mlp->unit.ship.shp_uid, &mlp->unit.ship);
	putsect(&sect);
	if (shp_check_one_mines(mlp)) {
	    stopping = 1;
	    emp_remque(qp);
	    free(qp);
	}
    }
    if (changed)
	writemap(actor);
    return stopping;
}

static int
shp_check_one_mines(struct ulist *mlp)
{
    struct sctstr sect;
    int actor;

    getsect(mlp->unit.ship.shp_x, mlp->unit.ship.shp_y, &sect);
    if (sect.sct_type != SCT_WATER)
	return 0;
    if (!sect.sct_mines)
	return 0;
    if (chance(DMINE_HITCHANCE(sect.sct_mines))) {
	actor = mlp->unit.ship.shp_own;
	shp_hit_mine(&mlp->unit.ship);
	sect.sct_mines--;
	if (map_set(actor, sect.sct_x, sect.sct_y, 'X', 0))
	    writemap(actor);
	putsect(&sect);
	putship(mlp->unit.ship.shp_uid, &mlp->unit.ship);
	if (!mlp->unit.ship.shp_own)
	    return 1;
    }
    return 0;
}

static int
shp_check_mines(struct emp_qelem *ship_list)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *mlp;
    int stopping = 0;

    for (qp = ship_list->q_back; qp != ship_list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
	if (shp_check_one_mines(mlp)) {
	    stopping = 1;
	    emp_remque(qp);
	    free(qp);
	}
    }
    return stopping;
}


static void
shp_stays(natid actor, char *str, struct ulist *mlp)
{
    mpr(actor, "%s %s & stays in %s\n",
	prship(&mlp->unit.ship), str,
	xyas(mlp->unit.ship.shp_x, mlp->unit.ship.shp_y, actor));
    mlp->unit.ship.shp_mobil = (int)mlp->mobil;
    putship(mlp->unit.ship.shp_uid, &mlp->unit.ship);
    emp_remque((struct emp_qelem *)mlp);
    free(mlp);
}

int
shp_check_nav(struct sctstr *sect, struct shpstr *shp)
{
    switch (dchr[sect->sct_type].d_nav) {
    case NAVOK:
	break;
    case NAV_CANAL:
	if (mchr[(int)shp->shp_type].m_flags & M_CANAL) {
	    if (sect->sct_effic < 2)
		return CN_CONSTRUCTION;
	} else
	    return CN_LANDLOCKED;
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
    case NAV_CANAL:
	return 1;
    default:
	return 0;
    }
}

static int
shp_count(struct emp_qelem *list, int wantflags, int nowantflags,
	  int x, int y)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *mlp;
    int count = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
	if (mlp->unit.ship.shp_x != x || mlp->unit.ship.shp_y != y)
	    continue;
	if (wantflags &&
	    (((struct mchrstr *)mlp->chrp)->m_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags &&
	    ((struct mchrstr *)mlp->chrp)->m_flags & nowantflags)
	    continue;
	++count;
    }
    return count;
}

static void
shp_damage_one(struct ulist *mlp, int dam)
{
    /* ship might have changed (launched interceptors, missile defense) */
    getship(mlp->unit.ship.shp_uid, &mlp->unit.ship);
    shipdamage(&mlp->unit.ship, dam);
    putship(mlp->unit.ship.shp_uid, &mlp->unit.ship);
    if (!mlp->unit.ship.shp_own) {
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
    struct ulist *mlp;
    int dam;
    int count;

    if (!totdam
	|| !(count = shp_count(list, wantflags, nowantflags, x, y)))
	return 0;
    dam = ldround((double)totdam / count, 1);
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
	if (mlp->unit.ship.shp_x != x || mlp->unit.ship.shp_y != y)
	    continue;
	if (wantflags &&
	    (((struct mchrstr *)mlp->chrp)->m_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags &&
	    ((struct mchrstr *)mlp->chrp)->m_flags & nowantflags)
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
    struct ulist *mlp;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
/* If the ship isn't in the requested sector, then continue */
	if (newx != mlp->unit.ship.shp_x || newy != mlp->unit.ship.shp_y)
	    continue;
	if (wantflags &&
	    (((struct mchrstr *)mlp->chrp)->m_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags &&
	    ((struct mchrstr *)mlp->chrp)->m_flags & nowantflags)
	    continue;
	return 1;
    }
    return 0;
}

static struct ulist *
most_valuable_ship(struct emp_qelem *list, coord x, coord y)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *mlp;
    struct ulist *mvs = NULL;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
	if (mlp->unit.ship.shp_x != x || mlp->unit.ship.shp_y != y)
	    continue;
	if (((struct mchrstr *)mlp->chrp)->m_flags & M_SUB)
	    continue;
	if (!((struct mchrstr *)mlp->chrp)->m_nxlight &&
	    !((struct mchrstr *)mlp->chrp)->m_nchoppers &&
	    ((struct mchrstr *)mlp->chrp)->m_cost < 1000 &&
	    !((struct mchrstr *)mlp->chrp)->m_nplanes &&
	    !((struct mchrstr *)mlp->chrp)->m_nland)
	    continue;
	if (!mvs) {
	    mvs = mlp;
	    continue;
	}
	if (((struct mchrstr *)mlp->chrp)->m_cost * mlp->unit.ship.shp_effic >
	    ((struct mchrstr *)mvs->chrp)->m_cost * mvs->unit.ship.shp_effic)
	    mvs = mlp;
    }
    return mvs;
}

static int
shp_easiest_target(struct emp_qelem *list, int wantflags, int nowantflags)
{
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *mlp;
    int hard;
    int easiest = 9876;		/* things start great for victim */
    int count = 0;

    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
	if (wantflags &&
	    (((struct mchrstr *)mlp->chrp)->m_flags & wantflags) != wantflags)
	    continue;
	if (nowantflags &&
	    ((struct mchrstr *)mlp->chrp)->m_flags & nowantflags)
	    continue;
	hard = shp_hardtarget(&mlp->unit.ship);
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
    int dam, sublaunch;
    int stopping = 0;
    struct emp_qelem msl_list, *qp, *newqp;
    struct plist *plp;
    struct ulist *mvs;

    mvs = most_valuable_ship(list, newx, newy);
    if (!mvs)
	return 0;

    msl_sel(&msl_list, newx, newy, victim, P_T | P_MAR, 0, MI_INTERDICT);

    for (qp = msl_list.q_back; qp != &msl_list; qp = newqp) {
	newqp = qp->q_back;
	plp = (struct plist *)qp;

	if (mvs && mission_pln_equip(plp, NULL, 'p') >= 0) {
	    if (msl_launch(&plp->plane, EF_SHIP, prship(&mvs->unit.ship),
			   newx, newy, victim, &sublaunch) < 0)
		goto use_up_msl;
	    stopping = 1;
	    if (msl_hit(&plp->plane,
			shp_hardtarget(&mvs->unit.ship), EF_SHIP,
			N_SHP_MISS, N_SHP_SMISS, sublaunch, victim)) {
		dam = pln_damage(&plp->plane, 'p', 1);
		mpr(victim,
		    "missile interdiction mission does %d damage to %s!\n",
		    dam, prship(&mvs->unit.ship));
		shp_damage_one(mvs, dam);
	    } else {
		dam = pln_damage(&plp->plane, 'p', 0);
		collateral_damage(newx, newy, dam);
	    }
	    mvs = most_valuable_ship(list, newx, newy);
	use_up_msl:
	    plp->plane.pln_effic = 0;
	    putplane(plp->plane.pln_uid, &plp->plane);
	}
	emp_remque(qp);
	free(qp);
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
    struct ulist *mlp;
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
	mlp = (struct ulist *)qp;
	if (((struct mchrstr *)mlp->chrp)->m_flags & M_SUB)
	    continue;
	if (natp->nat_flags & NF_COASTWATCH)
	    wu(0, sectp->sct_own,
	       "%s %s sighted at %s\n",
	       cname(mlp->unit.ship.shp_own),
	       prship(&mlp->unit.ship),
	       xyas(mlp->unit.ship.shp_x, mlp->unit.ship.shp_y,
		    sectp->sct_own));
	if (opt_HIDDEN)
	    setcont(sectp->sct_own, mlp->unit.ship.shp_own, FOUND_COAST);
    }

    return 1;
}

static int
shp_fort_interdiction(struct emp_qelem *list, coord newx, coord newy,
		      natid victim)
{
    struct nstr_sect ns;
    struct sctstr fsect;
    int trange, range;
    int dam;
    int stopping = 0;
    int totdam = 0;
    signed char notified[MAXNOC];
    int i;

    /* Inform neutral and worse */
    for (i = 0; i < MAXNOC; ++i) {
	if (relations_with(i, victim) <= NEUTRAL)
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
	if (relations_with(i, victim) >= NEUTRAL)
	    notified[i] = 0;
    }
    snxtsct_dist(&ns, newx, newy, fort_max_interdiction_range);
    while (nxtsct(&ns, &fsect)) {
	if (!notified[fsect.sct_own])
	    continue;
	range = roundrange(fortrange(&fsect));
	trange = mapdist(newx, newy, fsect.sct_x, fsect.sct_y);
	if (trange > range)
	    continue;
	dam = fort_fire(&fsect);
	putsect(&fsect);
	if (dam < 0)
	    continue;
	stopping = 1;
	totdam += dam;
	mpr(victim, "Incoming fire does %d damage!\n", dam);
#if 0
	mpr(victim, "%s fires at you for %d!\n",
	    xyas(fsect.sct_x, fsect.sct_y, victim), dam);
#endif
	wu(0, fsect.sct_own,
	   "%s fires at %s ships in %s for %d!\n",
	   xyas(fsect.sct_x, fsect.sct_y,
		fsect.sct_own),
	   cname(victim), xyas(newx, newy, fsect.sct_own), dam);
	nreport(fsect.sct_own, N_SHP_SHELL, victim, 1);
    }
    if (totdam > 0)
	shp_damage(list, totdam, 0, M_SUB, newx, newy);
    return stopping;
}

static int
shp_mission_interdiction(struct emp_qelem *list, coord x, coord y,
			 natid victim, int subs)
{
    char *what = subs ? "subs" : "ships";
    int wantflags = subs ? M_SUB : 0;
    int nowantflags = subs ? 0 : M_SUB;
    int mission = subs ? MI_SINTERDICT : MI_INTERDICT;
    int dam;

    dam = unit_interdict(x, y, victim, what,
			 shp_easiest_target(list, wantflags, nowantflags),
			 mission);
    if (dam >= 0)
	shp_damage(list, dam, wantflags, nowantflags, x, y);
    return dam >= 0;
}

static int
shp_interdict(struct emp_qelem *list, coord newx, coord newy, natid victim)
{
    int stopping = 0;

    if (shp_contains(list, newx, newy, 0, M_SUB)) {
	stopping |= shp_fort_interdiction(list, newx, newy, victim);

	if (shp_contains(list, newx, newy, 0, M_SUB)) {
	    stopping |= shp_mission_interdiction(list, newx, newy, victim, 0);
	    stopping |= shp_missile_interdiction(list, newx, newy, victim);
	}
    }
    if (shp_contains(list, newx, newy, M_SUB, 0))
	stopping |= shp_mission_interdiction(list, newx, newy, victim, 1);
    return stopping;
}

/* high value of hardtarget is harder to hit */
int
shp_hardtarget(struct shpstr *sp)
{
    struct sctstr sect;
    int vis, onsea;
    struct mchrstr *mcp = mchr + sp->shp_type;

    vis = shp_visib(sp);
    getsect(sp->shp_x, sp->shp_y, &sect);
    onsea = sect.sct_type == SCT_WATER;
    if (mcp->m_flags & M_SUB)
	vis *= 4;
    return (int)((sp->shp_effic / 100.0) *
		 (20 + shp_speed(sp) * onsea / 2.0 - vis));
}

static int
shp_hit_mine(struct shpstr *sp)
{
    double m;

    mpr(sp->shp_own, "Kawhomp! Mine detected in %s!\n",
	xyas(sp->shp_x, sp->shp_y, sp->shp_own));

    nreport(sp->shp_own, N_HIT_MINE, 0, 1);

    m = MINE_DAMAGE();
    if (mchr[sp->shp_type].m_flags & M_SWEEP)
	m /= 2.0;

    shipdamage(sp, ldround(m, 1));

    return (int)m;
}

int
shp_nav_one_sector(struct emp_qelem *list, int dir, natid actor,
		   int together)
{
    struct sctstr sect;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct ulist *mlp;
    struct emp_qelem done;
    coord dx;
    coord dy;
    coord newx;
    coord newy;
    int stopping = 0;
    double mobcost;
    char dp[80];
    int navigate;

    if (dir <= DIR_STOP || dir >= DIR_VIEW) {
	unit_put(list, actor);
	return 1;
    }
    dx = diroff[dir][0];
    dy = diroff[dir][1];
    for (qp = list->q_back; qp != list; qp = next) {
	next = qp->q_back;
	mlp = (struct ulist *)qp;
	newx = xnorm(mlp->unit.ship.shp_x + dx);
	newy = ynorm(mlp->unit.ship.shp_y + dy);
	getsect(newx, newy, &sect);
	navigate = shp_check_nav(&sect, &mlp->unit.ship);
	if (navigate != CN_NAVIGABLE ||
	    (sect.sct_own
	     && relations_with(sect.sct_own, actor) < FRIENDLY)) {
	    if (dchr[sect.sct_type].d_nav == NAV_CANAL &&
		!(((struct mchrstr *)mlp->chrp)->m_flags & M_CANAL) &&
		navigate == CN_LANDLOCKED)
		sprintf(dp,
			"is too large to fit into the canal system at %s",
			xyas(newx, newy, actor));
	    else
		sprintf(dp, "can't go to %s", xyas(newx, newy, actor));
	    if (together) {
		mpr(actor, "%s\n", dp);
		return 2;
	    } else {
		shp_stays(actor, dp, mlp);
		continue;
	    }
	}

	if (mlp->mobil <= 0.0) {
	    shp_stays(actor, "is out of mobility", mlp);
	    continue;
	}
	mobcost = shp_mobcost(&mlp->unit.ship);
	mlp->unit.ship.shp_x = newx;
	mlp->unit.ship.shp_y = newy;
	if (mlp->mobil - mobcost < -127) {
	    mlp->mobil = -127;
	} else {
	    mlp->mobil -= mobcost;
	}
	mlp->unit.ship.shp_mobil = (int)mlp->mobil;
	putship(mlp->unit.ship.shp_uid, &mlp->unit.ship);

	/* Now update the map for this ship */
	rad_map_set(mlp->unit.ship.shp_own,
		    mlp->unit.ship.shp_x, mlp->unit.ship.shp_y,
		    mlp->unit.ship.shp_effic, mlp->unit.ship.shp_tech,
		    ((struct mchrstr *)mlp->chrp)->m_vrnge);
    }
    if (QEMPTY(list))
	return stopping;
    stopping |= shp_sweep(list, 0, 0, actor);
    if (QEMPTY(list))
	return stopping;
    stopping |= shp_check_mines(list);
    if (QEMPTY(list))
	return stopping;

    /* interdict ships sector by sector */
    emp_initque(&done);
    while (!QEMPTY(list)) {
	mlp = (struct ulist *)list->q_back;
	newx = mlp->unit.ship.shp_x;
	newy = mlp->unit.ship.shp_y;
	stopping |= shp_interdict(list, newx, newy, actor);
	/* move survivors in this sector to done */
	for (qp = list->q_back; qp != list; qp = next) {
	    next = qp->q_back;
	    mlp = (struct ulist *)qp;
	    if (mlp->unit.ship.shp_x == newx &&
		mlp->unit.ship.shp_y == newy) {
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
    int hitchance, hit;
    double gun, eff, teff;

    snxtitem_dist(&ni, EF_SHIP, dx, dy, 1);

    while (nxtitem(&ni, &ship)) {
	if (!ship.shp_own)
	    continue;

	if (!(mchr[(int)ship.shp_type].m_flags & M_ANTIMISSILE))
	    continue;

	if (relations_with(ship.shp_own, bombown) >= NEUTRAL)
	    continue;

	if (ship.shp_effic < 60)
	    continue;

	if (ship.shp_item[I_MILIT] < 1)	/* do we have mil? */
	    continue;
	if (ship.shp_item[I_GUN] < 1)	/* we need at least 1 gun */
	    continue;
	if (!shp_supply(&ship, I_SHELL, 2))
	    continue;
	ship.shp_item[I_SHELL] -= 2;
	putship(ship.shp_uid, &ship);

	/* now calculate the odds */
	gun = shp_usable_guns(&ship);
	eff = ship.shp_effic / 100.0;
	teff = ship.shp_tech / (ship.shp_tech + 200.0);
	/* raise 4.5 for better interception -KHS */
	hitchance = (int)(gun * eff * teff * 4.5) - hardtarget;
	if (hitchance < 0)
	    hitchance = 0;
	if (hitchance > 100)
	    hitchance = 100;
	hit = pct_chance(hitchance);

	mpr(bombown, "%s anti-missile system activated...%s\n",
	    cname(ship.shp_own),
	    hit ? "KABOOOM!! Missile destroyed\n"
	    : "SWOOSH!!  anti-missile system failed!!");
	mpr(ship.shp_own, "Ship #%i anti-missile system activated!\n",
	    ship.shp_uid);
	mpr(ship.shp_own, "%d%% hitchance...%s\n", hitchance,
	    hit ? "KABOOOM!!  Incoming missile destroyed!\n"
	    : "SWOOSH!!  Missile evades anti-missile systems\n");

	if (hit)
	    return 1;
    }
    return 0;			/* all attempts failed */
}


/* Fire missiles at a ship which has fired shells */
void
shp_missdef(struct shpstr *sp, natid victim)
{
    struct emp_qelem list;
    struct ulist *mlp;
    int eff;
    char buf[512];

    emp_initque(&list);

    mlp = malloc(sizeof(struct ulist));
    mlp->chrp = (struct empobj_chr *)&mchr[(int)sp->shp_type];
    mlp->unit.ship = *sp;
    mlp->mobil = sp->shp_mobil;
    emp_insque(&mlp->queue, &list);
    sprintf(buf, "%s", prship(&mlp->unit.ship));

    eff = sp->shp_effic;
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
    if (!QEMPTY(&list))
	free(mlp);
}

double
shp_mobcost(struct shpstr *sp)
{
    return speed_factor(sp->shp_effic * 0.01 * shp_speed(sp),
			sp->shp_tech);
}

/*
 * Set SP's tech to TLEV along with everything else that depends on it.
 */
void
shp_set_tech(struct shpstr *sp, int tlev)
{
    struct mchrstr *mcp = mchr + sp->shp_type;

    if (CANT_HAPPEN(tlev < mcp->m_tech))
	tlev = mcp->m_tech;

    sp->shp_tech = tlev;
}
