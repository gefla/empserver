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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  nsc.c: Empire selection global structures
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004
 *  
 */

#include <config.h>

#include <stddef.h>
#include "misc.h"
#include "xy.h"
#include "loan.h"
#include "nsc.h"
#include "news.h"
#include "nuke.h"
#include "plane.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "trade.h"
#include "treaty.h"
#include "genitem.h"
#include "nat.h"
#include "map.h"
#include "commodity.h"
#include "lost.h"
#include "product.h"
#include "file.h"
#include "mission.h"
#include "plague.h"

#define fldoff(str, fld) offsetof(struct str, fld)

#define NSC_IELT(name, pfx, sfx, base, itype)		\
{NSC_SHORT, 0, 0, ((base) + (itype)*sizeof(u_short)),	\
sizeof(sfx) == 1 ? name : pfx sfx, EF_BAD}

#define NSC_IVEC(base, sfx)		\
NSC_IELT("civil", "c", sfx, base, I_CIVIL),	\
NSC_IELT("milit", "m", sfx, base, I_MILIT),	\
NSC_IELT("shell", "s", sfx, base, I_SHELL),	\
NSC_IELT("gun", "g", sfx, base, I_GUN),		\
NSC_IELT("petrol", "p", sfx, base, I_PETROL),	\
NSC_IELT("iron", "i", sfx, base, I_IRON),	\
NSC_IELT("dust", "d", sfx, base, I_DUST),	\
NSC_IELT("bar", "b", sfx, base, I_BAR),		\
NSC_IELT("food", "f", sfx, base, I_FOOD),	\
NSC_IELT("oil", "o", sfx, base, I_OIL),		\
NSC_IELT("lcm", "l", sfx, base, I_LCM),		\
NSC_IELT("hcm", "h", sfx, base, I_HCM),		\
NSC_IELT("uw", "u", sfx, base, I_UW),		\
NSC_IELT("rad", "r", sfx, base, I_RAD)

