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
 *  sect.c: Do production for sectors
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 */

#include <math.h>
#include "misc.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "item.h"
#include "news.h"
#include "file.h"
#include "xy.h"
#include "path.h"
#include "product.h"
#include "distribute.h"
#include "optlist.h"
#include "budg.h"
#include "player.h"
#include "land.h"
#include "ship.h"
#include "update.h"
#include "subs.h"
#include "common.h"
#include "lost.h"
#include "gen.h"

extern	float levels[MAXNOC][4];

int
dodeliver(struct sctstr *sp, int *vec)
{
	register int i;
	int	del[I_MAX+1];
	int	thresh;
	int	dir;
	int	plague;
	int	n;
	int	changed;

	if (sp->sct_mobil <= 0)
		return 0;
	if (getvec(VT_DEL, del, (s_char *)sp, EF_SECTOR) <= 0)
		return 0;
	changed = 0;
	plague = getvar(V_PSTAGE, (s_char *)sp, EF_SECTOR);
	for (i=1; i<=I_MAX; i++) {
		if (del[i] == 0)
			continue;
		thresh = del[i] & ~0x7;
		dir = del[i] & 0x7;
		n = deliver(sp, &ichr[i], dir, thresh, vec[i], plague);
		if (n > 0)  {
			vec[i] -= n;
			changed++;
			if (sp->sct_mobil <= 0)
				break;
		}
	}
	return changed;
}

/*
 * Increase sector efficiency if old type == new type.
 * decrease sector efficiency if old type != new type.
 * Return amount of work used.
 */
int
upd_buildeff(struct natstr *np, register struct sctstr *sp, int *workp, int *vec, int etu, int *desig, int sctwork, int *cost)
{
	register int work_cost = 0;
	int	buildeff_work = (int)(*workp / 2);
	int	n, hcms, lcms, neweff;
	u_char  old_type = *desig;

	*cost = 0;
	neweff = sp->sct_effic;

	if (*desig != sp->sct_newtype) {
		/*
		 * Tear down existing sector.
		 * Easier to destroy than to build.
		 */
		work_cost = (sp->sct_effic + 3) / 4;
		if (work_cost > buildeff_work)
			work_cost = buildeff_work;
		buildeff_work -= work_cost;
		n = sp->sct_effic - work_cost * 4;
		if (n <= 0) {
			n = 0;
			*desig = sp->sct_newtype;
		}
		neweff = n;
		*cost += work_cost;
		if (opt_BIG_CITY) {
		    if (!n && dchr[old_type].d_pkg == UPKG &&
			dchr[*desig].d_pkg != UPKG) {
			int maxpop = max_pop(np->nat_level[NAT_RLEV], sp);
			if (vec[I_CIVIL] > maxpop)
			    vec[I_CIVIL] = maxpop;
			if (vec[I_UW] > maxpop)
			    vec[I_UW] = maxpop;
			*workp = (vec[I_CIVIL] * sctwork) / 100.0
			    +(vec[I_MILIT] * 2 / 5.0) + vec[I_UW];
			*workp = roundavg((etu * (*workp)) / 100.0);
			
			buildeff_work = min((int)(*workp / 2), buildeff_work);
		    }
		}
	}
	if (np->nat_priorities[*desig]) {
		if (*desig == sp->sct_newtype) {
			work_cost = 100 - neweff;
			if (work_cost > buildeff_work)
				work_cost = buildeff_work;
			
			if (dchr[*desig].d_lcms>0){
				lcms = vec[I_LCM];
				lcms /= dchr[*desig].d_lcms;
				if (work_cost > lcms)
					work_cost = lcms;
			}
			if (dchr[*desig].d_hcms>0){
				hcms = vec[I_HCM];
				hcms /= dchr[*desig].d_hcms;
				if (work_cost > hcms)
					work_cost = hcms;
			}

			neweff += work_cost;
			*cost += work_cost*dchr[*desig].d_build;
			buildeff_work -= work_cost;
			
			if ((dchr[*desig].d_lcms>0) || 
			    (dchr[*desig].d_hcms>0)){
				vec[I_LCM] -= work_cost *
					      dchr[*desig].d_lcms;
				vec[I_HCM] -= work_cost *
					      dchr[*desig].d_hcms;
			}
		}
	}
	*workp = *workp/2 + buildeff_work;

	return neweff;
}

