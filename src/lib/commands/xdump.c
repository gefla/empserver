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
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "empobj.h"
#include "optlist.h"
#include "version.h"
#include "xdump.h"

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
 * Dump items of type TYPE selected by ARG to XD.
 * Return RET_OK on success, RET_SYN on error.
 */
static int
xditem(struct xdstr *xd, int type, char *arg)
{
    struct castr *ca;
    struct nstr_item ni;
    int n;
    char buf[2048];		/* FIXME buffer size? */

    ca = ef_cadef(type);
    if (!ca)
	return RET_SYN;

    if (!snxtitem(&ni, type, arg, NULL))
	return RET_SYN;

    xdhdr(xd, ef_nameof(type), 0);

    n = 0;
    while (nxtitem(&ni, buf)) {
	if (!xdvisible(type, buf))
	    continue;
	++n;
	xdflds(xd, ca, buf);
	xd->pr("\n");
    }

    xdftr(xd, n);

    return RET_OK;
}

/* Extended dump command */
int
xdump(void)
{
    char *p;
    char buf[1024];
    struct xdstr xd;
    struct natstr *natp;
    int type;
    int meta = 0;

    p = getstarg(player->argp[1], "Table name, or meta? ", buf);
    if (p && strcmp(p, "meta") == 0) {
	meta = 1;
	p = getstarg(player->argp[2], "Table name? ", buf);
    }
    if (!p || !*p)
	return RET_SYN;

    xdinit(&xd, player->cnum, 0, pr);
    natp = getnatp(player->cnum);
    type = isdigit(p[0]) ? atoi(p) : ef_byname(p);
    if (type < 0 || type >= EF_MAX)
	return RET_SYN;
    if (meta)
	return xdmeta(&xd, type);
    if ((EF_IS_GAME_STATE(type) || EF_IS_VIEW(type))
	&& !(natp->nat_stat == STAT_ACTIVE || player->god)) {
	pr("Access to table %s denied\n", ef_nameof(type));
	return RET_FAIL;
    }
    if (type == EF_VERSION && !player->argp[2])
	return xditem(&xd, type, "*"); /* backward compatibility */
    return xditem(&xd, type, player->argp[2]);
}
