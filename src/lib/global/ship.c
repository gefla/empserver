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
 *  ship.c: Ship characteristics
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
#include "ship.h"

/*
 * Table of ship types
 * Initialized on startup from ship.config and deity custom config (if any).
 * Terminated by a sentinel with null m_name.
 */
struct mchrstr mchr[SHP_TYPE_MAX + 2];

#define logx(a, b) (log((a)) / log((b)))
#define SHP_DEF(b, t) (t ? (b * (logx(t, 40.0) < 1.0 ? 1.0 : \
				 logx(t, 40.0))) : b)
#define SHP_SPD(b, t) (t ? (b * (logx(t, 35.0) < 1.0 ? 1.0 : \
				 logx(t, 35.0))) : b)
#define SHP_VIS(b, t) (b * (1 - (sqrt(t) / 50)))
#define SHP_RNG(b, t) (t ? (b * (logx(t, 35.0) < 1.0 ? 1.0 : \
				 logx(t, 35.0))) : b)
#define SHP_FIR(b, t) (t ? (b * (logx(t, 60.0) < 1.0 ? 1.0 : \
				 logx(t, 60.0))) : b)

int
m_armor(struct mchrstr *mcp, int tech)
{
    return SHP_DEF(mcp->m_armor, MAX(0, tech - mcp->m_tech));
}

int
m_speed(struct mchrstr *mcp, int tech)
{
    return SHP_SPD(mcp->m_speed, MAX(0, tech - mcp->m_tech));
}

int
m_visib(struct mchrstr *mcp, int tech)
{
    return SHP_VIS(mcp->m_visib, MAX(0, tech - mcp->m_tech));
}

int
m_frnge(struct mchrstr *mcp, int tech)
{
    return SHP_RNG(mcp->m_frnge, MAX(0, tech - mcp->m_tech));
}

int
m_glim(struct mchrstr *mcp, int tech)
{
    return SHP_FIR(mcp->m_glim, MAX(0, tech - mcp->m_tech));
}

int
shp_armor(struct shpstr *sp)
{
    return m_armor(mchr + sp->shp_type, sp->shp_tech);
}

int
shp_speed(struct shpstr *sp)
{
    return m_speed(mchr + sp->shp_type, sp->shp_tech);
}

int
shp_visib(struct shpstr *sp)
{
    return m_visib(mchr + sp->shp_type, sp->shp_tech);
}

int
shp_frnge(struct shpstr *sp)
{
    return m_frnge(mchr + sp->shp_type, sp->shp_tech);
}

int
shp_glim(struct shpstr *sp)
{
    return m_glim(mchr + sp->shp_type, sp->shp_tech);
}
