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
 *  product.c: Product information
 * 
 *  Known contributors to this file:
 *     
 */

#include <stddef.h>
#include "misc.h"
#include "product.h"
#include "sect.h"
#include "nat.h"


struct pchrstr pchr[] = {
/*       level      cost    nrndx nrdep nlndx   nlmin nllag effic  sname name */
    {{I_NONE, I_NONE, I_NONE}, {0, 0, 0},
     I_NONE, 0, 0, 0, 0, 0, 0, 0, 0,
     "unused", "",},
    {{I_LCM, I_HCM, I_NONE}, {2, 1, 0},
     I_SHELL, -1, 3, 0, 0, NAT_TLEV, 20, 10, 100,
     "shells", "shells",},
    {{I_OIL, I_LCM, I_HCM}, {1, 5, 10},
     I_GUN, -1, 30, 0, 0, NAT_TLEV, 20, 10, 100,
     "guns", "guns",},
    {{I_OIL, I_NONE, I_NONE}, {1, 0, 0},
     I_PETROL, -1, 1, 0, 0, NAT_TLEV, 20, 10, 1000,
     "petrol", "petrol",},
    {{I_NONE, I_NONE, I_NONE}, {0, 0, 0},
     I_IRON, -1, 0, offsetof(struct sctstr, sct_min), 0, -1, 0, 0, 100,
     "iron ore", "iron",},
    {{I_NONE, I_NONE, I_NONE}, {0, 0, 0},
     I_DUST, -1, 0, offsetof(struct sctstr, sct_gmin), 20, -1, 0, 0, 100,
     "gold dust", "dust",},
    {{I_DUST, I_NONE, I_NONE}, {5, 0, 0},
     I_BAR, -1, 10, 0, 0, -1, 0, 0, 100,
     "gold bars", "bars",},
    {{I_NONE, I_NONE, I_NONE}, {0, 0, 0},
     I_FOOD, -1, 0, offsetof(struct sctstr, sct_fertil), 0, NAT_TLEV, -10, 10, 900,
     "food", "food",},
    {{I_NONE, I_NONE, I_NONE}, {0, 0, 0},
     I_OIL, -1, 0, offsetof(struct sctstr, sct_oil), 10, NAT_TLEV, -10, 10, 100,
     "oil", "oil",},
    {{I_IRON, I_NONE, I_NONE}, {1, 0, 0},
     I_LCM, -1, 0, 0, 0, NAT_TLEV, -10, 10, 100,
     "light construction materials", "lcm",},
    {{I_IRON, I_NONE, I_NONE}, {2, 0, 0},
     I_HCM, -1, 0, 0, 0, NAT_TLEV, -10, 10, 100,
     "heavy construction materials", "hcm",},
    {{I_DUST, I_OIL, I_LCM}, {1, 5, 10},
     I_NONE, NAT_TLEV, 300, 0, 0, NAT_ELEV, 5, 10, 100,
     "technological breakthroughs", "tech",},
    {{I_DUST, I_OIL, I_LCM}, {1, 5, 10},
     I_NONE, NAT_RLEV, 90, 0, 0, NAT_ELEV, 5, 10, 100,
     "medical discoveries", "medical",},
    {{I_LCM, I_NONE, I_NONE}, {1, 0, 0},
     I_NONE, NAT_ELEV, 9, 0, 0, -1, 0, 0, 100,
     "a class of graduates", "edu",},
    {{I_LCM, I_NONE, I_NONE}, {1, 0, 0},
     I_NONE, NAT_HLEV, 9, 0, 0, -1, 0, 0, 100,
     "happy strollers", "happy",},
    {{I_NONE, I_NONE, I_NONE}, {0, 0, 0},
     I_RAD, -1, 2, offsetof(struct sctstr, sct_uran), 35, NAT_TLEV, 40, 10, 100,
     "radioactive materials", "rad",},
    {{I_NONE, I_NONE, I_NONE}, {0, 0, 0},
     I_DUST, -1, 0, offsetof(struct sctstr, sct_gmin), 20, -1, 0, 0, 75,
     "gold dust", "dust",},
    {{I_NONE, I_NONE, I_NONE}, {0, 0, 0},
     I_NONE, 0, 0, 0, 0, 0, 0, 0, 0, "", "",}
};

int prd_maxno = (sizeof(pchr) / sizeof(struct pchrstr)) - 1;
