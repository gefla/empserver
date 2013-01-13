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
 *  edit.c: Edit things (sectors, ships, planes, units, nukes, countries)
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff
 *     Chad Zabel, 1994
 *     Steve McClure, 1998-2000
 *     Ron Koenderink, 2003-2009
 *     Markus Armbruster, 2003-2013
 */

#include <config.h>

#include <ctype.h>
#include <limits.h>
#include "commands.h"
#include "item.h"
#include "land.h"
#include "news.h"
#include "optlist.h"
#include "plague.h"
#include "plane.h"
#include "ship.h"

#define END -1

static int docountry(char, int, char *, struct natstr *);
static int doland(char, int, char *, struct sctstr *);
static int doplane(char, int, char *, struct plnstr *);
static int doship(char, int, char *, struct shpstr *);
static int dounit(char, int, char *, struct lndstr *);
static int getin(char *, char **);
static void pr_land(struct lndstr *);
static void pr_plane(struct plnstr *);
static void pr_ship(struct shpstr *);
static void prnat(struct natstr *);
static void prsect(struct sctstr *);


int
edit(void)
{
    struct sctstr sect;
    struct plnstr plane;
    struct shpstr ship;
    struct lndstr land;
    char *what;
    char *ptr;
    char thing;
    int num;
    int arg;
    int err;
    int arg_index = 3;
    coord x, y;
    struct natstr *np;
    char buf[1024];
    char ewhat;

    what = getstarg(player->argp[1],
		    "Edit what (country, land, ship, plane, nuke, unit)? ",
		    buf);
    if (!what)
	return RET_SYN;
    ewhat = what[0];
    switch (ewhat) {
    case 'l':
	if (!(ptr = getstarg(player->argp[2], "Sector : ", buf)))
	    return RET_FAIL;
	if (!sarg_xy(ptr, &x, &y))
	    return RET_FAIL;
	if (!getsect(x, y, &sect))
	    return RET_FAIL;
	break;
    case 'c':
	np = natargp(player->argp[2], "Country? ");
	if (!np)
	    return RET_SYN;
	break;
    case 'p':
	if ((num = onearg(player->argp[2], "Plane number? ")) < 0)
	    return RET_SYN;
	if (!getplane(num, &plane))
	    return RET_SYN;
	break;
    case 's':
	if ((num = onearg(player->argp[2], "Ship number? ")) < 0)
	    return RET_SYN;
	if (!getship(num, &ship))
	    return RET_SYN;
	break;
    case 'u':
	if ((num = onearg(player->argp[2], "Unit number? ")) < 0)
	    return RET_SYN;
	if (!getland(num, &land))
	    return RET_SYN;
	break;
    case 'n':
	pr("Not implemented yet.\n");
	break;
    default:
	pr("huh?\n");
	return RET_SYN;
    }
    if (!player->argp[3]) {
	switch (ewhat) {
	case 'l':
	    prsect(&sect);
	    break;
	case 'c':
	    prnat(np);
	    break;
	case 'p':
	    pr_plane(&plane);
	    break;
	case 's':
	    pr_ship(&ship);
	    break;
	case 'u':
	    pr_land(&land);
	    break;
	}
    }
    for (;;) {
	if (player->argp[arg_index]) {
	    if (player->argp[arg_index+1]) {
		thing = player->argp[arg_index++][0];
		ptr = player->argp[arg_index++];
		arg = atoi(ptr);
	    } else
		return RET_SYN;
	} else if (arg_index == 3) {
	    err = getin(buf, &ptr);
	    if (err < 0)
		return RET_SYN;
	    if (err == 0) {
		switch (ewhat) {
		case 'c':
		    prnat(np);
		    break;
		case 'l':
		    prsect(&sect);
		    break;
		case 's':
		    pr_ship(&ship);
		    break;
		case 'u':
		    pr_land(&land);
		    break;
		case 'p':
		    pr_plane(&plane);
		    break;
		}
		return RET_OK;
	    }
	    thing = err;
	    arg = atoi(ptr);
	} else
	    return RET_OK;

	switch (ewhat) {
	case 'c':
	    if ((err = docountry(thing, arg, ptr, np)) != RET_OK)
		return err;
	    break;
	case 'l':
	    if (!check_sect_ok(&sect))
		return RET_FAIL;
	    if ((err = doland(thing, arg, ptr, &sect)) != RET_OK)
		return err;
	    if (!putsect(&sect))
		return RET_FAIL;
	    break;
	case 's':
	    if (!check_ship_ok(&ship))
		return RET_FAIL;
	    if ((err = doship(thing, arg, ptr, &ship)) != RET_OK)
		return err;
	    if (!ef_ensure_space(EF_SHIP, ship.shp_uid, 50))
		return RET_FAIL;
	    if (!putship(ship.shp_uid, &ship))
		return RET_FAIL;
	    break;
	case 'u':
	    if (!check_land_ok(&land))
		return RET_FAIL;
	    if ((err = dounit(thing, arg, ptr, &land)) != RET_OK)
		return err;
	    if (!ef_ensure_space(EF_LAND, land.lnd_uid, 50))
		return RET_FAIL;
	    if (!putland(land.lnd_uid, &land))
		return RET_FAIL;
	    break;
	case 'p':
	    if (!check_plane_ok(&plane))
		return RET_FAIL;
	    if ((err = doplane(thing, arg, ptr, &plane)) != RET_OK)
		return err;
	    if (!ef_ensure_space(EF_PLANE, plane.pln_uid, 50))
		return RET_FAIL;
	    if (!putplane(plane.pln_uid, &plane))
		return RET_FAIL;
	    break;
	}
    }
}

