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
    {NSC_VAR | V_CIVIL, "civil", 0},
    {NSC_VAR | V_MILIT, "milit", 0},
    {NSC_VAR | V_SHELL, "shell", 0},
    {NSC_VAR | V_GUN, "gun", 0},
    {NSC_VAR | V_PETROL, "petrol", 0},
    {NSC_VAR | V_IRON, "iron", 0},
    {NSC_VAR | V_DUST, "dust", 0},
    {NSC_VAR | V_BAR, "bar", 0},
    {NSC_VAR | V_FOOD, "food", 0},
    {NSC_VAR | V_OIL, "oil", 0},
    {NSC_VAR | V_LCM, "lcm", 0},
    {NSC_VAR | V_HCM, "hcm", 0},
    {NSC_VAR | V_UW, "uw", 0},
    {NSC_VAR | V_RAD, "rad", 0},
    {0, 0, 0}
};

struct castr sect_ca[] = {
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_own), "owner", 0},
    {NSC_XCOORD | NSC_OFF | fldoff(sctstr, sct_x), "xloc", 0},
    {NSC_YCOORD | NSC_OFF | fldoff(sctstr, sct_y), "yloc", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_type), "des", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_effic), "effic", 0},
    {NSC_SHORT | NSC_OFF | fldoff(sctstr, sct_mobil), "mobil", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_terr), "terr", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_terr), "terr0", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_terr1), "terr1", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_terr2), "terr2", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_terr3), "terr3", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_work), "work", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_coastal), "coastal", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_newtype), "newdes", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_min), "min", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_gmin), "gold", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_fertil), "fert", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_oil), "ocontent", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_uran), "uran", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_oldown), "oldown", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_off), "off", 0},
    {NSC_XCOORD | NSC_OFF | fldoff(sctstr, sct_dist_x), "xdist", 0},
    {NSC_YCOORD | NSC_OFF | fldoff(sctstr, sct_dist_y), "ydist", 0},
    {NSC_SHORT | NSC_OFF | fldoff(sctstr, sct_avail), "avail", 0},
#define distoff(itype) (fldoff(sctstr, sct_dist) + (itype)*sizeof(u_short))
    {NSC_USHORT | NSC_OFF | distoff(I_CIVIL), "c_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_MILIT), "m_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_UW), "u_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_SHELL), "s_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_GUN), "g_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_PETROL), "p_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_IRON), "i_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_DUST), "d_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_BAR), "b_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_FOOD), "f_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_OIL), "o_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_LCM), "l_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_HCM), "h_dist", 0},
    {NSC_USHORT | NSC_OFF | distoff(I_RAD), "r_dist", 0},
#undef distoff
#define deloff(itype) (fldoff(sctstr, sct_del) + (itype)*sizeof(u_short))
    {NSC_USHORT | NSC_OFF | deloff(I_CIVIL), "c_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_MILIT), "m_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_UW), "u_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_SHELL), "s_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_GUN), "g_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_PETROL), "p_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_IRON), "i_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_DUST), "d_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_BAR), "b_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_FOOD), "f_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_OIL), "o_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_LCM), "l_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_HCM), "h_del", 0},
    {NSC_USHORT | NSC_OFF | deloff(I_RAD), "r_del", 0},
#undef deloff
    {NSC_DEITY | NSC_USHORT | NSC_OFF | fldoff(sctstr, sct_mines), "mines", 0},
    {NSC_DEITY | NSC_USHORT | NSC_OFF | fldoff(sctstr, sct_pstage), "pstage", 0},
    {NSC_DEITY | NSC_USHORT | NSC_OFF | fldoff(sctstr, sct_ptime), "ptime", 0},
    {NSC_DEITY | NSC_USHORT | NSC_OFF | fldoff(sctstr, sct_che), "che", 0},
    {NSC_USHORT | NSC_OFF | fldoff(sctstr, sct_fallout), "fallout", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_road), "road", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_rail), "rail", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(sctstr, sct_defense), "dfense", 0},
    {NSC_TIME | NSC_OFF | fldoff(sctstr, sct_timestamp), "timestamp", 0},
    {0, 0, 0}
};

#define NSC_GENITEM \
{ NSC_UCHAR | NSC_OFF | fldoff(genitem, own), "owner", 0},\
{ NSC_SHORT | NSC_OFF | fldoff(genitem, uid), "uid", 0},\
{ NSC_XCOORD | NSC_OFF | fldoff(genitem, x), "xloc", 0},\
{ NSC_YCOORD | NSC_OFF | fldoff(genitem, y), "yloc", 0},\
{ NSC_UCHAR | NSC_OFF | fldoff(genitem, type), "type", 0},\
{ NSC_UCHAR | NSC_OFF | fldoff(genitem, effic), "effic", 0},\
{ NSC_CHAR  | NSC_OFF | fldoff(genitem, mobil), "mobil", 0},\
{ NSC_SHORT | NSC_OFF | fldoff(genitem, tech), "tech", 0},\
{ NSC_CHAR | NSC_OFF | fldoff(genitem, group), "group", 0},\
{ NSC_XCOORD | NSC_OFF | fldoff(genitem, opx), "opx", 0},\
{ NSC_YCOORD | NSC_OFF | fldoff(genitem, opy), "opy", 0},\
{ NSC_SHORT | NSC_OFF | fldoff(genitem, mission), "mission", 0}