struct castr ichr_ca[] = {
    {NSC_SITYPE(i_type), 0, 0, offsetof(struct ichrstr, i_vtype), "vtype",
     EF_ITEM},
    {NSC_STRING, 0, 0, offsetof(struct ichrstr, i_name), "name", EF_BAD},
    {NSC_INT, NSC_CONST, 0, offsetof(struct ichrstr, i_mnem), "mnem", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_value), "value", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_sell), "sell", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_lbs), "lbs", EF_BAD},
    {NSC_INT, 0, NUMPKG, offsetof(struct ichrstr, i_pkg), "pkg", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_melt_denom), "melt_denom",
     EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr pchr_ca[] = {
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_uid), "uid", EF_PRODUCT},
    {NSC_STRING, 0, 0, offsetof(struct pchrstr, p_name), "name", EF_BAD},
    {NSC_STRING, NSC_CONST, 0, offsetof(struct pchrstr, p_sname), "sname",
     EF_BAD},
    {NSC_SITYPE(i_type), 0, MAXPRCON, offsetof(struct pchrstr, p_ctype),
     "ctype", EF_ITEM},
    {NSC_USHORT, 0, MAXPRCON, offsetof(struct pchrstr, p_camt), "camt",
     EF_BAD},
    {NSC_SITYPE(i_type), 0, 0, offsetof(struct pchrstr, p_type), "type",
     EF_ITEM},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_level), "level", EF_LEVEL},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_cost), "cost", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nrndx), "nrndx", EF_RESOURCES},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nrdep), "nrdep", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nlndx), "nlndx", EF_LEVEL},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nlmin), "nlmin", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nllag), "nllag", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_effic), "effic", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr sect_ca[] = {
    {NSC_NATID, 0, 0, fldoff(sctstr, sct_own), "owner", EF_NATION},
    {NSC_XCOORD, 0, 0, fldoff(sctstr, sct_x), "xloc", EF_BAD},
    {NSC_YCOORD, 0, 0, fldoff(sctstr, sct_y), "yloc", EF_BAD},
    {NSC_TYPEID, 0, 0, fldoff(sctstr, sct_type), "des", EF_SECTOR_CHR},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_effic), "effic", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(sctstr, sct_mobil), "mobil", EF_BAD},
    {NSC_UCHAR, NSC_DEITY, 0, fldoff(sctstr, sct_loyal), "loyal", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_terr), "terr", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(sctstr, sct_terr), "terr0", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_terr1), "terr1", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_terr2), "terr2", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_terr3), "terr3", EF_BAD},
    {NSC_XCOORD, 0, 0, fldoff(sctstr, sct_dist_x), "xdist", EF_BAD},
    {NSC_YCOORD, 0, 0, fldoff(sctstr, sct_dist_y), "ydist", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(sctstr, sct_avail), "avail", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_work), "work", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(sctstr, sct_coastal), "coastal", EF_BAD},
    {NSC_TYPEID, 0, 0, fldoff(sctstr, sct_newtype), "newdes", EF_SECTOR_CHR},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_min), "min", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_gmin), "gold", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_fertil), "fert", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_oil), "ocontent", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_uran), "uran", EF_BAD},
    {NSC_NATID, 0, 0, fldoff(sctstr, sct_oldown), "oldown", EF_NATION},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_off), "off", EF_BAD},
    NSC_IVEC(fldoff(sctstr, sct_item), ""),
    NSC_IVEC(fldoff(sctstr, sct_dist), "_dist"),
    NSC_IVEC(fldoff(sctstr, sct_del), "_del"),
    {NSC_SHORT, NSC_DEITY, 0, fldoff(sctstr, sct_mines), "mines", EF_BAD},
    {NSC_SHORT, NSC_DEITY, 0, fldoff(sctstr, sct_pstage), "pstage",
     EF_PLAGUE_STAGES},
    {NSC_SHORT, NSC_DEITY, 0, fldoff(sctstr, sct_ptime), "ptime", EF_BAD},
    {NSC_UCHAR, NSC_DEITY, 0, fldoff(sctstr, sct_che), "che", EF_BAD},
    {NSC_NATID, NSC_DEITY, 0, fldoff(sctstr, sct_che_target), "che_target",
     EF_NATION},
    {NSC_USHORT, 0, 0, fldoff(sctstr, sct_fallout), "fallout", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(sctstr, sct_access), "access", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_road), "road", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_rail), "rail", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_defense), "dfense", EF_BAD},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(sctstr, sct_timestamp), "timestamp",
     EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr dchr_ca[] = {
    {NSC_UCHAR, 0, 0, offsetof(struct dchrstr, d_uid), "uid", EF_SECTOR_CHR},
    {NSC_STRING, 0, 0, offsetof(struct dchrstr, d_name), "name", EF_BAD},
    {NSC_INT, NSC_CONST, 0, offsetof(struct dchrstr, d_mnem), "mnem", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_prd), "prd", EF_PRODUCT},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_mcst), "mcst", EF_BAD},
    {NSC_SITYPE(d_navigation), 0, 0, offsetof(struct dchrstr, d_nav), "nav",
     EF_SECTOR_NAVIGATION},
    {NSC_SITYPE(i_packing), 0, 0, offsetof(struct dchrstr, d_pkg), "pkg",
     EF_PACKING},
    {NSC_FLOAT, 0, 0, offsetof(struct dchrstr, d_ostr), "ostr", EF_BAD},
    {NSC_FLOAT, 0, 0, offsetof(struct dchrstr, d_dstr), "dstr", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_value), "value", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_cost), "cost", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_build), "build", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_lcms), "lcms", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_hcms), "hcms", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_maxpop), "maxpop", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