static void
benefit(natid who, int good)
{
    if (!opt_GODNEWS)
	return;

    if (good) {
	if (who)
	    nreport(player->cnum, N_AIDS, who, 1);
    } else {
	if (who)
	    nreport(player->cnum, N_HURTS, who, 1);
    }
}

static void
noise(struct sctstr *sptr, char *name, int old, int new)
{
    pr("%s of %s changed from %d to %d\n",
       name, xyas(sptr->sct_x, sptr->sct_y, player->cnum), old, new);
    if (sptr->sct_own)
	wu(player->cnum, sptr->sct_own,
	   "%s in %s was changed from %d to %d by an act of %s\n",
	   name, xyas(sptr->sct_x, sptr->sct_y, sptr->sct_own),
	   old, new, cname(player->cnum));
    benefit(sptr->sct_own, old < new);
}

static void
prsect(struct sctstr *sect)
{
    pr("Location <L>: %s\t", xyas(sect->sct_x, sect->sct_y, player->cnum));
    pr("Distribution sector <D>: %s\n",
       xyas(sect->sct_dist_x, sect->sct_dist_y, player->cnum));
    pr("Designation <s>: %c\tNew designation <S>: %c\n",
       dchr[sect->sct_type].d_mnem, dchr[sect->sct_newtype].d_mnem);
    pr("own  oo eff mob min gld frt oil urn wrk lty che ctg plg ptime fall avail\n");
    pr("  o   O   e   m   i   g   f   c   u   w   l   x   X   p     t    F     a\n");
    pr("%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %5d %4d %5d\n",
       sect->sct_own, sect->sct_oldown, sect->sct_effic, sect->sct_mobil,
       sect->sct_min, sect->sct_gmin, sect->sct_fertil, sect->sct_oil,
       sect->sct_uran, sect->sct_work, sect->sct_loyal,
       sect->sct_che, sect->sct_che_target,
       sect->sct_pstage, sect->sct_ptime,
       sect->sct_fallout, sect->sct_avail);

    pr("Mines <M>: %d\n", sect->sct_mines);
    pr("Road %% <R>: %d\t", sect->sct_road);
    pr("Rail %% <r>: %d\t", sect->sct_rail);
    pr("Defense %% <d>: %d\n", sect->sct_defense);
}


