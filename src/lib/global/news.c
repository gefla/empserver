/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  news.c: News item characteristics
 *
 *  Known contributors to this file:
 *
 */

/*
 * News item characteristics; one entry (and two different
 * strings) per news item.  Be sure and increase N_MAX_VERB
 * in news.h if you add a news item.
 *
 * Order must correspond to ordering in news.h
 */

#include <config.h>

#include "news.h"

static char no_news[] = "does nothing in particular to %s";

struct rptstr rpt[] = {
    /*  nice  page     text */
    { 0, 0, 0, { no_news, no_news } },
    { N_WON_SECT, -4, N_FRONT, { "infantry capture %s territory",
				 "shock troops overrun one of %s's sectors" } },
    { N_SCT_LOSE, -4, N_FRONT, { "infantry beaten back by %s troops",
				 "shock troops annihilated in failed attack on %s" } },
    { N_SPY_SHOT, -1, N_SPY,   { "spy shot by %s firing squad",
				 "spy captured and shot by %s" } },
    { N_SENT_TEL, 1,  N_TELE,  { "sends a telegram to %s",
				 "telexes %s" } },
    { 5, 0, 0, { no_news, no_news } },
    { N_MAKE_LOAN, 2,  N_ECON,  { "bankers make a loan to %s",
				  "Ministry of Finance lends money to %s" } },
    { N_REPAY_LOAN, 1,  N_ECON,  { "repays a loan from %s",
				   "makes last payment on loan from %s" } },
    { N_MAKE_SALE, 0,  N_ECON,  { "makes a sale to %s",
				  "sells goods to %s" } },
    { N_OVFLY_SECT, -1, N_SKY,   { "violates %s airspace",
				   "overflies %s territory" } },
    { N_SCT_SHELL, -2, N_ARTY,  { "gunners bombard %s territory",
				  "artillery fires on %s sectors" } },
    { N_SHP_SHELL, -2, N_SEA,   { "shells a ship owned by %s",
				  "fires on %s ships" } },
    { N_TOOK_UNOCC, 0,  N_FRONT, { "takes over unoccupied land",
				   "attacks unowned land for some reason" } },
    { N_TORP_SHIP, 0,  N_SEA,   { "ships torpedoed by enemy wolf-packs",
				  "ships torpedoed by marauding U-boats" } },
    { N_FIRE_BACK, 0,  N_ARTY,  { "gunners fire on %s in self-defense",
				  "gunners decimate %s aggressors" } },
    { N_BROKE_SANCT, 0,  N_FOR,   { "breaks sanctuary",
				    "no longer has a sanctuary" } },
    { N_SCT_BOMB, -2, N_SKY,   { "planes divebomb one of %s's sectors",
				 "bombers wreak havoc on %s" } },
    { N_SHP_BOMB, -2, N_SEA,   { "divebombs a ship flying the flag of %s",
				 "airforce bombs %s ships" } },
    { N_BOARD_SHIP, -2, N_SEA,   { "seadogs board one of %s's ships",
				   "pirates board %s ship" } },
    { N_SHP_LOSE, -3, N_SEA,   { "is repelled by %s while attempting to board a ship",
				 "pirates prove inept at boarding %s's ships" } },
    { 20, 0, 0, { no_news, no_news } },
    { N_SEIZE_SECT, -2, N_ECON,  { "seizes a sector from %s to collect on a loan",
				   "collects one of %s's sectors in repayment of a loan" } },
    { 22, 0, 0, { no_news, no_news } },
    { 23, 0, 0, { no_news, no_news } },
    { 24, 0, 0, { no_news, no_news } },
    { N_HIT_MINE, 0,  N_SEA,   { "ship hits a mine",
				 "ship severely damaged in mine field" } },
    { N_DECL_ALLY, 5,  N_FOR,   { "announces an alliance with %s",
				  "/ %s alliance declared" } },
    { 27, 0, 0, { no_news, no_news } },
    { N_DECL_WAR, -5, N_FOR,   { "declares TOTAL WAR on %s",
				 "gets serious with %s and declares WAR" } },
    { N_DIS_ALLY, 0,  N_FOR,   { "diplomats disavow former alliance with %s",
				 "is no longer allied with %s" } },
    { N_DIS_WAR, 5,  N_FOR,   { "is no longer at war with %s",
				"Foreign Ministry declares \"No more war with %s\"" } },
    { N_OUT_PLAGUE, 0,  N_HOME,  { "reports outbreak of the black plague",
				   "sector infected with bubonic plague" } },
    { N_DIE_PLAGUE, 0,  N_HOME,  { "citizens die from bubonic plague",
				   "sector reports plague deaths" } },
    { N_NAME_CHNG, 0,  N_FOR,   { "goes through a name change",
				  "adopts a new country name" } },
    { N_DIE_FAMINE, 0,  N_HOME,  { "citizens starve in disastrous famine",
				   "loses citizens to mass starvation" } },
    { 35, 0, 0, { no_news, no_news } },
    { N_DOWN_PLANE, -3, N_SKY,   { "pilots down %s planes",
				   "victorious in air-to-air combat against %s" } },
    { N_NUKE, -10,N_MISS,  { "nuclear device devastates %s sector",
			     "explodes a nuclear device damaging %s territory" } },
    { N_FREEDOM_FIGHT, 0,  N_HOME,  { "terrorists shoot it out with %s special forces",
				      "underground rises up against %s occupation" } },
    { N_SHOOT_CIV, -2, N_HOME,  { "execution squads gun down helpless %s civilians",
				  "firing squads massacre defenseless %s workers" } },
    { N_LAUNCH,  0, N_MISS,  { "launches a satellite into orbit",
			       "continues its conquest of space with a successful launch" } },
    { N_SAT_KILL, -8, N_MISS,  { "strikes a %s big bird",
				 "rockets damage a %s satellite" } },
    { N_GIFT,  0, N_ECON,  { "makes a contribution to %s",
			     "helps out %s" } },
    { N_AIDS,  0, N_HOME,  { "aids %s with divine intervention",
			     "smiles upon %s" } },
    { N_HURTS,  0, N_HOME,  { "hurts %s with divine intervention",
			      "frowns upon %s" } },
    { N_TAKE,  0, N_ECON,  { "sacrifices to %s",
			     "makes divine payment to %s" } },
    { N_NUKE_STOP, -10,N_MISS,  { "abms intercept a %s missile",
				  "strategic defense systems destroy a %s missile" } },
    { N_SCT_MISS, -2, N_MISS,  { "missile fired at one of %s's sectors",
				 "missile wreaks havoc on %s" } },
    { N_SHP_MISS, -2, N_MISS,  { "missile fired at a ship flying the flag of %s",
				 "missiles hit %s ships" } },
    { N_TRADE,  2, N_ECON,  { "engages in friendly trade with %s",
			      "profits from a merchant venture with %s" } },
    { N_PIRATE_TRADE,  0, N_ECON,  { "pirates dispose of %s booty in trade",
				     "buccaneers sell their %s goods" } },
    { N_PIRATE_KEEP,  0, N_ECON,  { "pirates keep their %s booty",
				    "buccaneers laugh and bury their %s goods" } },
    { N_SHIP_TORP,  0, N_SEA,   { "ships torpedoed by %s torpedo-boats",
				  "ships sunk by marauding %s torpedo-boats" } },
    { N_SUB_BOMB,  0, N_SEA,   { "planes bomb a skulking %s submarine",
				 "planes drop depth-charges on a %s sub" } },
    { 54, 0, 0, { no_news, no_news } },
    { N_UNIT_BOMB, -2, N_SKY,   { "divebombs a brave %s unit",
				  "airforce bombs %s units" } },
    { N_LHIT_MINE, 0,  N_FRONT, { "troops run afoul of a landmine",
				  "troops severely damaged in mine field" } },
    { N_FIRE_F_ATTACK, -2, N_ARTY,  { "fortress gunners support troops attacking %s",
				      "fortress artillery massacres %s defenders" } },
    { N_FIRE_L_ATTACK, -2, N_ARTY,  { "gunners support troops attacking %s",
				      "artillery battery massacres %s defenders" } },
    { N_FIRE_S_ATTACK, -2, N_ARTY,  { "naval gunners support troops attacking %s",
				      "naval gunfire massacres %s defenders" } },
    { N_SACK_CAP, 10, N_FOR,   { "sacks %s capital",
				 "captures and pillages %s's capital" } },
    { N_UP_FRIENDLY, 3,  N_FOR,   { "announces friendly trade relations with %s",
				    "upgrades %s's trade status to triple-A" } },
    { N_DOWN_FRIENDLY, 0,  N_FOR,   { "downgrades relations with %s to friendly",
				      "cools relations with %s to friendly" } },
    { N_UP_NEUTRAL, 2,  N_FOR,   { "upgrades relations with %s to neutral",
				   "Foreign Ministry declares \"%s is A-OK.\"" } },
    { N_DOWN_NEUTRAL, 0,  N_FOR,   { "downgrades relations with %s to neutral",
				     "gives the cold shoulder to %s and declares neutral relations" } },
    { N_UP_HOSTILE, 3,  N_FOR,   { "upgrades relations with %s to hostile",
				   "forgives %s of past war crimes but remains hostile" } },
    { N_DOWN_HOSTILE, 3,  N_FOR,   { "downgrades relations with %s to hostile",
				     "is suspicious that %s has hostile intentions" } },
    { N_SCT_SMISS, 0,  N_MISS,  { "sector is struck by a sub-launched missile",
				  "sector devastated by missile" } },
    { N_SHP_SMISS, 0,  N_MISS,  { "ship is hit by a sub-launched missile",
				  "ship damaged by marine missile" } },
    { N_START_COL, 0,  N_COLONY,{ "pilgrims settle on a new island",
				  "discovers new land and starts a colony" } },
    { N_NUKE_SSTOP, 0,  N_MISS,  { "abms intercept an incoming sub-launched missile",
				   "strategic defense systems destroy an incoming sub-launched warhead"}},
    { N_LND_MISS, 2,  N_MISS,  { "missile fired at %s ground troops",
				 "missile hits %s troops" } },
    { N_LND_SMISS, 0,  N_MISS,  { "regiment struck by sub-launched missile",
				  "ground troops hit by missile" } },
    { N_AWON_SECT, -4, N_FRONT, { "navy secures a beachhead on %s territory",
				  "sailors take a coastal sector from %s" } },
    { N_PWON_SECT, -4, N_FRONT, { "paratroopers comandeer a sector from %s",
				  "air force parachutists overwhelm a %s sector" } },
    { N_PARA_UNOCC, 0,  N_COLONY,{ "parachutists grab unoccupied land",
				   "paratroopers break new ground" } },
    { N_ALOSE_SCT, 4,  N_FRONT, { "sailors repelled by %s coast-guard",
				  "naval forces massacred in failed assault of %s" } },
    { N_PLOSE_SCT, 4,  N_FRONT, { "paratroopers destroyed in failed air-assault on %s",
				  "loses parachutists in failed air-assault of %s" } },
    { 78, 0, 0, { no_news, no_news } },
    { 79, 0, 0, { no_news, no_news } },
    { N_WELCH_DEAL, 0,  N_ECON,  { "is unable to fulfill its financial obligations to %s",
				   "welches on a deal with %s" } },
    { N_LND_LOSE, -3, N_FRONT, { "is repelled by %s while attempting to board a land unit",
				 "forces prove inept at boarding %s's land unit" } },
    { N_BOARD_LAND, -2, N_FRONT, { "mercenaries board one of %s's land units",
				   "elite forces board %s land unit" } },
    { 0, 0, 0, { NULL, NULL } }
};
