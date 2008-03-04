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
 *  xdump.c: Extended dump
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2007
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
#include <stddef.h>
#include "commands.h"
#include "empobj.h"
#include "match.h"
#include "news.h"
#include "optlist.h"
#include "treaty.h"
#include "version.h"

/*
 * Evaluate a attribute of an object into VAL, return VAL.
 * CA describes the attribute.
 * PTR points to the context object.
 * IDX is the index within the attribute.
 */
static struct valstr *
xdeval(struct valstr *val, struct castr *ca, void *ptr, int idx)
{
    nstr_mksymval(val, ca, idx);
    return nstr_exec_val(val, player->cnum, ptr, NSC_NOTYPE);
}

/*
 * Dump VAL prefixed with SEP, return " ".
 * VAL must be evaluated.
 */
static char *
xdprval(struct valstr *val, char *sep)
{
    unsigned char *s, *e, *l;

    if (CANT_HAPPEN(val->val_cat != NSC_VAL)) {
	pr("%snil", sep);
	return " ";
    }

    switch (val->val_type) {
    case NSC_LONG:
	pr("%s%ld", sep, val->val_as.lng);
	break;
    case NSC_DOUBLE:
	pr("%s%#g", sep, val->val_as.dbl);
	break;
    case NSC_STRING:
	s = (unsigned char *)val->val_as.str.base;
	if (s) {
	    pr("%s\"", sep);
	    l = s + val->val_as.str.maxsz;
	    /* FIXME maxsz == INT_MAX ! */
	    for (;;) {
		for (e = s;
		     e < l && *e != '"' && *e != '\\' && isgraph(*e);
		     ++e)
		    ;
		pr("%.*s", (int)(e-s), s);
		if (e < l && *e)
		    pr("\\%03o", *e++);
		else
		    break;
		s = e;
	    }
	    pr("\"");
	} else
	    pr("%snil", sep);
	break;
    default:
	CANT_REACH();
	pr("%snil", sep);
    }
    return " ";
}

/*
 * Dump field values of a context object.
 * CA[] describes fields.
 * PTR points to context object.
 */
static void
xdflds(struct castr ca[], void *ptr)
{
    int i, j, n;
    struct valstr val;
    char *sep = "";

    for (i = 0; ca[i].ca_name; ++i) {
	if (ca[i].ca_flags & NSC_DEITY && !player->god)
	    continue;
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	n = ca[i].ca_type != NSC_STRINGY ? ca[i].ca_len : 0;
	j = 0;
	do {
	    xdeval(&val, &ca[i], ptr, j);
	    sep = xdprval(&val, sep);
	} while (++j < n);
    }
}

/*
 * Dump header for dump NAME.
 * If META, it's for the meta-data dump rather than the data dump.
 */
static void
xdhdr(char *name, int meta)
{
    pr("XDUMP %s%s %ld\n", meta ? "meta " : "", name, (long)time(NULL));
}

/* Dump footer for a dump that dumped N objects.  */
static void
xdftr(int n)
{
    pr("/%d\n", n);
}

/*
 * Is object P of type TYPE visible to the player?
 * TODO: Fold this into interators.
 */
