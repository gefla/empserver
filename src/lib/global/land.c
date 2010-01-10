/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  land.c: Land unit characteristics
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include <config.h>

#include <math.h>
#include "misc.h"
#include "land.h"

/*
 * Table of land unit types
 * Initialized on startup from land.config and deity custom config (if any).
 * Terminated by a sentinel with null l_name.
 */
struct lchrstr lchr[LND_TYPE_MAX + 2];

#define logx(a, b) (log((a)) / log((b)))
#define LND_ATTDEF(b, t) (((b) * (1.0 + ((sqrt((t)) / 100.0) * 4.0)))	\
			  > 127 ? 127 :					\
			  ((b) * (1.0 + ((sqrt((t)) / 100.0) * 4.0))))
#define LND_SPD(b, t) ((b * (1.0 + ((sqrt(t) / 100.0) * 2.1))) > 127	\
		       ? 127 : (b * (1.0 + ((sqrt(t) / 100.0) * 2.1))))
#define LND_VUL(b, t) ((b * (1.0 - ((sqrt(t) / 100.0) * 1.1))) < 0	\
		       ? 0 : (b * (1.0 - ((sqrt(t) / 100.0) * 1.1))))
#define LND_FRG(b, t) ((t) ?				     \
		       ((b) * (logx((t), 35.0) < 1.0 ? 1.0 : \
			       logx((t), 35.0))) : (b))
#define LND_DAM(b, t) ((t) ?				     \
		       ((b) * (logx((t), 60.0) < 1.0 ? 1.0 : \
			       logx((t), 60.0))) : (b))
#define LND_ACC(b, t) ((b * (1.0 - ((sqrt(t) / 100.0) * 1.1))) < 0	\
		       ? 0 : (b * (1.0 - ((sqrt(t) / 100.0) * 1.1))))
#define LND_AAF(b, t) ((b * (1.0 + ((sqrt(t) / 100.0) * 3.0))) > 127	\
		       ? 127 : (b * (1.0 + ((sqrt(t) / 100.0) * 3.0))))

float
l_att(struct lchrstr *lcp, int tech)
{
    return LND_ATTDEF(lcp->l_att, MAX(0, tech - lcp->l_tech));
}

float
l_def(struct lchrstr *lcp, int tech)
{
    return LND_ATTDEF(lcp->l_def, MAX(0, tech - lcp->l_tech));
}

int
l_vul(struct lchrstr *lcp, int tech)
{
    return LND_VUL(lcp->l_vul, MAX(0, tech - lcp->l_tech));
}

int
l_spd(struct lchrstr *lcp, int tech)
{
    return LND_SPD(lcp->l_spd, MAX(0, tech - lcp->l_tech));
}

int
l_frg(struct lchrstr *lcp, int tech)
{
    return LND_FRG(lcp->l_frg, MAX(0, tech - lcp->l_tech));
}

int
l_acc(struct lchrstr *lcp, int tech)
{
    return LND_ACC(lcp->l_acc, MAX(0, tech - lcp->l_tech));
}

int
l_dam(struct lchrstr *lcp, int tech)
{
    return LND_DAM(lcp->l_dam, MAX(0, tech - lcp->l_tech));
}

int
l_aaf(struct lchrstr *lcp, int tech)
{
    return LND_AAF(lcp->l_aaf, MAX(0, tech - lcp->l_tech));
}

float
lnd_att(struct lndstr *lp)
{
    return l_att(lchr + lp->lnd_type, lp->lnd_tech);
}

float
lnd_def(struct lndstr *lp)
{
    return l_def(lchr + lp->lnd_type, lp->lnd_tech);
}

int
lnd_vul(struct lndstr *lp)
{
    return l_vul(lchr + lp->lnd_type, lp->lnd_tech);
}

int
lnd_spd(struct lndstr *lp)
{
    return l_spd(lchr + lp->lnd_type, lp->lnd_tech);
}

int
lnd_vis(struct lndstr *lp)
{
    return lchr[lp->lnd_type].l_vis;
}

int
lnd_frg(struct lndstr *lp)
{
    return l_frg(lchr + lp->lnd_type, lp->lnd_tech);
}

int
lnd_acc(struct lndstr *lp)
{
    return l_acc(lchr + lp->lnd_type, lp->lnd_tech);
}

int
lnd_dam(struct lndstr *lp)
{
    return l_dam(lchr + lp->lnd_type, lp->lnd_tech);
}

int
lnd_aaf(struct lndstr *lp)
{
    return l_aaf(lchr + lp->lnd_type, lp->lnd_tech);
}