struct castr genitem_ca[] = {
    NSC_GENITEM,
    {0, 0, 0}
};

struct castr ship_ca[] = {
    NSC_GENITEM,
    {NSC_CHAR | NSC_OFF | fldoff(shpstr, shp_fleet), "fleet", 0},
    {NSC_CHAR | NSC_OFF | fldoff(shpstr, shp_nplane), "nplane", 0},
    {NSC_TIME | NSC_OFF | fldoff(shpstr, shp_timestamp), "timestamp", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(shpstr, shp_fuel), "fuel", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(shpstr, shp_nxlight), "nxlight", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(shpstr, shp_nchoppers), "nchoppers", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(shpstr, shp_autonav), "autonav", 0},
    {NSC_DEITY | NSC_USHORT | NSC_OFF | fldoff(shpstr, shp_pstage), "pstage", 0},
    {NSC_DEITY | NSC_USHORT | NSC_OFF | fldoff(shpstr, shp_ptime), "ptime", 0},
    {0, 0, 0}
};

struct castr plane_ca[] = {
    NSC_GENITEM,
    {NSC_UCHAR | NSC_OFF | fldoff(plnstr, pln_wing), "wing", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(plnstr, pln_range), "range", 0},
    {NSC_SHORT | NSC_OFF | fldoff(plnstr, pln_ship), "ship", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(plnstr, pln_att), "att", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(plnstr, pln_def), "def", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(plnstr, pln_harden), "harden", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(plnstr, pln_nuketype), "nuketype", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(plnstr, pln_flags), "flags", 0},
    {NSC_SHORT | NSC_OFF | fldoff(plnstr, pln_land), "land", 0},
    {NSC_TIME | NSC_OFF | fldoff(plnstr, pln_timestamp), "timestamp", 0},
    {0, 0, 0}
};

struct castr land_ca[] = {
    NSC_GENITEM,
    {NSC_CHAR | NSC_OFF | fldoff(lndstr, lnd_army), "army", 0},
    {NSC_SHORT | NSC_OFF | fldoff(lndstr, lnd_ship), "ship", 0},
    {NSC_SHORT | NSC_OFF | fldoff(lndstr, lnd_land), "land", 0},
    {NSC_CHAR | NSC_OFF | fldoff(lndstr, lnd_harden), "harden", 0},
    {NSC_SHORT | NSC_OFF | fldoff(lndstr, lnd_retreat), "retreat", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(lndstr, lnd_fuel), "fuel", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(lndstr, lnd_nxlight), "nxlight", 0},
    {NSC_DEITY | NSC_USHORT | NSC_OFF | fldoff(lndstr, lnd_pstage), "pstage", 0},
    {NSC_DEITY | NSC_USHORT | NSC_OFF | fldoff(lndstr, lnd_ptime), "ptime", 0},
    {NSC_FLOAT | NSC_OFF | fldoff(lndstr, lnd_att), "att", 0},
    {NSC_FLOAT | NSC_OFF | fldoff(lndstr, lnd_def), "def", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_vul), "vul", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_spd), "spd", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_vis), "vis", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_spy), "spy", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_rad), "rad", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_frg), "frg", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_acc), "acc", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_dam), "dam", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_ammo), "ammo", 0},
    {NSC_INT | NSC_OFF | fldoff(lndstr, lnd_aaf), "aaf", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(lndstr, lnd_fuelc), "fuelc", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(lndstr, lnd_fuelu), "fuelu", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(lndstr, lnd_maxlight), "maxlight", 0},
    {NSC_TIME | NSC_OFF | fldoff(lndstr, lnd_timestamp), "timestamp", 0},
    {0, 0, 0}
};

struct castr nuke_ca[] = {
    {NSC_UCHAR | NSC_OFF | fldoff(nukstr, nuk_own), "owner", 0},
    {NSC_SHORT | NSC_OFF | fldoff(nukstr, nuk_uid), "uid", 0},
    {NSC_XCOORD | NSC_OFF | fldoff(nukstr, nuk_x), "xloc", 0},
    {NSC_YCOORD | NSC_OFF | fldoff(nukstr, nuk_y), "yloc", 0},
    {NSC_CHAR | NSC_OFF | fldoff(nukstr, nuk_n), "number", 0},
#if !defined(_WIN32)
    {NSC_CHAR | NSC_OFF | fldoff(nukstr, nuk_types[0]), "types", N_MAXNUKE},
#else
    {NSC_CHAR | NSC_OFF | fldoff(nukstr, nuk_types), "types", N_MAXNUKE},
#endif
    {NSC_TIME | NSC_OFF | fldoff(nukstr, nuk_timestamp), "timestamp", 0},
    {0, 0, 0}
};

