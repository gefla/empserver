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
 *  plane.c: Plane characteristics
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Jeff Bailey
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <config.h>

#include <math.h>
#include "misc.h"
#include "plane.h"

/*
 * Table of plane types
 * Initialized on startup from plane.config and deity custom config (if any).
 * Terminated by a sentinel with null pl_name.
 */
struct plchrstr plchr[PLN_TYPE_MAX + 2];

#define logx(a, b) (log((a)) / log((b)))
#define PLN_ATTDEF(b, t) (b + ((b?1:0) * ((t/20)>10?10:(t/20))))
#define PLN_ACC(b, t) (b * (1.0 - (sqrt(t) / 50.)))
#define PLN_RAN(b, t) (t ? (b + (logx(t, 2.0))) : b)
#define PLN_LOAD(b, t) (t ? (b * (logx(t, 50.0) < 1.0 ? 1.0 : \
				  logx(t, 50.0))) : b)

int
pl_att(struct plchrstr *pcp, int tech)
{
    return PLN_ATTDEF(pcp->pl_att, MAX(0, tech - pcp->pl_tech));
}

int
pl_def(struct plchrstr *pcp, int tech)
{
    return PLN_ATTDEF(pcp->pl_def, MAX(0, tech - pcp->pl_tech));
}

int
pl_acc(struct plchrstr *pcp, int tech)
{
    return PLN_ACC(pcp->pl_acc, MAX(0, tech - pcp->pl_tech));
}

int
pl_range(struct plchrstr *pcp, int tech)
{
    return PLN_RAN(pcp->pl_range, MAX(0, tech - pcp->pl_tech));
}

int
pl_load(struct plchrstr *pcp, int tech)
{
    return PLN_LOAD(pcp->pl_load, MAX(0, tech - pcp->pl_tech));
}

int
pln_att(struct plnstr *pp)
{
    return pl_att(plchr + pp->pln_type, pp->pln_tech);
}

int
pln_def(struct plnstr *pp)
{
    return pl_def(plchr + pp->pln_type, pp->pln_tech);
}

int
pln_acc(struct plnstr *pp)
{
    return pl_acc(plchr + pp->pln_type, pp->pln_tech);
}

int
pln_range_max(struct plnstr *pp)
{
    return pl_range(plchr + pp->pln_type, pp->pln_tech);
}

int
pln_load(struct plnstr *pp)
{
    return pl_load(plchr + pp->pln_type, pp->pln_tech);
}