#define NSC_GENITEM(ef_type, ef_chr)					\
{ NSC_SHORT, 0, 0, fldoff(genitem, uid), "uid", ef_type},		\
{ NSC_NATID, 0, 0, fldoff(genitem, own), "owner", EF_NATION},		\
{ NSC_XCOORD, 0, 0, fldoff(genitem, x), "xloc", EF_BAD},		\
{ NSC_YCOORD, 0, 0, fldoff(genitem, y), "yloc", EF_BAD},		\
{ NSC_TYPEID, 0, 0, fldoff(genitem, type), "type", ef_chr},		\
{ NSC_CHAR, 0, 0, fldoff(genitem, effic), "effic", EF_BAD},		\
{ NSC_CHAR , 0, 0, fldoff(genitem, mobil), "mobil", EF_BAD},		\
{ NSC_SHORT, 0, 0, fldoff(genitem, tech), "tech", EF_BAD},		\
{ NSC_CHAR, NSC_EXTRA, 0, fldoff(genitem, group), "group", EF_BAD},	\
{ NSC_XCOORD, 0, 0, fldoff(genitem, opx), "opx", EF_BAD},		\
{ NSC_YCOORD, 0, 0, fldoff(genitem, opy), "opy", EF_BAD},		\
{ NSC_SHORT, 0, 0, fldoff(genitem, mission), "mission", EF_MISSIONS},	\
{ NSC_SHORT, 0, 0, fldoff(genitem, radius), "radius", EF_BAD}