static void
prnat(struct natstr *np)
{
    int i;

    pr("Country #: %2d\n", np->nat_cnum);
    pr("Name <n>: %-20s\t", np->nat_cnam);
    pr("Representative <r>: %-20s\n", np->nat_pnam);
    pr("BTUs <b>: %3d\t\t\t", np->nat_btu);
    pr("Reserves <m>: %5d\n", np->nat_reserve);
    pr("Capital <c>: %s\t\t",
       xyas(np->nat_xcap, np->nat_ycap, player->cnum));
    pr("Origin <o>: %3s\n",
       xyas(np->nat_xorg, np->nat_yorg, player->cnum));
    pr("Status <s>: 0x%x\t\t\t", np->nat_stat);
    pr("Seconds Used <u>: %3d\n", np->nat_timeused);
    pr("Technology <T>: %.2f\t\t", np->nat_level[NAT_TLEV]);
    pr("Research <R>: %.2f\n", np->nat_level[NAT_RLEV]);
    pr("Education <E>: %.2f\t\t", np->nat_level[NAT_ELEV]);
    pr("Happiness <H>: %.2f\n", np->nat_level[NAT_HLEV]);
    pr("Money <M>: $%6d\n", np->nat_money);
    pr("Telegrams <t>: %6d\n", np->nat_tgms);
    if (opt_HIDDEN) {
	pr("Countries contacted: ");
	for (i = 0; i < MAXNOC; i++) {
	    if (getcontact(np, i))
		pr("%d(%d) ", i, getcontact(np, i));
	}
	pr("\n");
    }
}

static void
pr_plane(struct plnstr *plane)
{
    pr("UID <U>: %d\t\t", plane->pln_uid);
    pr("Owner <O>: %d\t\t", plane->pln_own);
    pr("Location <l>: %s\n",
       xyas(plane->pln_x, plane->pln_y, player->cnum));
    pr("Efficiency <e>: %d\t", plane->pln_effic);
    pr("Mobility <m>: %d\n", plane->pln_mobil);
    pr("Tech <t>: %d\t\t", plane->pln_tech);
    pr("Wing <w>: %.1s\n", &plane->pln_wing);
    pr("Range <r>: %d\t\t", plane->pln_range);
    pr("Flags <f>: %d\n", plane->pln_flags);
    pr("Ship <s>: %d\t\t", plane->pln_ship);
    pr("Land Unit <y>: %d\t", plane->pln_land);
}

static void
pr_land(struct lndstr *land)
{
    pr("UID <U>: %d\n", land->lnd_uid);
    pr("Owner <O>: %d\n", land->lnd_own);
    pr("Location <L>: %s\n", xyas(land->lnd_x, land->lnd_y, player->cnum));
    pr("Efficiency <e>: %d\t", land->lnd_effic);
    pr("Mobility <M>: %d\n", land->lnd_mobil);
    pr("Tech <t>: %d\t\t", land->lnd_tech);
    pr("Army <a>: %.1s\n", &land->lnd_army);
    pr("Fortification <F>: %d\t", land->lnd_harden);
    pr("Land unit <Y>: %d\n", land->lnd_land);
    pr("Ship <S>: %d\t\t", land->lnd_ship);
    pr("Retreat percentage <Z>: %d\n", land->lnd_retreat);
    pr("Retreat path <R>: '%s'\t\tRetreat Flags <W>: %d\n",
       land->lnd_rpath, land->lnd_rflags);
    pr("civ mil  uw food shl gun  pet  irn  dst  oil  lcm  hcm rad\n");
    pr("  c   m   u    f   s   g    p    i    d    o    l    h   r\n");
    pr("%3d", land->lnd_item[I_CIVIL]);
    pr("%4d", land->lnd_item[I_MILIT]);
    pr("%4d", land->lnd_item[I_UW]);
    pr("%5d", land->lnd_item[I_FOOD]);
    pr("%4d", land->lnd_item[I_SHELL]);
    pr("%4d", land->lnd_item[I_GUN]);
    pr("%5d", land->lnd_item[I_PETROL]);
    pr("%5d", land->lnd_item[I_IRON]);
    pr("%5d", land->lnd_item[I_DUST]);
    pr("%5d", land->lnd_item[I_OIL]);
    pr("%5d", land->lnd_item[I_LCM]);
    pr("%5d", land->lnd_item[I_HCM]);
    pr("%4d", land->lnd_item[I_RAD]);
    pr("\n");
}

