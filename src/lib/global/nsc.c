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
 *  nscglb.c: Empire selection global structures
 * 
 *  Known contributors to this file:
 *  
 */

#include <struct.h>
#include "misc.h"
#include "var.h"
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

struct castr var_ca[] = {
    {NSC_DEITY | NSC_VAR | V_PSTAGE, "pstage", 0},
    {NSC_DEITY | NSC_VAR | V_PTIME, "ptime", 0},
    {NSC_DEITY | NSC_VAR | V_CHE, "che", 0},
    {NSC_DEITY | NSC_VAR | V_MINE, "lmine", 0},
    {NSC_VAR | V_FALLOUT, "fallout", 0},
    {NSC_ROUND | NSC_VAR | V_CIVIL, "civil", 0},
    {NSC_ROUND | NSC_VAR | V_MILIT, "milit", 0},
    {NSC_ROUND | NSC_VAR | V_SHELL, "shell", 0},
    {NSC_ROUND | NSC_VAR | V_GUN, "gun", 0},
    {NSC_ROUND | NSC_VAR | V_PETROL, "petrol", 0},
    {NSC_ROUND | NSC_VAR | V_IRON, "iron", 0},
    {NSC_ROUND | NSC_VAR | V_DUST, "dust", 0},
    {NSC_ROUND | NSC_VAR | V_BAR, "bar", 0},
    {NSC_ROUND | NSC_VAR | V_FOOD, "food", 0},
    {NSC_ROUND | NSC_VAR | V_OIL, "oil", 0},
    {NSC_ROUND | NSC_VAR | V_LCM, "lcm", 0},
    {NSC_ROUND | NSC_VAR | V_HCM, "hcm", 0},
    {NSC_ROUND | NSC_VAR | V_UW, "uw", 0},
    {NSC_ROUND | NSC_VAR | V_RAD, "rad", 0},
    {NSC_VAR | V_CDIST, "c_dist", 0},
    {NSC_VAR | V_MDIST, "m_dist", 0},
    {NSC_VAR | V_UDIST, "u_dist", 0},
    {NSC_VAR | V_SDIST, "s_dist", 0},
    {NSC_VAR | V_GDIST, "g_dist", 0},
    {NSC_VAR | V_PDIST, "p_dist", 0},
    {NSC_VAR | V_IDIST, "i_dist", 0},
    {NSC_VAR | V_DDIST, "d_dist", 0},
    {NSC_VAR | V_BDIST, "b_dist", 0},
    {NSC_VAR | V_FDIST, "f_dist", 0},
    {NSC_VAR | V_ODIST, "o_dist", 0},
    {NSC_VAR | V_LDIST, "l_dist", 0},
    {NSC_VAR | V_HDIST, "h_dist", 0},
    {NSC_VAR | V_RDIST, "r_dist", 0},
    {NSC_VAR | V_CDEL, "c_del", 0},
    {NSC_VAR | V_MDEL, "m_del", 0},
    {NSC_VAR | V_UDEL, "u_del", 0},
    {NSC_VAR | V_SDEL, "s_del", 0},
    {NSC_VAR | V_GDEL, "g_del", 0},
    {NSC_VAR | V_PDEL, "p_del", 0},
    {NSC_VAR | V_IDEL, "i_del", 0},
    {NSC_VAR | V_DDEL, "d_del", 0},
    {NSC_VAR | V_BDEL, "b_del", 0},
    {NSC_VAR | V_FDEL, "f_del", 0},
    {NSC_VAR | V_ODEL, "o_del", 0},
    {NSC_VAR | V_LDEL, "l_del", 0},
    {NSC_VAR | V_HDEL, "h_del", 0},
    {NSC_VAR | V_RDEL, "r_del", 0},
    {0, 0, 0}
};