struct castr treaty_ca[] = {
    {NSC_UCHAR | NSC_OFF | fldoff(trtstr, trt_cna), "cna", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(trtstr, trt_cnb), "cnb", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(trtstr, trt_status), "status", 0},
    {NSC_SHORT | NSC_OFF | fldoff(trtstr, trt_acond), "acond", 0},
    {NSC_SHORT | NSC_OFF | fldoff(trtstr, trt_bcond), "bcond", 0},
    {NSC_LONG | NSC_OFF | fldoff(trtstr, trt_exp), "exp", 0},
    {0, 0, 0}
};

struct castr loan_ca[] = {
    {NSC_UCHAR | NSC_OFF | fldoff(lonstr, l_loner), "loaner", 0},
    {NSC_SHORT | NSC_OFF | fldoff(lonstr, l_uid), "uid", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(lonstr, l_lonee), "loanee", 0},
    {NSC_CHAR | NSC_OFF | fldoff(lonstr, l_status), "status", 0},
    {NSC_SHORT | NSC_OFF | fldoff(lonstr, l_sell), "sell", 0},
    {NSC_INT | NSC_OFF | fldoff(lonstr, l_irate), "irate", 0},
    {NSC_INT | NSC_OFF | fldoff(lonstr, l_ldur), "ldur", 0},
    {NSC_LONG | NSC_OFF | fldoff(lonstr, l_amtpaid), "amtpaid", 0},
    {NSC_LONG | NSC_OFF | fldoff(lonstr, l_amtdue), "amtdue", 0},
    {NSC_TIME | NSC_OFF | fldoff(lonstr, l_lastpay), "lastpay", 0},
    {NSC_TIME | NSC_OFF | fldoff(lonstr, l_duedate), "duedate", 0},
    {0, 0, 0}
};

struct castr news_ca[] = {
    {NSC_CHAR | NSC_OFF | fldoff(nwsstr, nws_ano), "actor", 0},
    {NSC_CHAR | NSC_OFF | fldoff(nwsstr, nws_vrb), "action", 0},
    {NSC_CHAR | NSC_OFF | fldoff(nwsstr, nws_vno), "victim", 0},
    {NSC_CHAR | NSC_OFF | fldoff(nwsstr, nws_ntm), "times", 0},
    {NSC_LONG | NSC_OFF | fldoff(nwsstr, nws_when), "time", 0},
    {0, 0, 0}
};

struct castr lost_ca[] = {
    {NSC_UCHAR | NSC_OFF | fldoff(loststr, lost_owner), "owner", 0},
    {NSC_INT | NSC_OFF | fldoff(loststr, lost_uid), "uid", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(loststr, lost_type), "type", 0},
    {NSC_XCOORD | NSC_OFF | fldoff(loststr, lost_x), "x", 0},
    {NSC_YCOORD | NSC_OFF | fldoff(loststr, lost_y), "y", 0},
    {NSC_TIME | NSC_OFF | fldoff(loststr, lost_timestamp), "timestamp", 0},
    {0, 0, 0}
};

struct castr commodity_ca[] = {
    {NSC_UCHAR | NSC_OFF | fldoff(comstr, com_owner), "owner", 0},
    {NSC_SHORT | NSC_OFF | fldoff(comstr, com_uid), "uid", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(comstr, com_type), "type", 0},
    {NSC_INT | NSC_OFF | fldoff(comstr, com_amount), "amount", 0},
    {NSC_INT | NSC_OFF | fldoff(comstr, com_maxbidder), "maxbidder", 0},
    {NSC_TIME | NSC_OFF | fldoff(comstr, com_markettime), "markettime", 0},
    /* could let maxbidder access these, but we can't express that yet: */
    {NSC_DEITY | NSC_INT | NSC_OFF | fldoff(comstr, com_x), "xbuy", 0},
    {NSC_DEITY | NSC_INT | NSC_OFF | fldoff(comstr, com_y), "ybuy", 0},
    /* could let the owner access these, but we can't express that yet: */
    {NSC_DEITY | NSC_XCOORD | NSC_OFF | fldoff(comstr, sell_x), "xsell", 0},
    {NSC_DEITY | NSC_YCOORD | NSC_OFF | fldoff(comstr, sell_y), "ysell", 0},
    {NSC_DEITY | NSC_FLOAT | NSC_OFF | fldoff(comstr, com_price), "price", 0},
    {0, 0, 0}
};

