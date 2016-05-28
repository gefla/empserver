/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  powe.c: Do a power report
 *
 *  Known contributors to this file:
 *     Dave Pare
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2016
 *     Ron Koenderink, 2005-2008
 */

#include <config.h>
#include <unistd.h>
#include <math.h>

#include "commands.h"
#include "item.h"
#include "land.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"
#include "power.h"
#include "ship.h"

static void prpower(char *, struct powstr *, int);
static void out5(double, int, int);
static void gen_power(struct powstr *, int);
static int powcmp(const void *, const void *);
static void addtopow(short *, struct powstr *);
static float empunit_power(int, int, short[], int);
static float money_power(int);
static float power_tech_factor(float);
static float item_power(short[]);

int
powe(void)
{
    struct natstr *natp;
    int i;
    time_t pow_time;
    struct nstr_item ni;
    int save = 1;
    int num = MAXNOC;
    int power_generated = 0;
    struct natstr nat;
    struct powstr powbuf[MAXNOC];
    int targets[MAXNOC];
    int use_targets = 0;
    int no_numbers = 0;

    memset(targets, 0, sizeof(targets));

    i = 1;
    if (player->argp[1]) {
	switch (player->argp[1][0]) {
	case 'u':
	    if (player->god)
		save = 0;
	    /* fall through */
	case 'n':
	    i++;
	    natp = getnatp(player->cnum);
	    if (natp->nat_btu < 1)
		pr("\n  Insufficient BTUs, using the last report.\n\n");
	    else if (opt_AUTO_POWER && save)
		pr("\n  power new is disabled, using the last report.\n\n");
	    else {
		gen_power(powbuf, save);
		pow_time = time(NULL);
		power_generated = 1;
	    }
	}
    }

    if (player->argp[i]) {
	if (player->argp[i][0] == 'c') {
	    if (!snxtitem(&ni, EF_NATION, player->argp[i + 1], NULL))
		return RET_SYN;
	    while (nxtitem(&ni, &nat)) {
		if (nat.nat_stat == STAT_UNUSED)
		    continue;
		if (!player->god && nat.nat_stat != STAT_ACTIVE)
		    continue;
		targets[nat.nat_cnum] = 1;
	    }
	    use_targets = 1;
	} else
	    num = atoi(player->argp[i]);
    }

    if (num < 0) {
	if (!player->god)
	    return RET_SYN;
	num = -num;
	no_numbers = 1;
    }

    if (!power_generated) {
	pow_time = ef_mtime(EF_POWER);
	snxtitem_all(&ni, EF_POWER);
	if (!nxtitem(&ni, &powbuf[0])) {
	    pr("Power for this game has not been built yet.%s\n",
	       opt_AUTO_POWER ? "" : "  Type 'power new' to build it.");
	    return RET_FAIL;
	}
	for (i = 1; i < MAXNOC; i++) {
	    if (!nxtitem(&ni, &powbuf[i])) {
		CANT_REACH();
		memset(&powbuf[i], 0, sizeof(powbuf[i]));
	    }
	}
    }

    pr("     - = [   Empire Power Report   ] = -\n");
    pr("      as of %s\n         sects  eff civ", ctime(&pow_time));
    pr("  mil  shell gun pet  iron dust oil  pln ship unit money\n");
    for (i = 1; i < MAXNOC && num > 0; i++) {
	if (opt_HIDDEN) {
	    if (!player->god && powbuf[i].p_nation != player->cnum)
		continue;
	}
	if (use_targets && !targets[powbuf[i].p_nation])
	    continue;
	if (!use_targets && powbuf[i].p_power <= 0.0)
	    continue;
	prpower(cname(powbuf[i].p_nation), &powbuf[i],
		powbuf[i].p_nation != player->cnum && !player->god);
	if (player->god && !no_numbers)
	    pr("%9.2f\n", powbuf[i].p_power);
	num--;
    }
    if (!opt_HIDDEN || player->god) {
	pr("          ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----\n");
	prpower("worldwide", &powbuf[0], !player->god);
	pr("\n");
    }
    return RET_OK;
}

static void
prpower(char *name, struct powstr *pow, int round_flag)
{
    pr("%9.9s", name);
    out5(pow->p_sects, 5, round_flag);
    if (pow->p_sects)
	pr("%4.0f%%", pow->p_effic / pow->p_sects);
    else
	pr("   0%%");
    out5(pow->p_civil, 50, round_flag);
    out5(pow->p_milit, 50, round_flag);
    out5(pow->p_shell, 25, round_flag);
    out5(pow->p_guns, 5, round_flag);
    out5(pow->p_petrol, 50, round_flag);
    out5(pow->p_iron, 50, round_flag);
    out5(pow->p_dust, 50, round_flag);
    out5(pow->p_oil, 50, round_flag);
    out5(pow->p_planes, 10, round_flag);
    out5(pow->p_ships, 10, round_flag);
    out5(pow->p_units, 10, round_flag);
    out5(pow->p_money, 5000, round_flag);
    pr("\n");
}

static void
out5(double value, int round_val, int round_flag)
{
    double aval;

    if (value > round_val && round_flag)
	value = (int)(value / round_val + 0.5) * round_val;
    aval = fabs(value);
    if (aval < 1000.)
	pr("%4.0f ", value);
    else if (aval < 9.95e3)
	pr("%4.1fK", value / 1e3);
    else if (aval < 999.5e3)
	pr("%4.0fK", value / 1e3);
    else if (aval < 9.95e6)
	pr("%4.1fM", value / 1e6);
    else if (aval < 999.5e6)
	pr("%4.0fM", value / 1e6);
    else
	pr("%4.0fG", value / 1e9);
}

