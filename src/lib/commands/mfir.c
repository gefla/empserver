/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  multifire.c: Fire at other sectors/ships
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2015
 */

#include <config.h>

#include "chance.h"
#include "commands.h"
#include "empobj.h"
#include "news.h"
#include "optlist.h"
#include "retreat.h"

enum targ_type {	/* Targeting... */
    targ_land,		/* a sector with guns */
    targ_ship,		/* a ship with guns */
    targ_sub,		/* a submarine with depth charges */
    targ_bogus		/* a bogus sector with guns */
};

struct flist {
    struct emp_qelem queue;	/* list of fired things */
    short type;			/* EF_SECTOR, EF_SHIP or EF_LAND */
    int uid;
    coord x, y;
    int defdam;			/* damage defenders did */
    natid victim;
};

static int defend(struct emp_qelem *, struct emp_qelem *,
		  struct empobj *, natid, int *);
static void do_defdam(struct emp_qelem *, double);
static int quiet_bigdef(int, struct emp_qelem *, natid, natid, coord,
			coord, int *);
static void add_to_flist(struct emp_qelem *, struct empobj *, int, natid);
static void free_flist(struct emp_qelem *);
static struct flist *search_flist(struct emp_qelem *, struct empobj *);

int
multifire(void)
{
    static int ef_with_guns[] = { EF_SECTOR, EF_SHIP, EF_LAND, EF_BAD };
    char *ptr;
    double range;
    int trange, range2;
    coord fx;
    coord fy;
    coord x;
    coord y;
    int dam;
    int totaldefdam = 0;
    int vshipno;
    natid vict;
    struct shpstr fship;
    struct lndstr fland;
    struct sctstr fsect;
    char *sep = "";
    struct shpstr vship;
    struct sctstr vsect;
    enum targ_type target;
    struct nstr_item nbst;
    int type;
    struct empobj *attgp;
    char *p;
    int nfiring = 0;
    int ndefending = 0;
    union empobj_storage item;
    struct emp_qelem fired, defended;
    double odds;
    char buf[1024];

    emp_initque(&fired);
    emp_initque(&defended);
    p = getstarg(player->argp[1],
		 "Firing from ship(s), sect(s), or land unit(s)? ", buf);
    if (!p)
	return RET_SYN;
    type = ef_byname_from(p, ef_with_guns);
    if (opt_NO_FORT_FIRE && type == EF_SECTOR) {
	pr("Fort firing is disabled.\n");
	return RET_FAIL;
    }
    if (type < 0) {
	pr("Ships, land units or sectors only!\n");
	return RET_SYN;
    }
    if (!snxtitem(&nbst, type, player->argp[2], "Firing from? "))
	return RET_SYN;

    while (nxtitem(&nbst, &item)) {
	if (type == EF_LAND) {
	    if (!getland(item.land.lnd_uid, &fland))
		continue;
	    if (!getsect(item.land.lnd_x, item.land.lnd_y, &fsect))
		continue;
	    if (item.land.lnd_own != player->cnum)
		continue;

	    if (lchr[fland.lnd_type].l_dam == 0) {
		pr("Unit %d cannot fire!\n", fland.lnd_uid);
		continue;
	    }
	    if (fland.lnd_item[I_MILIT] < 1) {
		pr("Unit %d cannot fire because it has no military!\n",
		   fland.lnd_uid);
		continue;
	    }
	    if (fland.lnd_ship >= 0) {
		pr("Unit %d cannot fire because it is on a ship!\n",
		   fland.lnd_uid);
		continue;
	    }
	    if (fland.lnd_land >= 0) {
		pr("Unit %d cannot fire because it is on a land unit!\n",
		   fland.lnd_uid);
		continue;
	    }
	    if (fland.lnd_effic < LAND_MINFIREEFF) {
		pr("Unit %d cannot fire because it is less than %d%% efficient\n",
		   fland.lnd_uid, LAND_MINFIREEFF);
		continue;
	    }
	    if (fland.lnd_item[I_GUN] == 0) {
		pr("%s -- not enough guns\n", prland(&fland));
		continue;
	    }

	    if (fland.lnd_item[I_SHELL] == 0) {
		pr("%s -- not enough shells\n", prland(&fland));
		continue;
	    }
	    pr("%s%s ready to fire\n", sep, prland(&fland));
	    fx = fland.lnd_x;
	    fy = fland.lnd_y;
	} else if (type == EF_SHIP) {
	    if (!getship(item.ship.shp_uid, &fship))
		continue;
	    if (item.ship.shp_own != player->cnum)
		continue;
	    if (item.ship.shp_item[I_MILIT] < 1) {
		pr("Not enough mil on ship #%d\n", item.ship.shp_uid);
		continue;
	    }
	    if (mchr[item.ship.shp_type].m_glim == 0) {
		pr("Ships %d cannot fire guns!\n", item.ship.shp_uid);
		continue;
	    }
	    if (item.ship.shp_item[I_GUN] == 0) {
		pr("Not enough guns on ship #%d\n", item.ship.shp_uid);
		continue;
	    }
	    if (item.ship.shp_item[I_SHELL] == 0) {
		pr("Not enough shells on ship #%d\n", item.ship.shp_uid);
		continue;
	    }
	    if (item.ship.shp_effic < 60) {
		pr("Ship #%d is crippled!\n", item.ship.shp_uid);
		continue;
	    }
	    pr("%s%s ready to fire\n", sep, prship(&fship));
	    fx = fship.shp_x;
	    fy = fship.shp_y;
	} else {
	    if (!getsect(item.sect.sct_x, item.sect.sct_y, &fsect))
		continue;
	    if (item.sect.sct_own != player->cnum)
		continue;
	    if (item.sect.sct_type != SCT_FORTR)
		continue;
	    if (item.sect.sct_effic < FORTEFF) {
		pr("Fort not efficient enough to fire!\n");
		continue;
	    }
	    if (item.sect.sct_item[I_GUN] == 0) {
		pr("Not enough guns in sector %s!\n",
		   xyas(item.sect.sct_x, item.sect.sct_y, player->cnum));
		continue;
	    }
	    if (item.sect.sct_item[I_SHELL] == 0) {
		pr("Not enough shells in sector %s!\n",
		   xyas(item.sect.sct_x, item.sect.sct_y, player->cnum));
		continue;
	    }
	    if (item.sect.sct_item[I_MILIT] < 5) {
		pr("Not enough military in sector %s!\n",
		   xyas(item.sect.sct_x, item.sect.sct_y, player->cnum));
		continue;
	    }
	    pr("%sSector %s ready to fire\n", sep,
	       xyas(item.sect.sct_x, item.sect.sct_y, player->cnum));
	    fx = fsect.sct_x;
	    fy = fsect.sct_y;
	}
	sep = "\n";

	ptr = getstarg(player->argp[3], "Firing at? ", buf);
	if (!ptr)
	    return RET_FAIL;
	if (!*ptr)
	    continue;
	if (!issector(ptr)) {
	    vshipno = atoi(ptr);
	    if (vshipno < 0 || !getship(vshipno, &vship) ||
		(!vship.shp_own)) {
		pr("No such ship exists!\n");
		continue;
	    }
	    target = targ_ship;	/* targ_ship vs. targ_sub decided below */
	    vict = vship.shp_own;
	    x = vship.shp_x;
	    y = vship.shp_y;
	    if (!getsect(x, y, &vsect)) {
		pr("No such sector exists!\n");
		continue;
	    }
	} else {
	    if (!sarg_xy(ptr, &x, &y) || !getsect(x, y, &vsect)) {
		pr("No such sector exists!\n");
		continue;
	    }
	    /* We check the sector type, but we only use it for damage, not
	       reporting.  That way, you don't get extra information you wouldn't
	       normally get.  Besides, what if they want to slam water? :)  */
	    if (vsect.sct_type == SCT_SANCT || vsect.sct_type == SCT_WATER)
		target = targ_bogus;
	    else
		target = targ_land;
	    vict = vsect.sct_own;
	    x = vsect.sct_x;
	    y = vsect.sct_y;
	}

	trange = mapdist(x, y, fx, fy);

	if (type == EF_SHIP) {
	    if (!check_ship_ok(&fship))
		return RET_FAIL;
	    if (target == targ_ship) {
		if (fship.shp_uid == vship.shp_uid) {
		    pr("You can't fire upon yourself!\n");
		    continue;
		}
	    }
	    range = shp_fire_range(&fship);
	    range2 = roundrange(range);
	    /* Use depth charges against subs, but only when in range */
	    if (target == targ_ship && trange <= range2
		&& (mchr[vship.shp_type].m_flags & M_SUB)
		&& (mchr[fship.shp_type].m_flags & M_DCH))
		target = targ_sub;
	    if (target == targ_sub)
		dam = shp_dchrg(&fship);
	    else
		dam = shp_fire(&fship);
	    fship.shp_mission = 0;
	    putship(fship.shp_uid, &fship);
	    if (opt_NOMOBCOST == 0) {
		fship.shp_mobil = MAX(fship.shp_mobil - 15, -100);
		putship(fship.shp_uid, &fship);
	    }
	} else if (type == EF_LAND) {
	    if (!check_land_ok(&fland))
		return RET_FAIL;
	    if (target == targ_land) {
		if (fland.lnd_x == vsect.sct_x
		    && fland.lnd_y == vsect.sct_y) {
		    pr("You can't fire upon yourself!\n");
		    continue;
		}
	    }
	    range = lnd_fire_range(&fland);
	    range2 = roundrange(range);
	    dam = lnd_fire(&fland);
	    fland.lnd_mission = 0;
	    putland(fland.lnd_uid, &fland);
	    if (target == targ_ship) {
		if (chance(lnd_acc(&fland) / 100.0))
		    dam = ldround(dam / 2.0, 1);
	    }
	} else {
	    if (!check_sect_ok(&fsect))
		return RET_FAIL;
	    if (target == targ_land) {
		if (fsect.sct_x == vsect.sct_x
		    && fsect.sct_y == vsect.sct_y) {
		    pr("You can't fire upon yourself!\n");
		    continue;
		}
	    }
	    dam = fort_fire(&fsect);
	    putsect(&fsect);
	    range = fortrange(&fsect);
	    range2 = roundrange(range);
	}

	if (CANT_HAPPEN(dam < 0)) {
	    pr("Jammed!\n");
	    continue;
	}
	pr("range is %d.00 (%.2f)\n", range2, range);
	nfiring++;
	switch (target) {
	case targ_sub:
	    pr_beep();
	    pr("Kawhomp!!!\n");
	    break;
	default:
	    pr_beep();
	    pr("Kaboom!!!\n");
	    break;
	}

	/*
	 * If the player fires guns at a submarine, take care not to
	 * disclose it's a submarine: pretend the target is out of range.
	 */
	if (target == targ_ship && (mchr[vship.shp_type].m_flags & M_SUB))
	    range2 = -1;
	if (trange > range2) {
	    pr("Target out of range.\n");
	    continue;
	}

	switch (target) {
	case targ_bogus:
	case targ_land:
	    nreport(player->cnum, N_SCT_SHELL, vict, 1);
	    if (vict && vict != player->cnum)
		wu(0, vict,
		   "Country #%d shelled sector %s for %d damage.\n",
		   player->cnum, xyas(x, y, vict), dam);
	    pr("Shells hit sector %s for %d damage.\n",
	       xyas(x, y, player->cnum), dam);
	    break;
	case targ_ship:
	    nreport(player->cnum, N_SHP_SHELL, vict, 1);
	    /* fall through */
	default:
	    if (vict && vict != player->cnum) {
		wu(0, vict,
		   "Country #%d shelled %s in %s for %d damage.\n",
		   player->cnum, prship(&vship),
		   xyas(vship.shp_x, vship.shp_y, vict), dam);
	    }
	    pr("Shells hit %s in %s for %d damage.\n",
	       prsub(&vship),
	       xyas(vship.shp_x, vship.shp_y, player->cnum), dam);
	    break;
	}
	/*  Ok, now, check if we had a bogus target.  If so,
	   just continue on, since there is no defender. */
	if (target == targ_bogus)
	    continue;
	attgp = &item.gen;
	if (type == EF_LAND) {
	    getsect(fland.lnd_x, fland.lnd_y, &fsect);
	    attgp = (struct empobj *)&fsect;
	}
	totaldefdam = defend(&fired, &defended, attgp, vict, &ndefending);
	switch (target) {
	case targ_land:
	    getsect(x, y, &vsect);
	    sectdamage(&vsect, dam);
	    putsect(&vsect);
	    break;
	default:
	    getship(vshipno, &vship);
	    shipdamage(&vship, dam);
	    if (vship.shp_effic < SHIP_MINEFF)
		pr("%s sunk!\n", prsub(&vship));
	    putship(vship.shp_uid, &vship);
	    if (dam && (vship.shp_rflags & RET_INJURED))
		retreat_ship(&vship, vict, 'i');
	    else if (target == targ_sub && (vship.shp_rflags & RET_DCHRGED))
		retreat_ship(&vship, vict, 'd');
	    else if (totaldefdam == 0 && (vship.shp_rflags & RET_HELPLESS))
		retreat_ship(&vship, vict, 'h');
	    break;
	}
	switch (attgp->ef_type) {
	case EF_SECTOR:
	    break;
	case EF_SHIP:
	    if ((target == targ_ship) || (target == targ_sub)) {
		if (fship.shp_effic > SHIP_MINEFF) {
		    shp_missdef(&fship, vict);
		}
	    }
	    break;
	default:
	    CANT_REACH();
	}
    }

    free_flist(&defended);
    if (nfiring)
	odds = ((double)ndefending) / ((double)nfiring);
    else
	odds = 1.0;
    do_defdam(&fired, odds);
    return RET_OK;
}

