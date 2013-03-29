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
#include "actofgod.h"
#include "commands.h"
#include "item.h"
#include "land.h"
#include "news.h"
#include "optlist.h"
#include "plague.h"
#include "plane.h"
#include "ship.h"
#include "unit.h"

static void print_sect(struct sctstr *);
static void print_nat(struct natstr *);
static void print_plane(struct plnstr *);
static void print_land(struct lndstr *);
static void print_ship(struct shpstr *);
static char *getin(char *, char **);
static int edit_nat(struct natstr *, char *, char *);
static int edit_ship(struct shpstr *, char *, char *);
static int edit_land(struct lndstr *, char *, char *);
static int edit_plane(struct plnstr *, char *, char *);

int
edit(void)
{
    union empobj_storage item;
    char *what;
    char *key, *ptr;
    int num;
    int ret;
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
	if (!getsect(x, y, &item.sect))
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
	if (!getplane(num, &item.plane))
	    return RET_SYN;
	break;
    case 's':
	if ((num = onearg(player->argp[2], "Ship number? ")) < 0)
	    return RET_SYN;
	if (!getship(num, &item.ship))
	    return RET_SYN;
	break;
    case 'u':
	if ((num = onearg(player->argp[2], "Unit number? ")) < 0)
	    return RET_SYN;
	if (!getland(num, &item.land))
	    return RET_SYN;
	break;
    case 'n':
	pr("Not implemented yet.\n");
	return RET_FAIL;
    default:
	pr("huh?\n");
	return RET_SYN;
    }
    if (!player->argp[3]) {
	switch (ewhat) {
	case 'l':
	    print_sect(&item.sect);
	    break;
	case 'c':
	    print_nat(np);
	    break;
	case 'p':
	    print_plane(&item.plane);
	    break;
	case 's':
	    print_ship(&item.ship);
	    break;
	case 'u':
	    print_land(&item.land);
	    break;
	default:
	    CANT_REACH();
	}
    }
    for (;;) {
	if (player->argp[arg_index]) {
	    if (player->argp[arg_index+1]) {
		key = player->argp[arg_index++];
		ptr = player->argp[arg_index++];
	    } else
		return RET_SYN;
	} else if (arg_index == 3) {
	    key = getin(buf, &ptr);
	    if (!key)
		return RET_SYN;
	    if (!*key)
		return RET_OK;
	} else
	    return RET_OK;

	switch (ewhat) {
	case 'c':
	    ret = edit_nat(np, key, ptr);
	    if (ret != RET_OK)
		return ret;
	    break;
	case 'l':
	    if (!check_sect_ok(&item.sect))
		return RET_FAIL;
	    ret = edit_sect(&item.sect, key, ptr);
	    if (ret != RET_OK)
		return ret;
	    if (!putsect(&item.sect))
		return RET_FAIL;
	    break;
	case 's':
	    if (!check_ship_ok(&item.ship))
		return RET_FAIL;
	    ret = edit_ship(&item.ship, key, ptr);
	    if (ret != RET_OK)
		return ret;
	    if (!putship(item.ship.shp_uid, &item.ship))
		return RET_FAIL;
	    break;
	case 'u':
	    if (!check_land_ok(&item.land))
		return RET_FAIL;
	    ret = edit_land(&item.land, key, ptr);
	    if (ret != RET_OK)
		return ret;
	    if (!putland(item.land.lnd_uid, &item.land))
		return RET_FAIL;
	    break;
	case 'p':
	    if (!check_plane_ok(&item.plane))
		return RET_FAIL;
	    ret = edit_plane(&item.plane, key, ptr);
	    if (ret != RET_OK)
		return ret;
	    if (!putplane(item.plane.pln_uid, &item.plane))
		return RET_FAIL;
	    break;
	default:
	    CANT_REACH();
	}
    }
}

