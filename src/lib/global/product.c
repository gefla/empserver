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
 *  product.c: Product information
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "product.h"
#include "sect.h"
#include "nat.h"
#include "var.h"


struct pchrstr pchr[]  = {
/*       level      cost    nrndx nrdep nlndx   nlmin nllag effic  sname name */
{ 0, { 0 }, { 0 },
0,       0,         0,      0,    0,    0,        0,    0,  0,    
	"unused", "", },
{ 2, { V_LCM, V_HCM}, { 2, 1 },
V_SHELL, -1,        3,      0,    0,    NAT_TLEV, 20,  10, 100, 
	"shells",  "shells", },
{ 3, { V_OIL, V_LCM, V_HCM }, { 1, 5, 10 },
V_GUN,   -1,        30,     0,    0,    NAT_TLEV, 20,  10, 100, 
	"guns",  "guns",},
{ 1, { V_OIL }, { 1 },
V_PETROL,-1,        1,      0,    0,    NAT_TLEV, 20,  10, 1000,
	"petrol",  "petrol",},
{ 0, { 0 }, { 0 },
V_IRON,  -1,        0,OFFSET(sctstr, sct_min),0,    -1,       0,    0,  100,
	"iron ore",  "iron",	},
{ 0, { 0 }, { 0 },
V_DUST,  -1,        0,OFFSET(sctstr, sct_gmin),20,  -1,       0,    0,  100,
	"gold dust",  "dust",},
{ 1, { V_DUST }, { 5 },
V_BAR,   -1,        10,     0,    0,    -1,       0,    0,  100, 
	"gold bars", "bars",},
{ 0, { 0 }, { 0 },
V_FOOD,  -1,        0,OFFSET(sctstr, sct_fertil),0, NAT_TLEV, -10,  10, 900,
	"food", "food",},
{ 0, { 0 }, { 0 },
V_OIL,   -1,        0,OFFSET(sctstr, sct_oil),10,   NAT_TLEV, -10,  10, 100, 
	"oil", "oil",},
{ 1, { V_IRON }, { 1 },
V_LCM,   -1,        0,      0,    0,    NAT_TLEV, -10,  10, 100,
	"light construction materials", "lcm",},
{ 1, { V_IRON }, { 2 },
V_HCM,   -1,        0,      0,    0,    NAT_TLEV, -10,  10, 100,
	"heavy construction materials",  "hcm", },
{3,{V_DUST,V_OIL,V_LCM}, {1,5,10},
0,       NAT_TLEV, 5*ETUS,  0,    0,    NAT_ELEV, 5,    10, 100,
      "technological breakthroughs", "tech",},
{3,{V_DUST,V_OIL,V_LCM }, { 1, 5, 10 },
0,       NAT_RLEV, 90,     0,    0,    NAT_ELEV, 5,    10, 100,
      "medical discoveries", "medical",},
{ 1, { V_LCM }, { 1 },
0,       NAT_ELEV,  9,     0,    0,    -1,       0,    0,  100,
	"a class of graduates",  "edu",},
{ 1, { V_LCM }, { 1 },
0,       NAT_HLEV,  9,     0,    0,    -1,       0,    0,  100,
	"happy strollers",  "happy",},
{ 0, { 0 }, { 0 },
V_RAD, -1,          2,OFFSET(sctstr, sct_uran),35, NAT_TLEV, 40,  10, 100, 
	"radioactive materials",  "rad",},
{ 0, { 0 }, { 0 },
V_DUST,  -1,        0,OFFSET(sctstr, sct_gmin),20,  -1,       0,    0,  75,
	"gold dust",  "dust",},
{ 0, {0}, {0},
0, 0, 0, 0, 0, 0, 0, 0, 0, "", "",}
};

int	prd_maxno   = (sizeof(pchr) / sizeof(struct pchrstr)) - 1;