static int
defend(struct emp_qelem *al, struct emp_qelem *dl,
       struct empobj *attgp, natid vict, int *nd)
{
    int dam;
    int nfiring = 0;

    dam = quiet_bigdef(attgp->ef_type, dl, vict,
		       attgp->own, attgp->x, attgp->y, &nfiring);
    if (dam) {
	if (nfiring > *nd)
	    *nd = nfiring;
	add_to_flist(al, attgp, dam, vict);
    }

    return dam;
}

static void
do_defdam(struct emp_qelem *list, double odds)
{

    int dam, first = 1;
    natid vict;
    struct flist *fp;
    struct shpstr ship;
    struct sctstr sect;
    struct emp_qelem *qp, *next;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	fp = (struct flist *)qp;
	if (fp->type == EF_SHIP) {
	    if (!getship(fp->uid, &ship) || !ship.shp_own)
		continue;
	}
	if (first) {
	    pr_beep();
	    pr("\nDefenders fire back!\n");
	    first = 0;
	}
	dam = odds * fp->defdam;

	if (fp->type == EF_SHIP) {
	    vict = fp->victim;
	    pr("Return fire hit %s in %s for %d damage.\n",
	       prship(&ship),
	       xyas(ship.shp_x, ship.shp_y, player->cnum), dam);
	    if (vict)
		wu(0, vict,
		   "Return fire hit %s in %s for %d damage.\n",
		   prsub(&ship), xyas(ship.shp_x, ship.shp_y, vict), dam);
	    shipdamage(&ship, dam);
	    putship(ship.shp_uid, &ship);
	} else {
	    CANT_HAPPEN(fp->type != EF_SECTOR);
	    getsect(fp->x, fp->y, &sect);
	    vict = fp->victim;
	    pr("Return fire hit sector %s for %d damage.\n",
	       xyas(fp->x, fp->y, player->cnum), dam);
	    sectdamage(&sect, dam);
	    putsect(&sect);
	    if (vict)
		wu(0, vict, "Return fire hit sector %s for %d damage.\n",
		   xyas(fp->x, fp->y, vict), dam);
	}
	emp_remque(&fp->queue);
	free(fp);
    }
}

