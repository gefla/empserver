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
 *  main.c: World update main function
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1996
 *     Doug Hay, 1998
 */

#include "misc.h"
#include "nat.h"
#include "file.h"
#include "sect.h"
#include "player.h"
#include "empthread.h"
#include "var.h"
#include "budg.h"
#include "product.h"
#include "player.h"
#include "update.h"
#include "common.h"
#include "optlist.h"
#include <stdlib.h>
#if !defined(_WIN32)
#include <sys/time.h>
#endif

long	money[MAXNOC];
long	pops[MAXNOC];
long	sea_money[MAXNOC];
long	lnd_money[MAXNOC];
long	air_money[MAXNOC];
long    tpops[MAXNOC];

extern  int mil_dbl_pay;
int	update_pending = 0;

/*ARGSUSED*/
void
update_main(void *argv)
{
	extern	int etu_per_update;
	extern	double money_res;
	int	etu = etu_per_update;
	int	n;
	int	x;
	int     *bp;
	int	cn,cn2,rel;
	struct	natstr *cnp;
	struct	natstr *np;

	/* First, make sure all mobility is updated correctly. */
	if (opt_MOB_ACCESS) {
		mob_ship(etu);
		mob_sect(etu);
		mob_plane(etu);
		mob_land(etu);
	}
	update_pending = 1;
	player->proc = empth_self();
	player->cnum = 0;
	player->god = 1;
	/*
	 * set up all the variables which get used in the
	 * sector production routine (for producing education,
	 * happiness, and printing out the state of the nation)
	 */
	logerror("production update (%d etus)", etu);
	bzero((s_char *)pops, sizeof(pops));
	bzero((s_char *)air_money, sizeof(air_money));
	bzero((s_char *)sea_money, sizeof(sea_money));
	bzero((s_char *)lnd_money, sizeof(lnd_money));
	bp = (int *) calloc(WORLD_X * WORLD_Y * 7, sizeof(int));
	for (n=0; n<MAXNOC; n++) {
		money[n] = 0;
		if ((np = getnatp(n)) == (struct natstr *)0)
			continue;
		money[n] = np->nat_money;
		tpops[n] = count_pop(n);
	}

	logerror("preparing sectors...");
	prepare_sects(etu, bp);
	logerror("done preparing sectors.");
	logerror("producing for countries...");
	for (x=0; x<MAXNOC; x++){
		int	y,z,sb=0,sm=0,pb=0,pm=0,lm=0,lb=0;
		long    p_sect[SCT_MAXDEF+1][2];

		bzero((s_char *)p_sect, sizeof(p_sect));
		mil_dbl_pay = 0;
		if ((np = getnatp(x)) == (struct natstr *)0)
			continue;
                if (np->nat_stat & STAT_SANCT ) {
#ifdef DEBUG
		   logerror("Country %i is in sanctuary and did not update",x);
#endif
		   continue;
		} 
		np->nat_money += (int) (np->nat_reserve * money_res * etu);

		for(y=1; y<SCT_MAXDEF+8; y++) {
			for(z=0; z<SCT_MAXDEF+8; z++) {
			    if (np->nat_priorities[z] == y) {
				do_prod(z, etu, x, bp, p_sect,
					&sb, &sm, &pb, &pm, &lb, &lm);
			    }
			}
		}
		/* 0 is maintain, 1 is build */
		if (!sm) prod_ship(etu, x, bp, 0);
		if (!sb) prod_ship(etu, x, bp, 1);
		if (!pm) prod_plane(etu, x, bp, 0);
		if (!pb) prod_plane(etu, x, bp, 1);
		if (!lm) prod_land(etu, x, bp, 0);
		if (!lb) prod_land(etu, x, bp, 1);

		/* produce all sects that haven't produced yet */
		produce_sect(x, etu, bp, p_sect, -1);
		np->nat_money -= p_sect[SCT_CAPIT][1];
	}
	logerror("done producing for countries.");

	finish_sects(etu);
	prod_nat(etu);
	age_levels(etu);
	free(bp);
	/*flushwu();*/
	if (opt_SLOW_WAR) {
	  /* Update war declarations */
	  /* MOBILIZATION->SITZKRIEG->AT_WAR */
	  for (cn=1; cn < MAXNOC; cn++) {
		if ((cnp = getnatp(cn)) == 0)
			break;
		for (cn2=1; cn2 < MAXNOC; cn2++) {
			if (cn2 == cn)
				continue;
			rel=getrel(cnp,cn2);
			if (rel == MOBILIZATION){
			    rel = SITZKRIEG;
			    setrel(cn,cn2,rel);
			}
			else if (rel == SITZKRIEG){
			    rel = AT_WAR;
			    setrel(cn,cn2,rel);
			}
		}
	  }
	}
        /* Age contact */
        if (opt_LOSE_CONTACT) {
               for (cn=1; cn<MAXNOC; cn++) {
                        if ((cnp = getnatp(cn)) != NULL) agecontact(cnp);
                }
        }
	/* Only update mobility for non-MOB_ACCESS here, since it doesn't
	   get done for MOB_ACCESS anyway during the update */
	if (!opt_MOB_ACCESS) {
		mob_ship(etu);
		mob_sect(etu);
		mob_plane(etu);
		mob_land(etu);
	}
	if (opt_DEMANDUPDATE)
	    update_removewants();
	/* flush all mem file objects to disk */
	ef_flush(EF_NATION);
	ef_flush(EF_SECTOR);
	ef_flush(EF_SHIP);
	ef_flush(EF_PLANE);
	ef_flush(EF_LAND);
	delete_old_announcements();
	/* Clear all the telegram flags */
	for (cn = 0; cn < MAXNOC; cn++)
		clear_telegram_is_new(cn);
	update_pending = 0;
	logerror("End update");
	player_delete(player);
	empth_exit();
	/*NOTREACHED*/
}

void
do_prod(int sector_type, int etu, int n, int *bp, long int (*p_sect)[2], int *ship_build, int *ship_maint, int *plane_build, int *plane_maint, int *land_build, int *land_maint)
{
        extern double money_mil;
	struct natstr *np;

	np = getnatp(n);

	if (sector_type == PRI_SMAINT){
		prod_ship(etu,n,bp,0);
		*ship_maint=1;
	}
	else
	if (sector_type == PRI_SBUILD){
		prod_ship(etu,n,bp,1);
		*ship_build=1;
	}
	else
	if (sector_type == PRI_PMAINT){
		prod_plane(etu,n,bp,0);
		*plane_maint=1;
	}
	else
	if (sector_type == PRI_PBUILD){
		prod_plane(etu,n,bp,1);
		*plane_build=1;
	}
	else
	if (sector_type == PRI_LMAINT){
	        if (*land_build)
		  np->nat_money -= (int) (money_mil * etu * mil_dbl_pay);
		prod_land(etu,n,bp,0);
		*land_maint=1;
	}
	else
	if (sector_type == PRI_LBUILD){
		prod_land(etu,n,bp,1);
		*land_build=1;
	}
	else {
	    produce_sect(n,etu,bp,p_sect,sector_type);
	}
}