/*
 * enlistment sectors are special; they require military
 * to convert civ into mil in large numbers.
 * Conversion will happen much more slowly without
 * some mil initially.
 */
int
enlist(register int *vec, int etu, int *cost)
{
	int	maxmil;
	int	enlisted;

	/* Need to check treaties here */
	enlisted = 0;
	maxmil = (vec[I_CIVIL] / 2) - vec[I_MILIT];
	if (maxmil > 0) {
		enlisted = (etu * (10 + vec[I_MILIT]) * 0.05);
		if (enlisted > maxmil)
			enlisted = maxmil;
		vec[I_CIVIL] -= enlisted;
		vec[I_MILIT] += enlisted;
	}
	*cost = enlisted * 3;
	return enlisted;
}

/* Fallout is calculated here. */

extern int melt_item_denom[];

void
meltitems(int etus, int fallout, int own, int *vec, int type, int x, int y, int uid)
{
  int n;
  int melt;

  for (n = 1; n <= I_MAX; n++) {
    melt = roundavg(vec[n] * etus * (long)fallout /
		    (1000.0 * melt_item_denom[n]));
    if (melt > 5 && own) {
      if (type == EF_SECTOR)
	wu(0, own, "Lost %d %s to radiation in %s.\n",
	   (melt < vec[n] ? melt : vec[n]), ichr[n].i_name,
	   xyas(x, y, own));
      else if (type == EF_LAND)
	wu(0, own, "Unit #%d lost %d %s to radiation in %s.\n",
	   uid, (melt < vec[n] ? melt : vec[n]), ichr[n].i_name,
	   xyas(x, y, own));
      else if (type == EF_SHIP)
	wu(0, own, "Ship #%d lost %d %s to radiation in %s.\n",
	   uid, (melt < vec[n] ? melt : vec[n]), ichr[n].i_name,
	   xyas(x, y, own));
    }
    if (melt < vec[n])
      vec[n] -= melt;
    else
      vec[n] = 0;
  }
}

/*
 * do_fallout - calculate fallout for sectors.
 *
 * This is etu based.  But, do limit HUGE kill offs in large ETU
 * games, the melting etus rate is limited to 24 etus.
 */

void
do_fallout(register struct sctstr *sp, register int etus)
{
  int	vec[I_MAX+1];
  int	cvec[I_MAX+1];
  int   tvec[I_MAX+1];
  struct shpstr *spp;
  struct lndstr *lp;
  int   i;

  getvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR);
  getvec(VT_COND, cvec, (s_char *)sp, EF_SECTOR);
/* This check shouldn't be needed, but just in case. :) */
  if (!cvec[C_FALLOUT] || !sp->sct_updated)
    return;
  if (etus > 24)
    etus = 24;
#if 0
  wu(0,0,"Running fallout in %d,%d\n", sp->sct_x, sp->sct_y);
#endif
  meltitems(etus, cvec[C_FALLOUT], sp->sct_own, vec, EF_SECTOR,
	    sp->sct_x, sp->sct_y, 0);
  putvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR);
  for (i = 0; NULL != (lp = getlandp(i)); i++) {
    if (!lp->lnd_own)
      continue;
    if (lp->lnd_x != sp->sct_x || lp->lnd_y != sp->sct_y)
      continue;
    getvec(VT_ITEM, tvec, (s_char *)lp, EF_LAND);
    meltitems(etus, cvec[C_FALLOUT], lp->lnd_own, tvec, EF_LAND,
	      lp->lnd_x, lp->lnd_y, lp->lnd_uid);
    putvec(VT_ITEM, tvec, (s_char *)lp, EF_LAND);
  }
  for (i = 0; NULL != (spp = getshipp(i)); i++) {
    if (!spp->shp_own)
      continue;
    if (spp->shp_x != sp->sct_x || spp->shp_y != sp->sct_y)
      continue;
    if (mchr[(int)spp->shp_type].m_flags & M_SUB)
      continue;
    getvec(VT_ITEM, tvec, (s_char *)spp, EF_SHIP);
    meltitems(etus, cvec[C_FALLOUT], spp->shp_own, tvec, EF_SHIP,
	      spp->shp_x, spp->shp_y, spp->shp_uid);
    putvec(VT_ITEM, tvec, (s_char *)spp, EF_SHIP);
  }
