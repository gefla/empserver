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
 *  powe.c: Do a power report
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#ifdef Rel4
#include <string.h>
#endif /* Rel4 */
#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "ship.h"
#include "land.h"
#include "nuke.h"
#include "power.h"
#include "item.h"
#include "deity.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "optlist.h"
#include "commands.h"


#include <fcntl.h>

struct powsort {
    float powval;
    natid cnum;
};

static void addtopow(register int *vec, register struct powstr *pow);
static void gen_power(void);
static void out5(double value, int round_val, int round_flag);
static int powcmp(struct powsort *p1, struct powsort *p2);
static int set_target(s_char *, int *);

int
powe(void)
{
    struct natstr *natp;
    struct natstr *natp2;
    int round_flag;
    time_t pow_time;
    struct nstr_item ni;
    struct powstr pow;
    int num;
    int power_generated = 0;
    int targets[MAXNOC];
    int use_targets = 0;
    int got_targets = 0;
    int no_numbers = 0;
    s_char *p;

    memset(targets, 0, sizeof(targets));
    natp = getnatp(player->cnum);
    num = MAXNOC;
    if (player->argp[1] && player->argp[1][0] == 'n') {
	if (natp->nat_btu < 1)
	    pr("\n  Insufficient BTUs, using the last report.\n\n");
	else {
	    gen_power();
	    power_generated = 1;
	    if (player->argp[2] && (num = atoi(player->argp[2])) < 0) {
		num = MAXNOC;
		if (player->god)
		    no_numbers = 1;
		else
		    return RET_SYN;
	    }
	}
    } else if (player->argp[1] && player->argp[1][0] == 'c') {
	if (!player->argp[2])
	    return RET_SYN;
	if (strchr(player->argp[2], '/')) {
	    p = strtok(player->argp[2], "/");
	    do {
		got_targets |= set_target(p, targets);
	    } while (NULL != (p = strtok(0, "/")));
	} else {
	    got_targets |= set_target(player->argp[2], targets);
	}
	use_targets = 1;
    } else if (player->argp[1] && (num = atoi(player->argp[1])) < 0) {
	num = -(num);
	if (num > MAXNOC || num < 0)
	    num = MAXNOC;
	if (player->god)
	    no_numbers = 1;
	else
	    return RET_SYN;
    }

    if (use_targets && !got_targets)
	return RET_FAIL;

    if (!power_generated) {
	snxtitem_all(&ni, EF_POWER);
	if (!nxtitem(&ni, (s_char *)&pow)) {
	    pr("Power for this game has not been built yet.  Type 'power new' to build it.\n");
	    return RET_FAIL;
	}
    }
    pr("     - = [   Empire Power Report   ] = -\n");
    pow_time = ef_mtime(EF_POWER);
    pr("      as of %s\n         sects  eff civ", ctime(&pow_time));
    pr("  mil  shell gun pet  iron dust oil  pln ship unit money\n");
    snxtitem_all(&ni, EF_POWER);
    while ((nxtitem(&ni, (s_char *)&pow)) && num > 0) {
	if (pow.p_nation == 0 || pow.p_power <= 0.0)
	    continue;
	if (opt_HIDDEN) {
	    if (!player->god && pow.p_nation != player->cnum)
		continue;
	}
	natp2 = getnatp(pow.p_nation);
	if (natp2->nat_stat & STAT_GOD)
	    continue;
	if (use_targets && !targets[pow.p_nation])
	    continue;
	if (pow.p_nation != player->cnum && player->cnum != 0)
	    round_flag = 1;
	else
	    round_flag = 0;
	num--;
	pr("%9.9s", cname(pow.p_nation));
	out5(pow.p_sects, 5, round_flag);
	if (pow.p_sects)
	    pr("%4.0f%%", pow.p_effic / pow.p_sects);
	else
	    pr("   0%%");
	out5(pow.p_civil, 50, round_flag);
	out5(pow.p_milit, 50, round_flag);
	out5(pow.p_shell, 25, round_flag);
	out5(pow.p_guns, 5, round_flag);
	out5(pow.p_petrol, 50, round_flag);
	out5(pow.p_iron, 50, round_flag);
	out5(pow.p_dust, 50, round_flag);
	out5(pow.p_oil, 50, round_flag);
	out5(pow.p_planes, 10, round_flag);
	out5(pow.p_ships, 10, round_flag);
	out5(pow.p_units, 10, round_flag);
	out5(pow.p_money, 5000, round_flag);
	pr("\n");
	if (player->god != 0 && !no_numbers)
	    pr("%9.2f\n", pow.p_power);
    }
    if (((player->god != 0) && opt_HIDDEN) || !opt_HIDDEN) {
	pr("          ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----\n");
	getpower(0, &pow);
	pr("worldwide");
	out5(pow.p_sects, 5, 1);
	pr("%4.0f%%", pow.p_effic / (pow.p_sects + 0.1));
	out5(pow.p_civil, 50, 1);
	out5(pow.p_milit, 50, 1);
	out5(pow.p_shell, 25, 1);
	out5(pow.p_guns, 5, 1);
	out5(pow.p_petrol, 50, 1);
	out5(pow.p_iron, 50, 1);
	out5(pow.p_dust, 50, 1);
	out5(pow.p_oil, 50, 1);
	out5(pow.p_planes, 10, 1);
	out5(pow.p_ships, 10, 1);
	out5(pow.p_units, 10, 1);
	out5(pow.p_money, 5000, 1);
	pr("\n");
    }
    return RET_OK;
}

