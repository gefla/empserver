/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#include <config.h>

#include "misc.h"
#include "sect.h"
#include "product.h"

struct dchrstr bigcity_dchr = {
    SCT_CAPIT,   'c',       0, 2,  NAV_02,   UPKG, 1.0, 2.0,   30, 0, 10, 1, 2, 999, "city"
};

struct dchrstr dchr[] = {
    /*          mnem prd     mcst  flg       pkg  ostr dstr value  $ bld lcm hcm maxpop name */
    {SCT_WATER,  '.',       0, 0,  NAVOK,    NPKG, 0.0,  0.0,   0, -1, 0, 0, 0,   0, "sea"},
    {SCT_MOUNT,  '^', P_MDUST, 25, NAV_NONE, NPKG, 1.0,  4.0,   5, -1, 1, 0, 0,  99, "mountain"},
    {SCT_SANCT,  's',       0, 0,  NAV_NONE, NPKG, 0.0, 99.0, 127, -1, 0, 0, 0, 999, "sanctuary"},
    {SCT_WASTE,  '\\',      0, 0,  NAV_NONE, NPKG, 0.0, 99.0,   0, -1, 0, 0, 0,   0, "wasteland"},
    {SCT_RURAL,  '-',       0, 3,  NAV_NONE, NPKG, 1.0,  2.0,   1,  0, 0, 0, 0, 999, "wilderness"},
    {SCT_CAPIT,  'c',       0, 2,  NAV_NONE, NPKG, 1.0,  2.0,  30,  0, 1, 0, 0, 999, "capital"},
    {SCT_URAN,   'u', P_URAN,  2,  NAV_NONE, NPKG, 1.0,  2.0,  15,  0, 1, 0, 0, 999, "uranium mine"},
    {SCT_PARK,   'p', P_HLEV,  2,  NAV_NONE, NPKG, 1.0,  1.5,   5,  0, 1, 0, 0, 999, "park"},
    {SCT_ARMSF,  'd', P_GUN,   2,  NAV_NONE, NPKG, 1.0,  1.5,   7,  0, 1, 0, 0, 999, "defense plant"},
    {SCT_AMMOF,  'i', P_SHELL, 2,  NAV_NONE, NPKG, 1.0,  1.5,   6,  0, 1, 0, 0, 999, "shell industry"},
    {SCT_MINE,   'm', P_IRON,  2,  NAV_NONE, NPKG, 1.0,  2.0,   5,  0, 1, 0, 0, 999, "mine"},
    {SCT_GMINE,  'g', P_DUST,  2,  NAV_NONE, NPKG, 1.0,  2.0,   8,  0, 1, 0, 0, 999, "gold mine"},
    {SCT_HARBR,  'h',       0, 2,  NAV_02,   WPKG, 1.0,  1.5,  12,  0, 1, 0, 0, 999, "harbor"},
    {SCT_WAREH,  'w', 	    0, 2,  NAV_NONE, WPKG, 1.0,  1.5,   7,  0, 1, 0, 0, 999, "warehouse"},
    {SCT_AIRPT,  '*', 	    0, 2,  NAV_NONE, NPKG, 1.0,  1.25, 12,  0, 1, 0, 0, 999, "airfield"},
    {SCT_AGRI,   'a', P_FOOD,  2,  NAV_NONE, NPKG, 1.0,  1.5,   2,  0, 1, 0, 0, 999, "agribusiness"},
    {SCT_OIL,    'o', P_OIL,   2,  NAV_NONE, NPKG, 1.0,  1.5,   5,  0, 1, 0, 0, 999, "oil field"},
    {SCT_LIGHT,  'j', P_LCM,   2,  NAV_NONE, NPKG, 1.0,  1.5,   3,  0, 1, 0, 0, 999, "light manufacturing"},
    {SCT_HEAVY,  'k', P_HCM,   2,  NAV_NONE, NPKG, 1.0,  1.5,   4,  0, 1, 0, 0, 999, "heavy manufacturing"},
    {SCT_FORTR,  'f',       0, 2,  NAV_NONE, NPKG, 2.0,  4.0,  10,  0, 5, 0, 1, 999, "fortress"},
    {SCT_TECH,   't', P_TLEV,  2,  NAV_NONE, NPKG, 1.0,  1.5,  10,  0, 1, 0, 0, 999, "technical center"},
    {SCT_RSRCH,  'r', P_RLEV,  2,  NAV_NONE, NPKG, 1.0,  1.5,   9,  0, 1, 0, 0, 999, "research lab"},
    {SCT_NUKE,   'n',       0, 2,  NAV_NONE, NPKG, 1.0,  2.0,  10,  0, 1, 0, 0, 999, "nuclear plant"},
    {SCT_LIBR,   'l', P_ELEV,  2,  NAV_NONE, NPKG, 1.0,  1.5,   4,  0, 1, 0, 0, 999, "library/school"},
    {SCT_HIWAY,  '+', 	    0, 1,  NAV_NONE, NPKG, 1.0,  1.0,   3,  0, 1, 0, 0, 999, "highway"},
    {SCT_RADAR,  ')', 	    0, 2,  NAV_NONE, NPKG, 1.0,  1.5,   4,  0, 1, 0, 0, 999, "radar installation"},
    {SCT_HEADQ,  '!', 	    0, 2,  NAV_NONE, NPKG, 1.0,  1.5,  12,  0, 1, 0, 0, 999, "headquarters"},
    {SCT_BHEAD,  '#', 	    0, 1,  NAV_NONE, NPKG, 1.0,  1.5,   4,  0, 1, 0, 0, 999, "bridge head"},
    {SCT_BSPAN,  '=', 	    0, 1,  NAV_60,   NPKG, 1.0,  1.0,   5, -1, 1, 0, 0, 999, "bridge span"},
    {SCT_BANK,   'b', P_BAR,   2,  NAV_NONE, BPKG, 1.0,  2.25, 10,  0, 1, 0, 0, 999, "bank"},
    {SCT_REFINE, '%', P_PETROL, 2, NAV_NONE, NPKG, 1.0,  1.5,   2,  0, 1, 0, 0, 999, "refinery"},
    {SCT_ENLIST, 'e', 	    0, 2,  NAV_NONE, NPKG, 1.0,  2.0,   7,  0, 1, 0, 0, 999, "enlistment center"},
    {SCT_PLAINS, '~', 	    0, 2,  NAV_NONE, NPKG, 1.0,  1.5,   1, -1, 1, 0, 0,  49, "plains"},
    {SCT_BTOWER, '@', 	    0, 1,  NAV_NONE, NPKG, 1.0,  1.5,   4, -1, 1, 0, 0, 999, "bridge tower"},
    {0, 0, 0, 0, NAV_NONE, IPKG, 0, 0, 0, 0, 0, 0, 0, 0, NULL},
};

struct sctintrins intrchr[] = {
/* name              lcm hcm dollars mobility */
    {"road network", 2, 2, 2, 1},
    {"rail network", 1, 1, 1, 1},
    {"defense factor", 1, 1, 1, 1},
    {0, 0, 0, 0, 0}
};