struct castr sect_ca[] = {
    {NSC_UCHAR | fldoff(sctstr, sct_own), "owner", 0},
    {NSC_XCOORD | fldoff(sctstr, sct_x), "xloc", 0},
    {NSC_YCOORD | fldoff(sctstr, sct_y), "yloc", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_type), "des", 0},
    {NSC_ROUND | NSC_UCHAR | fldoff(sctstr, sct_effic), "effic", 0},
    {NSC_SHORT | fldoff(sctstr, sct_mobil), "mobil", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_terr), "terr", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_terr), "terr0", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_terr1), "terr1", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_terr2), "terr2", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_terr3), "terr3", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_work), "work", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_coastal), "coastal", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_newtype), "newdes", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_min), "min", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_gmin), "gold", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_fertil), "fert", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_oil), "ocontent", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_uran), "uran", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_oldown), "oldown", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_off), "off", 0},
    {NSC_XCOORD | fldoff(sctstr, sct_dist_x), "xdist", 0},
    {NSC_YCOORD | fldoff(sctstr, sct_dist_y), "ydist", 0},
    {NSC_SHORT | fldoff(sctstr, sct_avail), "avail", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_road), "road", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_rail), "rail", 0},
    {NSC_UCHAR | fldoff(sctstr, sct_defense), "dfense", 0},
    {NSC_TIME | fldoff(sctstr, sct_timestamp), "timestamp", 0},
    {0, 0, 0}
};

#define NSC_GENITEM \
{ NSC_UCHAR | fldoff(genitem, own), "owner", 0},\
{ NSC_SHORT | fldoff(genitem, uid), "uid", 0},\
{ NSC_XCOORD | fldoff(genitem, x), "xloc", 0},\
{ NSC_YCOORD | fldoff(genitem, y), "yloc", 0},\
{ NSC_UCHAR | fldoff(genitem, type), "type", 0},\
{ NSC_ROUND | NSC_UCHAR | fldoff(genitem, effic), "effic", 0},\
{ NSC_CHAR  | fldoff(genitem, mobil), "mobil", 0},\
{ NSC_SHORT | fldoff(genitem, sell), "sell", 0},\
{ NSC_ROUND | NSC_SHORT | fldoff(genitem, tech), "tech", 0},\
{ NSC_CHAR | fldoff(genitem, group), "group", 0},\
{ NSC_XCOORD | fldoff(genitem, opx), "opx", 0},\
{ NSC_YCOORD | fldoff(genitem, opy), "opy", 0},\
{ NSC_SHORT | fldoff(genitem, mission), "mission", 0}

struct castr genitem_ca[] = {
    NSC_GENITEM,
    {0, 0, 0}
};

struct castr ship_ca[] = {
    NSC_GENITEM,
    {NSC_CHAR | fldoff(shpstr, shp_fleet), "fleet", 0},
    {NSC_CHAR | fldoff(shpstr, shp_nplane), "nplane", 0},
    {NSC_TIME | fldoff(shpstr, shp_timestamp), "timestamp", 0},
    {NSC_UCHAR | fldoff(shpstr, shp_fuel), "fuel", 0},
    {NSC_UCHAR | fldoff(shpstr, shp_nxlight), "nxlight", 0},
    {NSC_UCHAR | fldoff(shpstr, shp_nchoppers), "nchoppers", 0},
    {NSC_UCHAR | fldoff(shpstr, shp_autonav), "autonav", 0},
    {0, 0, 0}
};

struct castr plane_ca[] = {
    NSC_GENITEM,
    {NSC_UCHAR | fldoff(plnstr, pln_wing), "wing", 0},
    {NSC_UCHAR | fldoff(plnstr, pln_range), "range", 0},
    {NSC_SHORT | fldoff(plnstr, pln_ship), "ship", 0},
    {NSC_UCHAR | fldoff(plnstr, pln_att), "att", 0},
    {NSC_UCHAR | fldoff(plnstr, pln_def), "def", 0},
    {NSC_UCHAR | fldoff(plnstr, pln_harden), "harden", 0},
    {NSC_UCHAR | fldoff(plnstr, pln_nuketype), "nuketype", 0},
    {NSC_UCHAR | fldoff(plnstr, pln_flags), "flags", 0},
    {NSC_SHORT | fldoff(plnstr, pln_land), "land", 0},
    {NSC_TIME | fldoff(plnstr, pln_timestamp), "timestamp", 0},
    {0, 0, 0}
};

