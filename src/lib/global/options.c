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
 *  options.c: Options definitions
 * 
 *  Known contributors to this file:
 *       Steve McClure, 1998
 *     
 */

#include <stdio.h>
#include "misc.h"
#include "options.h"
#include "optlist.h"

/* there must be an easier way - but can't think of one off hand */

#ifdef NO_FORT_FIRE
int opt_NO_FORT_FIRE = 1;
#else
int opt_NO_FORT_FIRE = 0;
#endif

#ifdef BRIDGETOWERS
int opt_BRIDGETOWERS = 1;
#else
int opt_BRIDGETOWERS = 0;
#endif

#ifdef GODNEWS
int opt_GODNEWS = 1;
#else
int opt_GODNEWS = 0;
#endif

#ifdef TREATIES
int opt_TREATIES = 1;
#else
int opt_TREATIES = 0;
#endif

#ifdef GO_RENEW
int opt_GO_RENEW = 1;
#else
int opt_GO_RENEW = 0;
#endif

#ifdef GUINEA_PIGS
int opt_GUINEA_PIGS = 1;
#else
int opt_GUINEA_PIGS = 0;
#endif

#ifdef DEFENSE_INFRA
int opt_DEFENSE_INFRA = 1;
#else
int opt_DEFENSE_INFRA = 0;
#endif

#ifdef LANDSPIES
int opt_LANDSPIES = 1;
#else
int opt_LANDSPIES = 0;
#endif

#ifdef MOB_ACCESS
int opt_MOB_ACCESS = 1;
#else
int opt_MOB_ACCESS = 0;
#endif

#ifdef LOANS
int opt_LOANS = 1;
#else
int opt_LOANS = 0;
#endif

#ifdef MARKET
int opt_MARKET = 1;
#else
int opt_MARKET = 0;
#endif

#ifdef NOFOOD
int opt_NOFOOD = 1;
#else
int opt_NOFOOD = 0;
#endif

#ifdef BLITZ
int opt_BLITZ = 1;
#else
int opt_BLITZ = 0;
#endif

#ifdef HIDDEN
int opt_HIDDEN = 1;
#else
int opt_HIDDEN = 0;
#endif

#ifdef LOSE_CONTACT
int opt_LOSE_CONTACT = 1;
#else
int opt_LOSE_CONTACT = 0;
#endif

#ifdef FUEL
int opt_FUEL = 1;
#else
int opt_FUEL = 0;
#endif

#ifdef TRADESHIPS
int opt_TRADESHIPS = 1;
#else
int opt_TRADESHIPS = 0;
#endif

#ifdef NOMOBCOST
int opt_NOMOBCOST = 1;
#else
int opt_NOMOBCOST = 0;
#endif

#ifdef SUPER_BARS
int opt_SUPER_BARS = 1;
#else
int opt_SUPER_BARS = 0;
#endif

#ifdef EASY_BRIDGES
int opt_EASY_BRIDGES = 1;
#else
int opt_EASY_BRIDGES = 0;
#endif

#ifdef ALL_BLEED
int opt_ALL_BLEED = 1;
#else
int opt_ALL_BLEED = 0;
#endif

#ifdef DRNUKE
int opt_DRNUKE = 1;
#else
int opt_DRNUKE = 0;
#endif

#ifdef SLOW_WAR
int opt_SLOW_WAR = 1;
#else
int opt_SLOW_WAR = 0;
#endif

#ifdef NO_PLAGUE
int opt_NO_PLAGUE = 1;
#else
int opt_NO_PLAGUE = 0;
#endif

#ifdef RES_POP
int opt_RES_POP = 1;
#else
int opt_RES_POP = 0;
#endif

#ifdef SHOWPLANE
int opt_SHOWPLANE = 1;
#else
int opt_SHOWPLANE = 0;
#endif

#ifdef PINPOINTMISSILE
int opt_PINPOINTMISSILE = 1;
#else
int opt_PINPOINTMISSILE = 0;
#endif

#ifdef FALLOUT
int opt_FALLOUT = 1;
#else
int opt_FALLOUT = 0;
#endif

#ifdef SAIL
int opt_SAIL = 1;
#else
int opt_SAIL = 0;
#endif

#ifdef UPDATESCHED
int opt_UPDATESCHED = 1;
#else
int opt_UPDATESCHED = 0;
#endif

#ifdef DEMANDUPDATE
int opt_DEMANDUPDATE = 1;
#else
int opt_DEMANDUPDATE = 0;
#endif

#ifdef BIG_CITY
int opt_BIG_CITY = 1;
#else
int opt_BIG_CITY = 0;
#endif

#ifdef INTERDICT_ATT
int opt_INTERDICT_ATT = 1;
#else
int opt_INTERDICT_ATT = 0;
#endif

#ifdef TECH_POP
int opt_TECH_POP = 1;
#else
int opt_TECH_POP = 0;
#endif
