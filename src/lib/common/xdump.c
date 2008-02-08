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
 *  xdump.c: Extended dumps
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2008
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
#include "file.h"
#include "nat.h"
#include "xdump.h"

/*
 * Initialize XD to dump for country CNUM.
 * Dump is to be delivered through callback PR.
 * Return XD.
 */
struct xdstr *
xdinit(struct xdstr *xd, natid cnum, void (*pr)(char *fmt, ...))
{
    xd->cnum = cnum;
    xd->divine = getnatp(cnum)->nat_stat == STAT_GOD;
    xd->pr = pr;
    return xd;
}

/*
 * Evaluate a attribute of an object into VAL, return VAL.
 * XD is the xdump descriptor.
 * TYPE is the attribute's type.
 * PTR points to the context object.
 * The attribute is stored there at offset OFF + IDX * S, where S is
 * its size.
 * LEN is the #array elements if it is an array, else zero.
 */
struct valstr *
xdeval(struct valstr *val, struct xdstr *xd,
       nsc_type type, void *ptr, ptrdiff_t off, int idx, int len)
{
    val->val_type = type;
    val->val_cat = NSC_OFF;
    val->val_as.sym.off = off;
    val->val_as.sym.len = len;
    val->val_as.sym.idx = idx;
    nstr_exec_val(val, xd->cnum, ptr, NSC_NOTYPE);
    return val;			/* FIXME nstr_exec_val() should return VAL */
}

/* Dump VAL prefixed with SEP, return " ".  */
char *
xdprval(struct xdstr *xd, struct valstr *val, char *sep)
{
    unsigned char *s, *e, *l;

    switch (val->val_type) {
    case NSC_LONG:
	xd->pr("%s%ld", sep, val->val_as.lng);
	break;
    case NSC_DOUBLE:
	xd->pr("%s%#g", sep, val->val_as.dbl);
	break;
    case NSC_STRING:
	s = (unsigned char *)val->val_as.str.base;
	if (s) {
	    xd->pr("%s\"", sep);
	    l = s + val->val_as.str.maxsz;
	    /* FIXME maxsz == INT_MAX ! */
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
	    xd->pr("\"");
	} else
	    xd->pr("%snil", sep);
	break;
    default:
	CANT_REACH();
	xd->pr("0");
    }
    return " ";
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
	    xdeval(&val, xd,
		   ca[i].ca_type, ptr, ca[i].ca_off, j, ca[i].ca_len);
	    sep = xdprval(xd, &val, sep);
	} while (++j < n);
    }
}

/*
 * Dump header for dump NAME.
 * If META, it's for the meta-data dump rather than the data dump.
 */
void
xdhdr(struct xdstr *xd, char *name, int meta)
{
    xd->pr("XDUMP %s%s %ld\n", meta ? "meta " : "", name, (long)time(NULL));
}

/* Dump footer for a dump that dumped N objects.  */
void
xdftr(struct xdstr *xd, int n)
{
    xd->pr("/%d\n", n);
}

/*
 * Dump meta-data for items of type TYPE to XD.
 * Return RET_OK.
 */
int
xdmeta(struct xdstr *xd, int type)
{
    struct castr *ca = ef_cadef(type);
    int i;
    int n = 0;

    if (!ca)
	return RET_SYN;

    xdhdr(xd, ef_nameof(type), 1);

    for (i = 0; ca[i].ca_name; i++) {
	if (ca[i].ca_flags & NSC_DEITY && !xd->divine)
	    continue;
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	xdflds(xd, mdchr_ca, &ca[i]);
	xd->pr("\n");
	n++;
    }

    xdftr(xd, n);

    return RET_OK;
}
