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
 *  sect.c: Sector designation characteristics
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Jeff Bailey
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

/* order must agree with sect.h */

#include "misc.h"
#include "sect.h"
#include "product.h"

struct dchrstr bigcity_dchr = {
    'c', 0, 2, NAV_02, UPKG, 1.0, 2.0, 30, 0, 10, 1, 2, 999, "city"
};

struct dchrstr dchr[SCT_MAXDEF + 2] = {
/*
  mnem prd     mcst  flg    pkg ostr dstr value $ bld lcm hcm maxpop  name    */
    {'.', 0, 0, NAVOK, NPKG, 0.0, 0.0, 0, -1, 0, 0, 0, 0, "sea"},
    {'^', P_MDUST, 25, 0, NPKG, 1.0, 4.0, 5, -1, 1, 0, 0, 99, "mountain"},
    {'s', 0, 0, 0, NPKG, 0.0, 99.0, 127, -1, 0, 0, 0, 999, "sanctuary"},
    {'\\', 0, 0, 0, NPKG, 0.0, 99.0, 0, -1, 0, 0, 0, 0, "wasteland"},
    {'-', 0, 3, 0, NPKG, 1.0, 2.0, 1, 0, 0, 0, 0, 999, "wilderness"},
    {'c', 0, 2, 0, NPKG, 1.0, 2.0, 30, 0, 1, 0, 0, 999, "capital"},
    {'u', P_URAN, 2, 0, NPKG, 1.0, 2.0, 15, 0, 1, 0, 0, 999, "uranium mine"},
    {'p', P_HLEV, 2, 0, NPKG, 1.0, 1.5, 5, 0, 1, 0, 0, 999, "park"},
    {'d', P_GUN, 2, 0, NPKG, 1.0, 1.5, 7, 0, 1, 0, 0, 999, "defense plant"},
    {'i', P_SHELL, 2, 0, NPKG, 1.0, 1.5, 6, 0, 1, 0, 0, 999, "shell industry"},
    {'m', P_IRON, 2, 0, NPKG, 1.0, 2.0, 5, 0, 1, 0, 0, 999, "mine"},
    {'g', P_DUST, 2, 0, NPKG, 1.0, 2.0, 8, 0, 1, 0, 0, 999, "gold mine"},
    {'h', 0, 2, NAV_02, WPKG, 1.0, 1.5, 12, 0, 1, 0, 0, 999, "harbor"},
    {'w', 0, 2, 0, WPKG, 1.0, 1.5, 7, 0, 1, 0, 0, 999, "warehouse"},
    {'*', 0, 2, 0, NPKG, 1.0, 1.25, 12, 0, 1, 0, 0, 999, "airfield"},
    {'a', P_FOOD, 2, 0, NPKG, 1.0, 1.5, 2, 0, 1, 0, 0, 999, "agribusiness"},
    {'o', P_OIL, 2, 0, NPKG, 1.0, 1.5, 5, 0, 1, 0, 0, 999, "oil field"},
    {'j', P_LCM, 2, 0, NPKG, 1.0, 1.5, 3, 0, 1, 0, 0, 999, "light manufacturing"},
    {'k', P_HCM, 2, 0, NPKG, 1.0, 1.5, 4, 0, 1, 0, 0, 999, "heavy manufacturing"},
    {'f', 0, 2, 0, NPKG, 2.0, 4.0, 10, 0, 5, 0, 1, 999, "fortress"},
    {'t', P_TLEV, 2, 0, NPKG, 1.0, 1.5, 10, 0, 1, 0, 0, 999, "technical center"},
    {'r', P_RLEV, 2, 0, NPKG, 1.0, 1.5, 9, 0, 1, 0, 0, 999, "research lab"},
    {'n', 0, 2, 0, NPKG, 1.0, 2.0, 10, 0, 1, 0, 0, 999, "nuclear plant"},
    {'l', P_ELEV, 2, 0, NPKG, 1.0, 1.5, 4, 0, 1, 0, 0, 999, "library/school"},
    {'+', 0, 1, 0, NPKG, 1.0, 1.0, 3, 0, 1, 0, 0, 999, "highway"},
    {')', 0, 2, 0, NPKG, 1.0, 1.5, 4, 0, 1, 0, 0, 999, "radar installation"},
    {'!', 0, 2, 0, NPKG, 1.0, 1.5, 12, 0, 1, 0, 0, 999, "headquarters"},
    {'#', 0, 1, 0, NPKG, 1.0, 1.5, 4, 0, 1, 0, 0, 999, "bridge head"},
    {'=', 0, 1, NAV_60, NPKG, 1.0, 1.0, 5, -1, 1, 0, 0, 999, "bridge span"},
    {'b', P_BAR, 2, 0, BPKG, 1.0, 2.25, 10, 0, 1, 0, 0, 999, "bank"},
    {'%', P_PETROL, 2, 0, NPKG, 1.0, 1.5, 2, 0, 1, 0, 0, 999, "refinery"},
    {'e', 0, 2, 0, NPKG, 1.0, 2.0, 7, 0, 1, 0, 0, 999, "enlistment center"},
    {'~', 0, 2, 0, NPKG, 1.0, 1.5, 1, -1, 1, 0, 0, 49, "plains"},
    {'@', 0, 1, 0, NPKG, 1.0, 1.5, 4, -1, 1, 0, 0, 999, "bridge tower"},
    {0, 0, 0, 0, IPKG, 0, 0, 0, 0, 0, 0, 0, 0, NULL},
    {0, 0, 0, 0, IPKG, 0, 0, 0, 0, 0, 0, 0, 0, NULL}
};

struct sctintrins intrchr[] = {
/* name              lcm hcm dollars mobility */
    {"road network", 2, 2, 2, 1},
    {"rail network", 1, 1, 1, 1},
    {"defense factor", 1, 1, 1, 1},
    {0, 0, 0, 0, 0}
};