static void
out5(double value, int round_val, int round_flag)
{
    if (value > round_val && round_flag)
	value = (int)(value / round_val + 0.5) * round_val;
    if (value < -995000.)
	pr("%4.0fM", value / 1e6);
    else if (value < -9950.)
	pr("%4.0fK", value / 1000.);
    else if (value < -999.)
	pr("%4.1fK", value / 1000.);
    else if (value < 1000.)
	pr("%4.0f ", value);
    else if (value < 10000.)
	pr("%4.1fK", value / 1000.);
    else if (value < 1e6)
	pr("%4.0fK", value / 1000.);
    else if (value < 1e7)
	pr("%4.1fM", value / 1e6);
    else if (value >= 1e9)
	pr("%4.0fG", value / 1e9);
    else
	pr("%4.0fM", value / 1e6);
}

static void
gen_power(void)
{
    extern double powe_cost;
    float *f_ptr;
    float *f_pt2;
    register struct powstr *pow;
    register int i;
    struct sctstr sect;
    struct plnstr plane;
    struct shpstr ship;
    struct lndstr land;
    struct powstr powbuf[MAXNOC];
    struct nstr_item ni;
    struct nstr_sect ns;
    int vec[I_MAX + 1];
    struct powsort order[MAXNOC];
    struct natstr *natp;
    float f;

    player->btused += powe_cost;
    memset(powbuf, 0, sizeof(powbuf));
    snxtsct_all(&ns);
    while (nxtsct(&ns, &sect)) {
	if (sect.sct_own == 0)
	    continue;
	pow = &powbuf[sect.sct_own];
	pow->p_sects += 1.0;
	pow->p_effic += sect.sct_effic;
	getvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
	addtopow(vec, pow);
    }
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (land.lnd_own == 0)
	    continue;
	pow = &powbuf[land.lnd_own];
	getvec(VT_ITEM, vec, (s_char *)&land, EF_LAND);
	addtopow(vec, pow);
	if (opt_NEWPOWER == 0) {
	    pow->p_power += lchr[(int)land.lnd_type].l_lcm / 10.0;
	    pow->p_power += lchr[(int)land.lnd_type].l_hcm / 5.0;
	} else {		/* old power */
	    f = ((float)(lchr[(int)land.lnd_type].l_lcm / 10.0)) *
		((float)land.lnd_effic) / 100.0;
	    f += ((float)(lchr[(int)land.lnd_type].l_hcm / 10.0)) *
		((float)land.lnd_effic / 100.0);
	    pow->p_power += f * 2;
	}			/* end NEWPOWER */
	pow->p_units += 1.0;
    }
    snxtitem_all(&ni, EF_SHIP);
    while (nxtitem(&ni, (s_char *)&ship)) {
	if (ship.shp_own == 0)
	    continue;
	pow = &powbuf[ship.shp_own];
	getvec(VT_ITEM, vec, (s_char *)&ship, EF_SHIP);
	addtopow(vec, pow);
	if (opt_NEWPOWER == 0) {
	    pow->p_power += mchr[(int)ship.shp_type].m_lcm / 10.0;
	    pow->p_power += mchr[(int)ship.shp_type].m_hcm / 5.0;
	} else {		/* old power formula */
	    f = ((float)(mchr[(int)ship.shp_type].m_lcm / 10.0)) *
		((float)ship.shp_effic) / 100.0;
	    f += ((float)(mchr[(int)ship.shp_type].m_hcm / 10.0)) *
		((float)ship.shp_effic / 100.0);
	    pow->p_power += f * 2;
	}			/* end NEWPOWER */
	pow->p_ships += 1.0;
    }
    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, (s_char *)&plane)) {
	if (plane.pln_own == 0)
	    continue;
	pow = &powbuf[plane.pln_own];
	pow->p_planes += 1.0;
	if (opt_NEWPOWER == 0)
	    pow->p_power += plane.pln_effic / 100.0;
	else {			/* old POWER */
	    natp = getnatp(plane.pln_own);
	    pow->p_power += 20 * (plane.pln_effic / 100.0) *
		(natp->nat_level[NAT_TLEV] / 500.0);
	}			/* end old POWER */
    }
    for (i = 1; NULL != (natp = getnatp(i)); i++) {
	pow = &powbuf[i];
	pow->p_nation = i;
	if ((natp->nat_stat & STAT_INUSE) == 0 ||
	    (natp->nat_stat & STAT_NORM) == 0) {
	    pow->p_power = 0.;
	    continue;
	}
	if (opt_NEWPOWER && (natp->nat_stat & STAT_GOD)) {
	    pow->p_power = 0.;
	    continue;
	}
	pow->p_money = natp->nat_money;
	pow->p_power += pow->p_money / 100.;

	pow->p_power += pow->p_petrol / (opt_NEWPOWER ? 500.0 : 50.0);

	if (opt_NEWPOWER == 0) {
	    pow->p_power += (pow->p_civil + pow->p_milit +
			     pow->p_shell) / 10.;
	    pow->p_power += (pow->p_iron + pow->p_dust +
			     pow->p_effic + pow->p_oil) / 10.;
	    pow->p_power += (pow->p_guns + pow->p_effic) / 3.;
	    pow->p_power += pow->p_ships;
	    pow->p_power += pow->p_sects * 3.0;
	    pow->p_power += pow->p_planes * 5.0;
	} else {		/* new POWER format */
	    pow->p_power += (pow->p_civil + pow->p_milit) / 10.0;
	    pow->p_power += (pow->p_shell) / 12.5;
	    pow->p_power += (pow->p_iron) / 100.0;
	    pow->p_power += (pow->p_dust / 5 +
			     pow->p_oil / 10 + pow->p_bars);
	    pow->p_power += (pow->p_guns) / 2.5;
	    if (pow->p_sects > 0)
		pow->p_power += (pow->p_sects *
				 ((pow->p_effic / pow->p_sects) /
				  100.0)) * 10.0;
	    if (natp->nat_level[NAT_TLEV] > 0.0)
		pow->p_power = pow->p_power *
		    (((float)natp->nat_level[NAT_TLEV]) / 500.0);
	    else
		pow->p_power = pow->p_power * (1.0 / 500.0);
	}			/* end new POWER */
	/* ack.  add this vec to the "world power" element */
	f_pt2 = &(powbuf[0].p_sects);
	f_ptr = &(pow->p_sects);
	while (f_ptr <= &(pow->p_power)) {
	    *f_pt2 += *f_ptr;
	    f_pt2++;
	    f_ptr++;
	}
    }
    for (i = 1; i < MAXNOC; i++) {
	struct natstr *np;
	int maxpop;
	float f = 1.0;

	if (opt_RES_POP) {
	    np = getnatp(i);
	    maxpop = max_pop(np->nat_level[NAT_RLEV], 0);
	    f = 1.0 + (((float)maxpop) / 10000.0);
	}
	powbuf[i].p_power *= f;
	order[i].powval = powbuf[i].p_power;
	order[i].cnum = i;
    }
    qsort((s_char *)&order[1], MAXNOC - 1, sizeof(*order),
	  (qsort_func_t)powcmp);
    putpower(0, &powbuf[0]);
    for (i = 1; i < MAXNOC; i++) {
	putpower(i, &powbuf[order[i].cnum]);
    }
}

static int
powcmp(struct powsort *p1, struct powsort *p2)
{
    if (p1->powval > p2->powval)
	return -1;
    if (p1->powval < p2->powval)
	return 1;
    return 0;
}

static void
addtopow(register int *vec, register struct powstr *pow)
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
    pow->p_power += vec[I_LCM] / 10.0;
    pow->p_power += vec[I_HCM] / 5.0;
}

static int
set_target(s_char *p, int *targets)
{
    int target;
    struct natstr *natp;

    if (!p)
	return 0;

    if (isdigit(*p))
	target = atoi(p);
    else
	target = cnumb(p);

    if (target > 0 && target < MAXNOC && (natp = getnatp(target))) {
	if (natp->nat_stat & STAT_GOD) {
	    pr("Country #%d is a deity country\n", target);
	} else if (!(natp->nat_stat & STAT_INUSE)) {
	    pr("Country #%d is not in use\n", target);
	} else if (!(natp->nat_stat & STAT_NORM)) {
	    pr("Country #%d is not a normal country\n", target);
	} else {
	    targets[target] = 1;
	    return 1;
	}
    } else
	pr("Bad country: \'%s\'\n", p);

    return 0;
}
