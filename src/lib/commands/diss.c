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
 *  diss.c: Dissolve
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "ship.h"
#include "plane.h"
#include "land.h"
#include "nat.h"
#include "loan.h"
#include "treaty.h"
#include "news.h"
#include "file.h"
#include "xy.h"
#include "nsc.h"
#include "tel.h"
#include "nuke.h"
#include "trade.h"
#include "commands.h"

int
diss(void)
{
	int	quit(void);
	struct	sctstr sect;
	struct	lndstr land;
	struct	shpstr ship;
	struct	plnstr plane;
	struct	trtstr treaty;
	struct	trdstr trade;
	struct	lonstr loan;
	struct	nstr_item ni;
	struct	nstr_sect ns;
	int	sctnum;
	int	damage;
	int	annoyed;
	int	n;
	s_char	*p;
	natid	cn;
	float	dummy;
	int	val;
	s_char	buf[1024];
	struct natstr *np;

	if (player->cnum == 0) {
		/* Dissolve the Deity??? */
		/* Probably someone broke in. */
		/* Send a tele to deity, and abort the session. */
		wu(0, 0, "Diety dissolve attempted!  Session was aborted.\n");
		quit();
	}
	if (player->argp[1])
		*player->argp[1] = 0;
	p = getstarg(player->argp[1], "Are you SURE you want to do this? (n) ", buf);
	if (p == 0 || *p != 'y')
		return RET_SYN;
	pr("\nIf you want some problem fixed and hope to force the issue\n");
	pr("by dissolving, DON'T DO IT.  Even if an earlier version of\n");
	pr("this game is brought up, YOU WILL BE GONE.\n");
	pr("Forcing the issue makes it less likely that it will be\n");
	pr("resolved the way you wish it.\n\n");
	pr("Be part of the solution, not part of the problem.\n\n");
	pr("If this is a test game, dissolving will not help the test.\n\n");
	pr("If you are down to 0 sectors, and a 25% fishing vessel with\n");
	pr("1 mil on it, well, I can see your point.  Go ahead.\n\n");
	pr("But at least pay back your loans, if you can.\n\n");

	if (player->argp[1])
		*player->argp[1] = 0;
	p = getstarg(player->argp[1], "Once again, are you SURE you want to do this? (n) ", buf);
	if (p == 0 || *p != 'y')
		return RET_SYN;

	np = getnatp(player->cnum);
	np->nat_stat = VIS;
	snxtsct_all(&ns);
	sctnum = 0;
	while (nxtsct(&ns, &sect)) {
		if (!player->owner)
			continue;
		annoyed = 75 - (random() % (sect.sct_loyal+40));
		if (annoyed <= 0 || sect.sct_oldown != sect.sct_own) {
			pr("Populace celebrates in %s\n",
				xyas(sect.sct_x, sect.sct_y, player->cnum));
			if (sect.sct_oldown != sect.sct_own) {
			        makelost(EF_SECTOR, sect.sct_own, 0, sect.sct_x, sect.sct_y);
			        makenotlost(EF_SECTOR, sect.sct_oldown, 0, sect.sct_x, sect.sct_y);
				sect.sct_own = sect.sct_oldown;
				if(sect.sct_own != 0)
				wu(0, sect.sct_own,
					"%s reverts back to you!\n",
					xyas(sect.sct_x, sect.sct_y,
					sect.sct_own));
			}
		} else if (annoyed < 25) {
			pr("Populace unmoved in %s\n",
				xyas(sect.sct_x, sect.sct_y, player->cnum));
			makelost(EF_SECTOR, sect.sct_own, 0, sect.sct_x, sect.sct_y);
			sect.sct_own = 0;
			sect.sct_oldown = 0;
		} else {
			pr("Rioting breaks out in %s\n",
				xyas(sect.sct_x, sect.sct_y, player->cnum));
			sectdamage(&sect, annoyed, 0);
			sctnum++;
			makelost(EF_SECTOR, sect.sct_own, 0, sect.sct_x, sect.sct_y);
			sect.sct_own = 0;
			sect.sct_oldown = 0;
		}
		putsect(&sect);
	}
	snxtitem_all(&ni, EF_LAND);
	while (nxtitem(&ni, (s_char *)&land)) {
		if (!player->owner)
			continue;

		pr("%s disbands!\n", prland(&land));
		land.lnd_effic = 0;
		putland(land.lnd_uid, &land);
	}
	snxtitem_all(&ni, EF_SHIP);
	while (nxtitem(&ni, (s_char *)&ship)) {
		if (!player->owner)
			continue;
		pr("%s ", prship(&ship));
		switch (random() % 3) {
		case 0:
			pr("scuttled!\n");
			ship.shp_effic = 0;
			break;
		case 1:
			pr("abandoned!\n");
			putvar(V_MILIT, 0, (s_char *)&ship, EF_SHIP);
			putvar(V_CIVIL, 0, (s_char *)&ship, EF_SHIP);
			putvar(V_UW, 0, (s_char *)&ship, EF_SHIP);
			break;
		case 2:
		default:
			damage = (random() % 80) + 10;
			pr("crew has rioted! (%d damage)\n", damage);
			shipdamage(&ship, damage);
			sctnum++;
			break;
		}
		putship(ship.shp_uid, &ship);
	}
	snxtitem_all(&ni, EF_PLANE);
	while (nxtitem(&ni, (s_char *)&plane)) {
		if (!player->owner)
			continue;
		pr("%s ", prplane(&plane));
		switch (random() % 2) {
		case 0:
			pr("blown up!\n");
			plane.pln_effic = 0;
			break;
		case 1:
		default:
			damage = (random() % 80) + 10;
			pr("crew has rioted! (%d damage)\n", damage);
			planedamage(&plane, damage);
			sctnum++;
			break;
		}
		putplane(plane.pln_uid, &plane);
	}
	snxtitem_all(&ni, EF_TREATY);
	while (nxtitem(&ni, (s_char *)&treaty)) {
		if (treaty.trt_status == TS_FREE)
			continue;
		if (treaty.trt_cna != player->cnum &&
		    treaty.trt_cnb != player->cnum)
			continue;
		cn = treaty.trt_cna == player->cnum ? treaty.trt_cnb : treaty.trt_cna;
		pr("No more treaty with %s\n", cname(cn));
		wu(0, cn, "Treaty #%d with %s voided\n",
			ni.cur, cname(player->cnum));
		treaty.trt_status = TS_FREE;
		puttre(ni.cur, &treaty);
	}
	snxtitem_all(&ni, EF_LOAN);
	while (nxtitem(&ni, (s_char *)&loan)) {
		if (loan.l_status == LS_FREE)
			continue;
		if (loan.l_loner != player->cnum && loan.l_lonee != player->cnum)
			continue;
		if (loan.l_loner == player->cnum) {
			pr("Loan #%d to %s will never be repaid\n",
				ni.cur, cname(loan.l_lonee));
			wu(0, loan.l_lonee,
				"Loan #%d from %s has been voided\n",
					ni.cur, cname(player->cnum));
		} else {
			pr("Loan #%d from %s will never be repaid\n",
				ni.cur, cname(loan.l_lonee));
			wu(0, loan.l_loner,
				"Loan #%d to %s has been voided\n",
				ni.cur, cname(player->cnum));
		}
		loan.l_status = LS_FREE;
		putloan(ni.cur, &loan);
	}
	/* clean up commodity and trade for this nation. */
	commlock();
	commread();
	for (n=1; n<I_MAX+1; n++) {
		val = commamt(player->cnum, n, &dummy);
		commset(player->cnum, n, -val);
	}
	commwrite();
	communlock();
	snxtitem_all(&ni, EF_TRADE);
	while (nxtitem(&ni, (s_char *)&trade)) {
		if (trade.trd_unitid >= 0 && trade.trd_owner == player->cnum) {
			trade.trd_unitid = -1;
			(void) puttrade(ni.cur, &trade);
		}
	}
	while (sctnum > 0) {
		n = sctnum > 127 ? 127 : sctnum;
		nreport(player->cnum, N_RIOT, 0, n);
		sctnum -= n;
	}
	filetruncate(mailbox(buf, player->cnum));
	np->nat_stat = STAT_INUSE;
	np->nat_btu = 0;
	player->nstat = VIS;
	wu(0, 0, "Country #%d (%s) has dissolved\n",
		player->cnum, cname(player->cnum));
	nreport(player->cnum, N_DISS_GOV, 0, 1);
	putnat(np);
	return RET_OK;
}