void
update_power(void)
{
    struct powstr powbuf[MAXNOC];

    gen_power(powbuf, 1);
}

static void
gen_power(struct powstr *powbuf, int save)
{
    float upower[MAXNOC];
    float *f_ptr;
    float *f_pt2;
    struct powstr *pow;
    int i;
    struct sctstr sect;
    struct plnstr plane;
    struct plchrstr *pcp;
    struct shpstr ship;
    struct mchrstr *mcp;
    struct lndstr land;
    struct lchrstr *lcp;
    struct nukstr nuke;
    struct nchrstr *ncp;
    struct nstr_item ni;
    struct nstr_sect ns;
    struct natstr *natp;

    player->btused += 10;
    memset(powbuf, 0, MAXNOC * sizeof(*powbuf));
    memset(upower, 0, sizeof(upower));
    snxtsct_all(&ns);
    while (nxtsct(&ns, &sect)) {
	if (sect.sct_own == 0)
	    continue;
	pow = &powbuf[sect.sct_own];
	pow->p_sects += 1.0;
	pow->p_effic += sect.sct_effic;
	addtopow(sect.sct_item, pow);
    }
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	lcp = &lchr[land.lnd_type];
	pow = &powbuf[land.lnd_own];
	addtopow(land.lnd_item, pow);
	upower[land.lnd_own] += empunit_power(land.lnd_effic,
					      land.lnd_tech,
					      lcp->l_mat, lcp->l_cost);
	if (!(lcp->l_flags & L_SPY))
	    pow->p_units += 1.0;
    }
    snxtitem_all(&ni, EF_SHIP);
    while (nxtitem(&ni, &ship)) {
	if (ship.shp_own == 0)
	    continue;
	mcp = &mchr[ship.shp_type];
	pow = &powbuf[ship.shp_own];
	addtopow(ship.shp_item, pow);
	upower[ship.shp_own] += empunit_power(ship.shp_effic,
					      ship.shp_tech,
					      mcp->m_mat, mcp->m_cost);
	pow->p_ships += 1.0;
    }
    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == 0)
	    continue;
	pcp = &plchr[plane.pln_type];
	pow = &powbuf[plane.pln_own];
	upower[plane.pln_own] += empunit_power(plane.pln_effic,
					       plane.pln_tech,
					       pcp->pl_mat, pcp->pl_cost);
	pow->p_planes += 1.0;
    }
    snxtitem_all(&ni, EF_NUKE);
    while (nxtitem(&ni, &nuke)) {
	if (nuke.nuk_own == 0)
	    continue;
	ncp = &nchr[nuke.nuk_type];
	pow = &powbuf[nuke.nuk_own];
	upower[nuke.nuk_own] += empunit_power(nuke.nuk_effic,
					      nuke.nuk_tech,
					      ncp->n_mat, ncp->n_cost);
    }
    for (i = 1; NULL != (natp = getnatp(i)); i++) {
	pow = &powbuf[i];
	pow->p_nation = i;
	if (natp->nat_stat != STAT_ACTIVE) {
	    pow->p_power = 0.;
	    continue;
	}
	pow->p_money = natp->nat_money;
	pow->p_power += money_power(natp->nat_money);

	if (pow->p_sects > 0)
	    pow->p_power += pow->p_sects
		* (pow->p_effic / pow->p_sects / 100.0)
		* 10.0;
	pow->p_power *= power_tech_factor(natp->nat_level[NAT_TLEV]);
	pow->p_power += upower[i];
	/* ack.  add this vec to the "world power" element */
	f_pt2 = &powbuf[0].p_sects;
	f_ptr = &pow->p_sects;
	while (f_ptr <= &pow->p_power) {
	    *f_pt2 += *f_ptr;
	    f_pt2++;
	    f_ptr++;
	}
    }
    qsort(&powbuf[1], MAXNOC - 1, sizeof(*powbuf), powcmp);
    if (!save)
	return;
    for (i = 0; i < MAXNOC; i++)
	putpower(i, &powbuf[i]);
#ifdef _WIN32
    /*
     * At least some versions of Windows fail to update mtime on
     * write(), they delay it until the write actually hits the disk.
     * Bad, because `power' displays that time.  Force it.
     */
    _commit(empfile[EF_POWER].fd);
#endif
}

static int
powcmp(const void *a, const void *b)
{
    const struct powstr *p1 = a;
    const struct powstr *p2 = b;

    if (p1->p_power > p2->p_power)
	return -1;
    if (p1->p_power < p2->p_power)
	return 1;
    return p1->p_nation - p2->p_nation;
}

static void
addtopow(short *vec, struct powstr *pow)
{
    pow->p_civil += vec[I_CIVIL];
    pow->p_milit += vec[I_MILIT];
    pow->p_shell += vec[I_SHELL];
    pow->p_guns += vec[I_GUN];
    pow->p_petrol += vec[I_PETROL];
    pow->p_iron += vec[I_IRON];
    pow->p_dust += vec[I_DUST];
    pow->p_food += vec[I_FOOD];
    pow->p_oil += vec[I_OIL];
    pow->p_bars += vec[I_BAR];
    pow->p_power += item_power(vec);
}

static float
empunit_power(int effic, int tech, short mat[], int cost)
{
    return (item_power(mat) + money_power(cost)) * (effic / 100.0)
	* power_tech_factor(tech);
}

static float
money_power(int money)
{
    return money / 100.0;
}

static float
power_tech_factor(float tech)
{
    return (20 + tech) / 500.0;
}

static float
item_power(short item[])
{
    int pow, i;

    pow = 0;
    for (i = I_NONE + 1; i <= I_MAX; i++)
	pow += item[i] * ichr[i].i_power;

    return pow / 1000.0;
}
