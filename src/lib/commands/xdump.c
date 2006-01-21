/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  xdump.c: Experimental extended dump
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2006
 */

#include <config.h>

#include <stddef.h>
#include "misc.h"
#include "file.h"
#include "match.h"
#include "news.h"
#include "nsc.h"
#include "optlist.h"
#include "commands.h"

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
 * Less important:
 * - News item characteristics: rpt[]
 * - News page headings: page_headings[] (TODO)
 * - Commands: player_coms[] (TODO)
 * - Configuration: configkeys[]
 *
 * Dynamic game data:
 * - Sectors: EF_SECTOR, sect_ca[] (already have dump)
 * - Land units: EF_LAND, land_ca[] (already have ldump)
 * - Lost: EF_LOST, lost_ca[] (already have lost)
 * - Nukes: EF_NUKE, nuke_ca[] (already have ndump)
 * - Planes: EF_PLANE, plane_ca[] (already have pdump)
 * - Ships: EF_SHIP, ship_ca[] (already have sdump)
 * - News: EF_NEWS, news_ca[]
 * - Treaties: EF_TREATY, treaty_ca[]
 * - Power: EF_POWER
 * - Nations: EF_NATION, nat_ca[]
 * - Loans: EF_LOAN, loan_ca[]
 * - Map: EF_MAP (TODO)
 * - Bmap: EF_BMAP (TODO)
 * - Market: EF_COMM, commodity_ca[]
 */

/* FIXME document dump format */
/* FIXME don't dump stuff that's useless due to options */

/*
 * Evaluate a attribute of an object into VAL, return VAL.
 * TYPE is the attribute's type.
 * PTR points to the context object.
 * The attribute is stored there at offset OFF + IDX * S, where S is
 * its size.
 */
static struct valstr *
xdeval(struct valstr *val, nsc_type type, void *ptr, ptrdiff_t off, int idx)
{
    val->val_type = type;
    val->val_cat = NSC_OFF;
    val->val_as.sym.off = off;
    val->val_as.sym.idx = idx;
    nstr_exec_val(val, player->cnum, ptr, NSC_NOTYPE);
    return val;			/* FIXME nstr_exec_val() should return VAL */
}

/* Dump VAL prefixed with SEP, return " ".  */
static char *
xdprval(struct valstr *val, char *sep)
{
    unsigned char *s, *e, *l;

    switch (val->val_type) {
    case NSC_TYPEID:
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
		for (e=s; e<l && *e != '"' && *e != '\\' && isgraph(*e); ++e) ;
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
	CANT_HAPPEN("Bad VAL type");
	pr("0");
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
	    xdeval(&val, ca[i].ca_type, ptr, ca[i].ca_off, j);
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
    struct genitem *gp = p;
    struct trtstr *tp = p;
    struct lonstr *lp = p;
    struct natstr *natp;
    int tlev;

    switch (type) {
    case EF_SECTOR:
	return gp->own == player->cnum || player->god;
    case EF_SHIP:
    case EF_PLANE:
    case EF_LAND:
    case EF_NUKE:
    case EF_LOST:
	return gp->own != 0 && (gp->own == player->cnum || player->god);
    case EF_NATION:
	if (gp->own == player->cnum || player->god)
	    return 1;
	/* fall through */
    case EF_COUNTRY:
	return ((struct natstr *)p)->nat_stat != STAT_UNUSED;
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
	if (opt_DRNUKE) {
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
    int check_owner = !player->god && (ef_flags(type) & EFF_OWNER) != 0;
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
	n++;
	pr("\n");
    }

    xdftr(n);

    return RET_OK;
}

/*
 * Dump configkeys[], return RET_OK.
 * If META, dump meta-data rather than data.
 */
static int
xdver(int meta)
{
    struct keymatch *kp;
    char *sep;
    int n;
    struct castr ca;
    struct valstr val;

    xdhdr("version", meta);

    if (meta) {
	n = 0;
	for (kp = configkeys; kp->km_key; ++kp) {
	    if (kp->km_type != NSC_NOTYPE && !(kp->km_flags & KM_INTERNAL)) {
		ca.ca_type = kp->km_type;
		ca.ca_flags = 0;
		ca.ca_len = 0;
		ca.ca_off = 0;
		ca.ca_name = kp->km_key;
		ca.ca_table = EF_BAD;
		xdflds(mdchr_ca, &ca);
		pr("\n");
		n++;
	    }
	}
	xdftr(n);
	return RET_OK;
    }

    sep = "";
    for (kp = configkeys; kp->km_key; ++kp) {
	if (kp->km_type != NSC_NOTYPE && !(kp->km_flags & KM_INTERNAL)) {
	    xdeval(&val, kp->km_type, kp->km_data, 0, 0);
	    sep = xdprval(&val, sep);
	}
    }
    pr("\n");

    xdftr(1);

    return RET_OK;
}

/* Experimental extended dump command */
int
xdump(void)
{
    char *p;
    char buf[1024];
    int type;
    int meta = 0;

    if (!opt_GUINEA_PIGS) {
	pr("You are not a guinea pig!\n");
	return RET_FAIL;
    }

    p = getstarg(player->argp[1], "What? ", buf);
    if (p && strcmp(p, "meta") == 0) {
	meta = 1;
	p = getstarg(player->argp[2], "What? ", buf);
    }
    if (!p)
	return RET_SYN;

    type = isdigit(p[0]) ? atoi(p) : ef_byname(p);
    if (type >= 0 && type < EF_MAX) {
	if (meta)
	    return xdmeta(type);
	else
	    return xditem(type, player->argp[2]);
    } else if (!strncmp(p, "ver", strlen(p))) {
	return xdver(meta);
    }

    return RET_SYN;
}