struct castr trade_ca[] = {
    {NSC_UCHAR | NSC_OFF | fldoff(trdstr, trd_owner), "owner", 0},
    {NSC_SHORT | NSC_OFF | fldoff(trdstr, trd_uid), "uid", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(trdstr, trd_type), "type", 0},
    {NSC_SHORT | NSC_OFF | fldoff(trdstr, trd_unitid), "unitid", 0},
    {NSC_LONG | NSC_OFF | fldoff(trdstr, trd_price), "price", 0},
    {NSC_INT | NSC_OFF | fldoff(trdstr, trd_maxbidder), "maxbidder", 0},
    {NSC_TIME | NSC_OFF | fldoff(trdstr, trd_markettime), "markettime", 0},
    /* could let the owner access these, but we can't express that yet: */
    {NSC_DEITY | NSC_XCOORD | NSC_OFF | fldoff(trdstr, trd_x), "xloc", 0},
    {NSC_DEITY | NSC_YCOORD | NSC_OFF | fldoff(trdstr, trd_y), "yloc", 0},
    {0, 0, 0}
};

struct castr nat_ca[] = {
    {NSC_UCHAR | NSC_OFF | fldoff(natstr, nat_cnum), "cnum", 0},
#if !defined(_WIN32)
    {NSC_CHAR | NSC_OFF | fldoff(natstr, nat_cnam[0]), "cnam", 20},
    {NSC_DEITY | NSC_CHAR | NSC_OFF | fldoff(natstr, nat_pnam[0]), "pnam", 20},
#else
    {NSC_CHAR | NSC_OFF | fldoff(natstr, nat_cnam), "cnam", 20},
    {NSC_DEITY | NSC_CHAR | NSC_OFF | fldoff(natstr, nat_pnam), "pnam", 20},
#endif
    {NSC_XCOORD | NSC_OFF | fldoff(natstr, nat_xstart), "xstart", 0},
    {NSC_YCOORD | NSC_OFF | fldoff(natstr, nat_ystart), "ystart", 0},
    {NSC_XCOORD | NSC_OFF | fldoff(natstr, nat_xcap), "xcap", 0},
    {NSC_YCOORD | NSC_OFF | fldoff(natstr, nat_ycap), "ycap", 0},
    {NSC_DEITY | NSC_XCOORD | NSC_OFF | fldoff(natstr, nat_xorg), "xorg", 0},
    {NSC_DEITY | NSC_YCOORD | NSC_OFF | fldoff(natstr, nat_yorg), "yorg", 0},
#ifdef MAYBE_LATER
    {NSC_CHAR | NSC_OFF | fldoff(natstr, nat_stat), "stat", 0},
    {NSC_CHAR | NSC_OFF | fldoff(natstr, nat_dayno), "dayno", 0},
    {NSC_CHAR | NSC_OFF | fldoff(natstr, nat_update), "update", 0},
    {NSC_UCHAR | NSC_OFF | fldoff(natstr, nat_missed), "missed", 0},
#endif /* MAYBE_LATER */
    {NSC_USHORT | NSC_OFF | fldoff(natstr, nat_tgms), "tgms", 0},
    {NSC_USHORT | NSC_OFF | fldoff(natstr, nat_ann), "ann", 0},
    {NSC_USHORT | NSC_OFF | fldoff(natstr, nat_minused), "minused", 0},
    {NSC_SHORT | NSC_OFF | fldoff(natstr, nat_btu), "btu", 0},
    {NSC_LONG | NSC_OFF | fldoff(natstr, nat_reserve), "reserve", 0},
    {NSC_LONG | NSC_OFF | fldoff(natstr, nat_money), "money", 0},
#ifdef MAYBE_LATER
    {NSC_LONG | NSC_OFF | fldoff(natstr, nat_last_login), "last_login", 0},
    {NSC_LONG | NSC_OFF | fldoff(natstr, nat_last_logout), "last_logout", 0},
    {NSC_LONG | NSC_OFF | fldoff(natstr, nat_newstim), "newstim", 0},
#endif /* MAYBE_LATER */
#if !defined(_WIN32)
    {NSC_FLOAT | NSC_OFF | fldoff(natstr, nat_level[0]), "level", 4},
#else
    {NSC_FLOAT | NSC_OFF | fldoff(natstr, nat_level), "level", 4},
#endif
/* {NSC_SHORT | NSC_OFF | fldoff(natstr, nat_relate[0]),"relate",MAXNOC}, */
/* {NSC_CHAR | NSC_OFF | fldoff(natstr, nat_priorities[0]),"priorities",39}, */
/* {NSC_LONG | NSC_OFF | fldoff(natstr, nat_flags),"flags",0}, */
    {0, 0, 0}
};