struct castr land_ca[] = {
    NSC_GENITEM,
    {NSC_CHAR | fldoff(lndstr, lnd_army), "army", 0},
    {NSC_SHORT | fldoff(lndstr, lnd_ship), "ship", 0},
    {NSC_SHORT | fldoff(lndstr, lnd_land), "land", 0},
    {NSC_CHAR | fldoff(lndstr, lnd_harden), "harden", 0},
    {NSC_SHORT | fldoff(lndstr, lnd_retreat), "retreat", 0},
    {NSC_UCHAR | fldoff(lndstr, lnd_fuel), "fuel", 0},
    {NSC_UCHAR | fldoff(lndstr, lnd_nxlight), "nxlight", 0},
    {NSC_FLOAT | fldoff(lndstr, lnd_att), "att", 0},
    {NSC_FLOAT | fldoff(lndstr, lnd_def), "def", 0},
    {NSC_INT | fldoff(lndstr, lnd_vul), "vul", 0},
    {NSC_INT | fldoff(lndstr, lnd_spd), "spd", 0},
    {NSC_INT | fldoff(lndstr, lnd_vis), "vis", 0},
    {NSC_INT | fldoff(lndstr, lnd_spy), "spy", 0},
    {NSC_INT | fldoff(lndstr, lnd_rad), "rad", 0},
    {NSC_INT | fldoff(lndstr, lnd_frg), "frg", 0},
    {NSC_INT | fldoff(lndstr, lnd_acc), "acc", 0},
    {NSC_INT | fldoff(lndstr, lnd_dam), "dam", 0},
    {NSC_INT | fldoff(lndstr, lnd_ammo), "ammo", 0},
    {NSC_INT | fldoff(lndstr, lnd_aaf), "aaf", 0},
    {NSC_UCHAR | fldoff(lndstr, lnd_fuelc), "fuelc", 0},
    {NSC_UCHAR | fldoff(lndstr, lnd_fuelu), "fuelu", 0},
    {NSC_UCHAR | fldoff(lndstr, lnd_maxlight), "maxlight", 0},
    {NSC_TIME | fldoff(lndstr, lnd_timestamp), "timestamp", 0},
    {0, 0, 0}
};

struct castr nuke_ca[] = {
    {NSC_UCHAR | fldoff(nukstr, nuk_own), "owner", 0},
    {NSC_SHORT | fldoff(nukstr, nuk_uid), "uid", 0},
    {NSC_XCOORD | fldoff(nukstr, nuk_x), "xloc", 0},
    {NSC_YCOORD | fldoff(nukstr, nuk_y), "yloc", 0},
    {NSC_CHAR | fldoff(nukstr, nuk_n), "number", 0},
#if !defined(_WIN32)
    {NSC_CHAR | fldoff(nukstr, nuk_types[0]), "types", N_MAXNUKE},
#else
    {NSC_CHAR | fldoff(nukstr, nuk_types), "types", N_MAXNUKE},
#endif
    {NSC_TIME | fldoff(nukstr, nuk_timestamp), "timestamp", 0},
    {0, 0, 0}
};

struct castr treaty_ca[] = {
    {NSC_UCHAR | fldoff(trtstr, trt_cna), "cna", 0},
    {NSC_UCHAR | fldoff(trtstr, trt_cnb), "cnb", 0},
    {NSC_UCHAR | fldoff(trtstr, trt_status), "status", 0},
    {NSC_SHORT | fldoff(trtstr, trt_acond), "acond", 0},
    {NSC_SHORT | fldoff(trtstr, trt_bcond), "bcond", 0},
    {NSC_LONG | fldoff(trtstr, trt_exp), "exp", 0},
    {0, 0, 0}
};

