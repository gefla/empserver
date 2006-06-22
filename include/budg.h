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

#define SCT_EFFIC (SCT_TYPE_MAX + 1)

void fill_update_array(int *bp, struct sctstr *sp);
int gt_bg_nmbr(int *bp, struct sctstr *sp, i_type comm);
void pt_bg_nmbr(int *bp, struct sctstr *sp, i_type comm, int amount);
void get_materials(struct sctstr *sp, int *bp, int *mvec, int check);

extern long money[MAXNOC];
extern long pops[MAXNOC];
extern long sea_money[MAXNOC];
extern long lnd_money[MAXNOC];
extern long air_money[MAXNOC];
extern long tpops[MAXNOC];
extern float levels[MAXNOC][4];

extern int mil_dbl_pay;

#endif