static void
pr_ship(struct shpstr *ship)
{
    struct natstr *natp;

    if (!(natp = getnatp(ship->shp_own)))
	return;
    pr("%s (#%d) %s\n", natp->nat_cnam, ship->shp_own, prship(ship));
    pr("UID <U>: %d\n", ship->shp_uid);
    pr("Owner <O>: %d\t\t\t", ship->shp_own);
    pr("Location <L>: %s\n", xyas(ship->shp_x, ship->shp_y, player->cnum));
    pr("Tech <T>: %d\t\t\t", ship->shp_tech);
    pr("Efficiency <E>: %d\n", ship->shp_effic);
    pr("Mobility <M>: %d\t\t", ship->shp_mobil);
    pr("Fleet <F>: %.1s\n", &ship->shp_fleet);
    pr("Retreat path <R>: '%s'\t\tRetreat Flags <W>: %d\n",
       ship->shp_rpath, ship->shp_rflags);
    pr("Plague Stage <a>: %d\t\t", ship->shp_pstage);
    pr("Plague Time <b>: %d\n", ship->shp_ptime);
    pr("civ mil  uw food shl gun  pet  irn  dst  oil  lcm  hcm rad\n");
    pr("  c   m   u    f   s   g    p    i    d    o    l    h   r\n");
    pr("%3d", ship->shp_item[I_CIVIL]);
    pr("%4d", ship->shp_item[I_MILIT]);
    pr("%4d", ship->shp_item[I_UW]);
    pr("%5d", ship->shp_item[I_FOOD]);
    pr("%4d", ship->shp_item[I_SHELL]);
    pr("%4d", ship->shp_item[I_GUN]);
    pr("%5d", ship->shp_item[I_PETROL]);
    pr("%5d", ship->shp_item[I_IRON]);
    pr("%5d", ship->shp_item[I_DUST]);
    pr("%5d", ship->shp_item[I_OIL]);
    pr("%5d", ship->shp_item[I_LCM]);
    pr("%5d", ship->shp_item[I_HCM]);
    pr("%4d", ship->shp_item[I_RAD]);
    pr("\n");
}

static int
errcheck(int num, int min, int max)
{
    if (num < min)
	return min;
    else if (num > max)
	return max;
    return num;
}

static int
getin(char *buf, char **valp)
{
    char *p;
    unsigned char thing;

    p = getstarg(NULL, "%c xxxxx -- thing value : ", buf);
    if (!p)
	return -1;
    if (!*p)
	return 0;
    for (; isspace(*p); p++) ;
    if (!*p)
	return -1;
    thing = *p;
    for (; *p && !isspace(*p); p++) ;
    for (; isspace(*p); p++) ;
    if (!*p)
	return -1;
    *valp = p;
    return thing;
}

#if 0	/* not needed right now */
static void
warn_deprecated(char key)
{
    pr("Key <%c> is deprecated and will go away in a future release\n", key);
}
#endif

