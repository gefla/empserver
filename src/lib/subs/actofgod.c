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
 *  actofgod.c: Deity meddling subroutines
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2013
 */

#include <config.h>

#include <stdarg.h>
#include "actofgod.h"
#include "file.h"
#include "news.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "unit.h"

static void
nreport_divine_aid(natid whom, int goodness)
{
    if (opt_GODNEWS && getnatp(whom)->nat_stat != STAT_GOD && goodness)
	nreport(player->cnum, goodness > 0 ? N_AIDS : N_HURTS, whom, 1);
}

void
report_god_takes(char *prefix, char *what, natid from)
{
    if (from && from != player->cnum) {
	wu(0, from, "%s%s taken from you by an act of %s!\n",
	   prefix, what, cname(player->cnum));
	nreport_divine_aid(from, -1);
    }
}

void
report_god_gives(char *prefix, char *what, natid to)
{
    if (to && to != player->cnum) {
	wu(0, to, "%s%s given to you by an act of %s!\n",
	   prefix, what, cname(player->cnum));
	nreport_divine_aid(to, 1);
    }
}

/*
 * Report deity meddling with sector SP.
 * Print a message (always), send a bulletin to the sector owner and
 * report news (sometimes).
 * NAME names what is being changed in the sector.
 * If CHANGE is zero, the meddling is a no-op (bulletin suppressed).
 * If CHANGE is negative, it's secret (bulletin suppressed).
 * If a bulletin is sent, report N_AIDS news for positive GOODNESS,
 * N_HURTS news for negative GOODNESS
 * The bulletin's text is like "NAME of sector X,Y changed <how> by an
 * act of <deity>, where <deity> is the deity's name, and <how> comes
 * from formatting printf-style FMT with optional arguments.
 */
void
divine_sct_change(struct sctstr *sp, char *name,
		  int change, int goodness, char *fmt, ...)
{
    va_list ap;
    char buf[4096];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (!change) {
	pr("%s of %s unchanged\n",
	   name, xyas(sp->sct_x, sp->sct_y, player->cnum));
	return;
    }

    pr("%s of %s changed %s\n",
       name, xyas(sp->sct_x, sp->sct_y, player->cnum), buf);
    if (change > 0 && sp->sct_own && sp->sct_own != player->cnum) {
	wu(0, sp->sct_own, "%s of %s changed %s by an act of %s\n",
	   name, xyas(sp->sct_x, sp->sct_y, sp->sct_own),
	   buf, cname(player->cnum));
	nreport_divine_aid(sp->sct_own, goodness);
    }
}

/*
 * Report deity meddling with NP.
 * Just like divine_sct_change(), only for nations.
 */
void
divine_nat_change(struct natstr *np, char *name,
		   int change, int goodness, char *fmt, ...)
{
    va_list ap;
    char buf[4096];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (!change) {
	pr("%s of %s unchanged\n", name, prnat(np));
	return;
    }

    pr("%s of %s changed %s\n", name, prnat(np), buf);
    if (change > 0 && np->nat_cnum != player->cnum) {
	wu(0, np->nat_cnum, "%s changed %s by an act of %s!\n",
	   name, buf, cname(player->cnum));
	nreport_divine_aid(np->nat_cnum, goodness);
    }
}

/*
 * Report deity meddling with UNIT.
 * Just like divine_sct_change(), only for ships, planes, land units,
 * nukes.
 */
void
divine_unit_change(struct empobj *unit, char *name,
		   int change, int goodness, char *fmt, ...)
{
    va_list ap;
    char buf[4096];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (!change) {
	pr("%s of %s unchanged\n", name, unit_nameof(unit));
	return;
    }

    pr("%s of %s changed %s\n", name, unit_nameof(unit), buf);
    if (change > 0 && unit->own && unit->own != player->cnum) {
	wu(0, unit->own, "%s of %s changed %s by an act of %s\n",
	   name, unit_nameof(unit), buf, cname(player->cnum));
	nreport_divine_aid(unit->own, goodness);
    }
}

