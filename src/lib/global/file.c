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
 *  fileglb.c: Empire selection global structures.
 * 
 *  Known contributors to this file:
 *     
 */

#include <struct.h>
#include "misc.h"
#include "var.h"
#include "xy.h"
#include "loan.h"
#include "nsc.h"
#include "nuke.h"
#include "plane.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "trade.h"
#include "treaty.h"
#include "file.h"
#include "power.h"
#include "news.h"
#include "nat.h"
#include "lost.h"
#include "optlist.h"

#include "gamesdef.h"
#include "commodity.h"

#if !defined(_WIN32)
#define FILEPATH(p) EMPPATH(data/p)
#else
#define FILEPATH(p) EMPPATH(data\\p)
#endif

struct empfile empfile[] = {
	{ "sect", FILEPATH(sector), EFF_COM|EFF_XY|EFF_OWNER,
	     0, sizeof(struct sctstr),
	    0, 0, 0, { fldoff(sctstr, sct_nv), fldoff(sctstr, sct_vtype[0]),
	    fldoff(sctstr, sct_vamt[0]) }, MAXSCTV, 
	    -1, -1, 0, 0, 0, 0, 0 },
	{ "ship", FILEPATH(ship), EFF_COM|EFF_XY|EFF_OWNER|EFF_GROUP,
	    0, sizeof(struct shpstr),
	    0, 0, 0, { fldoff(shpstr, shp_nv), fldoff(shpstr, shp_vtype[0]),
	    fldoff(shpstr, shp_vamt[0]) }, MAXSHPV, 
	    -1, -1, 0, 0, 0, 0, 0 },
	{ "plane", FILEPATH(plane), EFF_XY|EFF_OWNER|EFF_GROUP,
	     0, sizeof(struct plnstr),
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "land", FILEPATH(land), EFF_COM|EFF_XY|EFF_OWNER|EFF_GROUP,
	    0, sizeof(struct lndstr),
	    0, 0, 0, { fldoff(lndstr, lnd_nv), fldoff(lndstr, lnd_vtype[0]),
	    fldoff(lndstr, lnd_vamt[0]) }, MAXLNDV, 
	    -1, -1, 0, 0, 0, 0, 0 },
	{ "nuke", FILEPATH(nuke), EFF_XY|EFF_OWNER, 0, sizeof(struct nukstr),
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "news", FILEPATH(news),              0,
	     0, sizeof(struct nwsstr),
	     0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "treaty", FILEPATH(treaty), 0, 0, sizeof(struct trtstr),
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "trade", FILEPATH(trade),           0, 0, sizeof(struct trdstr),
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "pow", FILEPATH(power),     0, 0, sizeof(struct powstr),
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "nat", FILEPATH(nation),           0, 0, sizeof(struct natstr),
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "loan", FILEPATH(loan),              0,
	     0, sizeof(struct lonstr),
	     0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "map", FILEPATH(map),		0, 0, DEF_WORLD_X*DEF_WORLD_Y/2,
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "bmap", FILEPATH(bmap),		0, 0, DEF_WORLD_X*DEF_WORLD_Y/2,
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "commodity", FILEPATH(commodity),0,0,sizeof(struct comstr),
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 },
	{ "lost", FILEPATH(lostitems),    0, 0, sizeof(struct loststr),
	    0, 0, 0, { 0, 0, 0 }, 0, -1, -1, 0, 0, 0, 0, 0 }
};
