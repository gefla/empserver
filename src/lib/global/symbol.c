/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  symbol.c: Empire symbol tables
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2008
 */

#include <config.h>

#include "land.h"
#include "misc.h"
#include "mission.h"
#include "nat.h"
#include "news.h"
#include "nsc.h"
#include "nuke.h"
#include "plague.h"
#include "plane.h"
#include "retreat.h"
#include "sect.h"
#include "ship.h"
#include "treaty.h"

struct symbol agreement_statuses[] = {
    {AGREE_FREE, "free"},
    {AGREE_PROPOSED, "proposed"},
    {AGREE_SIGNED, "signed"},
    {0, NULL}
};

struct symbol land_chr_flags[] = {
    {L_ENGINEER, "engineer"},
    {L_SUPPLY, "supply"},
    {L_SECURITY, "security"},
    {L_LIGHT, "light"},
    {L_MARINE, "marine"},
    {L_RECON, "recon"},
    {L_RADAR, "radar"},
    {L_ASSAULT, "assault"},
    {L_FLAK, "flak"},
    {L_SPY, "spy"},
    {L_TRAIN, "train"},
    {L_HEAVY, "heavy"},
    {0, NULL}
};

struct symbol level[] = {
    {-1, "none"},
    {NAT_TLEV, "technology"},
    {NAT_RLEV, "research"},
    {NAT_ELEV, "education"},
    {NAT_HLEV, "happiness"},
    {0, NULL}
};

struct symbol meta_flags[] = {
    {NSC_DEITY, "deity"},
    {NSC_EXTRA, "extra"},
    {NSC_CONST, "const"},
    {NSC_BITS, "bits"},
    {0, NULL}
};

struct symbol meta_type[]  = {
    {NSC_LONG, "d"},
    {NSC_DOUBLE, "g"},
    {NSC_STRING, "s"},
    {NSC_CHAR, "d"},
    {NSC_UCHAR, "d"},
    {NSC_SHORT, "d"},
    {NSC_USHORT, "d"},
    {NSC_INT, "d"},
    {NSC_XCOORD, "d"},
    {NSC_YCOORD, "d"},
    {NSC_HIDDEN, "d"},
    {NSC_TIME, "d"},
    {NSC_FLOAT, "g"},
    {NSC_STRINGY,"c"},
    {NSC_NOTYPE, NULL}
};

struct symbol missions[] = {
    {MI_NONE, "none"},
    {MI_INTERDICT, "interdiction"},
    {MI_SUPPORT, "support"},
    {MI_RESERVE, "reserve"},
    {MI_ESCORT, "escort"},
    {MI_AIR_DEFENSE, "air defense"},
    {MI_DSUPPORT, "defensive support"},
    {MI_OSUPPORT, "offensive support"},
    {0, NULL}
};

struct symbol nation_flags[] = {
    {NF_INFORM, "inform"},
    {NF_FLASH, "flash"},
    {NF_BEEP, "beep"},
    {NF_COASTWATCH, "coastwatch"},
    {NF_SONAR, "sonar"},
    {NF_TECHLISTS, "techlists"},
    {NF_SACKED, "sacked"},
    {0, NULL}
};

struct symbol nation_rejects[] = {
    {REJ_TELE, "telegrams"},
    {REJ_TREA, "treaties"},
    {REJ_ANNO, "announcements"},
    {REJ_LOAN, "loans"},
    {0, NULL}
};

struct symbol nation_relations[] = {
    {-1, "unknown"},
    {AT_WAR, "at-war"},
    {SITZKRIEG, "sitzkrieg"},
    {MOBILIZATION, "mobilization"},
    {HOSTILE, "hostile"},
    {NEUTRAL, "neutral"},
    {FRIENDLY, "friendly"},
    {ALLIED, "allied"},
    {0, NULL}
};

struct symbol nation_status[] = {
    {STAT_UNUSED, "unused"},
    {STAT_NEW,	"new"},
    {STAT_VIS, "visitor"},
    {STAT_SANCT, "sanctuary"},
    {STAT_ACTIVE, "active"},
    {STAT_GOD, "deity"},
    {0, NULL}
};

struct symbol nuke_chr_flags[] = {
    {N_NEUT, "neutron"},
    {0, NULL}
};

struct symbol packing[] = {
    {IPKG, "inefficient"},
    {NPKG, "normal"},
    {WPKG, "warehouse"},
    {UPKG, "urban"},
    {BPKG, "bank"},
    {0, NULL}
};

