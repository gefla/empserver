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

#include "misc.h"
#include "news.h"

char *page_headings[N_MAX_PAGE + 1] = {
/* not used */ "Comics",
/* N_FOR    */ "Foreign Affairs",
/* N_FRONT  */ "The Front Line",
/* N_SEA    */ "The High Seas",
/* N_SKY    */ "Sky Watch",
/* N_MISS   */ "Guidance Systems",
/* N_ARTY   */ "Firestorms",
/* N_ECON   */ "Business & Economics",
/* N_COLONY */ "The Frontier",
/* N_HOME   */ "The Home Front",
/* N_SPY    */ "Espionage",
/* N_TELE   */ "Telecommunications"
};

struct rptstr rpt[N_MAX_VERB + 1] = {
/*  nice  page     text */
    { 0,  0,	   { "does nothing in particular to %s",
/*  0*/	  	     "does nothing to %s" } },
    { -4, N_FRONT, { "infantry capture %s territory",
/*  1*/	  	     "shock troops overrun one of %s's sectors" } },
    { -4, N_FRONT, { "infantry beaten back by %s troops",
/*  2*/	  	     "shock troops annihilated in failed attack on %s" } },
    { -1, N_SPY,   { "spy shot by %s firing squad",
/*  3*/	  	     "spy captured and shot by %s" } },
    { 1,  N_TELE,  { "sends a telegram to %s",
/*  4*/	  	     "telexes %s" } },
    { 3,  N_FOR,   { "diplomats sign a treaty with %s",
/*  5*/	  	     "ambassador agrees to a treaty with %s" } },
    { 2,  N_ECON,  { "bankers make a loan to %s",
/*  6*/	  	     "Ministry of Finance lends money to %s" } },
    { 1,  N_ECON,  { "repays a loan from %s",
/*  7*/	  	     "makes last payment on loan from %s" } },
    { 0,  N_ECON,  { "makes a sale to %s",
/*  8*/	  	     "sells goods to %s" } },
    { -1, N_SKY,   { "violates %s airspace",
/*  9*/	  	     "overflies %s territory" } },
    { -2, N_ARTY,  { "gunners bombard %s territory",
/* 10*/	  	     "artillery fires on %s sectors" } },
    { -2, N_SEA,   { "shells a ship owned by %s",
/* 11*/	  	     "fires on %s ships" } },
    { 0,  N_FRONT, { "takes over unoccupied land",
/* 12*/	  	     "attacks unowned land for some reason" } },
    { 0,  N_SEA,   { "ships torpedoed by enemy wolf-packs",
/* 13*/	  	     "ships torpedoed by marauding U-boats" } },
    { 0,  N_ARTY,  { "gunners fire on %s in self-defense",
/* 14*/	  	     "gunners decimate %s aggressors" } },
    { 0,  N_FOR,   { "breaks sanctuary",
/* 15*/	  	     "no longer has a sanctuary" } },
    { -2, N_SKY,   { "planes divebomb one of %s's sectors",
/* 16*/	  	     "bombers wreak havoc on %s" } },
    { -2, N_SEA,   { "divebombs a ship flying the flag of %s",
/* 17*/	  	     "airforce bombs %s ships" } },
    { -2, N_SEA,   { "seadogs board one of %s's ships",
/* 18*/	  	     "pirates board %s ship" } },
    { -3, N_SEA,   { "is repelled by %s while attempting to board a ship",
/* 19*/	  	     "pirates prove inept at boarding %s's ships" } },
    { -1, N_SKY,   { "anti-aircraft gunners fire on %s airforce",
/* 20*/	  	     "attempts to shoot down %s aircraft" } },
    { -2, N_ECON,  { "seizes a sector from %s to collect on a loan",
/* 21*/	  	     "collects one of %s's sectors in repayment of a loan" } },
    { -1, N_FOR,   { "considers an action which would violate a treaty with %s",
/* 22*/	  	     "decides not to violate treaty with %s (yet)" } },
    { -4, N_FOR,   { "violates a treaty with %s",
/* 23*/	  	     "actions violate treaty with %s" } },
    { 0,  N_FOR,   { "dissolves its government",
/* 24*/	  	     "throws in the towel" } },
    { 0,  N_SEA,   { "ship hits a mine",
/* 25*/	  	     "ship severely damaged in mine field" } },
    { 5,  N_FOR,   { "announces an alliance with %s",
/* 26*/	  	     "/ %s alliance declared" } },
    { 0,  N_FOR,   { "declares their neutrality toward %s", /* no longer used */
/* 27*/	  	     "announces neutral relations with %s" } },
    { -5, N_FOR,   { "declares TOTAL WAR on %s",
/* 28*/	  	     "gets serious with %s and declares WAR" } },
    { 0,  N_FOR,   { "diplomats disavow former alliance with %s",
/* 29*/	  	     "is no longer allied with %s" } },
    { 5,  N_FOR,   { "is no longer at war with %s",
/* 30*/	  	     "Foreign Ministry declares \"No more war with %s\"" } },
    { 0,  N_HOME,  { "reports outbreak of the black plague",
/* 31*/	  	     "sector infected with bubonic plague" } },
    { 0,  N_HOME,  { "citizens die from bubonic plague",
/* 32*/	  	     "sector reports plague deaths" } },
    { 0,  N_FOR,   { "goes through a name change",
/* 33*/	  	     "adopts a new country name" } },
    { 0,  N_HOME,  { "citizens starve in disastrous famine",
/* 34*/	  	     "loses citizens to mass starvation" } },
    { 0,  N_HOME,  { "endures lawless rioting",
/* 35*/	  	     "suffers from outbreaks of rioting" } },
    { -3, N_SKY,   { "pilots down %s planes",
/* 36*/	  	     "victorious in air-to-air combat against %s" } },
    { -10,N_MISS,  { "nuclear device devastates %s sector",
/* 37*/	  	     "explodes a nuclear device damaging %s territory" } },
    { 0,  N_HOME,  { "terrorists shoot it out with %s special forces",
/* 38*/	  	     "underground rises up against %s occupation" } },
    { -2, N_HOME,  { "execution squads gun down helpless %s civilians",
/* 39*/	  	     "firing squads massacre defenseless %s workers" } },
    {  0, N_MISS,  { "launches a satellite into orbit",
/* 40*/	  	     "continues its conquest of space with a successful launch" } },
    { -8, N_MISS,  { "strikes a %s big bird",
/* 41*/	  	     "rockets damage a %s satellite" } },
    {  4, N_ECON,  { "makes a contribution to %s",
/* 42*/	  	     "helps out %s" } },
    {  2, N_HOME,  { "aids %s with divine intervention",
/* 43*/	  	     "smiles upon %s" } },
    { -3, N_HOME,  { "hurts %s with divine intervention",
/* 44*/	  	     "frowns upon %s" } },
    { -4, N_ECON,  { "sacrifices to %s",
/* 45*/	  	     "makes divine payment to %s" } },
    { -10,N_MISS,  { "abms intercept a %s missile",
/* 46*/	  	     "strategic defense systems destroy a %s missile" } },
    { -2, N_MISS,  { "missile fired at one of %s's sectors",
/* 47*/	  	     "missile wreaks havoc on %s" } },
    { -2, N_MISS,  { "missile fired at a ship flying the flag of %s",
/* 48*/	  	     "missiles hit %s ships" } },
    {  2, N_ECON,  { "engages in friendly trade with %s",
/* 49*/	  	     "profits from a merchant venture with %s" } },
    {  0, N_ECON,  { "pirates dispose of %s booty in trade",
/* 50*/	  	     "buccaneers sell their %s goods" } },
    {  0, N_ECON,  { "pirates keep their %s booty",
/* 51*/	  	     "buccaneers laugh and bury their %s goods" } },
    {  0, N_SEA,   { "ships torpedoed by %s torpedo-boats",
/* 52*/	  	     "ships sunk by marauding %s torpedo-boats" } },
    {  0, N_SEA,   { "planes bomb a skulking %s submarine",
/* 53*/	  	     "planes drop depth-charges on a %s sub" } },
    { -2, N_SEA,    { "ship is blown up in a failed assault on %s",
/* 54*/	  	     "has a ship destroyed while assaulting %s" } },
    { -2, N_SKY,   { "divebombs a brave %s unit",
/* 55*/	  	     "airforce bombs %s units" } },
    { 0,  N_FRONT, { "troops run afoul of a landmine",
/* 56*/	  	     "troops severely damaged in mine field" } },
    { -2, N_ARTY,  { "fortress gunners support troops attacking %s",
/* 57*/	  	     "fortress artillery massacres %s defenders" } },
    { -2, N_ARTY,  { "gunners support troops attacking %s",
/* 58*/	  	     "artillery battery massacres %s defenders" } },
    { -2, N_ARTY,  { "naval gunners support troops attacking %s",
/* 59*/	  	     "naval gunfire massacres %s defenders" } },
    {-10, N_FOR,   { "sacks %s capital",
/* 60*/	  	     "captures and pillages %s's capital" } },
    { 3,  N_FOR,   { "announces friendly trade relations with %s",
/* 61*/	  	     "upgrades %s's trade status to triple-A" } },
    { 0,  N_FOR,   { "downgrades relations with %s to friendly",
/* 62*/	  	     "cools relations with %s to friendly" } },
    { 2,  N_FOR,   { "upgrades relations with %s to neutral",
/* 63*/	  	     "Foreign Ministry declares \"%s is A-OK.\"" } },
    { 0,  N_FOR,   { "downgrades relations with %s to neutral",
/* 64*/	  	     "gives the cold shoulder to %s and declares neutral relations" } },
    { 3,  N_FOR,   { "upgrades relations with %s to hostile",
/* 65*/	  	     "forgives %s of past war crimes but remains hostile" } },
    {-3,  N_FOR,   { "downgrades relations with %s to hostile",
/* 66*/	  	     "is suspicious that %s has hostile intentions" } },
    { 0,  N_MISS,  { "sector is struck by a sub-launched missile",
/* 67*/	  	     "sector devastated by missile" } },
    { 0,  N_MISS,  { "ship is hit by a sub-launched missile",
/* 68*/	  	     "ship damaged by marine missile" } },
    { 0,  N_COLONY,{ "pilgrims settle on a new island",
/* 69*/	  	     "discovers new land and starts a colony" } },
    { 0,  N_MISS,  { "abms intercept an incoming sub-launched missile",
/* 70*/	  	     "strategic defense systems destroy an incoming sub-launched warhead"}},
    {-2,  N_MISS,  { "missile fired at %s ground troops",
/* 71*/	  	     "missile hits %s troops" } },
    { 0,  N_MISS,  { "regiment struck by sub-launched missile",
/* 72*/	  	     "ground troops hit by missile" } },
    { -4, N_FRONT, { "navy secures a beachhead on %s territory",
/* 73*/	  	     "sailors take a coastal sector from %s" } },
    { -4, N_FRONT, { "paratroopers comandeer a sector from %s",
/* 74*/	  	     "air force parachutists overwhelm a %s sector" } },
    { 0,  N_COLONY,{ "parachutists grab unoccupied land",
/* 75*/	  	     "paratroopers break new ground" } },
    {-4,  N_FRONT, { "sailors repelled by %s coast-guard",
/* 76*/	  	     "naval forces massacred in failed assault of %s" } },
    {-4,  N_FRONT, { "paratroopers destroyed in failed air-assault on %s",
/* 77*/	  	     "loses parachutists in failed air-assault of %s" } },
    { 0,  N_ECON,  { "is forced into debt to %s by bad management",
/* 78*/	  	     "shows bad financial planning" } },
    { 0,  N_ECON,  { "has its credit rating downgraded to unworthy status",
/* 79*/	  	     "is denied credit by %s due to lack of collatoral" } },
    { 0,  N_ECON,  { "is unable to fulfill its financial obligations to %s",
/* 80*/	  	     "welches on a deal with %s" } },
    { -3, N_FRONT, { "is repelled by %s while attempting to board a land unit",
/* 81*/	  	     "forces prove inept at boarding %s's land unit" } },
    { -2, N_FRONT, { "mercenaries board one of %s's land units",
/* 82*/		     "elite forces board %s land unit" } }
};
