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
 *  nat.c: Nation subroutines
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2008
 */

#include <config.h>

#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "file.h"
#include "game.h"
#include "nat.h"
#include "optlist.h"
#include "tel.h"
#include "xy.h"

/*
 * Reset NATP for status STAT with origin/capital at X,Y.
 * Wipes everything but nat_cnum, nat_cnam and nat_pnam.
 * Also wipes realms and telegrams.
 */
struct natstr *
nat_reset(struct natstr *natp, enum nat_status stat, coord x, coord y)
{
    static struct range defrealm = { -8, -5, 10, 5, 0, 0 };
    struct realmstr newrealm;
    struct range absrealm;
    char buf[1024];
    int i;

    natp->nat_stat = stat;
    *natp->nat_hostaddr = '\0';
    *natp->nat_hostname = '\0';
    *natp->nat_userid = '\0';

    natp->nat_xcap = natp->nat_xorg = x;
    natp->nat_ycap = natp->nat_yorg = y;
    if (stat == STAT_SANCT)
	xyabsrange(natp, &defrealm, &absrealm);
    else
	memset(&absrealm, 0, sizeof(absrealm));
    for (i = 0; i < MAXNOR; i++) {
	ef_blank(EF_REALM, i + natp->nat_cnum * MAXNOR, &newrealm);
	newrealm.r_cnum = natp->nat_cnum;
	newrealm.r_realm = i;
	newrealm.r_xl = absrealm.lx;
	newrealm.r_xh = absrealm.hx;
	newrealm.r_yl = absrealm.ly;
	newrealm.r_yh = absrealm.hy;
	putrealm(&newrealm);
    }
    if (players_at_00) {
	natp->nat_xorg = 0;
	natp->nat_yorg = 0;
    }

    natp->nat_dayno = 0;
    natp->nat_minused = 0;
    natp->nat_update = 0;

    natp->nat_tgms = 0;
    close(creat(mailbox(buf, natp->nat_cnum), S_IRWUG));
    natp->nat_ann = 0;		/* FIXME number of annos */

    natp->nat_btu = stat == STAT_SANCT ? max_btus : 0;
    natp->nat_access = 0;
    game_tick_to_now(&natp->nat_access);
    natp->nat_reserve = 0;
    natp->nat_money = stat == STAT_SANCT ? start_cash : 0;
    natp->nat_last_login = natp->nat_last_login = 0;
    natp->nat_newstim = 0;
    natp->nat_annotim = 0;
    natp->nat_level[NAT_HLEV] = start_happiness;
    natp->nat_level[NAT_RLEV] = start_research;
    natp->nat_level[NAT_TLEV] = start_technology;
    natp->nat_level[NAT_ELEV] = start_education;
    for (i = 0; i < MAXNOC; i++)
	natp->nat_relate[i] = NEUTRAL;
    memset(natp->nat_contact, 0, sizeof(natp->nat_rejects));
    memset(natp->nat_rejects, 0, sizeof(natp->nat_rejects));
    natp->nat_flags =
	NF_FLASH | NF_BEEP | NF_COASTWATCH | NF_SONAR | NF_TECHLISTS;

    return natp;
}