struct castr loan_ca[] = {
    {NSC_UCHAR | fldoff(lonstr, l_loner), "loaner", 0},
    {NSC_SHORT | fldoff(lonstr, l_uid), "uid", 0},
    {NSC_UCHAR | fldoff(lonstr, l_lonee), "loanee", 0},
    {NSC_CHAR | fldoff(lonstr, l_status), "status", 0},
    {NSC_SHORT | fldoff(lonstr, l_sell), "sell", 0},
    {NSC_INT | fldoff(lonstr, l_irate), "irate", 0},
    {NSC_INT | fldoff(lonstr, l_ldur), "ldur", 0},
    {NSC_LONG | fldoff(lonstr, l_amtpaid), "amtpaid", 0},
    {NSC_LONG | fldoff(lonstr, l_amtdue), "amtdue", 0},
    {NSC_TIME | fldoff(lonstr, l_lastpay), "lastpay", 0},
    {NSC_TIME | fldoff(lonstr, l_duedate), "duedate", 0},
    {0, 0, 0}
};

struct castr news_ca[] = {
    {NSC_CHAR | fldoff(nwsstr, nws_ano), "actor", 0},
    {NSC_CHAR | fldoff(nwsstr, nws_vrb), "action", 0},
    {NSC_CHAR | fldoff(nwsstr, nws_vno), "victim", 0},
    {NSC_CHAR | fldoff(nwsstr, nws_ntm), "times", 0},
    {NSC_LONG | fldoff(nwsstr, nws_when), "time", 0},
    {0, 0, 0}
};

struct castr lost_ca[] = {
    {NSC_UCHAR | fldoff(loststr, lost_owner), "owner", 0},
    {NSC_INT | fldoff(loststr, lost_uid), "uid", 0},
    {NSC_UCHAR | fldoff(loststr, lost_type), "type", 0},
    {NSC_XCOORD | fldoff(loststr, lost_x), "x", 0},
    {NSC_YCOORD | fldoff(loststr, lost_y), "y", 0},
    {NSC_TIME | fldoff(loststr, lost_timestamp), "timestamp", 0},
    {0, 0, 0}
};

struct castr commodity_ca[] = {
    {NSC_UCHAR | fldoff(comstr, com_owner), "owner", 0},
    {NSC_SHORT | fldoff(comstr, com_uid), "uid", 0},
    {NSC_UCHAR | fldoff(comstr, com_type), "type", 0},
    {NSC_INT | fldoff(comstr, com_amount), "amount", 0},
    {NSC_FLOAT | fldoff(comstr, com_price), "price", 0},
    {NSC_INT | fldoff(comstr, com_maxbidder), "maxbidder", 0},
    {NSC_FLOAT | fldoff(comstr, com_maxprice), "maxprice", 0},
    {NSC_TIME | fldoff(comstr, com_markettime), "markettime", 0},
    {NSC_INT | fldoff(comstr, com_x), "xloc", 0},
    {NSC_INT | fldoff(comstr, com_y), "yloc", 0},
    {NSC_XCOORD | fldoff(comstr, sell_x), "xsell", 0},
    {NSC_YCOORD | fldoff(comstr, sell_y), "ysell", 0},
    {0, 0, 0}
};