static void
noise(struct sctstr *sptr, char *name, int old, int new)
{
    divine_sct_change(sptr, name, new != old, new - old,
		      "from %d to %d", old, new);
}

static void
print_sect(struct sctstr *sect)
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
print_nat(struct natstr *np)
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
print_plane(struct plnstr *plane)
{
    pr("%s %s\n", prnatid(plane->pln_own), prplane(plane));
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
print_land(struct lndstr *land)
{
    pr("%s %s\n", prnatid(land->lnd_own), prland(land));
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
print_ship(struct shpstr *ship)
{
    pr("%s %s\n", prnatid(ship->shp_own), prship(ship));
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

static char *
getin(char *buf, char **valp)
{
    char line[1024];
    char *argp[128];
    char *p;

    *valp = NULL;
    p = getstarg(NULL, "%c xxxxx -- thing value : ", line);
    if (!p)
	return NULL;
    switch (parse(p, buf, argp, NULL, NULL, NULL)) {
    case 0:
	return "";
    case 1:
	return NULL;
    default:
	*valp = argp[1];
	return argp[0];
    }
}

#if 0	/* not needed right now */
static void
warn_deprecated(char key)
{
    pr("Key <%c> is deprecated and will go away in a future release\n", key);
}
#endif

int
edit_sect_i(struct sctstr *sect, char *key, int arg)
{
    int new;

    switch (*key) {
    case 'o':
	if (arg < 0 || arg >= MAXNOC)
	    return RET_SYN;
	divine_sct_change_quiet(sect, "Owner", arg != sect->sct_own,
				"from %s to %s",
				prnatid(sect->sct_own), prnatid(arg));
	if (arg == sect->sct_own)
	    break;
	report_god_takes("Sector ",
			 xyas(sect->sct_x, sect->sct_y, sect->sct_own),
			 sect->sct_own);
	report_god_gives("Sector ",
			 xyas(sect->sct_x, sect->sct_y, arg),
			 arg);
	sect->sct_own = arg;
	break;
    case 'O':
	if (arg < 0 || arg >= MAXNOC)
	    return RET_SYN;
	divine_sct_change(sect, "Old owner", arg != sect->sct_oldown, 0,
			  "from %s to %s",
			  prnatid(sect->sct_oldown), prnatid(arg));
	sect->sct_oldown = arg;
	break;
    case 'e':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Efficiency", sect->sct_effic, new);
	sect->sct_effic = new;
	break;
    case 'm':
	new = LIMIT_TO(arg, -127, 127);
	noise(sect, "Mobility", sect->sct_mobil, new);
	sect->sct_mobil = new;
	break;
    case 'i':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Iron ore content", sect->sct_min, new);
	sect->sct_min = (unsigned char)new;
	break;
    case 'g':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Gold content", sect->sct_gmin, new);
	sect->sct_gmin = (unsigned char)new;
	break;
    case 'f':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Fertility", sect->sct_fertil, new);
	sect->sct_fertil = (unsigned char)new;
	break;
    case 'c':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Oil content", sect->sct_oil, new);
	sect->sct_oil = (unsigned char)new;
	break;
    case 'u':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Uranium content", sect->sct_uran, new);
	sect->sct_uran = (unsigned char)new;
	break;
    case 'w':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Workforce percentage", sect->sct_work, new);
	sect->sct_work = (unsigned char)new;
	break;
    case 'l':
	new = LIMIT_TO(arg, 0, 127);
	divine_sct_change_quiet(sect, "Loyalty", new != sect->sct_loyal,
				"from %d to %d", sect->sct_loyal, new);
	sect->sct_loyal = (unsigned char)new;
	break;
    case 'x':
	new = LIMIT_TO(arg, 0, CHE_MAX);
	divine_sct_change_quiet(sect, "Guerillas", new != sect->sct_che,
				"from %d to %d", sect->sct_che, new);
	sect->sct_che = new;
	break;
    case 'X':
	if (arg < 0 || arg >= MAXNOC)
	    return RET_SYN;
	divine_sct_change_quiet(sect, "Che target",
				arg != sect->sct_che_target,
				"from %s to %s",
				prnatid(sect->sct_che_target),
				prnatid(arg));
	sect->sct_che_target = arg;
	if (arg == 0)
	    sect->sct_che = 0;
	break;
    case 'p':
	new = LIMIT_TO(arg, 0, PLG_EXPOSED);
	divine_sct_change_quiet(sect, "Plague stage",
				new != sect->sct_pstage,
				"from %d to %d", sect->sct_pstage, new);
	sect->sct_pstage = new;
	break;
    case 't':
	new = LIMIT_TO(arg, 0, 32767);
	divine_sct_change_quiet(sect, "Plague time",
				new != sect->sct_ptime,
				"from %d to %d", sect->sct_ptime, new);
	sect->sct_ptime = new;
	break;
    case 'F':
	new = LIMIT_TO(arg, 0, FALLOUT_MAX);
	noise(sect, "Fallout", sect->sct_fallout, new);
	sect->sct_fallout = new;
	break;
    case 'a':
	new = LIMIT_TO(arg, 0, 9999);
	noise(sect, "Available workforce", sect->sct_avail, new);
	sect->sct_avail = new;
	break;
    case 'M':
	new = LIMIT_TO(arg, 0, MINES_MAX);
	if (sect->sct_own == sect->sct_oldown)
	    noise(sect, "Mines", sect->sct_mines, new);
	else
	    divine_sct_change_quiet(sect, "Mines", new != sect->sct_mines,
			      "from %d to %d", sect->sct_mines, new);
	sect->sct_mines = new;
	break;
    case 'R':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Road percentage", sect->sct_road, new);
	sect->sct_road = new;
	break;
    case 'r':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Rail percentage", sect->sct_rail, new);
	sect->sct_rail = new;
	break;
    case 'd':
	new = LIMIT_TO(arg, 0, 100);
	noise(sect, "Defense percentage", sect->sct_defense, new);
	sect->sct_defense = new;
	break;
    default:
	pr("huh? (%s)\n", key);
	return RET_SYN;
    }
    return RET_OK;
}

int
edit_sect(struct sctstr *sect, char *key, char *p)
{
    coord newx, newy;
    int new;
    struct sctstr newsect;

    switch (*key) {
    case 'L':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	if (newx == sect->sct_x && newy == sect->sct_y) {
	    pr("Sector %s unchanged\n", xyas(newx, newy, player->cnum));
	    break;
	}
	getsect(newx, newy, &newsect);
	pr("Sector %s duplicated to %s\n",
	   xyas(sect->sct_x, sect->sct_y, player->cnum),
	   xyas(newx, newy, player->cnum));
	report_god_takes("Sector ", xyas(newx, newy, newsect.sct_own),
			 newsect.sct_own);
	report_god_gives("Sector ", xyas(newx, newy, sect->sct_own),
			 sect->sct_own);
	sect->sct_x = newx;
	sect->sct_y = newy;
	ef_set_uid(EF_SECTOR, sect, XYOFFSET(newx, newy));
	break;
    case 'D':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	divine_sct_change_quiet(sect, "Distribution sector",
		newx != sect->sct_dist_x || newy != sect->sct_dist_y,
		"from %s to %s",
		xyas(sect->sct_dist_x, sect->sct_dist_y, player->cnum),
		xyas(newx, newy, player->cnum));
	if (newx == sect->sct_dist_x && newy == sect->sct_dist_y)
	    break;
	if (sect->sct_own && sect->sct_own != player->cnum)
	    wu(0, sect->sct_own,
	       "Distribution sector of %s changed from %s to %s"
	       " by an act of %s\n",
	       xyas(sect->sct_x, sect->sct_y, player->cnum),
	       xyas(sect->sct_dist_x, sect->sct_dist_y, player->cnum),
	       xyas(newx, newy, player->cnum),
	       cname(player->cnum));
	sect->sct_dist_x = newx;
	sect->sct_dist_y = newy;
	break;
    case 's':
	new = sct_typematch(p);
	if (new < 0)
	    return RET_SYN;
	divine_sct_change(sect, "Designation",
			  new != sect->sct_type, 0, "from %c to %c",
			  dchr[sect->sct_type].d_mnem, dchr[new].d_mnem);
	set_coastal(sect, sect->sct_type, new);
	sect->sct_type = new;
	break;
    case 'S':
	new = sct_typematch(p);
	if (new < 0)
	    return RET_SYN;
	divine_sct_change(sect, "New designation",
			  new != sect->sct_newtype, 0, "from %c to %c",
			  dchr[sect->sct_newtype].d_mnem, dchr[new].d_mnem);
	sect->sct_newtype = new;
	break;
    default:
	return edit_sect_i(sect, key, atoi(p));
    }
    return RET_OK;
}

static void
edit_level(struct natstr *np, int lvl, char *name, char *p)
{
    float new = (float)atof(p);

    new = MAX(0.0, new);
    divine_nat_change(np, name,
		      new != np->nat_level[lvl],
		      (new > np->nat_level[lvl]) - (new < np->nat_level[lvl]),
		      "from %.2f to %.2f", np->nat_level[lvl], new);
    np->nat_level[lvl] = new;
}

static int
edit_nat(struct natstr *np, char *key, char *p)
{
    coord newx, newy;
    natid nat = np->nat_cnum;
    int arg = atoi(p);

    switch (*key) {
    case 'n':
	if (!check_nat_name(p, nat))
	    return RET_SYN;
	divine_nat_change(np, "Country name", strcmp(np->nat_cnam, p), 0,
			  "from %s to %s", np->nat_cnam, p);
	if (opt_GODNEWS)
	    nreport(player->cnum, N_NAME_CHNG, 0, 1);
	strcpy(np->nat_cnam, p);
	break;
    case 'r':
	divine_nat_change(np, "Country representative",
		strncmp(p, np->nat_pnam, sizeof(np->nat_pnam) - 1), 0,
		"from %s to %.*s",
		np->nat_pnam, (int)sizeof(np->nat_pnam) - 1, p);
	strncpy(np->nat_pnam, p, sizeof(np->nat_pnam) - 1);
	break;
    case 't':
	arg = LIMIT_TO(arg, 0, USHRT_MAX);
	divine_nat_change_quiet(np, "Number of unread telegrams",
				arg != np->nat_tgms,
				"from %d to %d", np->nat_tgms, arg);
	np->nat_tgms = arg;
	break;
    case 'b':
	arg = LIMIT_TO(arg, 0, max_btus);
	divine_nat_change(np, "BTUs",
			  arg != np->nat_btu, arg - np->nat_btu,
			  "from %d to %d", np->nat_btu, arg);
	np->nat_btu = arg;
	break;
    case 'm':
	arg = LIMIT_TO(arg, 0, INT_MAX);
	divine_nat_change(np, "Military reserves",
			  arg != np->nat_reserve, arg - np->nat_reserve,
			  "from %d to %d", np->nat_reserve, arg);
	np->nat_reserve = arg;
	break;
    case 'c':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	if (newx == np->nat_xcap && newy == np->nat_ycap)
	    pr("Capital unchanged\n");
	else {
	    pr("Capital moved from %s to %s\n",
	       xyas(np->nat_xcap, np->nat_ycap, player->cnum),
	       xyas(newx, newy, player->cnum));
	    if (nat != player->cnum)
		wu(0, nat,
		   "Capital moved from %s to %s by an act of %s!\n",
		   xyas(np->nat_xcap, np->nat_ycap, nat),
		   xyas(newx, newy, nat), cname(player->cnum));
	}
	np->nat_xcap = newx;
	np->nat_ycap = newy;
	break;
    case 'o':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	if (newx == np->nat_xorg && newy == np->nat_yorg)
	    pr("Origin unchanged\n");
	else {
	    pr("Origin moved from %s to %s\n",
	       xyas(np->nat_xorg, np->nat_yorg, player->cnum),
	       xyas(newx, newy, player->cnum));
	    if (nat != player->cnum)
		wu(0, nat,
		   "Origin moved from %s to %s by an act of %s!\n",
		   xyas(np->nat_xorg, np->nat_yorg, nat),
		   xyas(newx, newy, nat), cname(player->cnum));
	}
	np->nat_xorg = newx;
	np->nat_yorg = newy;
	break;
    case 's':
	arg = LIMIT_TO(arg, STAT_UNUSED, STAT_GOD);
	divine_nat_change(np, "Status",
			  (enum nat_status)arg != np->nat_stat,
			  0, "to %s", nation_status[arg].name);
	np->nat_stat = arg;
	break;
    case 'u':
	arg = LIMIT_TO(arg, 0, m_m_p_d * 60);
	divine_nat_change(np, "Number of seconds used",
			  arg != np->nat_timeused, arg - np->nat_timeused,
			  "from %d to %d", np->nat_timeused, arg);
	np->nat_timeused = arg;
	break;
    case 'M':
	divine_nat_change(np, "Money",
			  arg != np->nat_money, arg - np->nat_money,
			  "from %d to %d", np->nat_money, arg);
	np->nat_money = arg;
	break;
    case 'T':
	edit_level(np, NAT_TLEV, "Technology", p);
	break;
    case 'R':
	edit_level(np, NAT_RLEV, "Research", p);
	break;
    case 'E':
	edit_level(np, NAT_ELEV, "Education", p);
	break;
    case 'H':
	edit_level(np, NAT_HLEV, "Happiness", p);
	break;
    default:
	pr("huh? (%s)\n", key);
	break;
    }
    putnat(np);
    return RET_OK;
}

static int
edit_unit(struct empobj *unit, char *key, char *p,
	  int mineff, char *group_name, int on_carrier)
{
    int arg = atoi(p);
    coord newx, newy;
    union empobj_storage newunit;
    char newgroup;
 
    switch (toupper(*key)) {
    case 'U':
	if (arg < 0)
	    return RET_SYN;
	if (arg == unit->uid) {
	    pr("%s unchanged\n", unit_nameof(unit));
	    break;
	}
	if (!ef_ensure_space(unit->ef_type, arg, 50)) {
	    pr("Can't copy to %s #%d\n", ef_nameof(unit->ef_type), arg);
	    return RET_FAIL;
	}
	pr("%s duplicated to (#%d)\n", unit_nameof(unit), arg);
	ef_set_uid(unit->ef_type, unit, arg);
	if (get_empobj(unit->ef_type, arg, &newunit) && newunit.gen.own) {
	    pr("Replacing %s of %s\n",
	       unit_nameof(&newunit.gen), prnatid(newunit.gen.own));
	    report_god_takes("", unit_nameof(&newunit.gen),
			     newunit.gen.own);
	}
	report_god_gives("", unit_nameof(unit), unit->own);
	break;
    case 'O':
	if (arg < 0 || arg >= MAXNOC)
	    return RET_SYN;
	divine_unit_change_quiet(unit, "Owner", arg != unit->own,
				 "from %s to %s",
				 prnatid(unit->own), prnatid(arg));
	if (arg != unit->own) {
	    report_god_takes("", unit_nameof(unit), unit->own);
	    report_god_gives("", unit_nameof(unit), arg);
	}
	unit->own = arg;
	break;
    case 'L':
	if (!sarg_xy(p, &newx, &newy))
	    return RET_SYN;
	if (on_carrier && (newx != unit->x || newy != unit->y)) {
	    pr("Can't move %s while it's loaded\n", unit_nameof(unit));
	    return RET_FAIL;
	}
	divine_unit_change_quiet(unit, "Location",
				 unit->own && unit->own != player->cnum,
				 "from %s to %s",
				 xyas(unit->x, unit->y, player->cnum),
				 xyas(newx, newy, player->cnum));
	if (unit->own && unit->own != player->cnum)
	    wu(0, unit->own,
	       "Location of %s changed from %s to %s by an act of %s!\n",
	       unit_nameof(unit),
	       xyas(unit->x, unit->y, unit->own),
	       xyas(newx, newy, unit->own),
	       cname(player->cnum));
	unit->x = newx;
	unit->y = newy;
	break;
    case 'E':
	arg = LIMIT_TO(arg, mineff, 100);
	divine_unit_change(unit, "Efficiency",
			   arg != unit->effic, arg - unit->effic,
			   "from %d to %d", unit->effic, arg);
	unit->effic = arg;
	break;
    case 'M':
	arg = LIMIT_TO(arg, -127, 127);
	divine_unit_change(unit, "Mobility",
			   arg != unit->mobil, arg - unit->mobil,
			   "from %d to %d", unit->mobil, arg);
	unit->mobil = arg;
	break;
    case 'F':
    case 'W':
    case 'A':
	if (p[0] == '~')
	    newgroup = 0;
	else if (isalpha(p[0]))
	    newgroup = p[0];
	else {
	    pr("%c: invalid %s\n", p[0], group_name);
	    return RET_FAIL;
	}
	divine_unit_change(unit, "Assignment", newgroup != unit->group, 0,
			   "from %s %c to %c", group_name,
			   unit->group ? unit->group : '~', p[0]);
	unit->group = newgroup;
	break;
    default:
	CANT_REACH();
    }
    return RET_OK;
}

static int
edit_ship(struct shpstr *ship, char *key, char *p)
{
    struct mchrstr *mcp = &mchr[ship->shp_type];
    int arg = atoi(p);
    struct ichrstr *ip;

    switch (*key) {
    case 'U':
    case 'O':
    case 'L':
    case 'E':
    case 'M':
    case 'F':
	return edit_unit((struct empobj *)ship, key, p,
			 SHIP_MINEFF, "fleet", 0);
    case 'T':
	arg = LIMIT_TO(arg, mcp->m_tech, SHRT_MAX);
	divine_unit_change((struct empobj *)ship, "Tech level",
			   arg != ship->shp_tech, arg - ship->shp_tech,
			   "from %d to %d", ship->shp_tech, arg);
	shp_set_tech(ship, arg);
	break;
    case 'a':
	arg = LIMIT_TO(arg, 0, PLG_EXPOSED);
	divine_unit_change_quiet((struct empobj *)ship, "Plague stage",
				 arg != ship->shp_pstage,
				 "from %d to %d", ship->shp_pstage, arg);
	ship->shp_pstage = arg;
	break;
    case 'b':
	arg = LIMIT_TO(arg, 0, 32767);
	divine_unit_change_quiet((struct empobj *)ship, "Plague time",
				 arg != ship->shp_ptime,
				 "from %d to %d", ship->shp_ptime, arg);
	ship->shp_ptime = arg;
	break;
    case 'R':
	divine_unit_change((struct empobj *)ship, "Retreat path",
		strncmp(p, ship->shp_rpath, sizeof(ship->shp_rpath) - 1),
		0, "from %s to %.*s",
		ship->shp_rpath, (int)sizeof(ship->shp_rpath) - 1, p);
	strncpy(ship->shp_rpath, p, sizeof(ship->shp_rpath) - 1);
	break;
    case 'W':
	divine_flag_change((struct empobj *)ship, "Retreat conditions",
			   ship->shp_rflags, arg, retreat_flags);
	ship->shp_rflags = arg;
	break;
    case 'c':
    case 'm':
    case 'u':
    case 'f':
    case 's':
    case 'g':
    case 'p':
    case 'i':
    case 'd':
    case 'o':
    case 'l':
    case 'h':
    case 'r':
	ip = item_by_name(key);
	arg = LIMIT_TO(arg, 0, mchr[ship->shp_type].m_item[ip->i_uid]);
	divine_unit_change_quiet((struct empobj *)ship, ip->i_name,
				 arg != ship->shp_item[ip->i_uid],
				 "from %d to %d",
				 ship->shp_item[ip->i_uid], arg);
	report_divine_gift(ship->shp_own, ip,
			   arg - ship->shp_item[ip->i_uid], prship(ship));
	ship->shp_item[ip->i_uid] = arg;
	break;
    default:
	pr("huh? (%s)\n", key);
	return RET_FAIL;
    }
    return RET_OK;
}

static int
edit_land(struct lndstr *land, char *key, char *p)
{
    struct lchrstr *lcp = &lchr[land->lnd_type];
    int arg = atoi(p);
    struct ichrstr *ip;

    switch (*key) {
    case 'U':
    case 'O':
    case 'L':
    case 'e':
    case 'M':
    case 'a':
	return edit_unit((struct empobj *)land, key, p,
			 LAND_MINEFF, "army",
			 land->lnd_ship >= 0 || land->lnd_land >= 0);
    case 't':
	arg = LIMIT_TO(arg, lcp->l_tech, SHRT_MAX);
	divine_unit_change((struct empobj *)land, "Tech level",
			   arg != land->lnd_tech, arg - land->lnd_tech,
			   "from %d to %d", land->lnd_tech, arg);
	lnd_set_tech(land, arg);
	break;
    case 'F':
	arg = LIMIT_TO(arg, 0, 127);
	divine_unit_change((struct empobj *)land, "Fortification",
			   arg != land->lnd_harden, arg - land->lnd_harden,
			   "from %d to %d", land->lnd_harden, arg);
	land->lnd_harden = arg;
	break;
    case 'S':
	if (arg < -1 || arg >= ef_nelem(EF_SHIP))
	    return RET_SYN;
	if (arg == land->lnd_ship) {
	    pr("Ship of %s unchanged\n", prland(land));
	    break;
	}
	divine_unload((struct empobj *)land, EF_SHIP, land->lnd_ship);
	if (arg >= 0) {
	    divine_unload((struct empobj *)land, EF_LAND, land->lnd_land);
	    land->lnd_land = -1;
	}
	divine_load((struct empobj *)land, EF_SHIP, arg);
	land->lnd_ship = arg;
	break;
    case 'Y':
	if (arg < -1 || arg >= ef_nelem(EF_LAND))
	    return RET_SYN;
	if (arg == land->lnd_land) {
	    pr("Land unit of %s unchanged\n", prland(land));
	    break;
	}
	divine_unload((struct empobj *)land, EF_LAND, land->lnd_land);
	if (arg >= 0) {
	    divine_unload((struct empobj *)land, EF_SHIP, land->lnd_ship);
	    land->lnd_ship = -1;
	}
	divine_load((struct empobj *)land, EF_LAND, arg);
	land->lnd_land = arg;
	break;
    case 'Z':
	arg = LIMIT_TO(arg, 0, 100);
	divine_unit_change((struct empobj *)land, "Retreat percentage",
			   arg != land->lnd_retreat, 0,
			   "from %d to %d", land->lnd_retreat, arg);
	land->lnd_retreat = arg;
	break;
    case 'R':
	divine_unit_change((struct empobj *)land, "Retreat path",
		strncmp(p, land->lnd_rpath, sizeof(land->lnd_rpath) - 1),
		0, "from %s to %.*s",
		land->lnd_rpath, (int)sizeof(land->lnd_rpath) - 1, p);
	strncpy(land->lnd_rpath, p, sizeof(land->lnd_rpath) - 1);
	break;
    case 'W':
	divine_flag_change((struct empobj *)land, "Retreat condition",
			   land->lnd_rflags, arg, retreat_flags);
	land->lnd_rflags = arg;
	break;
    case 'c':
    case 'm':
    case 'u':
    case 'f':
    case 's':
    case 'g':
    case 'p':
    case 'i':
    case 'd':
    case 'o':
    case 'l':
    case 'h':
    case 'r':
	ip = item_by_name(key);
	arg = LIMIT_TO(arg, 0, lchr[land->lnd_type].l_item[ip->i_uid]);
	divine_unit_change_quiet((struct empobj *)land, ip->i_name,
				 arg != land->lnd_item[ip->i_uid],
				 "from %d to %d",
				 land->lnd_item[ip->i_uid], arg);
	report_divine_gift(land->lnd_own, ip,
			   arg - land->lnd_item[ip->i_uid], prland(land));
	land->lnd_item[ip->i_uid] = arg;
	break;
    default:
	pr("huh? (%s)\n", key);
	return RET_FAIL;
    }
    return RET_OK;
}

static int
edit_plane(struct plnstr *plane, char *key, char *p)
{
    struct plchrstr *pcp = &plchr[plane->pln_type];
    int arg = atoi(p);

    switch (*key) {
    case 'U':
    case 'O':
    case 'l':
    case 'e':
    case 'm':
    case 'w':
	return edit_unit((struct empobj *)plane, key, p,
			 PLANE_MINEFF, "wing",
			 plane->pln_ship >= 0 || plane->pln_land >= 0);
    case 't':
	arg = LIMIT_TO(arg, pcp->pl_tech, SHRT_MAX);
	divine_unit_change((struct empobj *)plane, "Tech level",
			   arg != plane->pln_tech, arg - plane->pln_tech,
			   "from %d to %d", plane->pln_tech, arg);
	pln_set_tech(plane, arg);
	break;
    case 'r':
	arg = LIMIT_TO(arg, 0, pl_range(pcp, plane->pln_tech));
	divine_unit_change((struct empobj *)plane, "Range",
			   arg != plane->pln_range, 0,
			   "from %d to %d", plane->pln_range, arg);
	plane->pln_range = (unsigned char)arg;
	break;
    case 's':
	if (arg < -1 || arg >= ef_nelem(EF_SHIP))
	    return RET_SYN;
	if (arg == plane->pln_ship) {
	    pr("Ship of %s unchanged\n", prplane(plane));
	    break;
	}
	divine_unload((struct empobj *)plane, EF_SHIP, plane->pln_ship);
	if (arg >= 0) {
	    divine_unload((struct empobj *)plane, EF_LAND, plane->pln_land);
	    plane->pln_land = -1;
	}
	divine_load((struct empobj *)plane, EF_SHIP, arg);
	plane->pln_ship = arg;
	break;
    case 'y':
	if (arg < -1 || arg >= ef_nelem(EF_LAND))
	    return RET_SYN;
	if (arg == plane->pln_land) {
	    pr("Land unit of %s unchanged\n", prplane(plane));
	    break;
	}
	divine_unload((struct empobj *)plane, EF_LAND, plane->pln_land);
	if (arg >= 0) {
	    divine_unload((struct empobj *)plane, EF_SHIP, plane->pln_ship);
	    plane->pln_ship = -1;
	}
	divine_load((struct empobj *)plane, EF_LAND, arg);
	plane->pln_land = arg;
	break;
    case 'f':
	divine_flag_change((struct empobj *)plane, "Flags",
			   plane->pln_flags, arg, plane_flags);
	plane->pln_flags = arg;
	break;
    default:
	pr("huh? (%s)\n", key);
	return RET_FAIL;
    }
    return RET_OK;
}