struct symbol page_headings[] = {
    {N_NOTUSED, "Comics"},
    {N_FOR, "Foreign Affairs"},
    {N_FRONT, "The Front Line"},
    {N_SEA, "The High Seas"},
    {N_SKY, "Sky Watch"},
    {N_MISS, "Guidance Systems"},
    {N_ARTY, "Firestorms"},
    {N_ECON, "Business & Economics"},
    {N_COLONY, "The Frontier"},
    {N_HOME, "The Home Front"},
    {N_SPY, "Espionage"},
    {N_TELE, "Telecommunications"},
    {0, NULL}
};

struct symbol plague_stages[] = {
    {PLG_HEALTHY, "healthy"},
    {PLG_DYING, "dying"},
    {PLG_INFECT, "infect"},
    {PLG_INCUBATE, "incubate"},
    {PLG_EXPOSED, "exposed"},
    {0, NULL}
};

struct symbol plane_chr_flags[] = {
    {P_T, "tactical"},
    {P_B, "bomber"},
    {P_F, "intercept"},
    {P_C, "cargo"},
    {P_V, "VTOL"},
    {P_M, "missile"},
    {P_L, "light"},
    {P_S, "spy"},
    {P_I, "image"},
    {P_O, "satellite"},
    {P_N, "SDI"},
    {P_E, "x-light"},
    {P_K, "helo"},
    {P_A, "ASW"},
    {P_P, "para"},
    {P_ESC, "escort"},
    {P_MINE, "mine"},
    {P_SWEEP, "sweep"},
    {P_MAR, "marine"},
    {0, NULL}
};

struct symbol plane_flags[] = {
    {PLN_LAUNCHED, "launched"},
    {PLN_SYNCHRONOUS, "synchronous"},
    {PLN_AIRBURST, "airburst"},
    {0, NULL}
};

struct symbol resources[] = {
    /* names should match resource selector names in sect_ca[] */
    {0, "none"},
    {offsetof(struct sctstr, sct_min), "min"},
    {offsetof(struct sctstr, sct_gmin), "gold"},
    {offsetof(struct sctstr, sct_fertil), "fert"},
    {offsetof(struct sctstr, sct_oil), "ocontent"},
    {offsetof(struct sctstr, sct_uran), "uran"},
    {0, NULL}
};

struct symbol retreat_flags[] = {
    {RET_GROUP, "group"},
    {RET_INJURED, "injured"},
    {RET_TORPED, "torped"},
    {RET_SONARED, "sonared"},
    {RET_HELPLESS, "helpless"},
    {RET_BOMBED, "bombed"},
    {RET_DCHRGED, "depth-charged"},
    {RET_BOARDED, "boarded"},
    {0, NULL}
};

struct symbol sector_navigation[] = {
    {NAV_NONE, "land"},
    {NAVOK, "sea"},
    {NAV_02, "harbor"},
    {NAV_CANAL, "canal"},
    {NAV_60, "bridge"},
    {0, NULL}
};

struct symbol ship_chr_flags[] = {
    {M_FOOD, "fish"},
    {M_TORP, "torp"},
    {M_DCH, "dchrg"},
    {M_FLY, "plane"},
    {M_MSL, "miss"},
    {M_OIL, "oil"},
    {M_SONAR, "sonar"},
    {M_MINE, "mine"},
    {M_SWEEP, "sweep"},
    {M_SUB, "sub"},
    {M_LAND, "land"},
    {M_SUBT, "sub-torp"},
    {M_TRADE, "trade"},
    {M_SEMILAND, "semi-land"},
    {M_SUPPLY, "supply"},
    {M_CANAL, "canal"},
    {M_ANTIMISSILE, "anti-missile"},
    {0, NULL}
};

struct symbol treaty_flags[] = {
    {LNDATT, "no attacks on any land units"},
    {SEAATT, "no attacks on any ships"},
    {SEAFIR, "no shelling any ships"},
    {SUBFIR, "no depth-charging any subs"},
    {LANATT, "no sector attacks"},
    {LANFIR, "no shelling any land"},
    {NEWSHP, "no building ships"},
    {NEWNUK, "no new nuclear weapons"},
    {NEWPLN, "no building planes"},
    {NEWLND, "no building land units"},
    {TRTENL, "no enlistment"},
    {0, NULL}
};