struct castr trade_ca[] = {
    {NSC_UCHAR | fldoff(trdstr, trd_owner), "owner", 0},
    {NSC_SHORT | fldoff(trdstr, trd_uid), "uid", 0},
    {NSC_UCHAR | fldoff(trdstr, trd_type), "type", 0},
    {NSC_SHORT | fldoff(trdstr, trd_unitid), "unitid", 0},
    {NSC_LONG | fldoff(trdstr, trd_price), "price", 0},
    {NSC_INT | fldoff(trdstr, trd_maxbidder), "maxbidder", 0},
    {NSC_INT | fldoff(trdstr, trd_maxprice), "maxprice", 0},
    {NSC_TIME | fldoff(trdstr, trd_markettime), "markettime", 0},
    {NSC_XCOORD | fldoff(trdstr, trd_x), "xloc", 0},
    {NSC_YCOORD | fldoff(trdstr, trd_y), "yloc", 0},
    {0, 0, 0}
};

struct castr nat_ca[] = {
    {NSC_UCHAR | fldoff(natstr, nat_cnum), "cnum", 0},
#if !defined(_WIN32)
    {NSC_CHAR | fldoff(natstr, nat_cnam[0]), "cnam", 20},
    {NSC_DEITY | NSC_CHAR | fldoff(natstr, nat_pnam[0]), "pnam", 20},
#else
    {NSC_CHAR | fldoff(natstr, nat_cnam), "cnam", 20},
    {NSC_DEITY | NSC_CHAR | fldoff(natstr, nat_pnam), "pnam", 20},
#endif
    {NSC_XCOORD | fldoff(natstr, nat_xstart), "xstart", 0},
    {NSC_YCOORD | fldoff(natstr, nat_ystart), "ystart", 0},
    {NSC_XCOORD | fldoff(natstr, nat_xcap), "xcap", 0},
    {NSC_YCOORD | fldoff(natstr, nat_ycap), "ycap", 0},
    {NSC_DEITY | NSC_XCOORD | fldoff(natstr, nat_xorg), "xorg", 0},
    {NSC_DEITY | NSC_YCOORD | fldoff(natstr, nat_yorg), "yorg", 0},
#ifdef MAYBE_LATER
    {NSC_CHAR | fldoff(natstr, nat_stat), "stat", 0},
    {NSC_CHAR | fldoff(natstr, nat_dayno), "dayno", 0},
    {NSC_CHAR | fldoff(natstr, nat_update), "update", 0},
    {NSC_UCHAR | fldoff(natstr, nat_missed), "missed", 0},
#endif /* MAYBE_LATER */
    {NSC_USHORT | fldoff(natstr, nat_tgms), "tgms", 0},
    {NSC_USHORT | fldoff(natstr, nat_ann), "ann", 0},
    {NSC_USHORT | fldoff(natstr, nat_minused), "minused", 0},
    {NSC_SHORT | fldoff(natstr, nat_btu), "btu", 0},
    {NSC_LONG | fldoff(natstr, nat_reserve), "reserve", 0},
    {NSC_LONG | fldoff(natstr, nat_money), "money", 0},
#ifdef MAYBE_LATER
    {NSC_LONG | fldoff(natstr, nat_last_login), "last_login", 0},
    {NSC_LONG | fldoff(natstr, nat_last_logout), "last_logout", 0},
    {NSC_LONG | fldoff(natstr, nat_newstim), "newstim", 0},
#endif /* MAYBE_LATER */
#if !defined(_WIN32)
    {NSC_FLOAT | fldoff(natstr, nat_level[0]), "level", 4},
#else
    {NSC_FLOAT | fldoff(natstr, nat_level), "level", 4},
#endif
/* {NSC_SHORT | fldoff(natstr, nat_relate[0]),"relate",MAXNOC}, */
/* {NSC_CHAR | fldoff(natstr, nat_priorities[0]),"priorities",39}, */
/* {NSC_LONG | fldoff(natstr, nat_flags),"flags",0}, */
    {0, 0, 0}
};

struct castr map_ca[] = {
    {NSC_XCOORD | fldoff(mapstr, map_x), "x", 0},
    {NSC_YCOORD | fldoff(mapstr, map_y), "y", 0},
    {NSC_CHAR | fldoff(mapstr, map_des), "des", 0},
    {0, 0, 0}
};
