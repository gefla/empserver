/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  xdump.c: Extended dumps
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2008
 */

/*
 * Dump everything under the sun
 *
 * Static game data (configuration):
 * - Item characteristics: ichr[]
 * - Land unit characteristics: lchr[]
 * - Nuke characteristics: nchr[]
 * - Plane characteristics: plchr[]
 * - Product characteristics: pchr[]
 * - Sector designation characteristics: dchr[]
 * - Sector infrastructure characteristics: intrchr[]
 * - Ship characteristics: mchr[]
 * - News item characteristics: rpt[]
 * - News page headings: page_headings[]
 * - Commands: player_coms[] (TODO)
 * - Update schedule: update_time[] (not really static)
 * - Configuration: configkeys[]
 *
 * Dynamic game data:
 * - Sectors: EF_SECTOR (superseding dump)
 * - Land units: EF_LAND (superseding ldump)
 * - Lost: EF_LOST (superseding lost)
 * - Nukes: EF_NUKE (superseding ndump)
 * - Planes: EF_PLANE (superseding pdump)
 * - Ships: EF_SHIP (superseding sdump)
 * - News: EF_NEWS
 * - Treaties: EF_TREATY
 * - Power: EF_POWER (TODO)
 * - Nations: EF_NATION
 * - Loans: EF_LOAN
 * - Map: EF_MAP (TODO)
 * - Bmap: EF_BMAP (TODO)
 * - Market: EF_COMM
 */

/*
 * See doc/xdump for motivation, syntax, semantics, and rationale.
 * Make sure to keep it up-to-date!
 */

/* TODO don't dump stuff that's useless due to options */

#include <config.h>

#include <ctype.h>
#include <limits.h>
#include "file.h"
#include "nat.h"
#include "xdump.h"

/*
 * Initialize XD.
 * Translate dump for country CNUM, except when CNUM is NATID_BAD.
 * If HUMAN, dump in human-readable format.
 * Dump is to be delivered through callback PR.
 * Return XD.
 */
struct xdstr *
xdinit(struct xdstr *xd, natid cnum, int human, void (*pr)(char *fmt, ...))
{
    xd->cnum = cnum;
    xd->divine = cnum == NATID_BAD || getnatp(cnum)->nat_stat == STAT_GOD;
    xd->human = human;
    xd->pr = pr;
    return xd;
}

/*
 * Evaluate a attribute of an object into VAL, return VAL.
 * CA describes the attribute.
 * XD is the xdump descriptor.
 * PTR points to the context object.
 * IDX is the index within the attribute.
 */
static struct valstr *
xdeval(struct valstr *val, struct xdstr *xd,
       struct castr *ca, void *ptr, int idx)
{
    nstr_mksymval(val, ca, idx);
    return nstr_exec_val(val, xd->cnum, ptr, NSC_NOTYPE);
}

/*
 * Dump string STR to XD with funny characters escaped.
 * Dump at most N characters.
 */
static void
xdpresc(struct xdstr *xd, char *str, size_t n)
{
    unsigned char *s, *e, *l;

    s = (unsigned char *)str;
    l = s + n;
    for (;;) {
	for (e = s;
	     e < l && *e != '"' && *e != '\\' && isgraph(*e);
	     ++e)
	    ;
	xd->pr("%.*s", (int)(e-s), s);
	if (e < l && *e)
	    xd->pr("\\%03o", *e++);
	else
	    break;
	s = e;
    }
}

/*
 * Dump VAL prefixed with SEP to XD, in machine readable format.
 * VAL must be evaluated.
 * Return " ".
 */
static char *
xdprval_nosym(struct xdstr *xd, struct valstr *val, char *sep)
{
    if (CANT_HAPPEN(val->val_cat != NSC_VAL)) {
	xd->pr("%snil", sep);
	return " ";
    }

    switch (val->val_type) {
    case NSC_LONG:
	xd->pr("%s%ld", sep, val->val_as.lng);
	break;
    case NSC_DOUBLE:
	xd->pr("%s%#g", sep, val->val_as.dbl);
	break;
    case NSC_STRING:
	if (val->val_as.str.base) {
	    xd->pr("%s\"", sep);
	    xdpresc(xd, val->val_as.str.base, val->val_as.str.maxsz);
	    xd->pr("\"");
	} else
	    xd->pr("%snil", sep);
	break;
    default:
	CANT_REACH();
	xd->pr("%snil", sep);
    }
    return " ";
}

