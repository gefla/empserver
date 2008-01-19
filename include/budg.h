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
 *  budg.h: Budget related definitions
 * 
 *  Known contributors to this file:
 *     Ville Virrankoski, 1995
 */

#ifndef BUDG_H
#define BUDG_H

#include "types.h"
#include "item.h"

#define SCT_EFFIC (SCT_TYPE_MAX + 1)

struct bp *bp_alloc(void);
void bp_set_from_sect(struct bp *, struct sctstr *);
int bp_get_item(struct bp *, struct sctstr *, i_type);
void bp_put_item(struct bp *, struct sctstr *, i_type, int);
void bp_put_items(struct bp *, struct sctstr *, short *);
int bp_get_avail(struct bp *, struct sctstr *);
void bp_put_avail(struct bp *, struct sctstr *, int);

int get_materials(struct sctstr *, struct bp *, int *, int);

extern long money[MAXNOC];
extern long pops[MAXNOC];
extern long sea_money[MAXNOC];
extern long lnd_money[MAXNOC];
extern long air_money[MAXNOC];
extern long tpops[MAXNOC];
extern float levels[MAXNOC][4];

#endif