#ifdef	GODZILLA
  if ((cvec[C_FALLOUT] > 20) && chance(100))
	do_godzilla(sp);
#endif	/* GODZILLA */
}

void
spread_fallout(struct sctstr *sp, int etus)
{
  extern double fallout_spread;
  struct sctstr *ap;
  int tvec[I_MAX+1];
  int cvec[I_MAX+1];
  int n;
  register int	inc;

  if (etus > 24)
    etus = 24;
  getvec(VT_COND, cvec, (s_char *)sp, EF_SECTOR);
  for (n = DIR_FIRST; n <= DIR_LAST; n++) {
    ap = getsectp(sp->sct_x+diroff[n][0], sp->sct_y+diroff[n][1]);
    getvec(VT_COND, tvec, (char *)ap, EF_SECTOR);
    if (ap->sct_type == SCT_SANCT)
        continue;
    inc = roundavg(etus * fallout_spread * (cvec[C_FALLOUT])) - 1;
#if 0
    if (cvec[C_FALLOUT]) {
      wu(0,0,"Fallout from sector %d,%d to %d,%d is %d=%d*%e*%d\n",
	 sp->sct_x,sp->sct_y,sp->sct_x+diroff[n][0],
	 sp->sct_y+diroff[n][1], inc, etus,
	 fallout_spread, cvec[C_FALLOUT]);
    }
#endif
    if (inc < 0)
      inc = 0;
    tvec[C_FALLOUT] += inc;
    putvec(VT_COND, tvec, (char *)ap, EF_SECTOR);
  }
}

void
decay_fallout(struct sctstr *sp, int etus)
{
  extern double decay_per_etu;
  extern double fallout_spread;
  int cvec[I_MAX+1];
  int decay;

  if (etus > 24)
    etus = 24;
  getvec(VT_COND, cvec, (char *)sp, EF_SECTOR);
  decay = roundavg(((decay_per_etu + 6.0) * fallout_spread) *
		   (double)etus * (double)cvec[C_FALLOUT]);

#if 0
  if (decay || cvec[C_FALLOUT])
    wu(0,0,"Fallout decay in %d,%d is %d from %d\n", sp->sct_x, sp->sct_y, decay, cvec[C_FALLOUT]);
#endif

  cvec[C_FALLOUT] = (decay < cvec[C_FALLOUT]) ? (cvec[C_FALLOUT] - decay) : 0;
  if (cvec[C_FALLOUT] < 0)
    cvec[C_FALLOUT] = 0;
  putvec(VT_COND, cvec, (s_char *)sp, EF_SECTOR);
}

#define SHOULD_PRODUCE(sp,t)	(((sp->sct_type == t) || (t == -1)) ? 1 : 0)

/*
 * Produce only a set sector type for a specific nation
 * (or all, if sector_type == -1)
 *
 */
