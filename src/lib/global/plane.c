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
 *  plane.c: Plane characteristics
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Jeff Bailey
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

/*
 * plane characteristics
 * flags indicate capabilities of craft...
 * P_T: tactical, P_F: fighter/interceptor, P_B: bomber,
 * P_C: cargo, P_G: glider, P_V: vtol, P_M: (one-shot) missile
 * P_L: light, can land on carriers.
 * P_O: orbits (sat's & anti-sats), P_N: nuke killer (SDI)
 * P_X: stealthy
 * P_E: Extra light
 * P_K: Chopper
 * P_P: Can paratroop
 * P_A: ASW plane, can find subs when reconning, can bomb subs
 * P_R: recon plane, gets some info
 * P_I: Image capability (advanced spying)
 * 
 * Note if a plane is P_M (a missile) then it will automatically be made
 * P_V (vtol) in init_global()
 */

#include "misc.h"
#include "plane.h"

struct plchrstr plchr[] = {

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "f1   fighter 1",
     "f1   Sopwith Camel",
     8, 2, 400, 50, 90, 1, 1, 1, 4, 1, 1,
     0,
     P_V | P_T | P_F},

/* name        lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "f2   fighter 2",
     "f2   P-51 Mustang",
     8, 2, 400, 80, 80, 1, 4, 4, 8, 1, 1,
     0,
     P_L | P_T | P_F},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "jf1  jet fighter 1",
     "jf1  F-4 Phantom",
     12, 4, 1000, 125, 45, 1, 14, 14, 11, 2, 3,
     0,
     P_T | P_F | P_L},


/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "jf2  jet fighter 2",
     "jf2  AV-8B Harrier",
     12, 4, 1400, 195, 30, 1, 17, 17, 14, 2, 3,
     0,
     P_T | P_F | P_L | P_V},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "sf   stealth fighter",
     "sf   F-117A Nighthawk",
     15, 5, 3000, 325, 45, 3, 19, 19, 20, 2, 4,
     80,
     P_T | P_F | P_L},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "es   escort",
     "es   P-38 Lightning",
     9, 3, 700, 90, 60, 1, 5, 5, 15, 1, 2,
     0,
     P_ESC | P_T},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "jes  jet escort",
     "jes  F-14E jet escort",
     14, 8, 1400, 160, 60, 1, 10, 10, 25, 2, 3,
     0,
     P_ESC | P_T},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "lb   lt bomber",
     "lb   TBD-1 Devastator",
     10, 3, 550, 60, 50, 2, 0, 3, 7, 1, 1,
     0,
     P_L | P_V | P_T | P_B},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "jl   jet lt bomber",
     "jl   A-6 Intruder",
     14, 4, 1000, 130, 25, 3, 0, 9, 11, 2, 3,
     0,
     P_T | P_B | P_L},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "mb   medium bomber",
     "mb   medium bomber",
     14, 5, 1000, 80, 45, 4, 0, 5, 14, 3, 3,
     0,
     P_T | P_B},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "jmb  jet med bomber",
     "jfb  FB-111 Aardvark f/b",
     20, 10, 1800, 140, 30, 7, 8, 8, 20, 5, 5,
     0,
     P_T | P_B},
    {
     "hb   hvy bomber",
     "hb   B-26B Marauder",
     20, 6, 1100, 90, 90, 5, 0, 4, 15, 2, 2,
     0,
     P_B},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "jhb  jet hvy bomber",
     "jhb  B-52 Strato-Fortress",
     26, 13, 3200, 150, 80, 12, 0, 11, 35, 5, 6,
     0,
     P_B},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "sb   stealth bomber",
     "sb   B-2 stealth bomber",
     15, 5, 4000, 325, 25, 8, 0, 15, 28, 2, 5,
     80,
     P_T | P_B},


/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "as   anti-sub plane",
     "as   anti-sub plane",
     10, 3, 550, 100, 85, 2, 0, 3, 15, 2, 2,
     0,
     P_SWEEP | P_MINE | P_T | P_A},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "np   naval plane",
     "np   naval plane",
     20, 10, 1800, 135, 70, 3, 0, 4, 28, 4, 2,
     0,
     P_SWEEP | P_MINE | P_C | P_T | P_A | P_L},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "nc   naval chopper",
     "nc   AH-1 Cobra",
     8, 2, 800, 160, 55, 2, 0, 3, 11, 2, 2,
     0,
     P_V | P_SWEEP | P_T | P_A | P_K},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "ac   attack chopper",
     "ac   AH-64 Apache",
     8, 2, 800, 200, 15, 1, 0, 9, 11, 2, 2,
     40,
     P_V | P_T | P_K},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "tc   transport chopper",
     "tc   transport chopper",
     8, 2, 800, 135, 0, 5, 0, 3, 7, 2, 2,
     40,
     P_V | P_C | P_P | P_L | P_K},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "tr   transport",
     "tr   C-56 Lodestar",
     14, 5, 1000, 85, 0, 7, 0, 2, 15, 3, 3,
     0,
     P_C | P_P},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "jt   jet transport",
     "jt   C-141 Starlifter",
     18, 5, 1500, 160, 0, 16, 0, 9, 35, 3, 4,
     0,
     P_C | P_P},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "zep  Zeppelin",
     "zep  Zeppelin",
     6, 2, 1000, 70, 60, 2, 0, -3, 15, 3, 2,
     0,
     P_S | P_V | P_T | P_C},
/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "re   recon",
     "re   recon",
     12, 4, 800, 130, 0, 0, 0, 4, 15, 2, 2,
     20,
     P_S},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "sp   spyplane",
     "sp   E2-C Hawkeye",
     15, 5, 2000, 190, 0, 0, 0, 11, 32, 2, 5,
     50,
     P_S},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "lst  landsat",
     "lst  landsat",
     20, 20, 2000, 245, 0, 0, 0, 3, 41, 0, 9,
     0,
     P_O},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "ss   spysat",
     "ss   KH-7 spysat",
     20, 20, 4000, 305, 0, 0, 0, 3, 61, 0, 9,
     0,
     P_I | P_O | P_S},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "mi   naval missile",
     "mi   Harpoon",
     8, 2, 300, 160, 50, 6, 0, 5, 6, 0, 0,
     0,
     P_L | P_T | P_M | P_MAR},
/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "sam  sam",
     "sam  Sea Sparrow",
     3, 1, 200, 180, 0, 0, 0, 18, 2, 0, 0,
     0,
     P_L | P_F | P_E | P_M},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "ssm",
     "ssm  V2",
     15, 15, 800, 145, 60, 3, 0, 3, 4, 0, 0,
     0,
     P_T | P_M},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "srbm",
     "srbm Atlas",
     20, 20, 1000, 200, 60, 6, 0, 5, 9, 0, 0,
     0,
     P_T | P_M},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "irbm",
     "irbm Titan",
     20, 20, 1500, 260, 60, 8, 0, 10, 15, 0, 0,
     0,
     P_T | P_M},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "icbm",
     "icbm Minuteman",
     20, 20, 3000, 310, 60, 10, 0, 15, 41, 0, 0,
     0,
     P_T | P_M},
/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "slbm",
     "slbm Trident",
     20, 20, 2000, 280, 60, 8, 0, 6, 23, 0, 0,
     0,
     P_T | P_L | P_M},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "asat anti-sat",
     "asat anti-sat",
     20, 20, 2000, 305, 50, 6, 0, 7, 13, 0, 4,
     0,
     P_O | P_M},

/* name          lcm hcm   $$$  tch acc ld  at/df  ran mil gas stlth  flags */
    {
     "abm  anti-ballistic missile",
     "abm  Patriot",
     16, 8, 1500, 270, 50, 0, 0, 31, 12, 0, 0,
     0,
     P_N | P_M},
    {
     "", "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,
     0}
};

int pln_maxno = (sizeof(plchr) / sizeof(struct plchrstr)) - 1;
