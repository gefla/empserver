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
 *  cede.c: Give a sector to a neighbor
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Thomas Ruschak
 */

#include <config.h>

#include "commands.h"
#include "land.h"
#include "lost.h"
#include "nuke.h"
#include "path.h"
#include "plane.h"
#include "ship.h"

static void cede_hdr(void);
static int cede_sect(struct nstr_sect *, natid);
static int cede_ship(struct nstr_item *, natid);
static void grab_sect(struct sctstr *, natid);
static void grab_ship(struct shpstr *, natid);

int
cede(void)
{
    static int sct_or_shp[] = { EF_SECTOR, EF_SHIP, EF_BAD };
    natid to;
    int n;
    int is_sector = 0, is_ship = 0;
    char *p;
    struct nstr_sect ns;
    struct nstr_item ni;
    struct natstr *natp;
    char buf[1024];

    if ((p = getstarg(player->argp[1], "Cede what? ", buf)) == 0)
	return RET_SYN;
    if (snxtsct(&ns, p))
	is_sector = 1;
    if (snxtitem(&ni, EF_SHIP, p))
	is_ship = 1;
    if (!is_sector && !is_ship)
	return RET_SYN;
    if ((n = natarg(player->argp[2], "to which country? ")) < 0)
	return RET_SYN;
    if (is_sector && is_ship) {
	int type;

	if ((p =
	     getstarg(player->argp[3], "Cede sectors or ships (se, sh)? ",
		      buf)) == 0)
	    return RET_FAIL;
	if (strlen(p) > 4)
	    p[2] = 0;
	type = ef_byname_from(p, sct_or_shp);
	if (type < 0) {
	    pr("Please type 'se' or 'sh'!\n");
	    return RET_FAIL;
	}
    }


    if (n == player->cnum) {
	pr("Giving something to yourself?\n");
	return RET_FAIL;
    }
    natp = getnatp(n);
    if (natp->nat_stat != STAT_ACTIVE) {
	pr("You can only give to normal countries...\n");
	return RET_FAIL;
    }
    if (getrel(natp, player->cnum) < FRIENDLY) {
	pr("You can only cede to a country that is friendly towards you...\n");
	return RET_FAIL;
    }
    to = n;

    if (is_sector)
	return cede_sect(&ns, to);
    else
	return cede_ship(&ni, to);
}

static int
cede_sect(struct nstr_sect *ns, natid to)
{
    struct sctstr sect, osect;
    int nsect;
    int n, bad;
    int off_x, off_y;
    struct nstr_item ni;
    struct shpstr ship;

    prdate();
    nsect = 0;
    while (nxtsct(ns, &sect)) {
	if (!player->owner)
	    continue;
	if (sect.sct_mobil == 0) {
	    pr("%s has no mobility and cannot be ceded\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}

	bad = 1;
	for (n = 1; n <= 6; n++) {	/* Directions */
	    off_x = sect.sct_x + diroff[n][0];
	    off_y = sect.sct_y + diroff[n][1];

	    if (!getsect(off_x, off_y, &osect))
		continue;
	    if ((osect.sct_own == to) && (osect.sct_mobil))
		bad = 0;
	    if ((osect.sct_own == to) &&
		has_units_with_mob(osect.sct_x, osect.sct_y, to))
		bad = 0;
	}
	snxtitem_all(&ni, EF_SHIP);
	while (nxtitem(&ni, &ship)) {
	    if ((ship.shp_own == to) &&
		((ship.shp_x == sect.sct_x) && (ship.shp_y == sect.sct_y)))
		bad = 0;
	}
	if (bad) {
	    pr("%s has no sector with mobility adjacent to or ship in %s!\n",
	       cname(to), xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}

	if (nsect++ == 0)
	    cede_hdr();

	grab_sect(&sect, to);
	putsect(&sect);
	pr("  %s %d%% ceded\n", xyas(sect.sct_x, sect.sct_y, player->cnum),
	   (int)sect.sct_effic);
	wu(0, (natid)to, "%s ceded to you by %s\n",
	   xyas(sect.sct_x, sect.sct_y, to), cname(player->cnum));
    }
    pr("%d sector%s\n", nsect, splur(nsect));
    return RET_OK;
}

static void
cede_hdr(void)
{
    if (player->god)
	pr("own ");
    pr("  sect eff\n");
}



static void
grab_sect(struct sctstr *sp, natid to)
{
    struct plnstr *pp;
    struct lndstr *lp;
    struct nukstr *np;
    struct nstr_item ni;
    struct plnstr p;
    struct lndstr l;
    struct nukstr nuk;

    /* Wipe all the distribution info */
    memset(sp->sct_dist, 0, sizeof(sp->sct_dist));
    memset(sp->sct_del, 0, sizeof(sp->sct_del));
    sp->sct_dist_x = sp->sct_x;
    sp->sct_dist_y = sp->sct_y;

    pp = &p;
    snxtitem_xy(&ni, EF_PLANE, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, pp)) {
	if (pp->pln_own == 0)
	    continue;
	if (pp->pln_ship >= 0)
	    continue;
	if (pp->pln_own != player->cnum)
	    continue;
	if (pp->pln_flags & PLN_LAUNCHED)
	    continue;

	wu(0, to, "\t%s ceded to you by %s\n",
	   prplane(pp), cname(player->cnum));
	makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
	pp->pln_own = to;
	makenotlost(EF_PLANE, pp->pln_own, pp->pln_uid,
		    pp->pln_x, pp->pln_y);
	pp->pln_mobil = 0;
	pp->pln_mission = 0;
	putplane(pp->pln_uid, pp);
    }

    np = &nuk;
    snxtitem_xy(&ni, EF_NUKE, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, np)) {
	if (np->nuk_own == 0)
	    continue;

	wu(0, to, "\t%s ceded to you by %s\n",
	   prnuke(np), cname(player->cnum));
	makelost(EF_NUKE, np->nuk_own, np->nuk_uid, np->nuk_x, np->nuk_y);
	np->nuk_own = to;
	makenotlost(EF_NUKE, np->nuk_own, np->nuk_uid,
		    np->nuk_x, np->nuk_y);
	putnuke(ni.cur, np);
    }

    lp = &l;
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, lp)) {
	if (lp->lnd_own == 0)
	    continue;
	if (lp->lnd_ship == 0)
	    continue;
	if (lp->lnd_own != player->cnum)
	    continue;

	wu(0, to, "\t%s ceded to you by %s\n",
	   prland(lp), cname(player->cnum));
	makelost(EF_LAND, lp->lnd_own, lp->lnd_uid, lp->lnd_x, lp->lnd_y);
	makenotlost(EF_LAND, to, lp->lnd_uid, lp->lnd_x, lp->lnd_y);
	lp->lnd_own = to;
	lp->lnd_mobil = 0;
	lp->lnd_mission = 0;
	putland(ni.cur, lp);
    }

    sp->sct_avail = 0;

    if (sp->sct_oldown == to) {
	sp->sct_che = 0;	/* FIXME where do these guys go? */
	sp->sct_che_target = 0;
	sp->sct_loyal = 0;
    }

    if (sp->sct_oldown == to)
	sp->sct_loyal = 0;
    else			/* people don't like being given away */
	sp->sct_loyal = 50;

    sp->sct_dist_x = sp->sct_x;
    sp->sct_dist_y = sp->sct_y;
    makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
    makenotlost(EF_SECTOR, to, 0, sp->sct_x, sp->sct_y);
    if (sp->sct_oldown == sp->sct_own)
	sp->sct_oldown = to;
    sp->sct_own = to;
    sp->sct_mobil = 0;
}