/*
 * Dump symbol with value KEY from symbol table TYPE to XD.
 * Prefix with SEP, return " ".
 */
static char *
xdprsym(struct xdstr *xd, int key, int type, char *sep)
{
    char *sym = symbol_by_value(key, ef_ptr(type, 0));

    if (CANT_HAPPEN(!sym))
	xd->pr("%s%d", sep, key);
    else {
	xd->pr("%s", sep);
	xdpresc(xd, sym, INT_MAX);
    }
    return " ";
}

/*
 * Dump VAL prefixed with SEP to XD, return " ".
 * VAL must be evaluated.
 * CA describes the field from which the value was fetched.
 */
static char *
xdprval_sym(struct xdstr *xd, struct valstr *val, struct castr *ca,
	    char *sep)
{
    unsigned long bit;

    if (CANT_HAPPEN(val->val_cat != NSC_VAL)) {
	xd->pr("%snil", sep);
	return " ";
    }

    if (!xd->human || val->val_type != NSC_LONG
	|| ca->ca_table == EF_BAD || ef_cadef(ca->ca_table) != symbol_ca)
	return xdprval_nosym(xd, val, sep);

    if (ca->ca_flags & NSC_BITS) {
	xd->pr("%s(", sep);
	sep = "";
	for (bit = 1; bit; bit <<= 1) {
	    if (bit & val->val_as.lng)
		sep = xdprsym(xd, bit, ca->ca_table, sep);
	}
	xd->pr(")");
	return " ";
    }

    return xdprsym(xd, val->val_as.lng, ca->ca_table, sep);
}

/*
 * Dump field values of a context object to XD.
 * CA[] describes fields.
 * PTR points to context object.
 */
void
xdflds(struct xdstr *xd, struct castr ca[], void *ptr)
{
    int i, j, n;
    struct valstr val;
    char *sep = "";

    for (i = 0; ca[i].ca_name; ++i) {
	if (ca[i].ca_flags & NSC_DEITY && !xd->divine)
	    continue;
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	n = ca[i].ca_type != NSC_STRINGY ? ca[i].ca_len : 0;
	j = 0;
	do {
	    xdeval(&val, xd, &ca[i], ptr, j);
	    sep = xdprval_sym(xd, &val, &ca[i], sep);
	} while (++j < n);
    }
}

/*
 * Dump header for dump NAME to XD.
 * If META, it's for the meta-data dump rather than the data dump.
 */
void
xdhdr(struct xdstr *xd, char *name, int meta)
{
    if (xd->human) {
	xd->pr("config %s\n", name);
    } else
	xd->pr("XDUMP %s%s %ld\n",
	       meta ? "meta " : "",
	       name, (long)time(NULL));
}

/*
 * Dump column header to XD.
 * CA[] describes fields.
 * Does nothing unless XD is human-readable.
 */
void
xdcolhdr(struct xdstr *xd, struct castr ca[])
{
    int i, j, n;
    char *sep = "";

    if (!xd->human)
	return;

    for (i = 0; ca[i].ca_name; ++i) {
	if (ca[i].ca_flags & NSC_DEITY && !xd->divine)
	    continue;
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	n = ca[i].ca_type != NSC_STRINGY ? ca[i].ca_len : 0;
	if (n) {
	    for (j = 0; j < n; j++) {
		xd->pr("%s%s(%d)", sep, ca[i].ca_name, j);
		sep = " ";
	    }
	} else {
	    xd->pr("%s%s", sep, ca[i].ca_name);
	    sep = " ";
	}
    }
    xd->pr("\n");
}

/* Dump footer for a dump that dumped N objects to XD.  */
void
xdftr(struct xdstr *xd, int n)
{
    if (xd->human)
	xd->pr("/config\n");
    else
	xd->pr("/%d\n", n);
}