static int
quiet_bigdef(int type, struct emp_qelem *list, natid own, natid aown,
	     coord ax, coord ay, int *nfiring)
{
    double erange;
    struct shpstr ship;
    struct lndstr land;
    struct nstr_item ni;
    int dam, dam2;
    struct sctstr firing;
    struct nstr_sect ns;
    struct flist *fp;

    if (own == 0)
	return 0;
    dam = 0;
    snxtitem_dist(&ni, EF_SHIP, ax, ay, 8);
    while (nxtitem(&ni, &ship)) {
	if (!feels_like_helping(ship.shp_own, own, aown))
	    continue;

	if ((mchr[ship.shp_type].m_flags & M_SUB) && type != EF_SHIP)
	    continue;

	if (mchr[(int)ship.shp_type].m_flags & M_SUB) {
	    erange = torprange(&ship);
	    if (roundrange(erange) < ni.curdist)
		continue;
	    if (!line_of_sight(NULL, ship.shp_x, ship.shp_y, ax, ay))
		continue;
	    fp = search_flist(list, (struct empobj *)&ship);
	    if (fp)
		dam2 = fp->defdam;
	    else {
		dam2 = shp_torp(&ship, 0);
		putship(ship.shp_uid, &ship);
	    }
	    if (dam2 < 0)
		continue;
	    if (!chance(shp_torp_hitchance(&ship, ni.curdist)))
		dam2 = 0;
	} else {
	    erange = shp_fire_range(&ship);
	    if (roundrange(erange) < ni.curdist)
		continue;
	    fp = search_flist(list, (struct empobj *)&ship);
	    if (fp)
		dam2 = fp->defdam;
	    else {
		dam2 = shp_fire(&ship);
		putship(ship.shp_uid, &ship);
	    }
	    if (dam2 < 0)
		continue;
	    nreport(ship.shp_own, N_FIRE_BACK, player->cnum, 1);
	}
	(*nfiring)++;
	if (!fp)
	    add_to_flist(list, (struct empobj *)&ship, dam2, 0);
	dam += dam2;
    }
    snxtitem_dist(&ni, EF_LAND, ax, ay, 8);
    while (nxtitem(&ni, &land)) {
	if (!feels_like_helping(land.lnd_own, own, aown))
	    continue;

	erange = lnd_fire_range(&land);
	if (roundrange(erange) < ni.curdist)
	    continue;

	fp = search_flist(list, (struct empobj *)&land);
	if (fp)
	    dam2 = fp->defdam;
	else {
	    dam2 = lnd_fire(&land);
	    putland(land.lnd_uid, &land);
	}
	if (dam2 < 0)
	    continue;

	(*nfiring)++;
	if (!fp)
	    add_to_flist(list, (struct empobj *)&land, dam2, 0);
	nreport(land.lnd_own, N_FIRE_BACK, player->cnum, 1);
	if (type == EF_SHIP) {
	    if (chance(lnd_acc(&land) / 100.0))
		dam2 = ldround(dam2 / 2.0, 1);
	}
	dam += dam2;
    }

    /*
     * Determine if any nearby gun-equipped sectors are within
     * range and able to fire at an attacker.  Firing sectors
     * need to have guns, shells, and military.  Sector being
     * attacked is x,y -- attacker is at ax,ay.
     */

    if (!opt_NO_FORT_FIRE) {
	snxtsct_dist(&ns, ax, ay, 8);
	while (nxtsct(&ns, &firing)) {
	    if (!feels_like_helping(firing.sct_own, own, aown))
		continue;

	    erange = fortrange(&firing);
	    if (roundrange(erange) < ns.curdist)
		continue;

	    fp = search_flist(list, (struct empobj *)&firing);
	    if (fp)
		dam2 = fp->defdam;
	    else {
		dam2 = fort_fire(&firing);
		putsect(&firing);
	    }
	    if (dam2 < 0)
		continue;
	    (*nfiring)++;
	    if (!fp)
		add_to_flist(list, (struct empobj *)&firing, dam2, 0);
	    nreport(firing.sct_own, N_FIRE_BACK, player->cnum, 1);
	    dam += dam2;
	}
    }

    return *nfiring == 0 ? 0 : dam / *nfiring;
}

static void
add_to_flist(struct emp_qelem *list,
	     struct empobj *gp, int dam, natid victim)
{
    struct flist *fp;

    fp = malloc(sizeof(struct flist));
    fp->type = gp->ef_type;
    fp->uid = gp->uid;
    fp->x = gp->x;
    fp->y = gp->y;
    fp->defdam = dam;
    fp->victim = victim;
    emp_insque(&fp->queue, list);
}

static void
free_flist(struct emp_qelem *list)
{
    struct emp_qelem *qp, *next;
    struct flist *fp;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	fp = (struct flist *)qp;
	emp_remque(&fp->queue);
	free(fp);
    }
}

static int
uid_eq(struct emp_qelem *elem, void *key)
{
    return ((struct flist *)elem)->uid == ((struct empobj *)key)->uid;
}

static struct flist *
search_flist(struct emp_qelem *list, struct empobj *gp)
{
    return (struct flist *)emp_searchque(list, gp, uid_eq);
}
