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

#ifdef DEFENSE_INFRA
int opt_DEFENSE_INFRA = 1;
#else
int opt_DEFENSE_INFRA = 0;
#endif

#ifdef SHIP_DECAY
int opt_SHIP_DECAY = 1;
#else
int opt_SHIP_DECAY = 0;
#endif

#ifdef NO_HCMS
int opt_NO_HCMS = 1;
#else
int opt_NO_HCMS = 0;
#endif

#ifdef NO_LCMS
int opt_NO_LCMS = 1;
#else
int opt_NO_LCMS = 0;
#endif

#ifdef NO_OIL
int opt_NO_OIL = 1;
#else
int opt_NO_OIL = 0;
#endif

#ifdef PLANENAMES
int opt_PLANENAMES = 1;
#else
int opt_PLANENAMES = 0;
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

#ifdef SNEAK_ATTACK
int opt_SNEAK_ATTACK = 1;
#else
int opt_SNEAK_ATTACK = 0;
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

#ifdef NONUKES
int opt_NONUKES = 1;
#else
int opt_NONUKES = 0;
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

#ifdef NEWPOWER
int opt_NEWPOWER = 1;
#else
int opt_NEWPOWER = 0;
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

#ifdef NEW_STARVE
int opt_NEW_STARVE = 1;
#else
int opt_NEW_STARVE = 0;
#endif

#ifdef NEW_WORK
int opt_NEW_WORK = 1;
#else
int opt_NEW_WORK = 0;
#endif

#ifdef RES_POP
int opt_RES_POP = 1;
#else
int opt_RES_POP = 0;
#endif

#ifdef GRAB_THINGS
int opt_GRAB_THINGS = 1;
#else
int opt_GRAB_THINGS = 0;
#endif

#ifdef SHOWPLANE
int opt_SHOWPLANE = 1;
#else
int opt_SHOWPLANE = 0;
#endif

#ifdef ORBIT
int opt_ORBIT = 1;
#else
int opt_ORBIT = 0;
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

#ifdef NUKEFAILDETONATE
int opt_NUKEFAILDETONATE = 1;
#else
int opt_NUKEFAILDETONATE = 0;
#endif

#ifdef SHIPNAMES
int opt_SHIPNAMES = 1;
#else
int opt_SHIPNAMES = 0;
#endif

#ifdef NEUTRON
int opt_NEUTRON = 1;
#else
int opt_NEUTRON = 0;
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

#ifdef ROLLOVER_AVAIL
int opt_ROLLOVER_AVAIL = 1;
#else
int opt_ROLLOVER_AVAIL = 0;
#endif

struct option_list Options[] = {
    {"ALL_BLEED", &opt_ALL_BLEED},
    {"BIG_CITY", &opt_BIG_CITY},
    {"BLITZ", &opt_BLITZ},
    {"BRIDGETOWERS", &opt_BRIDGETOWERS},
    {"DEFENSE_INFRA", &opt_DEFENSE_INFRA},
    {"DEMANDUPDATE", &opt_DEMANDUPDATE},
    {"DRNUKE", &opt_DRNUKE},
    {"EASY_BRIDGES", &opt_EASY_BRIDGES},
    {"FALLOUT", &opt_FALLOUT},
    {"FUEL", &opt_FUEL},
    {"GODNEWS", &opt_GODNEWS},
    {"GO_RENEW", &opt_GO_RENEW},
    {"GRAB_THINGS", &opt_GRAB_THINGS},
    {"HIDDEN", &opt_HIDDEN},
    {"INTERDICT_ATT", &opt_INTERDICT_ATT},
    {"LANDSPIES", &opt_LANDSPIES},
    {"LOANS", &opt_LOANS},
    {"LOSE_CONTACT", &opt_LOSE_CONTACT},
    {"MARKET", &opt_MARKET},
    {"MOB_ACCESS", &opt_MOB_ACCESS},
    {"NEUTRON", &opt_NEUTRON},
    {"NEW_STARVE", &opt_NEW_STARVE},
    {"NEW_WORK", &opt_NEW_WORK},
    {"NEWPOWER", &opt_NEWPOWER},
    {"NO_FORT_FIRE", &opt_NO_FORT_FIRE},
    {"NO_HCMS", &opt_NO_HCMS},
    {"NO_LCMS", &opt_NO_LCMS},
    {"NO_OIL", &opt_NO_OIL},
    {"NO_PLAGUE", &opt_NO_PLAGUE},
    {"NOFOOD", &opt_NOFOOD},
    {"NOMOBCOST", &opt_NOMOBCOST},
    {"NONUKES", &opt_NONUKES},
    {"NUKEFAILDETONATE", &opt_NUKEFAILDETONATE},
    {"ORBIT", &opt_ORBIT},
    {"PINPOINTMISSILE", &opt_PINPOINTMISSILE},
    {"PLANENAMES", &opt_PLANENAMES},
    {"RES_POP", &opt_RES_POP},
    {"ROLLOVER_AVAIL", &opt_ROLLOVER_AVAIL},
    {"SAIL", &opt_SAIL},
    {"SHIP_DECAY", &opt_SHIP_DECAY},
    {"SHIPNAMES", &opt_SHIPNAMES},
    {"SHOWPLANE", &opt_SHOWPLANE},
    {"SLOW_WAR", &opt_SLOW_WAR},
    {"SNEAK_ATTACK", &opt_SNEAK_ATTACK},
    {"SUPER_BARS", &opt_SUPER_BARS},
    {"TECH_POP", &opt_TECH_POP},
    {"TRADESHIPS", &opt_TRADESHIPS},
    {"TREATIES", &opt_TREATIES},
    {"UPDATESCHED", &opt_UPDATESCHED},
    {NULL, NULL},
};
