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
 *  bp.c: Functions for build pointer (bp) handling
 * 
 *  Known contributors to this file:
 *     Ville Virrankoski, 1996
 */

#include "misc.h"
#include "nat.h"
#include "file.h"
#include "sect.h"
#include "var.h"
#include "budg.h"
#include "update.h"
#include "common.h"
#include "optlist.h"

static  int bud_key[I_MAX+2] = {0,1,2,3,4,0,0,0,0,0,0,5,6,0,0,7};

int
*get_wp(int *bp, struct sctstr *sp, int cm)
{
        return (bp + (sp->sct_x + (sp->sct_y * WORLD_X)) + WORLD_X * WORLD_Y * (cm - 1));
}

int
gt_bg_nmbr(int *bp, struct sctstr *sp, int comm)
{
        int   *wp;
	int   cm;
	int   svec[I_MAX+1];

	if ((cm = bud_key[comm]) == 0)  {
	  getvec(VT_ITEM, svec, (s_char *)sp, EF_SECTOR);
	  return svec[comm];
	} else {
	  wp = get_wp(bp, sp, cm);
	  return *wp;
	}
}

void pt_bg_nmbr(int *bp, struct sctstr *sp, int comm, int amount)
{
        int   *wp;
	int   cm;

	if ((cm = bud_key[comm]) != 0)  {
	  wp = get_wp(bp, sp, cm);
	  *wp = amount;
	} 
}

void
fill_update_array(int *bp, struct sctstr *sp)
{
        int   vec[I_MAX+1];
	int   i, k;
	int   *wp;

        if (getvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR) <= 0)
	  return;
	for (i=1;i<=I_MAX;i++) 
	  if ((k = bud_key[i]) != 0)  {
	    wp = get_wp(bp, sp, k); 
	    *wp = vec[i];
	  }
	wp = get_wp(bp, sp, bud_key[I_MAX+1]);
	*wp = sp->sct_avail;
}


