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
 *  nsc.c: Empire selection global structures
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004
 *  
 */

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

#define fldoff(str, fld) offsetof(struct str, fld)

#define NSC_IELT(name, pfx, sfx, base, itype)		\
{NSC_SHORT, 0, 0, ((base) + (itype)*sizeof(u_short)),	\
sizeof(sfx) == 1 ? name : pfx sfx}

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
    {NSC_STRING, 0, 0, offsetof(struct ichrstr, i_name), "name"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_mnem), "mnem"},
    {NSC_SITYPE(i_type), 0, 0, offsetof(struct ichrstr, i_vtype), "vtype"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_value), "value"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_sell), "sell"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_lbs), "lbs"},
    {NSC_INT, 0, NUMPKG, offsetof(struct ichrstr, i_pkg), "pkg"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr pchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct pchrstr, p_name), "name"},
    {NSC_STRING, 0, 0, offsetof(struct pchrstr, p_sname), "sname"},
    {NSC_SITYPE(i_type), 0, MAXPRCON, offsetof(struct pchrstr, p_ctype), "ctype"},
    {NSC_USHORT, 0, MAXPRCON, offsetof(struct pchrstr, p_camt), "camt"},
    {NSC_SITYPE(i_type), 0, 0, offsetof(struct pchrstr, p_type), "type"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_level), "level"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_cost), "cost"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nrndx), "nrndx"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nrdep), "nrdep"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nlndx), "nlndx"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nlmin), "nlmin"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_nllag), "nllag"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_effic), "effic"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr sect_ca[] = {
    {NSC_NATID, 0, 0, fldoff(sctstr, sct_own), "owner"},
    {NSC_XCOORD, 0, 0, fldoff(sctstr, sct_x), "xloc"},
    {NSC_YCOORD, 0, 0, fldoff(sctstr, sct_y), "yloc"},
    {NSC_TYPEID, 0, 0, fldoff(sctstr, sct_type), "des"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_effic), "effic"},
    {NSC_SHORT, 0, 0, fldoff(sctstr, sct_mobil), "mobil"},
    {NSC_UCHAR, NSC_DEITY, 0, fldoff(sctstr, sct_loyal), "loyal"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_terr), "terr"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(sctstr, sct_terr), "terr0"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_terr1), "terr1"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_terr2), "terr2"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_terr3), "terr3"},
    {NSC_XCOORD, 0, 0, fldoff(sctstr, sct_dist_x), "xdist"},
    {NSC_YCOORD, 0, 0, fldoff(sctstr, sct_dist_y), "ydist"},
    {NSC_SHORT, 0, 0, fldoff(sctstr, sct_avail), "avail"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_work), "work"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(sctstr, sct_coastal), "coastal"},
    {NSC_TYPEID, 0, 0, fldoff(sctstr, sct_newtype), "newdes"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_min), "min"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_gmin), "gold"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_fertil), "fert"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_oil), "ocontent"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_uran), "uran"},
    {NSC_NATID, 0, 0, fldoff(sctstr, sct_oldown), "oldown"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_off), "off"},
    NSC_IVEC(fldoff(sctstr, sct_item), ""),
    NSC_IVEC(fldoff(sctstr, sct_dist), "_dist"),
    NSC_IVEC(fldoff(sctstr, sct_del), "_del"),
    {NSC_SHORT, NSC_DEITY, 0, fldoff(sctstr, sct_mines), "mines"},
    {NSC_SHORT, NSC_DEITY, 0, fldoff(sctstr, sct_pstage), "pstage"},
    {NSC_SHORT, NSC_DEITY, 0, fldoff(sctstr, sct_ptime), "ptime"},
    {NSC_UCHAR, NSC_DEITY, 0, fldoff(sctstr, sct_che), "che"},
    {NSC_NATID, NSC_DEITY, 0, fldoff(sctstr, sct_che_target), "che_target"},
    {NSC_USHORT, 0, 0, fldoff(sctstr, sct_fallout), "fallout"},
    {NSC_TIME, 0, 0, fldoff(sctstr, sct_access), "access"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_road), "road"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_rail), "rail"},
    {NSC_UCHAR, 0, 0, fldoff(sctstr, sct_defense), "dfense"},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(sctstr, sct_timestamp), "timestamp"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr dchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct dchrstr, d_name), "name"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_mnem), "mnem"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_prd), "prd"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_mcst), "mcst"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_flg), "flg"},
    {NSC_SITYPE(i_packing), 0, 0, offsetof(struct dchrstr, d_pkg), "pkg"},
    {NSC_FLOAT, 0, 0, offsetof(struct dchrstr, d_ostr), "ostr"},
    {NSC_FLOAT, 0, 0, offsetof(struct dchrstr, d_dstr), "dstr"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_value), "value"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_cost), "cost"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_build), "build"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_lcms), "lcms"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_hcms), "hcms"},
    {NSC_INT, 0, 0, offsetof(struct dchrstr, d_maxpop), "maxpop"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

#define NSC_GENITEM						\
{ NSC_NATID, 0, 0, fldoff(genitem, own), "owner"},		\
{ NSC_SHORT, 0, 0, fldoff(genitem, uid), "uid"},		\
{ NSC_XCOORD, 0, 0, fldoff(genitem, x), "xloc"},		\
{ NSC_YCOORD, 0, 0, fldoff(genitem, y), "yloc"},		\
{ NSC_TYPEID, 0, 0, fldoff(genitem, type), "type"},		\
{ NSC_CHAR, 0, 0, fldoff(genitem, effic), "effic"},		\
{ NSC_CHAR , 0, 0, fldoff(genitem, mobil), "mobil"},		\
{ NSC_SHORT, 0, 0, fldoff(genitem, tech), "tech"},		\
{ NSC_CHAR, NSC_EXTRA, 0, fldoff(genitem, group), "group"},	\
{ NSC_XCOORD, 0, 0, fldoff(genitem, opx), "opx"},		\
{ NSC_YCOORD, 0, 0, fldoff(genitem, opy), "opy"},		\
{ NSC_SHORT, 0, 0, fldoff(genitem, mission), "mission"},	\
{ NSC_SHORT, 0, 0, fldoff(genitem, radius), "radius"}

struct castr ship_ca[] = {
    NSC_GENITEM,
    {NSC_CHAR, 0, 0, fldoff(shpstr, shp_fleet), "fleet"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(shpstr, shp_nplane), "nplane"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(shpstr, shp_nland), "nland"},
    {NSC_XCOORD, 0, 0, fldoff(shpstr, shp_destx[0]), "xstart"},
    {NSC_XCOORD, 0, 0, fldoff(shpstr, shp_destx[1]), "xend"},
    {NSC_YCOORD, 0, 0, fldoff(shpstr, shp_desty[0]), "ystart"},
    {NSC_YCOORD, 0, 0, fldoff(shpstr, shp_desty[1]), "yend"},
    {NSC_SITYPE(i_type), 0, TMAX, fldoff(shpstr, shp_tstart), "cargostart"},
    {NSC_SITYPE(i_type), 0, TMAX, fldoff(shpstr, shp_tend), "cargoend"},
    {NSC_SHORT, 0, TMAX, fldoff(shpstr, shp_lstart), "amtstart"},
    {NSC_SHORT, 0, TMAX, fldoff(shpstr, shp_lend), "amtend"},
    {NSC_UCHAR, 0, 0, fldoff(shpstr, shp_autonav), "autonav"},
    NSC_IVEC(fldoff(shpstr, shp_item), ""),
    {NSC_SHORT, NSC_DEITY, 0, fldoff(shpstr, shp_pstage), "pstage"},
    {NSC_SHORT, NSC_DEITY, 0, fldoff(shpstr, shp_ptime), "ptime"},
    {NSC_TIME, 0, 0, fldoff(shpstr, shp_access), "access"},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(shpstr, shp_timestamp), "timestamp"},
    {NSC_UCHAR, 0, 0, fldoff(shpstr, shp_mobquota), "mquota"},
    {NSC_STRINGY, 0, MAXSHPPATH, fldoff(shpstr, shp_path), "path"},
    {NSC_SHORT, 0, 0, fldoff(shpstr, shp_follow), "follow"},
    {NSC_STRINGY, 0, MAXSHPNAMLEN, fldoff(shpstr, shp_name), "name"},
    {NSC_UCHAR, 0, 0, fldoff(shpstr, shp_fuel), "fuel"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(shpstr, shp_nchoppers), "nchoppers"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(shpstr, shp_nxlight), "nxlight"},
    /* could let builder access these, but we can't express that yet: */
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(shpstr, shp_orig_x), "xbuilt"},
    {NSC_YCOORD, NSC_DEITY, 0, fldoff(shpstr, shp_orig_y), "ybuilt"},
    {NSC_NATID, NSC_DEITY, 0, fldoff(shpstr, shp_orig_own), "builder"},
    {NSC_INT, 0, 0, fldoff(shpstr, shp_rflags), "rflags"},
    {NSC_STRINGY, 0, RET_LEN, fldoff(shpstr, shp_rpath), "rpath"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr mchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct mchrstr, m_name), "name"},
    {NSC_USHORT, 0, I_MAX+1, offsetof(struct mchrstr, m_item), "item"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_lcm), "lcm"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_hcm), "hcm"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_armor), "armor"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_speed), "speed"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_visib), "visib"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_vrnge), "vrnge"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_frnge), "frnge"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_glim), "glim"},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_nxlight), "nxlight"},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_nchoppers), "nchoppers"},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_fuelc), "fuelc"},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_fuelu), "fuelu"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_tech), "tech"},
    {NSC_INT, 0, 0, offsetof(struct mchrstr, m_cost), "cost"},
    {NSC_LONG, 0, 0, offsetof(struct mchrstr, m_flags), "flags"},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_nplanes), "nplanes"},
    {NSC_UCHAR, 0, 0, offsetof(struct mchrstr, m_nland), "nland"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr plane_ca[] = {
    NSC_GENITEM,
    {NSC_CHAR, 0, 0, fldoff(plnstr, pln_wing), "wing"},
    {NSC_UCHAR, 0, 0, fldoff(plnstr, pln_range), "range"},
    {NSC_SHORT, 0, 0, fldoff(plnstr, pln_ship), "ship"},
    {NSC_SHORT, 0, 0, fldoff(plnstr, pln_land), "land"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(plnstr, pln_att), "att"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(plnstr, pln_def), "def"},
    {NSC_CHAR, 0, 0, fldoff(plnstr, pln_harden), "harden"},
    {NSC_CHAR, 0, 0, fldoff(plnstr, pln_nuketype), "nuketype"},
    {NSC_CHAR, 0, 0, fldoff(plnstr, pln_flags), "flags"},
    {NSC_TIME, 0, 0, fldoff(plnstr, pln_access), "access"},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(plnstr, pln_timestamp), "timestamp"},
    {NSC_FLOAT, 0, 0, fldoff(plnstr, pln_theta), "theta"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr plchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct plchrstr, pl_name), "name"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_lcm), "lcm"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_hcm), "hcm"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_cost), "cost"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_tech), "tech"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_acc), "acc"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_load), "load"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_att), "att"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_def), "def"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_range), "range"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_crew), "crew"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_fuel), "fuel"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_stealth), "stealth"},
    {NSC_INT, 0, 0, offsetof(struct plchrstr, pl_flags), "flags"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr land_ca[] = {
    NSC_GENITEM,
    {NSC_CHAR, 0, 0, fldoff(lndstr, lnd_army), "army"},
    {NSC_SHORT, 0, 0, fldoff(lndstr, lnd_ship), "ship"},
    {NSC_CHAR, 0, 0, fldoff(lndstr, lnd_harden), "harden"},
    {NSC_SHORT, 0, 0, fldoff(lndstr, lnd_retreat), "retreat"},
    {NSC_UCHAR, 0, 0, fldoff(lndstr, lnd_fuel), "fuel"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_nxlight), "nxlight"},
    {NSC_INT, 0, 0, fldoff(lndstr, lnd_rflags), "rflags"},
    {NSC_STRINGY, 0, RET_LEN, fldoff(lndstr, lnd_rpath), "rpath"},
    {NSC_UCHAR, 0, 0, fldoff(lndstr, lnd_rad_max), "react"},
    NSC_IVEC(fldoff(lndstr, lnd_item), ""),
    {NSC_SHORT, NSC_DEITY, 0, fldoff(lndstr, lnd_pstage), "pstage"},
    {NSC_SHORT, NSC_DEITY, 0, fldoff(lndstr, lnd_ptime), "ptime"},
    {NSC_SHORT, 0, 0, fldoff(lndstr, lnd_land), "land"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_nland), "nland"},
    {NSC_TIME, 0, 0, fldoff(lndstr, lnd_access), "access"},
    {NSC_FLOAT, NSC_EXTRA, 0, fldoff(lndstr, lnd_att), "att"},
    {NSC_FLOAT, NSC_EXTRA, 0, fldoff(lndstr, lnd_def), "def"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_vul), "vul"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_spd), "spd"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_vis), "vis"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_spy), "spy"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_rad), "rmax"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_frg), "frg"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_acc), "acc"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_dam), "dam"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_ammo), "ammo"},
    {NSC_INT, NSC_EXTRA, 0, fldoff(lndstr, lnd_aaf), "aaf"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_fuelc), "fuelc"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_fuelu), "fuelu"},
    {NSC_UCHAR, NSC_EXTRA, 0, fldoff(lndstr, lnd_maxlight), "maxlight"},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(lndstr, lnd_timestamp), "timestamp"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr lchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct lchrstr, l_name), "name"},
    {NSC_USHORT, 0, I_MAX+1, offsetof(struct mchrstr, m_item), "item"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_lcm), "lcm"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_hcm), "hcm"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_gun), "gun"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_shell), "shell"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_tech), "tech"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_cost), "cost"},
    {NSC_FLOAT, 0, 0, offsetof(struct lchrstr, l_att), "att"},
    {NSC_FLOAT, 0, 0, offsetof(struct lchrstr, l_def), "def"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_vul), "vul"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_spd), "spd"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_vis), "vis"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_spy), "spy"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_rad), "rad"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_frg), "frg"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_acc), "acc"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_dam), "dam"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_ammo), "ammo"},
    {NSC_INT, 0, 0, offsetof(struct lchrstr, l_aaf), "aaf"},
    {NSC_UCHAR, 0, 0, offsetof(struct lchrstr, l_fuelc), "fuelc"},
    {NSC_UCHAR, 0, 0, offsetof(struct lchrstr, l_fuelu), "fuelu"},
    {NSC_UCHAR, 0, 0, offsetof(struct lchrstr, l_nxlight), "nxlight"},
    {NSC_UCHAR, 0, 0, offsetof(struct lchrstr, l_mxland), "mxland"},
    {NSC_LONG, 0, 0, offsetof(struct lchrstr, l_flags), "flags"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr nuke_ca[] = {
    {NSC_NATID, 0, 0, fldoff(nukstr, nuk_own), "owner"},
    {NSC_SHORT, 0, 0, fldoff(nukstr, nuk_uid), "uid"},
    {NSC_XCOORD, 0, 0, fldoff(nukstr, nuk_x), "xloc"},
    {NSC_YCOORD, 0, 0, fldoff(nukstr, nuk_y), "yloc"},
    {NSC_CHAR, 0, 0, fldoff(nukstr, nuk_n), "number"},
    {NSC_SHORT, 0, N_MAXNUKE, fldoff(nukstr, nuk_types), "types"},
    {NSC_TIME, NSC_EXTRA, 0, fldoff(nukstr, nuk_timestamp), "timestamp"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr nchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct nchrstr, n_name), "name"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_lcm), "lcm"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_hcm), "hcm"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_oil), "oil"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_rad), "rad"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_blast), "blast"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_dam), "dam"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_cost), "cost"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_tech), "tech"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_weight), "weight"},
    {NSC_INT, 0, 0, offsetof(struct nchrstr, n_flags), "flags"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr treaty_ca[] = {
    {NSC_NATID, 0, 0, fldoff(trtstr, trt_cna), "cna"},
    {NSC_NATID, 0, 0, fldoff(trtstr, trt_cnb), "cnb"},
    {NSC_CHAR, 0, 0, fldoff(trtstr, trt_status), "status"},
    {NSC_SHORT, 0, 0, fldoff(trtstr, trt_acond), "acond"},
    {NSC_SHORT, 0, 0, fldoff(trtstr, trt_bcond), "bcond"},
    {NSC_TIME, 0, 0, fldoff(trtstr, trt_exp), "exp"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr tchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct tchrstr, t_name), "name"},
    {NSC_INT, 0, 0, offsetof(struct tchrstr, t_cond), "cond"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr loan_ca[] = {
    {NSC_NATID, 0, 0, fldoff(lonstr, l_loner), "loaner"},
    {NSC_SHORT, 0, 0, fldoff(lonstr, l_uid), "uid"},
    {NSC_NATID, 0, 0, fldoff(lonstr, l_lonee), "loanee"},
    {NSC_CHAR, 0, 0, fldoff(lonstr, l_status), "status"},
    {NSC_INT, 0, 0, fldoff(lonstr, l_irate), "irate"},
    {NSC_INT, 0, 0, fldoff(lonstr, l_ldur), "ldur"},
    {NSC_LONG, 0, 0, fldoff(lonstr, l_amtpaid), "amtpaid"},
    {NSC_LONG, 0, 0, fldoff(lonstr, l_amtdue), "amtdue"},
    {NSC_TIME, 0, 0, fldoff(lonstr, l_lastpay), "lastpay"},
    {NSC_TIME, 0, 0, fldoff(lonstr, l_duedate), "duedate"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr news_ca[] = {
    {NSC_NATID, 0, 0, fldoff(nwsstr, nws_ano), "actor"},
    {NSC_SHORT, 0, 0, fldoff(nwsstr, nws_uid), "uid"},
    {NSC_CHAR, 0, 0, fldoff(nwsstr, nws_vrb), "action"},
    {NSC_NATID, 0, 0, fldoff(nwsstr, nws_vno), "victim"},
    {NSC_CHAR, 0, 0, fldoff(nwsstr, nws_ntm), "times"},
    {NSC_TIME, 0, 0, fldoff(nwsstr, nws_when), "time"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr lost_ca[] = {
    {NSC_NATID, 0, 0, fldoff(loststr, lost_owner), "owner"},
    {NSC_INT, 0, 0, fldoff(loststr, lost_uid), "uid"},
    {NSC_CHAR, 0, 0, fldoff(loststr, lost_type), "type"},
    {NSC_SHORT, 0, 0, fldoff(loststr, lost_id), "id"},
    {NSC_XCOORD, 0, 0, fldoff(loststr, lost_x), "x"},
    {NSC_YCOORD, 0, 0, fldoff(loststr, lost_y), "y"},
    {NSC_TIME, 0, 0, fldoff(loststr, lost_timestamp), "timestamp"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr commodity_ca[] = {
    {NSC_NATID, 0, 0, fldoff(comstr, com_owner), "owner"},
    {NSC_SHORT, 0, 0, fldoff(comstr, com_uid), "uid"},
    {NSC_SITYPE(i_type), 0, 0, fldoff(comstr, com_type), "type"},
    {NSC_INT, 0, 0, fldoff(comstr, com_amount), "amount"},
    {NSC_INT, 0, 0, fldoff(comstr, com_maxbidder), "maxbidder"},
    {NSC_TIME, 0, 0, fldoff(comstr, com_markettime), "markettime"},
    /* could let maxbidder access these, but we can't express that yet: */
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(comstr, com_x), "xbuy"},
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(comstr, com_y), "ybuy"},
    /* could let the owner access these, but we can't express that yet: */
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(comstr, sell_x), "xsell"},
    {NSC_YCOORD, NSC_DEITY, 0, fldoff(comstr, sell_y), "ysell"},
    {NSC_FLOAT, 0, 0, fldoff(comstr, com_price), "price"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr trade_ca[] = {
    {NSC_NATID, 0, 0, fldoff(trdstr, trd_owner), "owner"},
    {NSC_SHORT, 0, 0, fldoff(trdstr, trd_uid), "uid"},
    {NSC_CHAR, 0, 0, fldoff(trdstr, trd_type), "type"},
    {NSC_SHORT, 0, 0, fldoff(trdstr, trd_unitid), "unitid"},
    {NSC_LONG, 0, 0, fldoff(trdstr, trd_price), "price"},
    {NSC_INT, 0, 0, fldoff(trdstr, trd_maxbidder), "maxbidder"},
    {NSC_TIME, 0, 0, fldoff(trdstr, trd_markettime), "markettime"},
    /* could let the owner access these, but we can't express that yet: */
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(trdstr, trd_x), "xloc"},
    {NSC_YCOORD, NSC_DEITY, 0, fldoff(trdstr, trd_y), "yloc"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr nat_ca[] = {
    {NSC_NATID, 0, 0, fldoff(natstr, nat_cnum), "cnum"},
    {NSC_CHAR, 0, 0, fldoff(natstr, nat_stat), "stat"},
    {NSC_STRINGY, 0, 20, fldoff(natstr, nat_cnam), "cname"},
    {NSC_STRINGY, NSC_DEITY, 20, fldoff(natstr, nat_pnam), "passwd"},
    {NSC_STRINGY, 0, 32, fldoff(natstr, nat_hostaddr), "ip"},
    {NSC_STRINGY, 0, 512, fldoff(natstr, nat_hostname), "hostname"},
    {NSC_STRINGY, 0, 32, fldoff(natstr, nat_userid), "userid"},
    {NSC_XCOORD, 0, 0, fldoff(natstr, nat_xstart), "xstart"},
    {NSC_YCOORD, 0, 0, fldoff(natstr, nat_ystart), "ystart"},
    {NSC_XCOORD, 0, 0, fldoff(natstr, nat_xcap), "xcap"},
    {NSC_YCOORD, 0, 0, fldoff(natstr, nat_ycap), "ycap"},
    {NSC_XCOORD, NSC_DEITY, 0, fldoff(natstr, nat_xorg), "xorg"},
    {NSC_YCOORD, NSC_DEITY, 0, fldoff(natstr, nat_yorg), "yorg"},
    {NSC_CHAR, 0, 0, fldoff(natstr, nat_update), "update"},
    {NSC_UCHAR, 0, 0, fldoff(natstr, nat_missed), "missed"},
    {NSC_USHORT, 0, 0, fldoff(natstr, nat_tgms), "tgms"},
    {NSC_USHORT, 0, 0, fldoff(natstr, nat_ann), "ann"},
    {NSC_USHORT, 0, 0, fldoff(natstr, nat_minused), "minused"},
    {NSC_SHORT, 0, 0, fldoff(natstr, nat_btu), "btu"},
    {NSC_LONG, 0, 0, fldoff(natstr, nat_reserve), "milreserve"},
    {NSC_LONG, 0, 0, fldoff(natstr, nat_money), "money"},
    {NSC_TIME, 0, 0, fldoff(natstr, nat_last_login), "login"},
    {NSC_TIME, 0, 0, fldoff(natstr, nat_last_logout), "logout"},
    {NSC_TIME, 0, 0, fldoff(natstr, nat_newstim), "newstim"},
    {NSC_TIME, 0, 0, fldoff(natstr, nat_annotim), "annotim"},
    {NSC_FLOAT, 0, 0, fldoff(natstr, nat_level[NAT_TLEV]), "tech"},
    {NSC_FLOAT, 0, 0, fldoff(natstr, nat_level[NAT_RLEV]), "research"},
    {NSC_FLOAT, 0, 0, fldoff(natstr, nat_level[NAT_ELEV]), "education"},
    {NSC_FLOAT, 0, 0, fldoff(natstr, nat_level[NAT_HLEV]), "happiness"},
    /* FIXME nat_b[], nat_relate[], nat_contact[], nat_rejects[], nat_priorities[] */
    {NSC_LONG, 0, 0, fldoff(natstr, nat_flags),"flags"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr intrchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct sctintrins, in_name), "name"},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_lcms), "lcms"},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_hcms), "hcms"},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_dcost), "dcost"},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_mcost), "mcost"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr rpt_ca[] = {
    {NSC_STRING, 0, NUM_RPTS, offsetof(struct rptstr, r_newstory), "newstory"},
    {NSC_INT, 0, 0, offsetof(struct rptstr, r_good_will), "good_will"},
    {NSC_INT, 0, 0, offsetof(struct rptstr, r_newspage), "newspage"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

struct castr empfile_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct empfile, name), "name"},
    {NSC_STRING, 0, 0, offsetof(struct empfile, file), "file_name"},
    {NSC_INT, 0, 0, offsetof(struct empfile, flags), "flags"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

