/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  options.h: Define which "optional" features should be enabled.
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Jeff Bailey
 *     Chad Zabel
 *     Thomas Rushack, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "gamesdef.h"

/* #define NO_LCMS *//* No lcms needed */
/* #define NO_HCMS *//* No hcms needed */

/* #define NO_FORT_FIRE *//* Forts cannot fire */

#define BRIDGETOWERS		/* Bridge towers are enabled */

#define GODNEWS			/* We inform the world when deities give/take away */

#define LANDSPIES		/* Enable the land unit type spies */

#define START_CASH 25000	/* set starting cash when capitol created */

			/* Setting BLITZ sets a couple of things.
			   1.  BTU's always max when you login. 
			   2.  Changing name doesn't take any btus or $$
			   enables BLITZ updates, also turns on NOFOOD, below */

/*#define START_UNITS	2 *//* How many free start units you get. Types are
   specified in src/lib/global/constants.c */

/* #define GO_RENEW *//* Gold and Oil are renewable resources */
/* #define GUINEA_PIGS *//* Experimental stuff not ready for prime time */
/* #define DEFENSE_INFRA *//* Allow the improvement of defensive infrastructure */

/*#define MOB_ACCESS *//* Mobility updates real-time */
/*#define TECH_POP *//* Technology costs more as population rises */

/*#define MARKET *//* Time-based MARKET and TRADING */
#define LOANS			/* Bail out other countries via S&L scandals */
#define TREATIES		/* Sign treaties with your friends and enemies */

/*#define HIDDEN*//* Hides information between players */
/*#define LOSE_CONTACT*//* Allows contact to be lost after a few updates */

/* Chainsaw Mods */
/*#define NONUKES*//* Eliminates nukes */
#ifndef NO_OIL			/* Can't have fuel with no oil! */
/*#define FUEL*//* Ships use fuel to move */
#endif
/*#define TRADESHIPS*//* Use Tradeships */
/*#define SLOW_WAR*//* Declaring war takes time */
/*#define SNEAK_ATTACK*//* Allows sneak attacks */
#define	ORBIT			/* Satalites will orbit each update */
#define	PINPOINTMISSILE /**/
#define	FALLOUT			/* Enables secondary effects caused by radiation */
#define	SAIL			/* A update routine to move ships */
#define	SHIPNAMES		/* Name your ships, removing this option saves space */
#define	NEUTRON			/* Enables Neurton Warheads */
#define NOMOBCOST		/* No mob cost for firing from ships */
/*#define SUPER_BARS *//* Bars can't be destroyed by fire */
#define EASY_BRIDGES		/* Bridges can be built anywhere */
#define ALL_BLEED		/* Tech bleeds to everyone */
/*#define DRNUKE*//* Need research to make nukes */
#define NO_PLAGUE		/* Plague is disabled */
/*#define ROLLOVER_AVAIL*/ 	/* Avail builds up across updates like mob */
/*#define RES_POP*//* population is limited by research */
/*#define BIG_CITY *//* allow 10x civs in 'c' sectors */
#define INTERDICT_ATT		/* interdict post-attack move in */
#define	SHOWPLANE /**/
#define	NUKEFAILDETONATE /**/
#define	MISSINGMISSILES		/* Missiles have a change to miss the target */
#define	UPDATESCHED		/* Used to controle update times and should always */
#ifdef UPDATESCHED		/* be used */
#define	DEMANDUPDATE		/* NOTE! Depends on UPDATESCHED! Don't use without it! */
#endif /* UPDATESCHED */
#ifdef BLITZ
#define NOFOOD			/* automatically have no food for blitzes */
#endif /* BLITZ */
/*#define	NOFOOD*/ /**/
#ifndef SLOW_WAR
#undef SNEAK_ATTACK		/* SNEAK_ATTACK is only useful with SLOW_WAR */
#endif /* SLOW_WAR */
#endif /* _OPTIONS_H_ */
