/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 *     Markus Armbruster, 2004-2016
 */

/*
 * Convention: uid selector comes first.  Table TYPE has an uid
 * selector if empfile[TYPE].cadef[0].ca_table == TYPE.
 */

#include <config.h>

#include <limits.h>
#include <stdlib.h>
#include "empobj.h"
#include "optlist.h"
#include "news.h"
#include "nsc.h"
#include "product.h"
#include "unit.h"
#include "version.h"

static void *nsc_ver(struct valstr *, struct natstr *, void *);
static void *nsc_ver_maxnoc(struct valstr *, struct natstr *, void *);
static void *nsc_ver_version(struct valstr *, struct natstr *, void *);
static void *nsc_sct_terr(struct valstr *, struct natstr *, void *);
static void *nsc_sct_track(struct valstr *, struct natstr *, void *);
static void *nsc_cargo_nplane(struct valstr *, struct natstr *, void *);
static void *nsc_cargo_nchopper(struct valstr *, struct natstr *, void *);
static void *nsc_cargo_nxlight(struct valstr *, struct natstr *, void *);
static void *nsc_cargo_nland(struct valstr *, struct natstr *, void *);
static void *nsc_pln_att(struct valstr *, struct natstr *, void *);
static void *nsc_pln_def(struct valstr *, struct natstr *, void *);
static void *nsc_pln_nuketype(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_att(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_def(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_vul(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_spd(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_vis(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_frg(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_acc(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_dam(struct valstr *, struct natstr *, void *);
static void *nsc_lnd_aaf(struct valstr *, struct natstr *, void *);
static void *nsc_lchr(struct valstr *, struct natstr *, void *);
static void *nsc_nws_timestamp(struct valstr *, struct natstr *, void *);
static void *nsc_meta_type(struct valstr *, struct natstr *, void *);
static void *nsc_meta_len(struct valstr *, struct natstr *, void *);

/* Ugly hacks to avoid illegibly long lines */
#define fldoff(fld) offsetof(CURSTR, fld)
#define empobjoff(fld) offsetof(struct empobj, fld)

#define NSC_IELT(name, pfx, sfx, base, itype)	\
    {sizeof(sfx) == 1 ? name : pfx sfx,		\
     ((base) + (itype)*sizeof(short)),		\
     NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP}

#define NSC_IVEC(base, sfx)			\
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

#define NSC_MELT(name, base, itype, flags, dump)	\
    {(name), ((base) + (itype)*sizeof(short)),		\
     NSC_SHORT, 0, NULL, EF_BAD, (flags), (dump) }

#define NSC_MVEC(base, mil_dump, oil_dump, rad_dump)	\
NSC_MELT("c_build", base, I_CIVIL, 0, CA_DUMP_ONLY),	\
NSC_MELT("m_build", base, I_MILIT, 0, mil_dump),	\
NSC_MELT("s_build", base, I_SHELL, 0, CA_DUMP_ONLY),	\
NSC_MELT("g_build", base, I_GUN, 0, CA_DUMP_ONLY),	\
NSC_MELT("p_build", base, I_PETROL, 0, CA_DUMP_ONLY),	\
NSC_MELT("i_build", base, I_IRON, 0, CA_DUMP_ONLY),	\
NSC_MELT("d_build", base, I_DUST, 0, CA_DUMP_ONLY),	\
NSC_MELT("b_build", base, I_BAR, 0, CA_DUMP_ONLY),	\
NSC_MELT("f_build", base, I_FOOD, 0, CA_DUMP_ONLY),	\
NSC_MELT("o_build", base, I_OIL, 0, oil_dump),		\
NSC_MELT("l_build", base, I_LCM, 0, CA_DUMP),		\
NSC_MELT("h_build", base, I_HCM, 0, CA_DUMP),		\
NSC_MELT("u_build", base, I_UW, 0, CA_DUMP_ONLY),	\
NSC_MELT("r_build", base, I_RAD, 0, rad_dump)

struct castr ichr_ca[] = {
#define CURSTR struct ichrstr
    {"uid", fldoff(i_uid), NSC_SITYPE(i_type), 0, NULL, EF_ITEM, 0,
     CA_DUMP},
    {"name", fldoff(i_name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"mnem", fldoff(i_mnem), NSC_STRINGY, 1, NULL, EF_BAD, 0,
     CA_DUMP_CONST},
    {"power", fldoff(i_power), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"sell", fldoff(i_sell), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"lbs", fldoff(i_lbs), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"pkg", fldoff(i_pkg), NSC_INT, NUMPKG, NULL, EF_BAD, 0, CA_DUMP},
    {"melt_denom", fldoff(i_melt_denom), NSC_INT, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr pchr_ca[] = {
#define CURSTR struct pchrstr
    {"uid", fldoff(p_uid), NSC_INT, 0, NULL, EF_PRODUCT, 0, CA_DUMP},
    {"name", fldoff(p_name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"sname", fldoff(p_sname), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"ctype", fldoff(p_ctype), NSC_SITYPE(i_type), MAXPRCON, NULL,
     EF_ITEM, 0, CA_DUMP},
    {"camt", fldoff(p_camt), NSC_USHORT, MAXPRCON, NULL, EF_BAD, 0,
     CA_DUMP},
    {"type", fldoff(p_type), NSC_SITYPE(i_type), 0, NULL, EF_ITEM, 0,
     CA_DUMP},
    {"level", fldoff(p_level), NSC_INT, 0, NULL, EF_LEVEL, 0, CA_DUMP},
    {"cost", fldoff(p_cost), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"nrndx", fldoff(p_nrndx), NSC_INT, 0, NULL, EF_RESOURCES, 0, CA_DUMP},
    {"nrdep", fldoff(p_nrdep), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"nlndx", fldoff(p_nlndx), NSC_INT, 0, NULL, EF_LEVEL, 0, CA_DUMP},
    {"nlmin", fldoff(p_nlmin), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"nllag", fldoff(p_nllag), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr sect_ca[] = {
#define CURSTR struct sctstr
    /* uid needs to be NSC_DEITY because it discloses true origin */
    {"uid", fldoff(sct_uid), NSC_INT, 0, NULL,
     EF_SECTOR, NSC_DEITY, CA_DUMP_NONE},
    {"timestamp", fldoff(sct_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    {"owner", fldoff(sct_own), NSC_NATID, 0, NULL, EF_NATION, 0, CA_DUMP},
    {"xloc", fldoff(sct_x), NSC_XCOORD, 0, NULL, EF_BAD, 0, CA_DUMP_CONST},
    {"yloc", fldoff(sct_y), NSC_YCOORD, 0, NULL, EF_BAD, 0, CA_DUMP_CONST},
    {"des", fldoff(sct_type), NSC_CHAR, 0, NULL, EF_SECTOR_CHR, 0,
     CA_DUMP},
    {"effic", fldoff(sct_effic), NSC_CHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"mobil", fldoff(sct_mobil), NSC_CHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"off", fldoff(sct_off), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"loyal", fldoff(sct_loyal), NSC_UCHAR, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"terr", 0, NSC_UCHAR, 0, nsc_sct_terr, EF_BAD, 0, CA_DUMP_NONE},
    {"terr0", fldoff(sct_terr), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"terr1", fldoff(sct_terr1), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"terr2", fldoff(sct_terr2), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"terr3", fldoff(sct_terr3), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"dterr", fldoff(sct_dterr), NSC_UCHAR, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"xdist", fldoff(sct_dist_x), NSC_XCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"ydist", fldoff(sct_dist_y), NSC_YCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"avail", fldoff(sct_avail), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"elev", fldoff(sct_elev), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"work", fldoff(sct_work), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"coastal", fldoff(sct_coastal), NSC_UCHAR, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"newdes", fldoff(sct_newtype), NSC_CHAR, 0, NULL, EF_SECTOR_CHR, 0,
     CA_DUMP},
    {"min", fldoff(sct_min), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"gold", fldoff(sct_gmin), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"fert", fldoff(sct_fertil), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"ocontent", fldoff(sct_oil), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"uran", fldoff(sct_uran), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"oldown", fldoff(sct_oldown), NSC_NATID, 0, NULL, EF_NATION, 0,
     CA_DUMP},
    {"track", 0, NSC_LONG, 0, nsc_sct_track, EF_BAD, 0, CA_DUMP_NONE},
    NSC_IVEC(fldoff(sct_item), ""),
    NSC_IVEC(fldoff(sct_dist), "_dist"),
    NSC_IVEC(fldoff(sct_del), "_del"),
    /* should let old owner access mines, but can't express that: */
    {"mines", fldoff(sct_mines), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"pstage", fldoff(sct_pstage), NSC_SHORT, 0, NULL,
     EF_PLAGUE_STAGES, NSC_DEITY, CA_DUMP},
    {"ptime", fldoff(sct_ptime), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"che", fldoff(sct_che), NSC_UCHAR, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"che_target", fldoff(sct_che_target), NSC_NATID, 0, NULL,
     EF_NATION, NSC_DEITY, CA_DUMP},
    {"fallout", fldoff(sct_fallout), NSC_USHORT, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"access", fldoff(sct_access), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"road", fldoff(sct_road), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"rail", fldoff(sct_rail), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"dfense", fldoff(sct_defense), NSC_UCHAR, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr dchr_ca[] = {
#define CURSTR struct dchrstr
    {"uid", fldoff(d_uid), NSC_UCHAR, 0, NULL, EF_SECTOR_CHR, 0, CA_DUMP},
    {"name", fldoff(d_name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"mnem", fldoff(d_mnem), NSC_STRINGY, 1, NULL, EF_BAD, 0,
     CA_DUMP_CONST},
    {"terrain", fldoff(d_terrain), NSC_UCHAR, 0, NULL, EF_SECTOR_CHR, 0,
     CA_DUMP},
    {"prd", fldoff(d_prd), NSC_INT, 0, NULL, EF_PRODUCT, 0, CA_DUMP},
    {"peffic", fldoff(d_peffic), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"mob0", fldoff(d_mob0), NSC_FLOAT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"mob1", fldoff(d_mob1), NSC_FLOAT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"nav", fldoff(d_nav), NSC_SITYPE(enum d_navigation), 0, NULL,
     EF_SECTOR_NAVIGATION, 0, CA_DUMP},
    {"pkg", fldoff(d_pkg), NSC_SITYPE(enum i_packing), 0, NULL,
     EF_PACKING, 0, CA_DUMP},
    {"ostr", fldoff(d_ostr), NSC_FLOAT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"dstr", fldoff(d_dstr), NSC_FLOAT, 0, NULL, EF_BAD, 0, CA_DUMP},
    NSC_MVEC(fldoff(d_mat), CA_DUMP_ONLY, CA_DUMP_ONLY, CA_DUMP_ONLY),
    {"bwork", fldoff(d_bwork), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"cost", fldoff(d_cost), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"maint", fldoff(d_maint), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"maxpop", fldoff(d_maxpop), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"flags", fldoff(d_flags), NSC_INT, 0, NULL,
     EF_SECTOR_CHR_FLAGS, NSC_BITS, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

#define NSC_GENITEM(ef_type, ef_chr)					\
    {"uid", empobjoff(uid), NSC_INT, 0, NULL, ef_type, 0, CA_DUMP},	\
    {"timestamp", empobjoff(timestamp), NSC_TIME, 0, NULL, EF_BAD, 0,	\
     CA_DUMP_NONE},							\
    {"owner", empobjoff(own), NSC_NATID, 0, NULL, EF_NATION, 0, CA_DUMP}, \
    {"xloc", empobjoff(x), NSC_XCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},	\
    {"yloc", empobjoff(y), NSC_YCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},	\
    {"type", empobjoff(type), NSC_CHAR, 0, NULL, ef_chr, 0, CA_DUMP},	\
    {"effic", empobjoff(effic), NSC_CHAR, 0, NULL, EF_BAD, 0, CA_DUMP},	\
    {"mobil", empobjoff(mobil), NSC_CHAR , 0, NULL, EF_BAD, 0, CA_DUMP}, \
    {"off", empobjoff(off), NSC_UCHAR , 0, NULL, EF_BAD, 0, CA_DUMP},	\
    {"tech", empobjoff(tech), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},	\
    {"group", empobjoff(group), NSC_STRINGY, 1, NULL, EF_BAD, 0,	\
     CA_DUMP_NONE},							\
    {"opx", empobjoff(opx), NSC_XCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},	\
    {"opy", empobjoff(opy), NSC_YCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},	\
    {"mission", empobjoff(mission), NSC_SHORT, 0, NULL, EF_MISSIONS, 0,	\
     CA_DUMP},								\
    {"radius", empobjoff(radius), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP}

struct castr ship_ca[] = {
#define CURSTR struct shpstr
    NSC_GENITEM(EF_SHIP, EF_SHIP_CHR),
    {"fleet", fldoff(shp_fleet), NSC_STRINGY, 1, NULL, EF_BAD, 0, CA_DUMP},
    NSC_IVEC(fldoff(shp_item), ""),
    {"pstage", fldoff(shp_pstage), NSC_SHORT, 0, NULL,
     EF_PLAGUE_STAGES, NSC_DEITY, CA_DUMP},
    {"ptime", fldoff(shp_ptime), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"access", fldoff(shp_access), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"name", fldoff(shp_name), NSC_STRINGY, MAXSHPNAMLEN, NULL,
     EF_BAD, 0, CA_DUMP},
    /* should let builder access xbuilt, ybuilt, but can't express that: */
    {"xbuilt", fldoff(shp_orig_x), NSC_XCOORD, 0, NULL,
     EF_BAD, NSC_DEITY, CA_DUMP},
    {"ybuilt", fldoff(shp_orig_y), NSC_YCOORD, 0, NULL,
     EF_BAD, NSC_DEITY, CA_DUMP},
    {"builder", fldoff(shp_orig_own), NSC_NATID, 0, NULL,
     EF_NATION, NSC_DEITY, CA_DUMP},
    {"rflags", fldoff(shp_rflags), NSC_INT, 0, NULL,
     EF_RETREAT_FLAGS, NSC_BITS, CA_DUMP},
    {"rpath", fldoff(shp_rpath), NSC_STRINGY, RET_LEN, NULL, EF_BAD, 0,
     CA_DUMP},
    {"nplane", 0, NSC_LONG, 0, nsc_cargo_nplane, EF_BAD, 0, CA_DUMP_NONE},
    {"nchoppers", 0, NSC_LONG, 0, nsc_cargo_nchopper, EF_BAD, 0,
     CA_DUMP_NONE},
    {"nxlight", 0, NSC_LONG, 0, nsc_cargo_nxlight, EF_BAD, 0,
     CA_DUMP_NONE},
    {"nland", 0, NSC_LONG, 0, nsc_cargo_nland, EF_BAD, 0, CA_DUMP_NONE},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr mchr_ca[] = {
#define CURSTR struct mchrstr
    {"type", fldoff(m_type), NSC_CHAR, 0, NULL, EF_SHIP_CHR, 0, CA_DUMP},
    {"name", fldoff(m_name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP},
    NSC_IVEC(fldoff(m_item), ""),
    NSC_MVEC(fldoff(m_mat), CA_DUMP_ONLY, CA_DUMP_ONLY, CA_DUMP_ONLY),
    {"armor", fldoff(m_armor), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"speed", fldoff(m_speed), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"visib", fldoff(m_visib), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"vrnge", fldoff(m_vrnge), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"frnge", fldoff(m_frnge), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"glim", fldoff(m_glim), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"nxlight", fldoff(m_nxlight), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"nchoppers", fldoff(m_nchoppers), NSC_UCHAR, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"bwork", fldoff(m_bwork), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"tech", fldoff(m_tech), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"cost", fldoff(m_cost), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"flags", fldoff(m_flags), NSC_INT, 0, NULL,
     EF_SHIP_CHR_FLAGS, NSC_BITS, CA_DUMP},
    {"nplanes", fldoff(m_nplanes), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"nland", fldoff(m_nland), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr plane_ca[] = {
#define CURSTR struct plnstr
    NSC_GENITEM(EF_PLANE, EF_PLANE_CHR),
    {"wing", fldoff(pln_wing), NSC_STRINGY, 1, NULL, EF_BAD, 0, CA_DUMP},
    {"range", fldoff(pln_range), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"harden", fldoff(pln_harden), NSC_CHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"ship", fldoff(pln_ship), NSC_INT, 0, NULL, EF_SHIP, 0, CA_DUMP},
    {"land", fldoff(pln_land), NSC_INT, 0, NULL, EF_LAND, 0, CA_DUMP},
    {"flags", fldoff(pln_flags), NSC_INT, 0, NULL,
     EF_PLANE_FLAGS, NSC_BITS, CA_DUMP},
    {"access", fldoff(pln_access), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"theta", fldoff(pln_theta), NSC_FLOAT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"att", 0, NSC_LONG, 0, nsc_pln_att, EF_BAD, 0, CA_DUMP_NONE},
    {"def", 0, NSC_LONG, 0, nsc_pln_def, EF_BAD, 0, CA_DUMP_NONE},
    {"nuketype", 0, NSC_LONG, 0, nsc_pln_nuketype, EF_BAD, 0,
     CA_DUMP_NONE},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr plchr_ca[] = {
#define CURSTR struct plchrstr
    {"type", fldoff(pl_type), NSC_CHAR, 0, NULL, EF_PLANE_CHR, 0, CA_DUMP},
    {"name", fldoff(pl_name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP},
    NSC_MVEC(fldoff(pl_mat), CA_DUMP, CA_DUMP_ONLY, CA_DUMP_ONLY),
    {"bwork", fldoff(pl_bwork), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"tech", fldoff(pl_tech), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"cost", fldoff(pl_cost), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"acc", fldoff(pl_acc), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"load", fldoff(pl_load), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"att", fldoff(pl_att), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"def", fldoff(pl_def), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"range", fldoff(pl_range), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"fuel", fldoff(pl_fuel), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"stealth", fldoff(pl_stealth), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"flags", fldoff(pl_flags), NSC_INT, 0, NULL,
     EF_PLANE_CHR_FLAGS, NSC_BITS, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr land_ca[] = {
#define CURSTR struct lndstr
    NSC_GENITEM(EF_LAND, EF_LAND_CHR),
    {"army", fldoff(lnd_army), NSC_STRINGY, 1, NULL, EF_BAD, 0, CA_DUMP},
    {"ship", fldoff(lnd_ship), NSC_INT, 0, NULL, EF_SHIP, 0, CA_DUMP},
    {"harden", fldoff(lnd_harden), NSC_CHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"retreat", fldoff(lnd_retreat), NSC_SHORT, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"rflags", fldoff(lnd_rflags), NSC_INT, 0, NULL,
     EF_RETREAT_FLAGS, NSC_BITS, CA_DUMP},
    {"rpath", fldoff(lnd_rpath), NSC_STRINGY, RET_LEN, NULL, EF_BAD, 0,
     CA_DUMP},
    NSC_IVEC(fldoff(lnd_item), ""),
    {"pstage", fldoff(lnd_pstage), NSC_SHORT, 0, NULL,
     EF_PLAGUE_STAGES, NSC_DEITY, CA_DUMP},
    {"ptime", fldoff(lnd_ptime), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"land", fldoff(lnd_land), NSC_INT, 0, NULL, EF_LAND, 0, CA_DUMP},
    {"access", fldoff(lnd_access), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"att", 0, NSC_DOUBLE, 0, nsc_lnd_att, EF_BAD, 0, CA_DUMP_NONE},
    {"def", 0, NSC_DOUBLE, 0, nsc_lnd_def, EF_BAD, 0, CA_DUMP_NONE},
    {"vul", 0, NSC_LONG, 0, nsc_lnd_vul, EF_BAD, 0, CA_DUMP_NONE},
    {"spd", 0, NSC_LONG, 0, nsc_lnd_spd, EF_BAD, 0, CA_DUMP_NONE},
    {"vis", 0, NSC_LONG, 0, nsc_lnd_vis, EF_BAD, 0, CA_DUMP_NONE},
    {"frg", 0, NSC_LONG, 0, nsc_lnd_frg, EF_BAD, 0, CA_DUMP_NONE},
    {"acc", 0, NSC_LONG, 0, nsc_lnd_acc, EF_BAD, 0, CA_DUMP_NONE},
    {"dam", 0, NSC_LONG, 0, nsc_lnd_dam, EF_BAD, 0, CA_DUMP_NONE},
    {"aaf", 0, NSC_LONG, 0, nsc_lnd_aaf, EF_BAD, 0, CA_DUMP_NONE},
    {"nland", 0, NSC_LONG, 0, nsc_cargo_nland, EF_BAD, 0, CA_DUMP_NONE},
    {"nxlight", 0, NSC_LONG, 0, nsc_cargo_nxlight, EF_BAD, 0,
     CA_DUMP_NONE},
#undef CURSTR
#define CURSTR struct lchrstr
    {"spy", fldoff(l_spy), NSC_INT, 0, nsc_lchr, EF_BAD, 0, CA_DUMP_NONE},
    {"rmax", fldoff(l_rad), NSC_INT, 0, nsc_lchr, EF_BAD, 0, CA_DUMP_NONE},
    {"ammo", fldoff(l_ammo), NSC_INT, 0, nsc_lchr, EF_BAD, 0,
     CA_DUMP_NONE},
    {"maxlight", fldoff(l_nxlight), NSC_UCHAR, 0, nsc_lchr,
     EF_BAD, 0, CA_DUMP_NONE},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr lchr_ca[] = {
#define CURSTR struct lchrstr
    {"type", fldoff(l_type), NSC_CHAR, 0, NULL, EF_LAND_CHR, 0, CA_DUMP},
    {"name", fldoff(l_name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP},
    NSC_IVEC(fldoff(l_item), ""),
    NSC_MVEC(fldoff(l_mat), CA_DUMP_ONLY, CA_DUMP_ONLY, CA_DUMP_ONLY),
    {"bwork", fldoff(l_bwork), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"tech", fldoff(l_tech), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"cost", fldoff(l_cost), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"att", fldoff(l_att), NSC_FLOAT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"def", fldoff(l_def), NSC_FLOAT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"vul", fldoff(l_vul), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"spd", fldoff(l_spd), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"vis", fldoff(l_vis), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"spy", fldoff(l_spy), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"rmax", fldoff(l_rad), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"frg", fldoff(l_frg), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"acc", fldoff(l_acc), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"dam", fldoff(l_dam), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"ammo", fldoff(l_ammo), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"aaf", fldoff(l_aaf), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"flags", fldoff(l_flags), NSC_INT, 0, NULL,
     EF_LAND_CHR_FLAGS, NSC_BITS, CA_DUMP},
    {"nxlight", fldoff(l_nxlight), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"nland", fldoff(l_nland), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr nuke_ca[] = {
#define CURSTR struct nukstr
    NSC_GENITEM(EF_NUKE, EF_NUKE_CHR),
    {"stockpile", fldoff(nuk_stockpile), NSC_STRINGY, 1, NULL, EF_BAD, 0,
     CA_DUMP},
    {"plane", fldoff(nuk_plane), NSC_INT, 0, NULL, EF_PLANE, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr nchr_ca[] = {
#define CURSTR struct nchrstr
    {"type", fldoff(n_type), NSC_CHAR, 0, NULL, EF_NUKE_CHR, 0, CA_DUMP},
    {"name", fldoff(n_name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP},
    NSC_MVEC(fldoff(n_mat), CA_DUMP_ONLY, CA_DUMP, CA_DUMP),
    {"blast", fldoff(n_blast), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"dam", fldoff(n_dam), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"bwork", fldoff(n_bwork), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"tech", fldoff(n_tech), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"cost", fldoff(n_cost), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"weight", fldoff(n_weight), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"flags", fldoff(n_flags), NSC_INT, 0, NULL,
     EF_NUKE_CHR_FLAGS, NSC_BITS, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr loan_ca[] = {
#define CURSTR struct lonstr
    {"uid", fldoff(l_uid), NSC_INT, 0, NULL, EF_LOAN, 0, CA_DUMP},
    {"timestamp", fldoff(l_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    {"loaner", fldoff(l_loner), NSC_NATID, 0, NULL, EF_NATION, 0, CA_DUMP},
    {"loanee", fldoff(l_lonee), NSC_NATID, 0, NULL, EF_NATION, 0, CA_DUMP},
    {"status", fldoff(l_status), NSC_CHAR, 0, NULL,
     EF_AGREEMENT_STATUS, 0, CA_DUMP},
    {"irate", fldoff(l_irate), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"ldur", fldoff(l_ldur), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"amtpaid", fldoff(l_amtpaid), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"amtdue", fldoff(l_amtdue), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"lastpay", fldoff(l_lastpay), NSC_TIME, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"duedate", fldoff(l_duedate), NSC_TIME, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr news_ca[] = {
#define CURSTR struct nwsstr
    {"timestamp", 0, NSC_LONG, 0, nsc_nws_timestamp, EF_BAD, 0,
     CA_DUMP_NONE},
    {"actor", fldoff(nws_ano), NSC_NATID, 0, NULL, EF_NATION, 0, CA_DUMP},
    {"action", fldoff(nws_vrb), NSC_UCHAR, 0, NULL, EF_NEWS_CHR, 0,
     CA_DUMP},
    {"victim", fldoff(nws_vno), NSC_NATID, 0, NULL, EF_NATION, 0, CA_DUMP},
    {"times", fldoff(nws_ntm), NSC_USHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"duration", fldoff(nws_duration), NSC_SHORT, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"time", fldoff(nws_when), NSC_TIME, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr lost_ca[] = {
#define CURSTR struct loststr
    /* no need for uid as long as it's not referenced from other tables */
    {"timestamp", fldoff(lost_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP},
    {"owner", fldoff(lost_owner), NSC_NATID, 0, NULL, EF_NATION, 0,
     CA_DUMP},
    {"type", fldoff(lost_type), NSC_SHORT, 0, NULL, EF_TABLE, 0, CA_DUMP},
    /* id's ca_table given by type, but can't express that: */
    {"id", fldoff(lost_id), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"x", fldoff(lost_x), NSC_XCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"y", fldoff(lost_y), NSC_YCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr commodity_ca[] = {
#define CURSTR struct comstr
    {"uid", fldoff(com_uid), NSC_INT, 0, NULL, EF_COMM, 0, CA_DUMP},
    {"timestamp", fldoff(com_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    {"owner", fldoff(com_owner), NSC_NATID, 0, NULL, EF_NATION, 0,
     CA_DUMP},
    {"type", fldoff(com_type), NSC_SITYPE(i_type), 0, NULL, EF_ITEM, 0,
     CA_DUMP},
    {"amount", fldoff(com_amount), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"price", fldoff(com_price), NSC_FLOAT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"maxbidder", fldoff(com_maxbidder), NSC_INT, 0, NULL, EF_NATION, 0,
     CA_DUMP},
    {"markettime", fldoff(com_markettime), NSC_TIME, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    /* should let maxbidder access xbuy, ybuy, but can't express that: */
    {"xbuy", fldoff(com_x), NSC_XCOORD, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"ybuy", fldoff(com_y), NSC_XCOORD, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    /* should let owner access xsell, ysell, but can't express that: */
    {"xsell", fldoff(sell_x), NSC_XCOORD, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"ysell", fldoff(sell_y), NSC_YCOORD, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr trade_ca[] = {
#define CURSTR struct trdstr
    {"uid", fldoff(trd_uid), NSC_INT, 0, NULL, EF_TRADE, 0, CA_DUMP},
    {"timestamp", fldoff(trd_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    {"owner", fldoff(trd_owner), NSC_NATID, 0, NULL, EF_NATION, 0,
     CA_DUMP},
    {"type", fldoff(trd_type), NSC_SHORT, 0, NULL, EF_TABLE, 0, CA_DUMP},
    /* unitid's ca_table given by type, but can't express that: */
    {"unitid", fldoff(trd_unitid), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"price", fldoff(trd_price), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"maxbidder", fldoff(trd_maxbidder), NSC_INT, 0, NULL, EF_NATION, 0,
     CA_DUMP},
    {"markettime", fldoff(trd_markettime), NSC_TIME, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    /* should let maxbidder access xloc, yloc, but can't express that: */
    {"xloc", fldoff(trd_x), NSC_XCOORD, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"yloc", fldoff(trd_y), NSC_YCOORD, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr cou_ca[] = {
#define CURSTR struct natstr
    /*
     * This is the owner's view, i.e. it applies only to the own
     * nation.  The public view nat_ca[], which applies to all
     * nations, has the same selectors with different flags: NSC_DEITY
     * is set except for cnum (which must come first) and all
     * CA_DUMP_NONE selectors; these become CA_DUMP except for
     * timestamp (which must come second).
     * nat_ca[] should also make tech, research, education and
     * happiness available, but we can't express the obfuscation
     * necessary for foreign levels.
     */
    {"cnum", fldoff(nat_cnum), NSC_NATID, 0, NULL, EF_NATION, 0, CA_DUMP},
    {"timestamp", fldoff(nat_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    {"stat", fldoff(nat_stat), NSC_SITYPE(enum nat_status), 0, NULL,
     EF_NATION_STATUS, 0, CA_DUMP_NONE},
    {"flags", fldoff(nat_flags), NSC_INT, 0, NULL,
     EF_NATION_FLAGS, NSC_BITS, CA_DUMP},
    {"cname", fldoff(nat_cnam), NSC_STRINGY, 20, NULL, EF_BAD, 0,
     CA_DUMP_NONE},
    {"passwd", fldoff(nat_pnam), NSC_STRINGY, 20, NULL,
     EF_BAD, NSC_DEITY, CA_DUMP_NONE},
    {"ip", fldoff(nat_hostaddr), NSC_STRINGY, 46, NULL, EF_BAD, 0,
     CA_DUMP},
    {"userid", fldoff(nat_userid), NSC_STRINGY, 32, NULL, EF_BAD, 0,
     CA_DUMP},
    {"xcap", fldoff(nat_xcap), NSC_XCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"ycap", fldoff(nat_ycap), NSC_YCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"xorg", fldoff(nat_xorg), NSC_XCOORD, 0, NULL,
     EF_BAD, NSC_DEITY, CA_DUMP_NONE},
    {"yorg", fldoff(nat_yorg), NSC_YCOORD, 0, NULL,
     EF_BAD, NSC_DEITY, CA_DUMP_NONE},
    {"update", fldoff(nat_update), NSC_CHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"tgms", fldoff(nat_tgms), NSC_USHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"ann", fldoff(nat_ann), NSC_USHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"timeused", fldoff(nat_timeused), NSC_INT, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"btu", fldoff(nat_btu), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"access", fldoff(nat_access), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"milreserve", fldoff(nat_reserve), NSC_INT, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"money", fldoff(nat_money), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"login", fldoff(nat_last_login), NSC_TIME, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"logout", fldoff(nat_last_logout), NSC_TIME, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"newstim", fldoff(nat_newstim), NSC_TIME, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"annotim", fldoff(nat_annotim), NSC_TIME, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"tech", fldoff(nat_level[NAT_TLEV]), NSC_FLOAT, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"research", fldoff(nat_level[NAT_RLEV]), NSC_FLOAT, 0, NULL,
     EF_BAD, 0, CA_DUMP},
    {"education", fldoff(nat_level[NAT_ELEV]), NSC_FLOAT, 0, NULL,
     EF_BAD, 0, CA_DUMP},
    {"happiness", fldoff(nat_level[NAT_HLEV]), NSC_FLOAT, 0, NULL,
     EF_BAD, 0, CA_DUMP},
    {"relations", fldoff(nat_relate), NSC_UCHAR, MAXNOC, NULL,
     EF_NATION_RELATIONS, NSC_HIDDEN, CA_DUMP_NONE},
    {"rejects", fldoff(nat_rejects), NSC_UCHAR, MAXNOC, NULL,
     EF_NATION_REJECTS, NSC_BITS, CA_DUMP_NONE},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr nat_ca[sizeof(cou_ca) / sizeof(*cou_ca)];
/* initialized by nsc_init() */

struct castr contact_ca[] = {
#define CURSTR struct contactstr
    {"uid", fldoff(con_uid), NSC_INT, 0, NULL, EF_CONTACT, 0, CA_DUMP},
    {"timestamp", fldoff(con_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    /* mortals know there's contact (relations show), but not how strong */
    {"contacts", fldoff(con_contact), NSC_UCHAR, MAXNOC, NULL,
     EF_BAD, NSC_DEITY, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr reject_ca[] = {
#define CURSTR struct rejectstr
    {"uid", fldoff(rej_uid), NSC_INT, 0, NULL, EF_REJECT, 0, CA_DUMP},
    {"timestamp", fldoff(rej_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr realm_ca[] = {
#define CURSTR struct realmstr
    /* uid is encoded in cnum, realm */
    {"timestamp", fldoff(r_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    {"cnum", fldoff(r_cnum), NSC_NATID, 0, NULL, EF_NATION, 0,
     CA_DUMP_CONST},
    {"realm", fldoff(r_realm), NSC_USHORT, 0, NULL, EF_BAD, 0,
     CA_DUMP_CONST},
    {"xl", fldoff(r_xl), NSC_XCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"xh", fldoff(r_xh), NSC_XCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"yl", fldoff(r_yl), NSC_YCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"yh", fldoff(r_yh), NSC_YCOORD, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr game_ca[] = {
#define CURSTR struct gamestr
    /* no need for uid */
    {"timestamp", fldoff(game_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0, CA_DUMP_NONE},
    {"upd_disable", fldoff(game_upd_disable), NSC_CHAR, 0, NULL,
     EF_BAD, 0, CA_DUMP},
    {"down", fldoff(game_down), NSC_CHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"turn", fldoff(game_turn), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"tick", fldoff(game_tick), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY,
     CA_DUMP},
    {"rt", fldoff(game_rt), NSC_TIME, 0, NULL, EF_BAD, NSC_DEITY, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr intrchr_ca[] = {
#define CURSTR struct sctintrins
    /* no need for uid as long as it's not referenced from other tables */
    {"name", fldoff(in_name), NSC_STRING, 0, NULL, EF_BAD, 0,
     CA_DUMP_CONST},
    NSC_MVEC(fldoff(in_mat), CA_DUMP_ONLY, CA_DUMP_ONLY, CA_DUMP_ONLY),
    {"bmobil", fldoff(in_bmobil), NSC_SHORT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"cost", fldoff(in_cost), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP},
    {"enable", fldoff(in_enable), NSC_UCHAR, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr rpt_ca[] = {
#define CURSTR struct rptstr
    {"uid", fldoff(r_uid), NSC_CHAR, 0, NULL, EF_NEWS_CHR, 0, CA_DUMP},
    {"newstory", fldoff(r_newstory), NSC_STRING, NUM_RPTS, NULL,
     EF_BAD, 0, CA_DUMP},
    {"good_will", fldoff(r_good_will), NSC_INT, 0, NULL, EF_BAD, 0,
     CA_DUMP},
    {"newspage", fldoff(r_newspage), NSC_INT, 0, NULL,
     EF_PAGE_HEADINGS, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr update_ca[] = {
    {"time", 0, NSC_TIME, 0, NULL, EF_BAD, 0, CA_DUMP},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
};

struct castr empfile_ca[] = {
#define CURSTR struct empfile
    {"uid", fldoff(uid), NSC_INT, 0, NULL, EF_TABLE, 0, CA_DUMP},
    {"name", fldoff(name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP_CONST},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr symbol_ca[] = {
#define CURSTR struct symbol
    /*
     * value is is const because it has to match what is compiled into
     * the server.  name is const because clients key on it.
     */
    {"value", fldoff(value), NSC_INT, 0, NULL, EF_BAD, 0, CA_DUMP_CONST},
    {"name", fldoff(name), NSC_STRING, 0, NULL, EF_BAD, 0, CA_DUMP_CONST},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

struct castr mdchr_ca[] = {
#define CURSTR struct castr
    /* no need for uid */
    /* name must come first, clients may rely on it */
    {"name", fldoff(ca_name), NSC_STRING, 0, NULL, EF_BAD, 0,
     CA_DUMP_CONST},
    {"type", fldoff(ca_type), NSC_LONG, 0, nsc_meta_type,
     EF_META_TYPE, 0, CA_DUMP_CONST},
    {"flags", fldoff(ca_flags), NSC_INT, 0, NULL,
     EF_META_FLAGS, NSC_BITS, CA_DUMP_CONST},
    {"len", fldoff(ca_len), NSC_LONG, 0, nsc_meta_len,
     EF_BAD, 0, CA_DUMP_CONST},
    {"table", fldoff(ca_table), NSC_INT, 0, NULL, EF_TABLE, 0,
     CA_DUMP_CONST},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0, CA_DUMP}
#undef CURSTR
};

void
nsc_init(void)
{
    static struct castr version_ca0 = {
	"version", 0, NSC_STRING, 0, nsc_ver_version, EF_BAD, 0, CA_DUMP
    };
    static struct castr version_ca1 = {
	"maxnoc", 0, NSC_LONG, 0, nsc_ver_maxnoc, EF_BAD, 0, CA_DUMP
    };
    static struct castr *ca;
    struct keymatch *kp;
    int n, i;

    /* derive empfile[EF_VERSION].cadef from configkeys[] */
    n = 0;
    for (kp = configkeys; kp->km_key; ++kp) {
	if (kp->km_type != NSC_NOTYPE && !(kp->km_flags & KM_INTERNAL))
	    n++;
    }
    ca = calloc(2 + n + 1, sizeof(*ca));
    ca[0] = version_ca0;
    ca[1] = version_ca1;
    i = 2;
    for (kp = configkeys; kp->km_key; ++kp) {
	if (kp->km_type != NSC_NOTYPE && !(kp->km_flags & KM_INTERNAL)) {
	    ca[i].ca_type = kp->km_type;
	    ca[i].ca_off = kp - configkeys;
	    ca[i].ca_name = kp->km_key;
	    ca[i].ca_table = EF_BAD;
	    ca[i].ca_get = nsc_ver;
	    i++;
	}
    }
    empfile[EF_VERSION].cadef = ca;

    /* derive nat_ca[] from cou_ca[] */
    nat_ca[0] = cou_ca[0];	/* cnum */
    nat_ca[1] = cou_ca[1];	/* timestamp */
    for (i = 2; cou_ca[i].ca_name; i++) {
	nat_ca[i] = cou_ca[i];
	if (nat_ca[i].ca_dump == CA_DUMP_NONE)
	    nat_ca[i].ca_dump = CA_DUMP;
	else
	    nat_ca[i].ca_flags |= NSC_DEITY;
    }
    nat_ca[i] = cou_ca[i];	/* sentinel */
}

/*
 * Virtual selectors
 */

static void *
nsc_ver(struct valstr *val, struct natstr *np, void *ptr)
{
    struct keymatch *kp = &configkeys[val->val_as.sym.off];
    val->val_as.sym.off = 0;
    val->val_as.sym.get = NULL;
    return kp->km_data;
}

static void *
nsc_ver_maxnoc(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = MAXNOC;
    return NULL;
}

static void *
nsc_ver_version(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.str.base = version;
    val->val_as.str.maxsz = INT_MAX; /* really SIZE_MAX, but that's C99 */
    return NULL;
}

static void *
nsc_sct_terr(struct valstr *val, struct natstr *np, void *ptr)
{
    if (!np || np->nat_stat == STAT_GOD)
	val->val_as.sym.off = offsetof(struct sctstr, sct_dterr);
    else
	val->val_as.sym.off = offsetof(struct sctstr, sct_terr);
    val->val_as.sym.get = NULL;
    return ptr;
}

static void *
nsc_sct_track(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = sct_rail_track(ptr);
    return NULL;
}

static void *
nsc_cargo_nplane(struct valstr *val, struct natstr *np, void *ptr)
{
    struct empobj *obj = ptr;
    int n, nch, nxl;

    n = unit_nplane(obj->ef_type, obj->uid, &nch, &nxl, NULL);
    val->val_as.lng = n - nch - nxl;
    return NULL;
}

static void *
nsc_cargo_nchopper(struct valstr *val, struct natstr *np, void *ptr)
{
    struct empobj *obj = ptr;
    int n;

    unit_nplane(obj->ef_type, obj->uid, &n, NULL, NULL);
    val->val_as.lng = n;
    return NULL;
}

static void *
nsc_cargo_nxlight(struct valstr *val, struct natstr *np, void *ptr)
{
    struct empobj *obj = ptr;
    int n;

    unit_nplane(obj->ef_type, obj->uid, NULL, &n, NULL);
    val->val_as.lng = n;
    return NULL;
}

static void *
nsc_cargo_nland(struct valstr *val, struct natstr *np, void *ptr)
{
    struct empobj *obj = ptr;

    val->val_as.lng = unit_cargo_count(obj->ef_type, obj->uid, EF_LAND);
    return NULL;
}

static void *
nsc_pln_def(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = pln_def(ptr);;
    return NULL;
}

static void *
nsc_pln_att(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = pln_att(ptr);
    return NULL;
}

static void *
nsc_pln_nuketype(struct valstr *val, struct natstr *np, void *ptr)
{
    struct nukstr *nukp = getnukep(nuk_on_plane(ptr));
    val->val_as.lng = nukp ? nukp->nuk_type : -1;
    return NULL;
}

static void *
nsc_lnd_att(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.dbl = lnd_att(ptr);
    return NULL;
}

static void *
nsc_lnd_def(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.dbl = lnd_def(ptr);
    return NULL;
}

static void *
nsc_lnd_vul(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = lnd_vul(ptr);
    return NULL;
}

static void *
nsc_lnd_spd(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = lnd_spd(ptr);
    return NULL;
}

static void *
nsc_lnd_vis(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = lnd_vis(ptr);
    return NULL;
}

static void *
nsc_lnd_frg(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = lnd_frg(ptr);
    return NULL;
}

static void *
nsc_lnd_acc(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = lnd_acc(ptr);
    return NULL;
}

static void *
nsc_lnd_dam(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = lnd_dam(ptr);
    return NULL;
}

static void *
nsc_lnd_aaf(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.lng = lnd_aaf(ptr);
    return NULL;
}

static void *
nsc_lchr(struct valstr *val, struct natstr *np, void *ptr)
{
    val->val_as.sym.get = NULL;
    return lchr + ((struct lndstr *)ptr)->lnd_type;
}

static void *
nsc_nws_timestamp(struct valstr *val, struct natstr *natp, void *ptr)
{
    struct nwsstr *np = ptr;

    val->val_as.lng = np->nws_when + np->nws_duration;
    return NULL;
}

static void *
nsc_meta_type(struct valstr *val, struct natstr *natp, void *ptr)
{
    val->val_as.lng = nstr_promote(((struct castr *)ptr)->ca_type);
    return NULL;
}

static void *
nsc_meta_len(struct valstr *val, struct natstr *natp, void *ptr)
{
    struct castr *ca = ptr;

    val->val_as.lng = ca->ca_type == NSC_STRINGY ? 0 : ca->ca_len;
    return NULL;
}
