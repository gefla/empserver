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
 *  shoo.c: Shoot some conquered populace or pigeons.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "var.h"
#include "sect.h"
#include "news.h"
#include "item.h"
#include "path.h"
#include "nat.h"
#include "file.h"
#include "nsc.h"
#include "land.h"
#include "commands.h"

int
shoo(void)
{
	struct sctstr sect;
	struct	nstr_sect nstr;
	struct	nstr_item ni;
	int     nshot;
	int     mob_cost;
	double	m;
	int	item;
	struct	ichrstr *ip;
	struct	lndstr land;
	int	targets;
	s_char	*p;
	int	vec[I_MAX+1], mil, nsec;
	s_char	prompt[128];
	s_char	buf[128];

	ip = whatitem(player->argp[1], "Shoot what <civ or uw> ");
	if (ip == 0 || (ip->i_vtype != V_CIVIL && ip->i_vtype != V_UW))
		return RET_SYN;
	item = ip - ichr;
	if (!snxtsct(&nstr, player->argp[2]))
		return RET_SYN;
	sprintf(prompt, "number of %s to shoot (max 999)? ", ip->i_name);
	p = getstarg(player->argp[3], prompt, buf);
	if (p == 0 || (targets = atoi(p)) <= 0)
		return RET_SYN;
	if (targets > 999)
		targets = 999;
	while (nxtsct(&nstr, &sect)) {
		if (!player->owner)
			continue;
		if (getvec(VT_ITEM, vec, (s_char *) &sect, EF_SECTOR) <= 0)
			continue;
		mil = vec[I_MILIT];
		nsec = 0;
		snxtitem_xy(&ni, EF_LAND, sect.sct_x, sect.sct_y);
                while (nxtitem(&ni, (s_char *)&land)){
                        mil += total_mil(&land);

			if (lchr[(int)land.lnd_type].l_flags & L_SECURITY){
				mil += total_mil(&land);
				nsec++;
			}
		}

		if (vec[item] == 0 || vec[I_CIVIL] > mil * 10)
			continue;
		nshot = vec[item] > targets ? targets : vec[item];
		m = ((double)nshot + 4.0) / 5.0;

		if (m > sect.sct_mobil) {
			nshot = sect.sct_mobil * 5;
			m = sect.sct_mobil;
		}
		/*
		 * Each security unit lowers the cost of
		 * shooting a person by 10%. However, you
		 * can't go lower than 50% of normal cost
		 */
		if (nsec > 5)
			nsec=5;
		m = ((float)m * (1.0-((float)nsec * 0.1)));
		if (nshot <= 0)
			continue;
		if (m < 0)
			m = 0;
		if (m > sect.sct_mobil)
			m = sect.sct_mobil;
		mob_cost = roundavg(m);
		sect.sct_mobil -= (u_char)mob_cost;
		vec[item] -= nshot;
		pr("BANG!! (thump) %d %s shot in %s!\n",
		       nshot, ip->i_name, 
		       xyas(sect.sct_x, sect.sct_y,player->cnum));
		if (chance(nshot / 100.0))
			nreport(player->cnum, N_SHOOT_CIV, sect.sct_oldown, 1);
		if (vec[item] <= 0 && ip->i_vtype == V_CIVIL &&
		    (sect.sct_own != sect.sct_oldown)) {
			sect.sct_oldown = sect.sct_own;
			pr("  %s is now completely yours\n",
			       xyas(sect.sct_x, sect.sct_y,player->cnum));
		}
		putvar(ip->i_vtype, vec[item], (s_char *)&sect, EF_SECTOR);
		putsect(&sect);
	}
	return RET_OK;
}