static int
xdvisible(int type, void *p)
{
    struct empobj *gp = p;
    struct trtstr *tp = p;
    struct lonstr *lp = p;
    struct natstr *natp;
    int tlev;

    switch (type) {
    case EF_SECTOR:
    case EF_REALM:
	return gp->own == player->cnum || player->god;
    case EF_SHIP:
    case EF_PLANE:
    case EF_LAND:
    case EF_NUKE:
    case EF_LOST:
	return gp->own != 0 && (gp->own == player->cnum || player->god);
    case EF_NATION:
	return ((struct natstr *)p)->nat_stat != STAT_UNUSED;
    case EF_COUNTRY:
	return gp->own == player->cnum;
    case EF_NEWS:
	return ((struct nwsstr *)p)->nws_vrb != 0
	    && (!opt_HIDDEN || player->god); /* FIXME */
    case EF_TREATY:
	return tp->trt_status != TS_FREE
	    && (tp->trt_cna == player->cnum || tp->trt_cnb == player->cnum
		|| player->god);
    case EF_LOAN:
	if (lp->l_status == LS_FREE)
	    return 0;
	if (lp->l_status == LS_SIGNED)
	    return 1;
	return lp->l_loner == player->cnum || lp->l_lonee == player->cnum
	    || player->god;
    case EF_TRADE:
    case EF_COMM:
	return gp->own != 0;
    case EF_SHIP_CHR:
	tlev = ((struct mchrstr *)p)->m_tech;
	goto tech;
    case EF_PLANE_CHR:
	tlev = ((struct plchrstr *)p)->pl_tech;
	goto tech;
    case EF_LAND_CHR:
	tlev = ((struct lchrstr *)p)->l_tech;
    tech:
	natp = getnatp(player->cnum);
	return player->god || tlev <= (int)(1.25 * natp->nat_level[NAT_TLEV]);
    case EF_NUKE_CHR:
	tlev = ((struct nchrstr *)p)->n_tech;
	if (drnuke_const > MIN_DRNUKE_CONST) {
	    natp = getnatp(player->cnum);
	    if (tlev > (int)((int)(1.25 * natp->nat_level[NAT_RLEV])
			     / drnuke_const))
		return player->god;
	}
	goto tech;
    case EF_NEWS_CHR:
	return ((struct rptstr *)p)->r_newspage != 0;
    case EF_TABLE:
	return ((struct empfile *)p)->cadef != NULL;
    default:
	return 1;
    }
}

/*
 * Dump items of type TYPE selected by ARG.
 * Return RET_OK on success, RET_SYN on error.
 */
static int
xditem(int type, char *arg)
{
    struct castr *ca;
    struct nstr_item ni;
    int n;
    char buf[2048];		/* FIXME buffer size? */

    ca = ef_cadef(type);
    if (!ca)
	return RET_SYN;

    if (!snxtitem(&ni, type, arg))
	return RET_SYN;

    xdhdr(ef_nameof(type), 0);

    n = 0;
    while (nxtitem(&ni, buf)) {
	if (!xdvisible(type, buf))
	    continue;
	++n;
	xdflds(ca, buf);
	pr("\n");
    }

    xdftr(n);

    return RET_OK;
}

/*
 * Dump meta-data for items of type TYPE.
 * Return RET_OK.
 */
static int
xdmeta(int type)
{
    struct castr *ca = ef_cadef(type);
    int i;
    int n = 0;

    if (!ca)
	return RET_SYN;

    xdhdr(ef_nameof(type), 1);

    for (i = 0; ca[i].ca_name; i++) {
	if (ca[i].ca_flags & NSC_DEITY && !player->god)
	    continue;
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	xdflds(mdchr_ca, &ca[i]);
	pr("\n");
	n++;
    }

    xdftr(n);

    return RET_OK;
}

/* Extended dump command */
int
xdump(void)
{
    char *p;
    char buf[1024];
    int type;
    int meta = 0;
    struct natstr *natp;

    p = getstarg(player->argp[1], "Table name, or meta? ", buf);
    if (p && strcmp(p, "meta") == 0) {
	meta = 1;
	p = getstarg(player->argp[2], "Table name? ", buf);
    }
    if (!p || !*p)
	return RET_SYN;

    natp = getnatp(player->cnum);
    type = isdigit(p[0]) ? atoi(p) : ef_byname(p);
    if (type < 0 || type >= EF_MAX)
	return RET_SYN;

    if (meta)
	return xdmeta(type);
    if ((EF_IS_GAME_STATE(type) || EF_IS_VIEW(type))
	&& !(natp->nat_stat == STAT_ACTIVE || player->god)) {
	pr("Access to table %s denied\n", ef_nameof(type));
	return RET_FAIL;
    }
    if (type == EF_VERSION && !player->argp[2])
	return xditem(type, "*"); /* backward compatibility */
    return xditem(type, player->argp[2]);
}
