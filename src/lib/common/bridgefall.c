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
 *  bridgefall.c: Knock a bridge down
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "var.h"
#include "sect.h"
#include "path.h"
#include "file.h"
#include "xy.h"
#include "plane.h"
#include "land.h"
#include "nsc.h"
#include "common.h"
#include "subs.h"
#include "lost.h"
#include "optlist.h"

void
bridgefall(struct sctstr *sp, struct emp_qelem *list)
{
	register int i;
	register int j;
	struct	sctstr sect;
	struct	sctstr bh_sect;
	int	nx;
	int	ny;
	int	nnx;
	int	nny;

	for (i = 1; i <= 6; i++) {
		nx = sp->sct_x + diroff[i][0];
		ny = sp->sct_y + diroff[i][1];
		getsect(nx, ny, &sect);
		if (sect.sct_type != SCT_BSPAN)
			continue;
		for (j = 1; j <= 6; j++) {
			nnx = nx + diroff[j][0];
			nny = ny + diroff[j][1];
			if (nnx == sp->sct_x && nny == sp->sct_y)
				continue;
			getsect(nnx, nny, &bh_sect);
 			if (bh_sect.sct_type == SCT_BHEAD &&
			    bh_sect.sct_newtype == SCT_BHEAD)
				break;
			if (bh_sect.sct_type == SCT_BTOWER)
			    break;
			/* With EASY_BRIDGES, it just has to be next to any
			   land */
			if (opt_EASY_BRIDGES) {
			    if (bh_sect.sct_type != SCT_WATER &&
				bh_sect.sct_type != SCT_BSPAN)
				break;
			}
		}
		if (j > 6) {
		  knockdown(&sect, list);
		  putsect(&sect);
		}
	}
}

/* Knock down a bridge span.  Note that this does NOT write the
 * sector out to the database, it's up to the caller to do that. */
void
knockdown(struct sctstr *sp, struct emp_qelem *list)
{
  struct lndstr land;
  struct plnstr plane;
  struct nstr_item ni;
  int mines;
  struct natstr *np;

  if (sp->sct_type == SCT_BTOWER)
      mpr(sp->sct_own, "Crumble... SCREEEECH!  Splash! Bridge tower falls at %s!\n",
	  xyas(sp->sct_x, sp->sct_y, sp->sct_own));
  else
      mpr(sp->sct_own, "Crumble... SCREEEECH!  Splash! Bridge falls at %s!\n",
	  xyas(sp->sct_x, sp->sct_y, sp->sct_own));
  sp->sct_type = SCT_WATER;
  sp->sct_newtype = SCT_WATER;
  makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
  sp->sct_own = 0;
  sp->sct_oldown = 0;
  sp->sct_mobil = 0;
  sp->sct_effic = 0;

  /* Sink all the units */
  snxtitem_xy(&ni,EF_LAND,sp->sct_x,sp->sct_y);
  while (nxtitem(&ni, (s_char *)&land)){
    if (land.lnd_own == 0)
      continue;
    if(land.lnd_x != sp->sct_x || land.lnd_y != sp->sct_y)
      continue;
    if (land.lnd_ship >= 0)
      continue;
    np = getnatp(land.lnd_own);
    if (np->nat_flags & NF_BEEP)
      mpr(land.lnd_own, "\07");
    mpr(land.lnd_own, "     AARGH! %s tumbles to its doom!\n", prland(&land));
    makelost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x, land.lnd_y);
    land.lnd_own = 0;
    land.lnd_effic = 0;
    putland(land.lnd_uid, &land);
  }
  /* Sink all the planes */
  snxtitem_xy(&ni,EF_PLANE,sp->sct_x,sp->sct_y);
  while (nxtitem(&ni, (s_char *)&plane)){
    if (plane.pln_own == 0)
      continue;
    if(plane.pln_x != sp->sct_x || plane.pln_y != sp->sct_y)
      continue;
    if (plane.pln_flags & PLN_LAUNCHED)
      continue;
    if (plane.pln_ship >= 0)
      continue;
    /* Is this plane flying in this list? */
    if (ac_isflying(&plane, list))
      continue;
    np = getnatp(plane.pln_own);
    if (np->nat_flags & NF_BEEP)
      mpr(plane.pln_own, "\07");
    mpr(plane.pln_own, "     AARGH! %s tumbles to its doom!\n", prplane(&plane));
    makelost(EF_PLANE, plane.pln_own, plane.pln_uid, plane.pln_x, plane.pln_y);
    plane.pln_own = 0;
    plane.pln_effic = 0;
    putplane(plane.pln_uid, &plane);
  }
  /*
   * save only the mines; zero the rest of the
   * commodities.
   */
  mines = getvar(V_MINE, (caddr_t)sp, EF_SECTOR);
  sp->sct_nv = 0;
  if (mines > 0)
    (void) putvar(V_MINE, mines, (caddr_t)sp, EF_SECTOR);
}

