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
 *  nuke.c: Nuke characteristics
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "nuke.h"


struct nchrstr nchr[N_MAXNUKE] = {
    /* name         lcm  hcm  oil  rads blst dam    cost  tech lbs flags */
    {"10kt  fission", 50, 50, 25, 70, 3, 70, 10000, 280, 4, 0},
    {"15kt  fission", 50, 50, 25, 80, 3, 90, 15000, 290, 5, 0},
    {"50kt  fission", 60, 60, 30, 90, 3, 100, 25000, 300, 6, 0},
    {"100kt fission", 75, 75, 40, 120, 4, 125, 30000, 310, 8, 0},
    {"5kt   fusion", 15, 15, 15, 30, 2, 80, 12500, 315, 1, 0},
    {"75kt  fusion", 40, 40, 35, 50, 3, 90, 20000, 320, 3, 0},
    {"250kt fusion", 50, 50, 45, 60, 4, 110, 25000, 330, 4, 0},
    {"500kt fusion", 60, 60, 50, 80, 5, 120, 35000, 340, 5, 0},
    {"1mt   fusion", 75, 75, 50, 110, 6, 150, 40000, 350, 5, 0},
    {"60kt  neutron", 60, 60, 30, 100, 3, 30, 30000, 355, 2, N_NEUT},
    {"3mt   fusion", 100, 100, 75, 130, 7, 170, 45000, 360, 6, 0},
    {"5mt   fusion", 120, 120, 100, 150, 8, 190, 50000, 370, 8, 0},
    {"120kt neutron", 75, 75, 40, 120, 5, 50, 36000, 375, 3, N_NEUT},
    {"", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {"", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int nuk_maxno = (sizeof(nchr) / sizeof(struct nchrstr)) - 1;