static int
cede_ship(struct nstr_item *ni, natid to)
{
    struct shpstr ship;
    struct shpstr tship;
    struct sctstr sect;
    struct nstr_item tni;
    int nships = 0;
    int bad = 0;

    while (nxtitem(ni, &ship)) {

	if (!player->owner || ship.shp_own == 0)
	    continue;

	bad = 1;
	snxtitem_xy(&tni, EF_SHIP, ship.shp_x, ship.shp_y);
	while (nxtitem(&tni, &tship) && bad)
	    if (tship.shp_own == to)
		bad = 0;

	getsect(ship.shp_x, ship.shp_y, &sect);
	if (bad && (sect.sct_own != to)) {
	    pr("%s isn't in a %s sector, and is not with a %s ship!\n",
	       prship(&ship), cname(to), cname(to));
	    continue;
	}
	grab_ship(&ship, to);
	putship(ship.shp_uid, &ship);
	nships++;
	wu(0, to, "%s ceded to you by %s\n",
	   prship(&ship), cname(player->cnum));
	pr("%s ceded to %s\n", prship(&ship), cname(to));
    }
    pr("    %d ship%s\n", nships, splur(nships));

    return RET_OK;
}

static void
grab_ship(struct shpstr *sp, natid to)
{
    struct plnstr *pp;
    struct lndstr *lp;
    struct nstr_item ni;
    struct plnstr p;
    struct lndstr l;

    pp = &p;
    snxtitem_xy(&ni, EF_PLANE, sp->shp_x, sp->shp_y);
    while (nxtitem(&ni, pp)) {
	if (pp->pln_own == 0)
	    continue;
	if (pp->pln_flags & PLN_LAUNCHED)
	    continue;
	if (pp->pln_ship != sp->shp_uid)
	    continue;
	if (pp->pln_own != player->cnum)
	    continue;

	wu(0, to, "\t%s ceded to you by %s\n",
	   prplane(pp), cname(player->cnum));
	makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
	pp->pln_own = to;
	makenotlost(EF_PLANE, pp->pln_own, pp->pln_uid,
		    pp->pln_x, pp->pln_y);
	pp->pln_mobil = 0;
	pp->pln_mission = 0;
	putplane(pp->pln_uid, pp);
    }

    lp = &l;
    snxtitem_xy(&ni, EF_LAND, sp->shp_x, sp->shp_y);
    while (nxtitem(&ni, lp)) {
	if (lp->lnd_own == 0)
	    continue;
	if (lp->lnd_ship != sp->shp_uid)
	    continue;
	if (lp->lnd_own != player->cnum)
	    continue;

	wu(0, to, "\t%s ceded to you by %s\n",
	   prland(lp), cname(player->cnum));
	makelost(EF_LAND, lp->lnd_own, lp->lnd_uid, lp->lnd_x, lp->lnd_y);
	makenotlost(EF_LAND, to, lp->lnd_uid, lp->lnd_x, lp->lnd_y);
	lp->lnd_own = to;
	lp->lnd_mobil = 0;
	lp->lnd_mission = 0;
	putland(ni.cur, lp);
    }

    makelost(EF_SHIP, sp->shp_own, sp->shp_uid, sp->shp_x, sp->shp_y);
    sp->shp_own = to;
    makenotlost(EF_SHIP, sp->shp_own, sp->shp_uid, sp->shp_x, sp->shp_y);
}