static int
doland(char op, int arg, char *p, struct sctstr *sect)
{
    natid newown, oldown;
    coord newx, newy;
    int new, old;
    int des;
    switch (op) {
    case 'o':
	if (arg < 0)
	    return RET_SYN;
	newown = (natid)errcheck(arg, 0, MAXNOC - 1);
	pr("Owner of %s changed from %s (#%d) to %s (#%d).\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum),
	   cname(sect->sct_own), sect->sct_own, cname(newown), newown);
	if (sect->sct_own) {
	    wu(player->cnum, sect->sct_own,
	       "Sector %s lost to deity intervention\n",
	       xyas(sect->sct_x, sect->sct_y, sect->sct_own));
	}
	benefit(sect->sct_own, 0);
	sect->sct_own = newown;
	if (newown) {
	    wu(player->cnum, newown,
	       "Sector %s gained from deity intervention\n",
	       xyas(sect->sct_x, sect->sct_y, newown));
	}
	benefit(newown, 1);
	break;
    case 'O':
	if (arg < 0)
	    return RET_SYN;
	oldown = (natid)errcheck(arg, 0, MAXNOC - 1);
	pr("Old owner of %s changed from %s (#%d) to %s (#%d).\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum),
	   cname(sect->sct_oldown),
	   sect->sct_oldown, cname(oldown), oldown);
	sect->sct_oldown = oldown;
	break;
    case 'e':
	new = errcheck(arg, 0, 100);
	noise(sect, "Efficiency", sect->sct_effic, new);
	sect->sct_effic = new;
	break;
    case 'm':
	new = errcheck(arg, -127, 255);
	noise(sect, "Mobility", sect->sct_mobil, new);
	sect->sct_mobil = new;
	break;
    case 'i':
	new = errcheck(arg, 0, 127);
	noise(sect, "Iron ore content", sect->sct_min, new);
	sect->sct_min = (unsigned char)new;
	break;
    case 'g':
	new = errcheck(arg, 0, 127);
	noise(sect, "Gold content", sect->sct_gmin, new);
	sect->sct_gmin = (unsigned char)new;
	break;
    case 'f':
	new = errcheck(arg, 0, 127);
	noise(sect, "Fertility", sect->sct_fertil, new);
	sect->sct_fertil = (unsigned char)new;
	break;
    case 'c':
	new = errcheck(arg, 0, 127);
	noise(sect, "Oil content", sect->sct_oil, new);
	sect->sct_oil = (unsigned char)new;
	break;
    case 'u':
	new = errcheck(arg, 0, 127);
	noise(sect, "Uranium content", sect->sct_uran, new);
	sect->sct_uran = (unsigned char)new;
	break;
    case 'w':
	new = errcheck(arg, 0, 100);
	noise(sect, "Workforce percentage", sect->sct_work, new);
	sect->sct_work = (unsigned char)new;
	break;
    case 'l':
	new = errcheck(arg, 0, 127);
	pr("Loyalty of %s changed from %d to %d%%\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum),
	   sect->sct_loyal, new);
	sect->sct_loyal = (unsigned char)new;
	break;
    case 'x':
	old = sect->sct_che;
	new = errcheck(arg, 0, CHE_MAX);
	pr("Guerillas in %s changed from %d to %d\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum), old, new);
	sect->sct_che = new;
	break;
    case 'X':
	old = sect->sct_che_target;
	new = errcheck(arg, 0, MAXNOC - 1);
	pr("Che target of %s changed from %s (#%d) to %s (#%d).\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum),
	   cname(old), old, cname(new), new);
	sect->sct_che_target = new;
	if (new == 0)
	    sect->sct_che = 0;
	break;
    case 'p':
	old = sect->sct_pstage;
	new = errcheck(arg, 0, PLG_EXPOSED);
	pr("Plague stage of %s changed from %d to %d%%\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum), old, new);
	sect->sct_pstage = new;
	break;
    case 't':
	old = sect->sct_ptime;
	new = errcheck(arg, 0, 255);
	pr("Plague time of %s changed from %d to %d%%\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum), old, new);
	sect->sct_ptime = new;
	break;
    case 'F':
	old = sect->sct_fallout;
	new = errcheck(arg, 0, FALLOUT_MAX);
	pr("Fallout for sector %s changed from %d to %d\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum), old, new);
	sect->sct_fallout = new;
	break;
    case 'a':
	new = errcheck(arg, 0, 9999);
	noise(sect, "Available workforce", sect->sct_avail, new);
	sect->sct_avail = new;
	break;
    case 'M':
	new = errcheck(arg, 0, MINES_MAX);
	sect->sct_mines = new;
	pr("Mines changed to %d\n", new);
	break;
    case 'L':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	sect->sct_x = newx;
	sect->sct_y = newy;
	ef_set_uid(EF_SECTOR, &sect, XYOFFSET(newx, newy));
	break;
    case 'D':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	pr("Distribution location for sector %s changed from %s to %s\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum),
	   xyas(sect->sct_dist_x, sect->sct_dist_y, player->cnum),
	   xyas(newx, newy, player->cnum));
	sect->sct_dist_x = newx;
	sect->sct_dist_y = newy;
	break;
    case 's':
	des = sct_typematch(p);
	if (des < 0)
	    return RET_SYN;
	pr("Designation for sector %s changed from %c to %c\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum),
	   dchr[sect->sct_type].d_mnem, dchr[des].d_mnem);
	set_coastal(sect, sect->sct_type, des);
	sect->sct_type = des;
	break;
    case 'S':
	des = sct_typematch(p);
	if (des < 0)
	    return RET_SYN;
	pr("New designation for sector %s changed from %c to %c\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum),
	   dchr[sect->sct_newtype].d_mnem, dchr[des].d_mnem);
	sect->sct_newtype = des;
	break;
    case 'R':
	if (arg > 100)
	    arg = 100;
	if (arg < 0)
	    arg = 0;
	noise(sect, "Road percentage", sect->sct_road, arg);
	sect->sct_road = arg;
	break;
    case 'r':
	if (arg > 100)
	    arg = 100;
	if (arg < 0)
	    arg = 0;
	noise(sect, "Rail percentage", sect->sct_rail, arg);
	sect->sct_rail = arg;
	break;
    case 'd':
	if (arg > 100)
	    arg = 100;
	if (arg < 0)
	    arg = 0;
	noise(sect, "Defense percentage", sect->sct_defense, arg);
	sect->sct_defense = arg;
	break;
    default:
	pr("huh? (%c)\n", op);
	return RET_SYN;
    }
    return RET_OK;
}


static int
docountry(char op, int arg, char *p, struct natstr *np)
{
    coord newx, newy;
    natid nat = np->nat_cnum;
    float farg = (float)atof(p);

    switch (op) {
    case 'n':
	if (!check_nat_name(p, nat))
	    return RET_SYN;
	pr("Country name changed from %s to %s\n", np->nat_cnam, p);
	strcpy(np->nat_cnam, p);
	break;
    case 'r':
	pr("Country representative changed from %s to %s\n",
	   np->nat_pnam, p);
	strncpy(np->nat_pnam, p, sizeof(np->nat_pnam) - 1);
	break;
    case 't':
	np->nat_tgms = arg;
	break;
    case 'b':
	arg = errcheck(arg, 0, 1024);
	pr("BTU's changed from %d to %d\n", np->nat_btu, arg);
	np->nat_btu = arg;
	break;
    case 'm':
	benefit(nat, np->nat_reserve < arg);
	pr("Military reserves changed from %d to %d\n",
	   np->nat_reserve, arg);
	wu(player->cnum, nat,
	   "Military reserves changed from %d to %d by divine intervention.\n",
	   np->nat_reserve, arg);
	np->nat_reserve = arg;
	break;
    case 'c':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	pr("Capital coordinates changed from %s to %s\n",
	   xyas(np->nat_xcap, np->nat_ycap, player->cnum),
	   xyas(newx, newy, player->cnum));
	np->nat_xcap = newx;
	np->nat_ycap = newy;
	break;
    case 'o':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	pr("Origin coordinates changed from %s to %s\n",
	   xyas(np->nat_xorg, np->nat_yorg, player->cnum),
	   xyas(newx, newy, player->cnum));
	np->nat_xorg = newx;
	np->nat_yorg = newy;
	break;
    case 's':
	np->nat_stat = errcheck(arg, STAT_UNUSED, STAT_GOD);
	break;
    case 'u':
	arg = errcheck(arg, 0, m_m_p_d * 60);
	pr("Number of seconds used changed from %d to %d.\n",
	   np->nat_timeused, arg);
	np->nat_timeused = arg;
	break;
    case 'M':
	pr("Money changed from %d to %d\n", np->nat_money, arg);
	wu(player->cnum, nat,
	   "Money changed from %d to %d by divine intervention.\n",
	   np->nat_money, arg);
	np->nat_money = arg;
	break;
    case 'T':
	pr("Tech changed from %.2f to %.2f.\n",
	   np->nat_level[NAT_TLEV], farg);
	np->nat_level[NAT_TLEV] = farg;
	break;
    case 'R':
	pr("Research changed from %.2f to %.2f.\n",
	   np->nat_level[NAT_RLEV], farg);
	np->nat_level[NAT_RLEV] = farg;
	break;
    case 'E':
	pr("Education changed from %.2f to %.2f.\n",
	   np->nat_level[NAT_ELEV], farg);
	np->nat_level[NAT_ELEV] = farg;
	break;
    case 'H':
	pr("Happiness changed from %.2f to %.2f.\n",
	   np->nat_level[NAT_HLEV], farg);
	np->nat_level[NAT_HLEV] = farg;
	break;
    default:
	pr("huh? (%c)\n", op);
	break;
    }
    putnat(np);
    return RET_OK;
}


static int
doship(char op, int arg, char *p, struct shpstr *ship)
{
    coord newx, newy;

    newx = newy = 0;
    switch (op) {
    case 'a':
	ship->shp_pstage = arg;
	break;
    case 'b':
	ship->shp_ptime = arg;
	break;
    case 'R':
	strncpy(ship->shp_rpath, p, sizeof(ship->shp_rpath) - 1);
	break;
    case 'W':
	ship->shp_rflags = arg;
	break;
    case 'U':
	ef_set_uid(EF_SHIP, ship, arg);
	break;
    case 'O':
	if (ship->shp_own)
	    wu(player->cnum, ship->shp_own,
	       "%s taken from you by deity intervention!\n", prship(ship));
	if (arg && arg < MAXNOC) {
	    wu(player->cnum, (natid)arg,
	       "%s given to you by deity intervention!\n", prship(ship));
	    ship->shp_own = (natid)arg;
	} else if (!arg)
	    ship->shp_effic = 0;
	break;
    case 'L':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	ship->shp_x = newx;
	ship->shp_y = newy;
	break;
    case 'T':
	shp_set_tech(ship,
		     errcheck(arg,
			      mchr[(int)ship->shp_type].m_tech, SHRT_MAX));
	break;
    case 'E':
	ship->shp_effic = errcheck(arg, SHIP_MINEFF, 100);
	break;
    case 'M':
	ship->shp_mobil = arg;
	break;
    case 'F':
	if (p[0] == '~')
	    ship->shp_fleet = 0;
	else if (isalpha(p[0]))
	    ship->shp_fleet = p[0];
	else {
	    pr("%c: invalid fleet\n", p[0]);
	    return RET_FAIL;
	}
	break;
    case 'c':
	ship->shp_item[I_CIVIL] = arg;
	break;
    case 'm':
	ship->shp_item[I_MILIT] = arg;
	break;
    case 'u':
	ship->shp_item[I_UW] = arg;
	break;
    case 'f':
	ship->shp_item[I_FOOD] = arg;
	break;
    case 's':
	ship->shp_item[I_SHELL] = arg;
	break;
    case 'g':
	ship->shp_item[I_GUN] = arg;
	break;
    case 'p':
	ship->shp_item[I_PETROL] = arg;
	break;
    case 'i':
	ship->shp_item[I_IRON] = arg;
	break;
    case 'd':
	ship->shp_item[I_DUST] = arg;
	break;
    case 'o':
	ship->shp_item[I_OIL] = arg;
	break;
    case 'l':
	ship->shp_item[I_LCM] = arg;
	break;
    case 'h':
	ship->shp_item[I_HCM] = arg;
	break;
    case 'r':
	ship->shp_item[I_RAD] = arg;
	break;
    default:
	pr("huh? (%c)\n", op);
	return RET_FAIL;
    }
    return RET_OK;
}

static int
dounit(char op, int arg, char *p, struct lndstr *land)
{
    coord newx, newy;

    newx = newy = 0;
    switch (op) {
    case 'Y':
	land->lnd_land = arg;
	break;
    case 'U':
	ef_set_uid(EF_LAND, land, arg);
	break;
    case 'O':
	if (land->lnd_own)
	    wu(player->cnum, land->lnd_own,
	       "%s taken from you by deity intervention!\n", prland(land));

	if (arg && arg < MAXNOC) {
	    wu(player->cnum, (natid)arg,
	       "%s given to you by deity intervention!\n", prland(land));
	    land->lnd_own = (natid)arg;
	} else if (!arg)
	    land->lnd_effic = 0;
	break;
    case 'L':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	land->lnd_x = newx;
	land->lnd_y = newy;
	break;
    case 'e':
	land->lnd_effic = errcheck(arg, LAND_MINEFF, 100);
	break;
    case 'M':
	land->lnd_mobil = arg;
	break;
    case 't':
	lnd_set_tech(land,
		     errcheck(arg,
			      lchr[(int)land->lnd_type].l_tech, SHRT_MAX));
	break;
    case 'a':
	if (p[0] == '~')
	    land->lnd_army = 0;
	else if (isalpha(p[0]))
	    land->lnd_army = p[0];
	else {
	    pr("%c: invalid army\n", p[0]);
	    return RET_FAIL;
	}
	break;
    case 'F':
	land->lnd_harden = errcheck(arg, 0, 255);
	break;
    case 'S':
	land->lnd_ship = arg;
	break;
    case 'Z':
	land->lnd_retreat = arg;
	break;
    case 'R':
	strncpy(land->lnd_rpath, p, sizeof(land->lnd_rpath) - 1);
	break;
    case 'W':
	land->lnd_rflags = arg;
	break;
    case 'c':
	land->lnd_item[I_CIVIL] = arg;
	break;
    case 'm':
	land->lnd_item[I_MILIT] = arg;
	break;
    case 'u':
	land->lnd_item[I_UW] = arg;
	break;
    case 'f':
	land->lnd_item[I_FOOD] = arg;
	break;
    case 's':
	land->lnd_item[I_SHELL] = arg;
	break;
    case 'g':
	land->lnd_item[I_GUN] = arg;
	break;
    case 'p':
	land->lnd_item[I_PETROL] = arg;
	break;
    case 'i':
	land->lnd_item[I_IRON] = arg;
	break;
    case 'd':
	land->lnd_item[I_DUST] = arg;
	break;
    case 'o':
	land->lnd_item[I_OIL] = arg;
	break;
    case 'l':
	land->lnd_item[I_LCM] = arg;
	break;
    case 'h':
	land->lnd_item[I_HCM] = arg;
	break;
    case 'r':
	land->lnd_item[I_RAD] = arg;
	break;
    default:
	pr("huh? (%c)\n", op);
	return RET_FAIL;
    }
    return RET_OK;
}


static int
doplane(char op, int arg, char *p, struct plnstr *plane)
{
    coord newx, newy;

    switch (op) {
    case 'U':
	ef_set_uid(EF_PLANE, plane, arg);
	break;
    case 'l':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	plane->pln_x = newx;
	plane->pln_y = newy;
	break;
    case 'O':
	if (plane->pln_own)
	    wu(player->cnum, plane->pln_own,
	       "%s taken from you by deity intervention!\n",
	       prplane(plane));
	if (arg && arg < MAXNOC) {
	    plane->pln_own = (natid)arg;
	    wu(player->cnum, plane->pln_own,
	       "%s given to you by deity intervention!\n", prplane(plane));
	} else if (!arg)
	    plane->pln_effic = 0;
	break;
    case 'e':
	plane->pln_effic = errcheck(arg, PLANE_MINEFF, 100);
	break;
    case 'm':
	plane->pln_mobil = errcheck(arg, -127, 255);
	break;
    case 't':
	pln_set_tech(plane,
		     errcheck(arg,
			      plchr[(int)plane->pln_type].pl_tech, SHRT_MAX));
	break;
    case 'w':
	if (p[0] == '~')
	    plane->pln_wing = 0;
	else if (isalpha(p[0]))
	    plane->pln_wing = p[0];
	else {
	    pr("%c: invalid wing\n", p[0]);
	    return RET_FAIL;
	}
	break;
    case 'r':
	plane->pln_range = (unsigned char)arg;
	break;
    case 's':
	plane->pln_ship = arg;
	break;
    case 'y':
	plane->pln_land = arg;
	break;
    case 'f':
	plane->pln_flags = arg;
	break;
    default:
	pr("huh? (%c)\n", op);
	return RET_FAIL;
    }
    return RET_OK;
}
