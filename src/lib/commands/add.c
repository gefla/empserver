/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  add.c: Add a new country to the game
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     
 */

#include <stdio.h>
#include <string.h>
#include "misc.h"
#include "player.h"
#include "plague.h"
#include "sect.h"
#include "nat.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "tel.h"
#include "land.h"
#include "commands.h"
#include "optlist.h"

int
add(void)
{
    struct natstr *natp;
    struct sctstr sect;
    struct nstr_sect nstr;
    int i;
    char cntryname[sizeof(natp->nat_cnam)];
    char pname[sizeof(natp->nat_pnam)];
    natid coun;
    natid freecn;
    char prompt[128];
    char buf[1024];
    char *p;
    int stat;
    struct nstr_item ni;
    struct lndstr land;

    for (freecn = 0; NULL != (natp = getnatp(freecn)); freecn++) {
	if ((natp->nat_stat & STAT_INUSE) == 0)
	    break;
    }
    if (freecn < MAXNOC)
	sprintf(prompt, "New country number? (%d is unused) ", freecn);
    else
	strcpy(prompt, "New country number? (they all seem to be used) ");
    p = getstarg(player->argp[1], prompt, buf);
    if (p == 0 || *p == 0)
	return RET_SYN;
    i = atoi(p);
    if (i >= MAXNOC) {
	pr("Max # countries is %d\n", MAXNOC);
	return RET_FAIL;
    }
    coun = i;
    if (coun == 0) {
	pr("Not allowed to add country #0\n");
	return RET_FAIL;
    }
    natp = getnatp(coun);
    p = getstarg(player->argp[2], "Country Name? ", buf);
    if (p == 0 || *p == 0)
	return RET_SYN;
    if (strlen(p) >= sizeof(cntryname)) {
	pr("Country name too long\n");
	return RET_FAIL;
    }
    strcpy(cntryname, p);
    p = getstarg(player->argp[3], "Representative? ", buf);
    if (p == 0 || *p == 0)
	return RET_SYN;
    if (strlen(p) >= sizeof(pname)) {
	pr("Representative too long\n");
	return RET_FAIL;
    }
    strcpy(pname, p);
    p = getstarg(player->argp[4],
		 "Status? (visitor, new, active, god, delete) ", buf);
    if (p == 0 || *p == 0)
	return RET_SYN;
    switch (*p) {
    case 'v':
	stat = STAT_INUSE;
	break;
    case 'n':
	stat = STAT_NEW | STAT_INUSE;
	break;
    case 'a':
	stat = STAT_NORM | STAT_INUSE;
	break;
    case 'g':
	stat = STAT_GOD | STAT_NORM | STAT_INUSE;
	break;
    case 'd':
	stat = 0;
	break;
    default:
	pr("Illegal status\n");
	return RET_SYN;
    }
    p = getstarg(player->argp[5],
		 "Check, wipe, or ignore existing sectors (c|w|i) ", buf);
    if (p == 0)
	return RET_SYN;
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == coun) {
	    makelost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x,
		     land.lnd_y);
	    land.lnd_own = 0;
	    pr("Land unit %d wiped\n", land.lnd_uid);
	    putland(land.lnd_uid, &land);
	}
    }
    natp->nat_stat = stat;
    strcpy(natp->nat_cnam, cntryname);
    strcpy(natp->nat_pnam, pname);
    if (*p != 'w' && *p != 'c') {
	pr("Any existing sectors ignored\n");
    } else {
	pr("Checking sectors...\n");
	snxtsct_all(&nstr);
	while (nxtsct(&nstr, &sect)) {
	    if (sect.sct_own != coun)
		continue;
	    pr("%s ", xyas(nstr.x, nstr.y, player->cnum));
	    if (*p == 'w') {
		sect.sct_mobil = 0;
		sect.sct_effic = 0;
		sect.sct_road = 0;
		sect.sct_rail = 0;
		sect.sct_defense = 0;
		sect.sct_own = 0;
		sect.sct_oldown = 0;
		if (sect.sct_type == SCT_BSPAN ||
		    sect.sct_type == SCT_BTOWER)
		    sect.sct_newtype = sect.sct_type = SCT_WATER;
		else if (sect.sct_type != SCT_MOUNT &&
		    sect.sct_type != SCT_PLAINS)
		    sect.sct_newtype = sect.sct_type = SCT_RURAL;
		/* No dist path */
		sect.sct_dist_x = sect.sct_x;
		sect.sct_dist_y = sect.sct_y;
		memset(sect.sct_item, 0, sizeof(sect.sct_item));
		memset(sect.sct_del, 0, sizeof(sect.sct_del));
		memset(sect.sct_dist, 0, sizeof(sect.sct_dist));
		sect.sct_mines = 0;
		sect.sct_pstage = PLG_HEALTHY;
		sect.sct_ptime = 0;
		sect.sct_che = 0;
		sect.sct_che_target = 0;
		sect.sct_fallout = 0;
		putsect(&sect);
		pr("wiped\n");
	    } else {
		pr("\n");
	    }
	}
    }

    if ((natp->nat_stat & (STAT_INUSE | STAT_NORM | STAT_GOD)) ==
	STAT_INUSE) {
	*natp->nat_hostaddr = '\0';
	*natp->nat_hostname = '\0';
	*natp->nat_userid = '\0';
	natp->nat_btu = 0;
	natp->nat_reserve = 0;
	natp->nat_tgms = 0;
	natp->nat_ystart = 0;
	natp->nat_xstart = 0;
	natp->nat_ycap = 0;
	natp->nat_xcap = 0;
	natp->nat_yorg = 0;
	natp->nat_xorg = 0;
	natp->nat_dayno = 0;
	natp->nat_minused = 0;
	memset(natp->nat_b, 0, sizeof(natp->nat_b));
	natp->nat_last_login = natp->nat_last_login = 0;
	natp->nat_money = 0;
	natp->nat_level[NAT_TLEV] = start_technology;
	natp->nat_level[NAT_RLEV] = start_research;
	natp->nat_level[NAT_ELEV] = start_education;
	natp->nat_level[NAT_HLEV] = start_happiness;
	for (i = 0; i < MAXNOC / 4; i++)
	    natp->nat_rejects[i] = 0;
	natp->nat_newstim = 0;
	natp->nat_annotim = 0;
	(void)creat(mailbox(buf, coun), 0660);
    } else
	pr("No special initializations done...\n");

    for (i = 0; i <= PRI_MAX; i++)
	natp->nat_priorities[i] = -1;
    natp->nat_flags =
	NF_FLASH | NF_BEEP | NF_COASTWATCH | NF_SONAR | NF_TECHLISTS;
    for (i = 0; i < MAXNOC; i++)
	natp->nat_relate[i] = NEUTRAL;
    putnat(natp);
    return 0;
}