void
produce_sect(int natnum, int etu, int *bp, long int (*p_sect)[2], int sector_type)
{
	register struct sctstr *sp;
	register struct natstr *np;
	int	vec[I_MAX+1];
	int	work, cost, ecost, pcost, sctwork;
	int	n, desig, maxpop, neweff, amount;

	for (n=0; NULL != (sp = getsectid(n)); n++) {
		if (sp->sct_type == SCT_WATER)
			continue;
		if (sp->sct_own != natnum)
			continue;
		if (sp->sct_updated != 0)
			continue;
		if (!SHOULD_PRODUCE(sp,sector_type))
			continue;

		if ((sp->sct_type == SCT_CAPIT) && (sp->sct_effic > 60)) {
		        p_sect[SCT_CAPIT][0]++;
			p_sect[SCT_CAPIT][1] += etu;
		}

       		if (getvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR) <= 0)
			continue;
		/* If everybody is dead, the sector reverts to unowned. 
	 	* This is also checked at the end of the production in
	 	* they all starved or were plagued off.
	 	*/
		if (vec[I_CIVIL] == 0 && vec[I_MILIT] == 0 &&
			!has_units(sp->sct_x,sp->sct_y,sp->sct_own,0)) {
		        makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
			sp->sct_own = 0;
			sp->sct_oldown = 0;
			continue;
		}

		sp->sct_updated = 1;
		work = 0;

		np = getnatp(natnum);

		/* do_feed trys to supply.  So, we need to enable cacheing
		   here */
		bp_enable_cachepath();

		sctwork = do_feed(sp, np, vec, &work, bp, etu);

		bp_disable_cachepath();
		bp_clear_cachepath();

		if (sp->sct_off || np->nat_money < 0) {
		  if (!player->simulation) {
		    putvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR);
		    sp->sct_off = 0;
		  }
		  continue;
		}
		if ((np->nat_priorities[sp->sct_type] == 0) &&
		    (sp->sct_type == sp->sct_newtype) &&
		    ((pchr[dchr[sp->sct_type].d_prd].p_cost != 0) ||
		     (sp->sct_type == SCT_ENLIST))){
		    if (!player->simulation) {
			putvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR);
			logerror("Skipping %s production for country %s\n",
				 dchr[sp->sct_type].d_name,np->nat_cnam);
		    }
		    continue;
		}

		neweff = sp->sct_effic;
		amount = 0;
		pcost = cost = ecost = 0;

		desig = sp->sct_type;

		if ((sp->sct_effic < 100 || sp->sct_type != sp->sct_newtype) &&
			np->nat_money > 0) {
       			neweff = upd_buildeff(np, sp, &work, vec, etu, &desig, sctwork, &cost);
			pt_bg_nmbr(bp, sp, I_LCM, vec[I_LCM]);
			pt_bg_nmbr(bp, sp, I_HCM, vec[I_HCM]);
			p_sect[SCT_EFFIC][0]++;
			p_sect[SCT_EFFIC][1] += cost;
			if (!player->simulation) {
			  np->nat_money -= cost;
			  /* No longer tear down infrastructure
			  if (sp->sct_type != desig) {
			    sp->sct_road = 0;
			    sp->sct_defense = 0;
			  } else if (neweff < sp->sct_effic) {
			    sp->sct_road -= (sp->sct_road * (sp->sct_effic - neweff) / 100.0);
			    sp->sct_defense -= (sp->sct_defense * (sp->sct_effic - neweff) / 100.0);
			    if (sp->sct_road < 0)
			      sp->sct_road = 0;
			    if (sp->sct_defense < 0)
			      sp->sct_defense = 0;
			  }
			  */
			  sp->sct_type = desig;
			  sp->sct_effic = neweff;
			  if (!opt_DEFENSE_INFRA)
			    sp->sct_defense = sp->sct_effic;
			}
		}

		if ((np->nat_priorities[desig] == 0) &&
		    ((pchr[dchr[desig].d_prd].p_cost != 0) ||
		     (desig == SCT_ENLIST))) {
		    if (!player->simulation) {
			putvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR);
			logerror("Skipping %s production for country %s\n",
				 dchr[sp->sct_type].d_name,np->nat_cnam);
		    }
		    continue;
		}

		if (desig == SCT_ENLIST && neweff >= 60 &&
			sp->sct_own == sp->sct_oldown) {
			p_sect[desig][0] += enlist(vec, etu, &ecost);
			p_sect[desig][1] += ecost;
			if (!player->simulation)
			  np->nat_money -= ecost;
		      }

		/*
	 	* now do the production (if sector effic >= 60%)
	 	*/

		if (neweff >= 60) {
		    if (np->nat_money > 0 && dchr[desig].d_prd)
		        work -= produce(np, sp, vec, work, sctwork, desig, neweff, &pcost, &amount);
		}

		pt_bg_nmbr(bp, sp, I_MAX+1, work);
		p_sect[desig][0] += amount;
		p_sect[desig][1] += pcost;
		if (!player->simulation) {
		  maxpop = max_pop(np->nat_level[NAT_RLEV], sp);
		  if (vec[I_CIVIL] > maxpop)
		    vec[I_CIVIL] = maxpop;
		  if (vec[I_UW] > maxpop)
		    vec[I_UW] = maxpop;
		  putvec(VT_ITEM, vec, (s_char *)sp, EF_SECTOR);
		  sp->sct_avail = work;
		  np->nat_money -= pcost;
		}
	}
}
