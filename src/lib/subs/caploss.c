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
 *  caploss.c: Lose your capital (kiss of death)
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *    
 */

#include "misc.h"
#include "player.h"
#include "nat.h"
#include "var.h"
#include "sect.h"
#include "file.h"
#include "xy.h"
#include "news.h"
#include "path.h"
#include "loan.h"
#include "commodity.h"
#include "prototypes.h"
#include "optlist.h"

void
caploss(struct sctstr *sp, natid coun, s_char *msg)
{
	struct	natstr *natp;
	struct  lonstr loan;
	struct  comstr comm;
	long	lose;
	long	gain;
	struct	sctstr sect;
	int	n;
	int loan_num = 0;
	int comm_num = 0;

	if (coun == 0) return;
	natp = getnatp(coun);
	if ((xrel(natp,natp->nat_xcap) != xrel(natp,sp->sct_x)) ||
	      (yrel(natp,natp->nat_ycap) != yrel(natp,sp->sct_y)))
	     return;
	if (coun == player->cnum) {
		player->nstat &= ~CAP;
		return;
	}
	/* Ok, has the country owner reset their capital yet after it was last sacked? */
	if (natp->nat_flags & NF_SACKED)
	    return; /* No, so not really the capital yet, so return */
	pr(msg, natp->nat_cnam);
	gain = lose = natp->nat_money / 2;
	if (lose < 3000)
	    lose = 3000;
	n = roll(6);
	getsect(sp->sct_x + diroff[n][0],
		sp->sct_y + diroff[n][1],
		&sect);
	natp->nat_xcap = sect.sct_x;
	natp->nat_ycap = sect.sct_y;
	natp->nat_money -= lose;
	/* Your capital has now been sacked, no more sacking until you reset it */
	natp->nat_flags |= NF_SACKED;
	putnat(natp);
	wu(0, coun,
	   "* %s just sacked your capital! *\n",
	   cname(player->cnum));
	
	if (gain >= 0)  {
	    gain = (0.2 + 0.8 * (sp->sct_effic/100.0)) * gain;
	    player->dolcost -= gain;
	} else
	    gain = 0;
	wu(0, coun,
	   "You lost $%d and they gained $%d\n",
	   lose, gain);
	wu(0, coun,
	   "Your capital has been moved to %s. You must use 'capital' to reset it.\n",
	   xyas(natp->nat_xcap, natp->nat_ycap, coun));
	wu(0, 0, "%s just took %s's capital and gained $%d\n",
	   cname(player->cnum), cname(coun),
	   -(int)(player->dolcost));
	if (opt_LOANS) {
	    for (loan_num = 0; getloan(loan_num, &loan); loan_num++) {
		if (loan.l_ldur != 0 && loan.l_loner == coun) {
		    loan.l_loner = player->cnum;
		    putloan(loan_num, &loan);
		    pr("Loan %d has been transfered over to you\n",
		       loan_num);
		}
	    }
	}
	if (opt_MARKET) {
	    for (comm_num = 0; getcomm(comm_num, &comm); comm_num++) {
		if (comm.com_owner == coun) {
		    if (comm.com_owner == comm.com_maxbidder)
			comm.com_maxbidder = player->cnum;
		    comm.com_owner = player->cnum;
		    putcomm(comm_num, &comm);
		    pr("You now own commodity #%d.\n",
		       comm_num);
		}
	    }
	}
	nreport(player->cnum, N_SACK_CAP, coun, 1);
}
