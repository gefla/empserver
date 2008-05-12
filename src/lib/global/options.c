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
 *  options.c: Options definitions
 * 
 *  Known contributors to this file:
 *       Steve McClure, 1998
 *       Markus Armbruster, 2005-2006
 */

#include <config.h>

#include "optlist.h"

int opt_ALL_BLEED = 1;
int opt_AUTO_POWER = 0;
int opt_BLITZ = 1;
int opt_BRIDGETOWERS = 1;
int opt_EASY_BRIDGES = 1;
int opt_FALLOUT = 1;
int opt_GODNEWS = 1;
int opt_GO_RENEW = 0;
int opt_GUINEA_PIGS = 0;
int opt_HIDDEN = 0;
int opt_INTERDICT_ATT = 1;
int opt_LANDSPIES = 1;
int opt_LOANS = 1;
int opt_LOSE_CONTACT = 0;
int opt_MARKET = 0;
int opt_MOB_ACCESS = 0;
int opt_NOFOOD = 1;
int opt_NOMOBCOST = 1;
int opt_NO_FORT_FIRE = 0;
int opt_NO_PLAGUE = 1;
int opt_PINPOINTMISSILE = 1;
int opt_RES_POP = 0;
int opt_SAIL = 1;
int opt_SHOWPLANE = 1;
int opt_SLOW_WAR = 0;
int opt_SUPER_BARS = 0;
int opt_TECH_POP = 0;
int opt_TRADESHIPS = 0;
int opt_TREATIES = 1;