struct castr ship_ca[] = {
    NSC_GENITEM(EF_SHIP, EF_SHIP_CHR),
    {NSC_CHAR, 0, 0, fldoff(shpstr, shp_fleet), "fleet", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(shpstr, shp_nplane), "nplane", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(shpstr, shp_nland), "nland", EF_BAD},
    {NSC_XCOORD, 0, 0, fldoff(shpstr, shp_destx[0]), "xstart", EF_BAD},
    {NSC_XCOORD, 0, 0, fldoff(shpstr, shp_destx[1]), "xend", EF_BAD},
    {NSC_YCOORD, 0, 0, fldoff(shpstr, shp_desty[0]), "ystart", EF_BAD},
    {NSC_YCOORD, 0, 0, fldoff(shpstr, shp_desty[1]), "yend", EF_BAD},
    {NSC_SITYPE(i_type), 0, TMAX, fldoff(shpstr, shp_tstart), "cargostart",
     EF_ITEM},
    {NSC_SITYPE(i_type), 0, TMAX, fldoff(shpstr, shp_tend), "cargoend",
     EF_ITEM},
    {NSC_SHORT, 0, TMAX, fldoff(shpstr, shp_lstart), "amtstart", EF_BAD},
    {NSC_SHORT, 0, TMAX, fldoff(shpstr, shp_lend), "amtend", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(shpstr, shp_autonav), "autonav", EF_BAD},
    NSC_IVEC(fldoff(shpstr, shp_item), ""),
    {NSC_SHORT, NSC_DEITY, 0, fldoff(shpstr, shp_pstage), "pstage",
     EF_PLAGUE_STAGES},
    {NSC_SHORT, NSC_DEITY, 0, fldoff(shpstr, shp_ptime), "ptime", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(shpstr, shp_access), "access", EF_BAD},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(shpstr, shp_timestamp), "timestamp",
     EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(shpstr, shp_mobquota), "mquota", EF_BAD},
    {NSC_STRINGY, 0, MAXSHPPATH, fldoff(shpstr, shp_path), "path", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(shpstr, shp_follow), "follow", EF_BAD},
    {NSC_STRINGY, 0, MAXSHPNAMLEN, fldoff(shpstr, shp_name), "name", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(shpstr, shp_fuel), "fuel", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(shpstr, shp_nchoppers), "nchoppers",
     EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(shpstr, shp_nxlight), "nxlight", EF_BAD},
    /* could let builder access these, but we can't express that yet: */
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(shpstr, shp_orig_x), "xbuilt", EF_BAD},
    {NSC_YCOORD, NSC_DEITY, 0, fldoff(shpstr, shp_orig_y), "ybuilt", EF_BAD},
    {NSC_NATID, NSC_DEITY, 0, fldoff(shpstr, shp_orig_own), "builder",
     EF_NATION},
    {NSC_INT, NSC_BITS, 0, fldoff(shpstr, shp_rflags), "rflags",
     EF_RETREAT_FLAGS},
    {NSC_STRINGY, 0, RET_LEN, fldoff(shpstr, shp_rpath), "rpath", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr mchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct mchrstr, m_name), "name", EF_BAD},
    NSC_IVEC(offsetof(struct mchrstr, m_item), ""),
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_lcm), "lcm", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_hcm), "hcm", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_armor), "armor", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_speed), "speed", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_visib), "visib", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_vrnge), "vrnge", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_frnge), "frnge", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_glim), "glim", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_nxlight), "nxlight", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_nchoppers), "nchoppers",
     EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_fuelc), "fuelc", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_fuelu), "fuelu", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_tech), "tech", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_cost), "cost", EF_BAD},
    {NSC_LONG, NSC_BITS, 0, offsetof(struct mchrstr, m_flags), "flags",
     EF_SHIP_CHR_FLAGS},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_nplanes), "nplanes", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_nland), "nland", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr plane_ca[] = {
    NSC_GENITEM(EF_PLANE, EF_PLANE_CHR),
    {NSC_CHAR, 0, 0, fldoff(plnstr, pln_wing), "wing", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(plnstr, pln_range), "range", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(plnstr, pln_ship), "ship", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(plnstr, pln_land), "land", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(plnstr, pln_att), "att", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(plnstr, pln_def), "def", EF_BAD},
    {NSC_CHAR, 0, 0, fldoff(plnstr, pln_harden), "harden", EF_BAD},
    {NSC_CHAR, 0, 0, fldoff(plnstr, pln_nuketype), "nuketype", EF_BAD},
    {NSC_CHAR, NSC_BITS, 0, fldoff(plnstr, pln_flags), "flags",
     EF_PLANE_FLAGS},
    {NSC_TIME, 0, 0, fldoff(plnstr, pln_access), "access", EF_BAD},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(plnstr, pln_timestamp), "timestamp",
     EF_BAD},
    {NSC_FLOAT, 0, 0, fldoff(plnstr, pln_theta), "theta", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr plchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct plchrstr, pl_name), "name", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_lcm), "lcm", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_hcm), "hcm", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_cost), "cost", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_tech), "tech", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_acc), "acc", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_load), "load", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_att), "att", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_def), "def", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_range), "range", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_crew), "crew", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_fuel), "fuel", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_stealth), "stealth", EF_BAD},
    {NSC_INT, NSC_BITS, 0, offsetof(struct plchrstr, pl_flags), "flags",
     EF_PLANE_CHR_FLAGS},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr land_ca[] = {
    NSC_GENITEM(EF_LAND, EF_LAND_CHR),
    {NSC_CHAR, 0, 0, fldoff(lndstr, lnd_army), "army", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(lndstr, lnd_ship), "ship", EF_BAD},
    {NSC_CHAR, 0, 0, fldoff(lndstr, lnd_harden), "harden", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(lndstr, lnd_retreat), "retreat", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(lndstr, lnd_fuel), "fuel", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_nxlight), "nxlight", EF_BAD},
    {NSC_INT, NSC_BITS, 0, fldoff(lndstr, lnd_rflags), "rflags",
     EF_RETREAT_FLAGS},
    {NSC_STRINGY, 0, RET_LEN, fldoff(lndstr, lnd_rpath), "rpath", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(lndstr, lnd_rad_max), "react", EF_BAD},
    NSC_IVEC(fldoff(lndstr, lnd_item), ""),
    {NSC_SHORT, NSC_DEITY, 0, fldoff(lndstr, lnd_pstage), "pstage",
     EF_PLAGUE_STAGES},
    {NSC_SHORT, NSC_DEITY, 0, fldoff(lndstr, lnd_ptime), "ptime", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(lndstr, lnd_land), "land", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_nland), "nland", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(lndstr, lnd_access), "access", EF_BAD},
    {NSC_FLOAT, NSC_EXTRA, 0, fldoff(lndstr, lnd_att), "att", EF_BAD},
    {NSC_FLOAT, NSC_EXTRA, 0, fldoff(lndstr, lnd_def), "def", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_vul), "vul", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_spd), "spd", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_vis), "vis", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_spy), "spy", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_rad), "rmax", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_frg), "frg", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_acc), "acc", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_dam), "dam", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_ammo), "ammo", EF_BAD},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_aaf), "aaf", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_fuelc), "fuelc", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_fuelu), "fuelu", EF_BAD},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_maxlight), "maxlight",
     EF_BAD},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(lndstr, lnd_timestamp), "timestamp",
     EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr lchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct lchrstr, l_name), "name", EF_BAD},
    NSC_IVEC(offsetof(struct lchrstr, l_item), ""),
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_lcm), "lcm", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_hcm), "hcm", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_gun), "gun", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_shell), "shell", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_tech), "tech", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_cost), "cost", EF_BAD},
    {NSC_FLOAT, 0, 0, offsetof(struct lchrstr, l_att), "att", EF_BAD},
    {NSC_FLOAT, 0, 0, offsetof(struct lchrstr, l_def), "def", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_vul), "vul", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_spd), "spd", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_vis), "vis", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_spy), "spy", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_rad), "rad", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_frg), "frg", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_acc), "acc", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_dam), "dam", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_ammo), "ammo", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_aaf), "aaf", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct lchrstr, l_fuelc), "fuelc", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct lchrstr, l_fuelu), "fuelu", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct lchrstr, l_nxlight), "nxlight", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct lchrstr, l_mxland), "mxland", EF_BAD},
    {NSC_LONG, NSC_BITS, 0, offsetof(struct lchrstr, l_flags), "flags",
     EF_LAND_CHR_FLAGS},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr nuke_ca[] = {
    {NSC_SHORT, 0, 0, fldoff(nukstr, nuk_uid), "uid", EF_NUKE},
    {NSC_NATID, 0, 0, fldoff(nukstr, nuk_own), "owner", EF_NATION},
    {NSC_XCOORD, 0, 0, fldoff(nukstr, nuk_x), "xloc", EF_BAD},
    {NSC_YCOORD, 0, 0, fldoff(nukstr, nuk_y), "yloc", EF_BAD},
    {NSC_CHAR, 0, 0, fldoff(nukstr, nuk_n), "number", EF_BAD},
    {NSC_SHORT, 0, N_MAXNUKE, fldoff(nukstr, nuk_types), "types", EF_BAD},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(nukstr, nuk_timestamp), "timestamp",
     EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr nchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct nchrstr, n_name), "name", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_lcm), "lcm", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_hcm), "hcm", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_oil), "oil", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_rad), "rad", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_blast), "blast", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_dam), "dam", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_cost), "cost", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_tech), "tech", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_weight), "weight", EF_BAD},
    {NSC_INT, NSC_BITS, 0, offsetof(struct nchrstr, n_flags), "flags",
     EF_NUKE_CHR_FLAGS},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr treaty_ca[] = {
    {NSC_SHORT, 0, 0, fldoff(trtstr, trt_uid), "uid", EF_TREATY},
    {NSC_NATID, 0, 0, fldoff(trtstr, trt_cna), "cna", EF_NATION},
    {NSC_NATID, 0, 0, fldoff(trtstr, trt_cnb), "cnb", EF_NATION},
    {NSC_CHAR, 0, 0, fldoff(trtstr, trt_status), "status",
     EF_AGREEMENT_STATUS},
    {NSC_SHORT, NSC_BITS, 0, fldoff(trtstr, trt_acond), "acond",
     EF_TREATY_FLAGS},
    {NSC_SHORT, NSC_BITS, 0, fldoff(trtstr, trt_bcond), "bcond",
     EF_TREATY_FLAGS},
    {NSC_TIME, 0, 0, fldoff(trtstr, trt_exp), "exp", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr loan_ca[] = {
    {NSC_SHORT, 0, 0, fldoff(lonstr, l_uid), "uid", EF_LOAN},
    {NSC_NATID, 0, 0, fldoff(lonstr, l_loner), "loaner", EF_NATION},
    {NSC_NATID, 0, 0, fldoff(lonstr, l_lonee), "loanee", EF_NATION},
    {NSC_CHAR, 0, 0, fldoff(lonstr, l_status), "status",
     EF_AGREEMENT_STATUS},
    {NSC_INT, 0, 0, fldoff(lonstr, l_irate), "irate", EF_BAD},
    {NSC_INT, 0, 0, fldoff(lonstr, l_ldur), "ldur", EF_BAD},
    {NSC_LONG, 0, 0, fldoff(lonstr, l_amtpaid), "amtpaid", EF_BAD},
    {NSC_LONG, 0, 0, fldoff(lonstr, l_amtdue), "amtdue", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(lonstr, l_lastpay), "lastpay", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(lonstr, l_duedate), "duedate", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr news_ca[] = {
    {NSC_SHORT, 0, 0, fldoff(nwsstr, nws_uid), "uid", EF_NEWS},
    {NSC_NATID, 0, 0, fldoff(nwsstr, nws_ano), "actor", EF_NATION},
    {NSC_CHAR, 0, 0, fldoff(nwsstr, nws_vrb), "action", EF_NEWS_CHR},
    {NSC_NATID, 0, 0, fldoff(nwsstr, nws_vno), "victim", EF_NATION},
    {NSC_CHAR, 0, 0, fldoff(nwsstr, nws_ntm), "times", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(nwsstr, nws_when), "time", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr lost_ca[] = {
    {NSC_INT, 0, 0, fldoff(loststr, lost_uid), "uid", EF_LOST},
    {NSC_NATID, 0, 0, fldoff(loststr, lost_owner), "owner", EF_NATION},
    {NSC_CHAR, 0, 0, fldoff(loststr, lost_type), "type", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(loststr, lost_id), "id", EF_BAD},
    {NSC_XCOORD, 0, 0, fldoff(loststr, lost_x), "x", EF_BAD},
    {NSC_YCOORD, 0, 0, fldoff(loststr, lost_y), "y", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(loststr, lost_timestamp), "timestamp", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr commodity_ca[] = {
    {NSC_SHORT, 0, 0, fldoff(comstr, com_uid), "uid", EF_COMM},
    {NSC_NATID, 0, 0, fldoff(comstr, com_owner), "owner", EF_NATION},
    {NSC_SITYPE(i_type), 0, 0, fldoff(comstr, com_type), "type", EF_ITEM},
    {NSC_INT, 0, 0, fldoff(comstr, com_amount), "amount", EF_BAD},
    {NSC_FLOAT, 0, 0, fldoff(comstr, com_price), "price", EF_BAD},
    {NSC_INT, 0, 0, fldoff(comstr, com_maxbidder), "maxbidder", EF_NATION},
    {NSC_TIME, 0, 0, fldoff(comstr, com_markettime), "markettime", EF_BAD},
    /* could let maxbidder access these, but we can't express that yet: */
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(comstr, com_x), "xbuy", EF_BAD},
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(comstr, com_y), "ybuy", EF_BAD},
    /* could let the owner access these, but we can't express that yet: */
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(comstr, sell_x), "xsell", EF_BAD},
    {NSC_YCOORD, NSC_DEITY, 0, fldoff(comstr, sell_y), "ysell", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr trade_ca[] = {
    {NSC_SHORT, 0, 0, fldoff(trdstr, trd_uid), "uid", EF_TRADE},
    {NSC_NATID, 0, 0, fldoff(trdstr, trd_owner), "owner", EF_NATION},
    {NSC_CHAR, 0, 0, fldoff(trdstr, trd_type), "type", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(trdstr, trd_unitid), "unitid", EF_BAD},
    {NSC_LONG, 0, 0, fldoff(trdstr, trd_price), "price", EF_BAD},
    {NSC_INT, 0, 0, fldoff(trdstr, trd_maxbidder), "maxbidder", EF_NATION},
    {NSC_TIME, 0, 0, fldoff(trdstr, trd_markettime), "markettime", EF_BAD},
    /* could let the maxbidder access these, but we can't express that yet: */
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(trdstr, trd_x), "xloc", EF_BAD},
    {NSC_YCOORD, NSC_DEITY, 0, fldoff(trdstr, trd_y), "yloc", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr nat_ca[] = {
    {NSC_NATID, 0, 0, fldoff(natstr, nat_cnum), "cnum", EF_NATION},
    {NSC_SITYPE(nat_status), 0, 0, fldoff(natstr, nat_stat), "stat",
     EF_NATION_STATUS},
    {NSC_STRINGY, 0, 20, fldoff(natstr, nat_cnam), "cname", EF_BAD},
    {NSC_STRINGY, NSC_DEITY, 20, fldoff(natstr, nat_pnam), "passwd", EF_BAD},
    {NSC_STRINGY, 0, 32, fldoff(natstr, nat_hostaddr), "ip", EF_BAD},
    {NSC_STRINGY, 0, 512, fldoff(natstr, nat_hostname), "hostname", EF_BAD},
    {NSC_STRINGY, 0, 32, fldoff(natstr, nat_userid), "userid", EF_BAD},
    {NSC_XCOORD, 0, 0, fldoff(natstr, nat_xcap), "xcap", EF_BAD},
    {NSC_YCOORD, 0, 0, fldoff(natstr, nat_ycap), "ycap", EF_BAD},
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(natstr, nat_xorg), "xorg", EF_BAD},
    {NSC_YCOORD, NSC_DEITY, 0, fldoff(natstr, nat_yorg), "yorg", EF_BAD},
    {NSC_CHAR, 0, 0, fldoff(natstr, nat_dayno), "dayno", EF_BAD},
    {NSC_CHAR, 0, 0, fldoff(natstr, nat_update), "update", EF_BAD},
    {NSC_UCHAR, 0, 0, fldoff(natstr, nat_missed), "missed", EF_BAD},
    {NSC_USHORT, 0, 0, fldoff(natstr, nat_tgms), "tgms", EF_BAD},
    {NSC_USHORT, 0, 0, fldoff(natstr, nat_ann), "ann", EF_BAD},
    {NSC_USHORT, 0, 0, fldoff(natstr, nat_minused), "minused", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(natstr, nat_btu), "btu", EF_BAD},
    {NSC_LONG, 0, 0, fldoff(natstr, nat_reserve), "milreserve", EF_BAD},
    {NSC_LONG, 0, 0, fldoff(natstr, nat_money), "money", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(natstr, nat_last_login), "login", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(natstr, nat_last_logout), "logout", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(natstr, nat_newstim), "newstim", EF_BAD},
    {NSC_TIME, 0, 0, fldoff(natstr, nat_annotim), "annotim", EF_BAD},
    {NSC_FLOAT, 0, 0, fldoff(natstr, nat_level[NAT_TLEV]), "tech", EF_BAD},
    {NSC_FLOAT, 0, 0, fldoff(natstr, nat_level[NAT_RLEV]), "research", EF_BAD},
    {NSC_FLOAT, 0, 0, fldoff(natstr, nat_level[NAT_ELEV]), "education",
     EF_BAD},
    {NSC_FLOAT, 0, 0, fldoff(natstr, nat_level[NAT_HLEV]), "happiness",
     EF_BAD},
    {NSC_SHORT, 0, MAXNOC, fldoff(natstr, nat_relate), "relations",
     EF_NATION_RELATIONS},
    {NSC_UCHAR, NSC_DEITY, MAXNOC, fldoff(natstr, nat_contact), "contacts",
     EF_BAD},
    /* FIXME nat_rejects[], nat_priorities[] */
    {NSC_LONG, NSC_BITS, 0, fldoff(natstr, nat_flags), "flags",
     EF_NATION_FLAGS},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr cou_ca[] = {
    {NSC_NATID, 0, 0, fldoff(natstr, nat_cnum), "cnum", EF_NATION},
    {NSC_CHAR, 0, 0, fldoff(natstr, nat_stat), "stat",
     EF_NATION_STATUS},
    {NSC_STRINGY, 0, 20, fldoff(natstr, nat_cnam), "cname", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr realm_ca[] = {
    {NSC_SHORT, 0, 0, fldoff(realmstr, r_uid), "uid", EF_REALM},
    {NSC_NATID, 0, 0, fldoff(realmstr, r_cnum), "cnum", EF_NATION},
    {NSC_USHORT, 0, 0, fldoff(realmstr, r_realm), "realm", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(realmstr, r_xl), "xl", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(realmstr, r_xh), "xh", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(realmstr, r_yl), "yl", EF_BAD},
    {NSC_SHORT, 0, 0, fldoff(realmstr, r_yh), "yh", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr intrchr_ca[] = {
    {NSC_STRING, NSC_CONST, 0, offsetof(struct sctintrins, in_name), "name",
     EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_lcms), "lcms", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_hcms), "hcms", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_dcost), "dcost", EF_BAD},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_mcost), "mcost", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr rpt_ca[] = {
    {NSC_CHAR, 0, 0, offsetof(struct rptstr, r_uid), "uid", EF_NEWS_CHR},
    {NSC_STRING, 0, NUM_RPTS, offsetof(struct rptstr, r_newstory), "newstory",
     EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct rptstr, r_good_will), "good_will", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct rptstr, r_newspage), "newspage",
     EF_PAGE_HEADINGS},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr empfile_ca[] = {
    {NSC_INT, 0, 0, offsetof(struct empfile, uid), "uid", EF_TABLE},
    {NSC_STRING, 0, 0, offsetof(struct empfile, name), "name", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
};

struct castr symbol_ca[] = {
    {NSC_INT, 0, 0, offsetof(struct symbol, value), "value", EF_BAD},
    {NSC_STRING, 0, 0, offsetof(struct symbol, name), "name", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
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
    {M_OILER, "oiler"},
    {M_SUPPLY, "supply"},
    {M_CANAL, "canal"},
    {M_ANTIMISSILE, "anti-missile"},
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
    {P_X, "stealth"},
    {P_N, "SDI"},
    {P_H, "half-stealth"},
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

struct symbol nuke_chr_flags[] = {
    {N_NEUT, "neutron"},
    {0, NULL}
};

struct castr mdchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct castr, ca_name), "name", EF_BAD},
    {NSC_CHAR, 0, 0, offsetof(struct castr, ca_type), "type", EF_META_TYPE},
    {NSC_UCHAR, NSC_BITS, 0, offsetof(struct castr, ca_flags), "flags",
     EF_META_FLAGS},
    {NSC_USHORT, 0, 0, offsetof(struct castr, ca_len), "len", EF_BAD},
    {NSC_INT, 0, 0, offsetof(struct castr, ca_table), "table", EF_BAD},
    {NSC_NOTYPE, 0, 0, 0, NULL, EF_BAD}
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
    {NSC_TYPEID, "d"},
    {NSC_CHAR, "d"},
    {NSC_UCHAR, "d"},
    {NSC_SHORT, "d"},
    {NSC_USHORT, "d"},
    {NSC_INT, "d"},
    {NSC_XCOORD, "d"},
    {NSC_YCOORD, "d"},
    {NSC_TIME, "t"},
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

struct symbol plane_flags[] = {
    {PLN_LAUNCHED, "launched"},
    {PLN_SYNCHRONOUS, "synchronous"},
    {PLN_AIRBURST, "airbust"},
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

struct symbol nation_status[] = {
    {STAT_UNUSED, "unused"},
    {STAT_NEW,	"new"},
    {STAT_VIS, "visitor"},
    {STAT_SANCT, "sanctuary"},
    {STAT_ACTIVE, "active"},
    {STAT_GOD, "deity"}
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

struct symbol nation_relations[] = {
    {AT_WAR, "at-war"},
    {SITZKRIEG, "sitzkrieg"},
    {MOBILIZATION, "mobilization"},
    {HOSTILE, "hostile"},
    {NEUTRAL, "neutral"},
    {FRIENDLY, "friendly"},
    {ALLIED, "allied"},
    {0, NULL}
};

struct symbol level[] = {
    {NAT_TLEV, "technology"},
    {NAT_RLEV, "research"},
    {NAT_ELEV, "education"},
    {NAT_HLEV, "happiness"},
    {0, NULL}
};

struct symbol agreement_statuses[] = {
    {AGREE_FREE, "free"},
    {AGREE_PROPOSED, "proposed"},
    {AGREE_SIGNED, "signed"},
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

struct symbol packing[] = {
    {IPKG, "inefficient"},
    {NPKG, "normal"},
    {WPKG, "warehouse"},
    {UPKG, "urban"},
    {BPKG, "bank"},
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

struct symbol sector_navigation[] = { /* for d_nav */
    {NAV_NONE, "land"},
    {NAVOK, "sea"},
    {NAV_02, "harbor"},
    {NAV_CANAL, "canal"},
    {NAV_60, "bridge"},
    {0, NULL}
};
