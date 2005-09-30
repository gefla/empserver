/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  file.c: Empire game data file descriptions.
 * 
 *  Known contributors to this file:
 *     
 */

#include <stddef.h>
#include "misc.h"
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

#include "gamesdef.h"
#include "commodity.h"

struct empfile empfile[] = {
    {"sect", "sector", EFF_XY | EFF_OWNER,
     0, sizeof(struct sctstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"ship", "ship", EFF_XY | EFF_OWNER | EFF_GROUP,
     0, sizeof(struct shpstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"plane", "plane", EFF_XY | EFF_OWNER | EFF_GROUP,
     0, sizeof(struct plnstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"land", "land", EFF_XY | EFF_OWNER | EFF_GROUP,
     0, sizeof(struct lndstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"nuke", "nuke", EFF_XY | EFF_OWNER,
     0, sizeof(struct nukstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"news", "news", 0,
     0, sizeof(struct nwsstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"treaty", "treaty", 0,
     0, sizeof(struct trtstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"trade", "trade", 0,
     0, sizeof(struct trdstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"pow", "power", 0,
     0, sizeof(struct powstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"nat", "nation", EFF_OWNER,
     0, sizeof(struct natstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"loan", "loan", 0,
     0, sizeof(struct lonstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"map", "map", 0,
     0, DEF_WORLD_X * DEF_WORLD_Y / 2, NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"bmap", "bmap", 0,
     0, DEF_WORLD_X * DEF_WORLD_Y / 2, NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"commodity", "commodity", 0,
     0, sizeof(struct comstr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL},
    {"lost", "lostitems", EFF_OWNER,
     0, sizeof(struct loststr), NULL, NULL, NULL,
     -1, -1, 0, 0, NULL, 0, NULL}
};
