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
 *  thre.c: Describe/set threshold for given commodity
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "xy.h"
#include "item.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "commands.h"

/*
 * threshold <COMM> <SECTS> <THRESH>
 */
int
thre(void)
{
    extern	struct ichrstr *whatitem();
    struct	sctstr sect;
    struct	nstr_sect nstr;
    int	val;
    struct	ichrstr *ip;
    s_char	*p;
    int	thresh;
    int	type;
    s_char	prompt[128];
    s_char	buf[128];
    
    if ((ip = whatitem(player->argp[1], "What commodity? ")) == 0)
	return RET_SYN;
    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    type = V_DIST(ip->i_vtype & (~VT_TYPE));
    if (player->argp[3] && *player->argp[3] &&
	(*player->argp[3] < '0' || *player->argp[3] > '9')) {
	pr ("Threshold must be a number\n");
	return RET_SYN;
    }
    while (!player->aborted && nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	val = getvar(type, (s_char *)&sect, EF_SECTOR);
	if (val > 0)
	    sprintf(prompt, "%s %s  old threshold %d new? ",
		    xyas(nstr.x, nstr.y, player->cnum),
		    dchr[sect.sct_type].d_name, val);
	else
	    sprintf(prompt, "%s %s  threshold? ",
		    xyas(nstr.x, nstr.y, player->cnum),
		    dchr[sect.sct_type].d_name);
	if ((p = getstarg(player->argp[3], prompt, buf)) == 0)
	    return RET_FAIL;
	if (!check_sect_ok(&sect))
	    return RET_FAIL;
	if (*p == '\0' || *p == '-')
	    continue;
	thresh = atoi(p);
	if (thresh > 10000)
	    thresh = 10000;
	if ((val > 0) && (val == thresh)) {
	    pr("%s threshold unchanged (left at %d)\n",
	       xyas(nstr.x, nstr.y, player->cnum), val);
	    continue;
	}
	if (val > 0 && (player->argp[3] != 0 && *player->argp[3] != 0))
	    pr("%s old threshold %d\n",
	       xyas(nstr.x, nstr.y, player->cnum),
	       val);
	if (putvar(type, thresh, (s_char *)&sect, EF_SECTOR) < 0)
	    pr("No room for threshold in %s\n",
	       xyas(nstr.x, nstr.y, player->cnum));
	putsect(&sect);
    }
    return RET_OK;
}