static void
divine_load_unload(struct empobj *unit, int type, int uid, char *act)
{
    if (uid < 0)
	return;

    pr("%s %s %s #%d\n",
       unit_nameof(unit), act, ef_nameof(type), uid);
    if (unit->own && unit->own != player->cnum)
	wu(0, unit->own,
	   "%s %s %s #%d by an act of %s!\n",
	   unit_nameof(unit), act, ef_nameof(type), uid,
	   cname(player->cnum));
    /* carrier owner could differ; can't be bothered to report to him */
}

void
divine_load(struct empobj *unit, int type, int uid)
{
    union empobj_storage carrier;

    divine_load_unload(unit, type, uid, "loaded onto");
    if (get_empobj(type, uid, &carrier)
	&& (unit->x != carrier.gen.x || unit->y != carrier.gen.y)) {
	pr("%s teleported from %s to %s!",
	   unit_nameof(unit), xyas(unit->x, unit->y, player->cnum),
	   xyas(carrier.gen.x, carrier.gen.y, player->cnum));
	unit_teleport(unit, carrier.gen.x, carrier.gen.y);
    }
}

void
divine_unload(struct empobj *unit, int type, int uid)
{
    divine_load_unload(unit, type, uid, "unloaded from");
}

static int
fmtflags (char *buf, size_t sz, int flags, struct symbol symtab[], int all)
{
    char *sep = "";
    int n, i;
    char *p;

    if (sz)
	buf[0] = 0;
    n = 0;
    for (i = 0; i < 32; i++) {
	if (!(flags & bit(i)))
	    continue;
	p = symbol_by_value(bit(i), symtab);
	if (p)
	    n += snprintf(buf + n, sz - n, "%s%s", sep, p);
	else if (all)
	    n += snprintf(buf + n, sz - n, "%s#%d", sep, i);
	if (CANT_HAPPEN((size_t)n >= sz)) {
	    buf = NULL;
	    sz = n;
	}
	sep = ", ";
    }
    return n;
}

void
divine_flag_change(struct empobj *unit, char *name,
		   int old, int new, struct symbol sym[])
{
    char set[1024], clr[1024];

    if (new == old) {
	pr("%s of %s unchanged\n", name, unit_nameof(unit));
	return;
    }

    fmtflags(set, sizeof(set), new & ~old, sym, 1);
    fmtflags(clr, sizeof(clr), old & ~new, sym, 1);
    pr("%s of %s changed: %s%s%s%s%s\n",
       name, unit_nameof(unit),
       set, set[0] ? " set" : "",
       set[0] && clr[0] ? ", and " : "",
       clr, clr[0] ? " cleared" : "");

    if (fmtflags(set, sizeof(set), new & ~old, sym, 0)
	+ fmtflags(clr, sizeof(clr), old & ~new, sym, 0))
	wu(0, unit->own, "%s of %s changed by an act of %s: %s%s%s%s%s\n",
	   name, unit_nameof(unit), cname(player->cnum),
	   set, set[0] ? " set" : "",
	   set[0] && clr[0] ? " and " : "",
	   clr, clr[0] ? " cleared" : "");
}

/*
 * Report deity giving/taking commodities to/from WHOM.
 * Give AMT of IP in PLACE.
 */
void
report_divine_gift(natid whom, struct ichrstr *ip, int amt, char *place)
{
    if (whom && whom != player->cnum && amt) {
	if (amt > 0) {
	    if (opt_GODNEWS && getnatp(whom)->nat_stat != STAT_GOD)
		nreport(player->cnum, N_GIFT, whom, 1);
	    wu(0, whom, "%s gave you %d %s in %s\n",
	       cname(player->cnum), amt, ip->i_name, place);
	} else {
	    if (opt_GODNEWS && getnatp(whom)->nat_stat != STAT_GOD)
		nreport(whom, N_TAKE, player->cnum, 1);
	    wu(0, whom, "%s stole %d %s from %s\n",
	       cname(player->cnum), -amt, ip->i_name, place);
	}
    }
}
