/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2004-2009
 */

/*
 * Convention: uid selector comes first.  Table TYPE has an uid
 * selector if empfile[TYPE].cadef[0].ca_table == TYPE.
 */

#include <config.h>

#include <stdlib.h>
#include "empobj.h"
#include "file.h"
#include "optlist.h"
#include "nsc.h"
#include "product.h"
#include "unit.h"

static void *nsc_ver(struct valstr *, struct natstr *, void *);
static void *nsc_ver_maxnoc(struct valstr *, struct natstr *, void *);
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

/* Ugly hack to improve legibility by avoid long lines */
#define fldoff(fld) offsetof(CURSTR, fld)
#define empobjoff(fld) offsetof(struct empobj, fld)

#define NSC_IELT(name, pfx, sfx, base, itype)	\
    {sizeof(sfx) == 1 ? name : pfx sfx,		\
     ((base) + (itype)*sizeof(unsigned short)),	\
     NSC_SHORT, 0, NULL, EF_BAD, 0}

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

struct castr ichr_ca[] = {
#define CURSTR struct ichrstr
    {"uid", fldoff(i_uid), NSC_SITYPE(i_type), 0, NULL, EF_ITEM, 0},
    {"name", fldoff(i_name), NSC_STRING, 0, NULL, EF_BAD, 0},
    {"mnem", fldoff(i_mnem), NSC_STRINGY, 1, NULL, EF_BAD, NSC_CONST},
    {"value", fldoff(i_value), NSC_INT, 0, NULL, EF_BAD, 0},
    {"sell", fldoff(i_sell), NSC_INT, 0, NULL, EF_BAD, 0},
    {"lbs", fldoff(i_lbs), NSC_INT, 0, NULL, EF_BAD, 0},
    {"pkg", fldoff(i_pkg), NSC_INT, NUMPKG, NULL, EF_BAD, 0},
    {"melt_denom", fldoff(i_melt_denom), NSC_INT, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr pchr_ca[] = {
#define CURSTR struct pchrstr
    {"uid", fldoff(p_uid), NSC_INT, 0, NULL, EF_PRODUCT, 0},
    {"name", fldoff(p_name), NSC_STRING, 0, NULL, EF_BAD, 0},
    {"sname", fldoff(p_sname), NSC_STRING, 0, NULL, EF_BAD, NSC_CONST},
    {"ctype", fldoff(p_ctype), NSC_SITYPE(i_type), MAXPRCON, NULL,
     EF_ITEM, 0},
    {"camt", fldoff(p_camt), NSC_USHORT, MAXPRCON, NULL, EF_BAD, 0},
    {"type", fldoff(p_type), NSC_SITYPE(i_type), 0, NULL, EF_ITEM, 0},
    {"level", fldoff(p_level), NSC_INT, 0, NULL, EF_LEVEL, 0},
    {"cost", fldoff(p_cost), NSC_INT, 0, NULL, EF_BAD, 0},
    {"nrndx", fldoff(p_nrndx), NSC_INT, 0, NULL, EF_RESOURCES, 0},
    {"nrdep", fldoff(p_nrdep), NSC_INT, 0, NULL, EF_BAD, 0},
    {"nlndx", fldoff(p_nlndx), NSC_INT, 0, NULL, EF_LEVEL, 0},
    {"nlmin", fldoff(p_nlmin), NSC_INT, 0, NULL, EF_BAD, 0},
    {"nllag", fldoff(p_nllag), NSC_INT, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr sect_ca[] = {
#define CURSTR struct sctstr
    /* uid needs to be NSC_DEITY because it discloses true origin */
    {"uid", fldoff(sct_uid), NSC_INT, 0, NULL,
     EF_SECTOR, NSC_DEITY | NSC_EXTRA},
    {"timestamp", fldoff(sct_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, NSC_EXTRA},
    {"owner", fldoff(sct_own), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"xloc", fldoff(sct_x), NSC_XCOORD, 0, NULL, EF_BAD, NSC_CONST},
    {"yloc", fldoff(sct_y), NSC_YCOORD, 0, NULL, EF_BAD, NSC_CONST},
    {"des", fldoff(sct_type), NSC_CHAR, 0, NULL, EF_SECTOR_CHR, 0},
    {"effic", fldoff(sct_effic), NSC_CHAR, 0, NULL, EF_BAD, 0},
    {"mobil", fldoff(sct_mobil), NSC_CHAR, 0, NULL, EF_BAD, 0},
    {"off", fldoff(sct_off), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"loyal", fldoff(sct_loyal), NSC_UCHAR, 0, NULL, EF_BAD, NSC_DEITY},
    {"terr", 0, NSC_UCHAR, 0, nsc_sct_terr, EF_BAD, NSC_EXTRA},
    {"terr0", fldoff(sct_terr), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"terr1", fldoff(sct_terr1), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"terr2", fldoff(sct_terr2), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"terr3", fldoff(sct_terr3), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"dterr", fldoff(sct_dterr), NSC_UCHAR, 0, NULL, EF_BAD, NSC_DEITY},
    {"xdist", fldoff(sct_dist_x), NSC_XCOORD, 0, NULL, EF_BAD, 0},
    {"ydist", fldoff(sct_dist_y), NSC_YCOORD, 0, NULL, EF_BAD, 0},
    {"avail", fldoff(sct_avail), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"elev", fldoff(sct_elev), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY},
    {"work", fldoff(sct_work), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"coastal", fldoff(sct_coastal), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"newdes", fldoff(sct_newtype), NSC_CHAR, 0, NULL, EF_SECTOR_CHR, 0},
    {"min", fldoff(sct_min), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"gold", fldoff(sct_gmin), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"fert", fldoff(sct_fertil), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"ocontent", fldoff(sct_oil), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"uran", fldoff(sct_uran), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"oldown", fldoff(sct_oldown), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"track", 0, NSC_LONG, 0, nsc_sct_track, EF_BAD, NSC_EXTRA},
    NSC_IVEC(fldoff(sct_item), ""),
    NSC_IVEC(fldoff(sct_dist), "_dist"),
    NSC_IVEC(fldoff(sct_del), "_del"),
    /* should let old owner access mines, but can't express that: */
    {"mines", fldoff(sct_mines), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY},
    {"pstage", fldoff(sct_pstage), NSC_SHORT, 0, NULL,
     EF_PLAGUE_STAGES, NSC_DEITY},
    {"ptime", fldoff(sct_ptime), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY},
    {"che", fldoff(sct_che), NSC_UCHAR, 0, NULL, EF_BAD, NSC_DEITY},
    {"che_target", fldoff(sct_che_target), NSC_NATID, 0, NULL,
     EF_NATION, NSC_DEITY},
    {"fallout", fldoff(sct_fallout), NSC_USHORT, 0, NULL, EF_BAD, 0},
    {"access", fldoff(sct_access), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"road", fldoff(sct_road), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"rail", fldoff(sct_rail), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"dfense", fldoff(sct_defense), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr dchr_ca[] = {
#define CURSTR struct dchrstr
    {"uid", fldoff(d_uid), NSC_UCHAR, 0, NULL, EF_SECTOR_CHR, 0},
    {"name", fldoff(d_name), NSC_STRING, 0, NULL, EF_BAD, 0},
    {"mnem", fldoff(d_mnem), NSC_STRINGY, 1, NULL, EF_BAD, NSC_CONST},
    {"terrain", fldoff(d_terrain), NSC_UCHAR, 0, NULL, EF_SECTOR_CHR, 0},
    {"prd", fldoff(d_prd), NSC_INT, 0, NULL, EF_PRODUCT, 0},
    {"peffic", fldoff(d_peffic), NSC_INT, 0, NULL, EF_BAD, 0},
    {"mob0", fldoff(d_mob0), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"mob1", fldoff(d_mob1), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"nav", fldoff(d_nav), NSC_SITYPE(enum d_navigation), 0, NULL,
     EF_SECTOR_NAVIGATION, 0},
    {"pkg", fldoff(d_pkg), NSC_SITYPE(enum i_packing), 0, NULL,
     EF_PACKING, 0},
    {"ostr", fldoff(d_ostr), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"dstr", fldoff(d_dstr), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"value", fldoff(d_value), NSC_INT, 0, NULL, EF_BAD, 0},
    {"cost", fldoff(d_cost), NSC_INT, 0, NULL, EF_BAD, 0},
    {"build", fldoff(d_build), NSC_INT, 0, NULL, EF_BAD, 0},
    {"lcms", fldoff(d_lcms), NSC_INT, 0, NULL, EF_BAD, 0},
    {"hcms", fldoff(d_hcms), NSC_INT, 0, NULL, EF_BAD, 0},
    {"maint", fldoff(d_maint), NSC_INT, 0, NULL, EF_BAD, 0},
    {"maxpop", fldoff(d_maxpop), NSC_INT, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

#define NSC_GENITEM(ef_type, ef_chr)					\
    {"uid", empobjoff(uid),  NSC_INT, 0, NULL, ef_type, 0},		\
    {"timestamp", empobjoff(timestamp), NSC_TIME, 0, NULL, EF_BAD, NSC_EXTRA}, \
    {"owner", empobjoff(own),  NSC_NATID, 0, NULL, EF_NATION, 0},	\
    {"xloc", empobjoff(x),  NSC_XCOORD, 0, NULL, EF_BAD, 0},		\
    {"yloc", empobjoff(y),  NSC_YCOORD, 0, NULL, EF_BAD, 0},		\
    {"type", empobjoff(type),  NSC_CHAR, 0, NULL, ef_chr, 0},		\
    {"effic", empobjoff(effic),  NSC_CHAR, 0, NULL, EF_BAD, 0},		\
    {"mobil", empobjoff(mobil),  NSC_CHAR , 0, NULL, EF_BAD, 0},	\
    {"off", empobjoff(off),  NSC_UCHAR , 0, NULL, EF_BAD, 0},		\
    {"tech", empobjoff(tech),  NSC_SHORT, 0, NULL, EF_BAD, 0},		\
    {"group", empobjoff(group),  NSC_STRINGY, 1, NULL, EF_BAD, NSC_EXTRA}, \
    {"opx", empobjoff(opx),  NSC_XCOORD, 0, NULL, EF_BAD, 0},		\
    {"opy", empobjoff(opy),  NSC_YCOORD, 0, NULL, EF_BAD, 0},		\
    {"mission", empobjoff(mission),  NSC_SHORT, 0, NULL, EF_MISSIONS, 0}, \
    {"radius", empobjoff(radius),  NSC_SHORT, 0, NULL, EF_BAD, 0}

struct castr ship_ca[] = {
#define CURSTR struct shpstr
    NSC_GENITEM(EF_SHIP, EF_SHIP_CHR),
    {"fleet", fldoff(shp_fleet), NSC_STRINGY, 1, NULL, EF_BAD, 0},
    {"xstart", fldoff(shp_destx[0]), NSC_XCOORD, 0, NULL, EF_BAD, 0},
    {"xend", fldoff(shp_destx[1]), NSC_XCOORD, 0, NULL, EF_BAD, 0},
    {"ystart", fldoff(shp_desty[0]), NSC_YCOORD, 0, NULL, EF_BAD, 0},
    {"yend", fldoff(shp_desty[1]), NSC_YCOORD, 0, NULL, EF_BAD, 0},
    {"cargostart", fldoff(shp_tstart), NSC_SITYPE(i_type), TMAX, NULL,
     EF_ITEM, 0},
    {"cargoend", fldoff(shp_tend), NSC_SITYPE(i_type), TMAX, NULL,
     EF_ITEM, 0},
    {"amtstart", fldoff(shp_lstart), NSC_SHORT, TMAX, NULL, EF_BAD, 0},
    {"amtend", fldoff(shp_lend), NSC_SHORT, TMAX, NULL, EF_BAD, 0},
    {"autonav", fldoff(shp_autonav), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    NSC_IVEC(fldoff(shp_item), ""),
    {"pstage", fldoff(shp_pstage), NSC_SHORT, 0, NULL,
     EF_PLAGUE_STAGES, NSC_DEITY},
    {"ptime", fldoff(shp_ptime), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY},
    {"access", fldoff(shp_access), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"mquota", fldoff(shp_mobquota), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"path", fldoff(shp_path), NSC_STRINGY, MAXSHPPATH, NULL, EF_BAD, 0},
    {"follow", fldoff(shp_follow), NSC_INT, 0, NULL, EF_BAD, 0},
    {"name", fldoff(shp_name), NSC_STRINGY, MAXSHPNAMLEN, NULL,
     EF_BAD, 0},
    /* should let builder access xbuilt, ybuilt, but can't express that: */
    {"xbuilt", fldoff(shp_orig_x), NSC_XCOORD, 0, NULL,
     EF_BAD, NSC_DEITY},
    {"ybuilt", fldoff(shp_orig_y), NSC_YCOORD, 0, NULL,
     EF_BAD, NSC_DEITY},
    {"builder", fldoff(shp_orig_own), NSC_NATID, 0, NULL,
     EF_NATION, NSC_DEITY},
    {"rflags", fldoff(shp_rflags), NSC_INT, 0, NULL,
     EF_RETREAT_FLAGS, NSC_BITS},
    {"rpath", fldoff(shp_rpath), NSC_STRINGY, RET_LEN, NULL, EF_BAD, 0},
    {"nplane", 0, NSC_LONG, 0, nsc_cargo_nplane, EF_BAD, NSC_EXTRA},
    {"nchoppers", 0, NSC_LONG, 0, nsc_cargo_nchopper, EF_BAD, NSC_EXTRA},
    {"nxlight", 0, NSC_LONG, 0, nsc_cargo_nxlight, EF_BAD, NSC_EXTRA},
    {"nland", 0, NSC_LONG, 0, nsc_cargo_nland, EF_BAD, NSC_EXTRA},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr mchr_ca[] = {
#define CURSTR struct mchrstr
    {"type", fldoff(m_type), NSC_CHAR, 0, NULL, EF_SHIP_CHR, 0},
    {"name", fldoff(m_name), NSC_STRING, 0, NULL, EF_BAD, 0},
    NSC_IVEC(fldoff(m_item), ""),
    {"l_build", fldoff(m_lcm), NSC_INT, 0, NULL, EF_BAD, 0},
    {"h_build", fldoff(m_hcm), NSC_INT, 0, NULL, EF_BAD, 0},
    {"armor", fldoff(m_armor), NSC_INT, 0, NULL, EF_BAD, 0},
    {"speed", fldoff(m_speed), NSC_INT, 0, NULL, EF_BAD, 0},
    {"visib", fldoff(m_visib), NSC_INT, 0, NULL, EF_BAD, 0},
    {"vrnge", fldoff(m_vrnge), NSC_INT, 0, NULL, EF_BAD, 0},
    {"frnge", fldoff(m_frnge), NSC_INT, 0, NULL, EF_BAD, 0},
    {"glim", fldoff(m_glim), NSC_INT, 0, NULL, EF_BAD, 0},
    {"nxlight", fldoff(m_nxlight), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"nchoppers", fldoff(m_nchoppers), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"tech", fldoff(m_tech), NSC_INT, 0, NULL, EF_BAD, 0},
    {"cost", fldoff(m_cost), NSC_INT, 0, NULL, EF_BAD, 0},
    {"flags", fldoff(m_flags), NSC_LONG, 0, NULL,
     EF_SHIP_CHR_FLAGS, NSC_BITS},
    {"nplanes", fldoff(m_nplanes), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"nland", fldoff(m_nland), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr plane_ca[] = {
#define CURSTR struct plnstr
    NSC_GENITEM(EF_PLANE, EF_PLANE_CHR),
    {"wing", fldoff(pln_wing), NSC_STRINGY, 1, NULL, EF_BAD, 0},
    {"range", fldoff(pln_range), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"ship", fldoff(pln_ship), NSC_INT, 0, NULL, EF_BAD, 0},
    {"land", fldoff(pln_land), NSC_INT, 0, NULL, EF_BAD, 0},
    {"harden", fldoff(pln_harden), NSC_CHAR, 0, NULL, EF_BAD, 0},
    {"flags", fldoff(pln_flags), NSC_CHAR, 0, NULL,
     EF_PLANE_FLAGS, NSC_BITS},
    {"access", fldoff(pln_access), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"theta", fldoff(pln_theta), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"att", 0, NSC_LONG, 0, nsc_pln_att, EF_BAD, NSC_EXTRA},
    {"def", 0, NSC_LONG, 0, nsc_pln_def, EF_BAD, NSC_EXTRA},
    {"nuketype", 0, NSC_LONG, 0, nsc_pln_nuketype, EF_BAD, NSC_EXTRA},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr plchr_ca[] = {
#define CURSTR struct plchrstr
    {"type", fldoff(pl_type), NSC_CHAR, 0, NULL, EF_PLANE_CHR, 0},
    {"name", fldoff(pl_name), NSC_STRING, 0, NULL, EF_BAD, 0},
    {"l_build", fldoff(pl_lcm), NSC_INT, 0, NULL, EF_BAD, 0},
    {"h_build", fldoff(pl_hcm), NSC_INT, 0, NULL, EF_BAD, 0},
    {"cost", fldoff(pl_cost), NSC_INT, 0, NULL, EF_BAD, 0},
    {"tech", fldoff(pl_tech), NSC_INT, 0, NULL, EF_BAD, 0},
    {"acc", fldoff(pl_acc), NSC_INT, 0, NULL, EF_BAD, 0},
    {"load", fldoff(pl_load), NSC_INT, 0, NULL, EF_BAD, 0},
    {"att", fldoff(pl_att), NSC_INT, 0, NULL, EF_BAD, 0},
    {"def", fldoff(pl_def), NSC_INT, 0, NULL, EF_BAD, 0},
    {"range", fldoff(pl_range), NSC_INT, 0, NULL, EF_BAD, 0},
    {"crew", fldoff(pl_crew), NSC_INT, 0, NULL, EF_BAD, 0},
    {"fuel", fldoff(pl_fuel), NSC_INT, 0, NULL, EF_BAD, 0},
    {"stealth", fldoff(pl_stealth), NSC_INT, 0, NULL, EF_BAD, 0},
    {"flags", fldoff(pl_flags), NSC_INT, 0, NULL,
     EF_PLANE_CHR_FLAGS, NSC_BITS},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr land_ca[] = {
#define CURSTR struct lndstr
    NSC_GENITEM(EF_LAND, EF_LAND_CHR),
    {"army", fldoff(lnd_army), NSC_STRINGY, 1, NULL, EF_BAD, 0},
    {"ship", fldoff(lnd_ship), NSC_INT, 0, NULL, EF_BAD, 0},
    {"harden", fldoff(lnd_harden), NSC_CHAR, 0, NULL, EF_BAD, 0},
    {"retreat", fldoff(lnd_retreat), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"rflags", fldoff(lnd_rflags), NSC_INT, 0, NULL,
     EF_RETREAT_FLAGS, NSC_BITS},
    {"rpath", fldoff(lnd_rpath), NSC_STRINGY, RET_LEN, NULL, EF_BAD, 0},
    NSC_IVEC(fldoff(lnd_item), ""),
    {"pstage", fldoff(lnd_pstage), NSC_SHORT, 0, NULL,
     EF_PLAGUE_STAGES, NSC_DEITY},
    {"ptime", fldoff(lnd_ptime), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY},
    {"land", fldoff(lnd_land), NSC_INT, 0, NULL, EF_BAD, 0},
    {"access", fldoff(lnd_access), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"att", 0, NSC_DOUBLE, 0, nsc_lnd_att, EF_BAD, NSC_EXTRA},
    {"def", 0, NSC_DOUBLE, 0, nsc_lnd_def, EF_BAD, NSC_EXTRA},
    {"vul", 0, NSC_LONG, 0, nsc_lnd_vul, EF_BAD, NSC_EXTRA},
    {"spd", 0, NSC_LONG, 0, nsc_lnd_spd, EF_BAD, NSC_EXTRA},
    {"vis", 0, NSC_LONG, 0, nsc_lnd_vis, EF_BAD, NSC_EXTRA},
    {"frg", 0, NSC_LONG, 0, nsc_lnd_frg, EF_BAD, NSC_EXTRA},
    {"acc", 0, NSC_LONG, 0, nsc_lnd_acc, EF_BAD, NSC_EXTRA},
    {"dam", 0, NSC_LONG, 0, nsc_lnd_dam, EF_BAD, NSC_EXTRA},
    {"aaf", 0, NSC_LONG, 0, nsc_lnd_aaf, EF_BAD, NSC_EXTRA},
    {"nland", 0, NSC_LONG, 0, nsc_cargo_nland, EF_BAD, NSC_EXTRA},
    {"nxlight", 0, NSC_LONG, 0, nsc_cargo_nxlight, EF_BAD, NSC_EXTRA},
#undef CURSTR
#define CURSTR struct lchrstr
    {"spy", fldoff(l_spy), NSC_INT, 0, nsc_lchr, EF_BAD, NSC_EXTRA},
    {"rmax", fldoff(l_rad), NSC_INT, 0, nsc_lchr, EF_BAD, NSC_EXTRA},
    {"ammo", fldoff(l_ammo), NSC_INT, 0, nsc_lchr, EF_BAD, NSC_EXTRA},
    {"maxlight", fldoff(l_nxlight), NSC_UCHAR, 0, nsc_lchr,
     EF_BAD, NSC_EXTRA},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr lchr_ca[] = {
#define CURSTR struct lchrstr
    {"type", fldoff(l_type), NSC_CHAR, 0, NULL, EF_LAND_CHR, 0},
    {"name", fldoff(l_name), NSC_STRING, 0, NULL, EF_BAD, 0},
    NSC_IVEC(fldoff(l_item), ""),
    {"l_build", fldoff(l_lcm), NSC_INT, 0, NULL, EF_BAD, 0},
    {"h_build", fldoff(l_hcm), NSC_INT, 0, NULL, EF_BAD, 0},
    {"tech", fldoff(l_tech), NSC_INT, 0, NULL, EF_BAD, 0},
    {"cost", fldoff(l_cost), NSC_INT, 0, NULL, EF_BAD, 0},
    {"att", fldoff(l_att), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"def", fldoff(l_def), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"vul", fldoff(l_vul), NSC_INT, 0, NULL, EF_BAD, 0},
    {"spd", fldoff(l_spd), NSC_INT, 0, NULL, EF_BAD, 0},
    {"vis", fldoff(l_vis), NSC_INT, 0, NULL, EF_BAD, 0},
    {"spy", fldoff(l_spy), NSC_INT, 0, NULL, EF_BAD, 0},
    {"rmax", fldoff(l_rad), NSC_INT, 0, NULL, EF_BAD, 0},
    {"frg", fldoff(l_frg), NSC_INT, 0, NULL, EF_BAD, 0},
    {"acc", fldoff(l_acc), NSC_INT, 0, NULL, EF_BAD, 0},
    {"dam", fldoff(l_dam), NSC_INT, 0, NULL, EF_BAD, 0},
    {"ammo", fldoff(l_ammo), NSC_INT, 0, NULL, EF_BAD, 0},
    {"aaf", fldoff(l_aaf), NSC_INT, 0, NULL, EF_BAD, 0},
    {"nxlight", fldoff(l_nxlight), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"nland", fldoff(l_nland), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"flags", fldoff(l_flags), NSC_LONG, 0, NULL,
     EF_LAND_CHR_FLAGS, NSC_BITS},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr nuke_ca[] = {
#define CURSTR struct nukstr
    NSC_GENITEM(EF_NUKE, EF_NUKE_CHR),
    {"plane", fldoff(nuk_plane), NSC_INT, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr nchr_ca[] = {
#define CURSTR struct nchrstr
    {"type", fldoff(n_type), NSC_CHAR, 0, NULL, EF_NUKE_CHR, 0},
    {"name", fldoff(n_name), NSC_STRING, 0, NULL, EF_BAD, 0},
    {"l_build", fldoff(n_lcm), NSC_INT, 0, NULL, EF_BAD, 0},
    {"h_build", fldoff(n_hcm), NSC_INT, 0, NULL, EF_BAD, 0},
    {"o_build", fldoff(n_oil), NSC_INT, 0, NULL, EF_BAD, 0},
    {"r_build", fldoff(n_rad), NSC_INT, 0, NULL, EF_BAD, 0},
    {"blast", fldoff(n_blast), NSC_INT, 0, NULL, EF_BAD, 0},
    {"dam", fldoff(n_dam), NSC_INT, 0, NULL, EF_BAD, 0},
    {"cost", fldoff(n_cost), NSC_INT, 0, NULL, EF_BAD, 0},
    {"tech", fldoff(n_tech), NSC_INT, 0, NULL, EF_BAD, 0},
    {"weight", fldoff(n_weight), NSC_INT, 0, NULL, EF_BAD, 0},
    {"flags", fldoff(n_flags), NSC_INT, 0, NULL,
     EF_NUKE_CHR_FLAGS, NSC_BITS},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr treaty_ca[] = {
#define CURSTR struct trtstr
    {"uid", fldoff(trt_uid), NSC_INT, 0, NULL, EF_TREATY, 0},
    {"timestamp", fldoff(trt_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, NSC_EXTRA},
    {"cna", fldoff(trt_cna), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"cnb", fldoff(trt_cnb), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"status", fldoff(trt_status), NSC_CHAR, 0, NULL,
     EF_AGREEMENT_STATUS, 0},
    {"acond", fldoff(trt_acond), NSC_SHORT, 0, NULL,
     EF_TREATY_FLAGS, NSC_BITS},
    {"bcond", fldoff(trt_bcond), NSC_SHORT, 0, NULL,
     EF_TREATY_FLAGS, NSC_BITS},
    {"exp", fldoff(trt_exp), NSC_TIME, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr loan_ca[] = {
#define CURSTR struct lonstr
    {"uid", fldoff(l_uid), NSC_INT, 0, NULL, EF_LOAN, 0},
    {"timestamp", fldoff(l_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, NSC_EXTRA},
    {"loaner", fldoff(l_loner), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"loanee", fldoff(l_lonee), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"status", fldoff(l_status), NSC_CHAR, 0, NULL,
     EF_AGREEMENT_STATUS, 0},
    {"irate", fldoff(l_irate), NSC_INT, 0, NULL, EF_BAD, 0},
    {"ldur", fldoff(l_ldur), NSC_INT, 0, NULL, EF_BAD, 0},
    {"amtpaid", fldoff(l_amtpaid), NSC_LONG, 0, NULL, EF_BAD, 0},
    {"amtdue", fldoff(l_amtdue), NSC_LONG, 0, NULL, EF_BAD, 0},
    {"lastpay", fldoff(l_lastpay), NSC_TIME, 0, NULL, EF_BAD, 0},
    {"duedate", fldoff(l_duedate), NSC_TIME, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr news_ca[] = {
#define CURSTR struct nwsstr
    {"timestamp", 0, NSC_LONG, 0, nsc_nws_timestamp, EF_BAD, NSC_EXTRA},
    {"actor", fldoff(nws_ano), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"action", fldoff(nws_vrb), NSC_UCHAR, 0, NULL, EF_NEWS_CHR, 0},
    {"victim", fldoff(nws_vno), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"times", fldoff(nws_ntm), NSC_CHAR, 0, NULL, EF_BAD, 0},
    {"duration", fldoff(nws_duration), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"time", fldoff(nws_when), NSC_TIME, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr lost_ca[] = {
#define CURSTR struct loststr
   /* no need for uid as long as it's not referenced from other tables */
    {"timestamp", fldoff(lost_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, 0},
    {"owner", fldoff(lost_owner), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"type", fldoff(lost_type), NSC_CHAR, 0, NULL, EF_TABLE, 0},
    {"id", fldoff(lost_id), NSC_INT, 0, NULL, EF_BAD, 0},
    {"x", fldoff(lost_x), NSC_XCOORD, 0, NULL, EF_BAD, 0},
    {"y", fldoff(lost_y), NSC_YCOORD, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr commodity_ca[] = {
#define CURSTR struct comstr
    {"uid", fldoff(com_uid), NSC_INT, 0, NULL, EF_COMM, 0},
    {"timestamp", fldoff(com_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, NSC_EXTRA},
    {"owner", fldoff(com_owner), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"type", fldoff(com_type), NSC_SITYPE(i_type), 0, NULL, EF_ITEM, 0},
    {"amount", fldoff(com_amount), NSC_INT, 0, NULL, EF_BAD, 0},
    {"price", fldoff(com_price), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"maxbidder", fldoff(com_maxbidder), NSC_INT, 0, NULL, EF_NATION, 0},
    {"markettime", fldoff(com_markettime), NSC_TIME, 0, NULL, EF_BAD, 0},
    /* should let maxbidder access xbuy, ybuy, but can't express that: */
    {"xbuy", fldoff(com_x), NSC_XCOORD, 0, NULL, EF_BAD, NSC_DEITY},
    {"ybuy", fldoff(com_y), NSC_XCOORD, 0, NULL, EF_BAD, NSC_DEITY},
    /* should let owner access xsell, ysell, but can't express that: */
    {"xsell", fldoff(sell_x), NSC_XCOORD, 0, NULL, EF_BAD, NSC_DEITY},
    {"ysell", fldoff(sell_y), NSC_YCOORD, 0, NULL, EF_BAD, NSC_DEITY},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr trade_ca[] = {
#define CURSTR struct trdstr
    {"uid", fldoff(trd_uid), NSC_INT, 0, NULL, EF_TRADE, 0},
    {"timestamp", fldoff(trd_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, NSC_EXTRA},
    {"owner", fldoff(trd_owner), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"type", fldoff(trd_type), NSC_CHAR, 0, NULL, EF_TABLE, 0},
    {"unitid", fldoff(trd_unitid), NSC_INT, 0, NULL, EF_BAD, 0},
    {"price", fldoff(trd_price), NSC_LONG, 0, NULL, EF_BAD, 0},
    {"maxbidder", fldoff(trd_maxbidder), NSC_INT, 0, NULL, EF_NATION, 0},
    {"markettime", fldoff(trd_markettime), NSC_TIME, 0, NULL, EF_BAD, 0},
    /* should let maxbidder access xloc, yloc, but can't express that: */
    {"xloc", fldoff(trd_x), NSC_XCOORD, 0, NULL, EF_BAD, NSC_DEITY},
    {"yloc", fldoff(trd_y), NSC_YCOORD, 0, NULL, EF_BAD, NSC_DEITY},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr cou_ca[] = {
#define CURSTR struct natstr
    /*
     * This is the owner's view, i.e. it applies only to the own
     * nation.  The public view nat_ca[], which applies to all
     * nations, has the same selectors with different flags: NSC_DEITY
     * is set except for cnum (which must come first) and all
     * NSC_EXTRA selectors; NSC_EXTRA is cleared except for timestamp
     * (which must come second).
     * nat_ca[] should also make tech, research, education and
     * happiness available, but we can't express the obfuscation
     * necessary for foreign levels.
     */
    {"cnum", fldoff(nat_cnum), NSC_NATID, 0, NULL, EF_NATION, 0},
    {"timestamp", fldoff(nat_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, NSC_EXTRA},
    {"stat", fldoff(nat_stat), NSC_SITYPE(enum nat_status), 0, NULL,
     EF_NATION_STATUS, NSC_EXTRA},
    {"cname", fldoff(nat_cnam), NSC_STRINGY, 20, NULL, EF_BAD, NSC_EXTRA},
    {"passwd", fldoff(nat_pnam), NSC_STRINGY, 20, NULL,
     EF_BAD, NSC_DEITY | NSC_EXTRA},
    {"ip", fldoff(nat_hostaddr), NSC_STRINGY, 32, NULL, EF_BAD, 0},
    {"hostname", fldoff(nat_hostname), NSC_STRINGY, 512, NULL, EF_BAD, 0},
    {"userid", fldoff(nat_userid), NSC_STRINGY, 32, NULL, EF_BAD, 0},
    {"xcap", fldoff(nat_xcap), NSC_XCOORD, 0, NULL, EF_BAD, 0},
    {"ycap", fldoff(nat_ycap), NSC_YCOORD, 0, NULL, EF_BAD, 0},
    {"xorg", fldoff(nat_xorg), NSC_XCOORD, 0, NULL,
     EF_BAD, NSC_DEITY | NSC_EXTRA},
    {"yorg", fldoff(nat_yorg), NSC_YCOORD, 0, NULL,
     EF_BAD, NSC_DEITY | NSC_EXTRA},
    {"update", fldoff(nat_update), NSC_CHAR, 0, NULL, EF_BAD, 0},
    {"tgms", fldoff(nat_tgms), NSC_USHORT, 0, NULL, EF_BAD, 0},
    {"ann", fldoff(nat_ann), NSC_USHORT, 0, NULL, EF_BAD, 0},
    {"timeused", fldoff(nat_timeused), NSC_INT, 0, NULL, EF_BAD, 0},
    {"btu", fldoff(nat_btu), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"access", fldoff(nat_access), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"milreserve", fldoff(nat_reserve), NSC_LONG, 0, NULL, EF_BAD, 0},
    {"money", fldoff(nat_money), NSC_LONG, 0, NULL, EF_BAD, 0},
    {"login", fldoff(nat_last_login), NSC_TIME, 0, NULL, EF_BAD, 0},
    {"logout", fldoff(nat_last_logout), NSC_TIME, 0, NULL, EF_BAD, 0},
    {"newstim", fldoff(nat_newstim), NSC_TIME, 0, NULL, EF_BAD, 0},
    {"annotim", fldoff(nat_annotim), NSC_TIME, 0, NULL, EF_BAD, 0},
    {"tech", fldoff(nat_level[NAT_TLEV]), NSC_FLOAT, 0, NULL, EF_BAD, 0},
    {"research", fldoff(nat_level[NAT_RLEV]), NSC_FLOAT, 0, NULL,
     EF_BAD, 0},
    {"education", fldoff(nat_level[NAT_ELEV]), NSC_FLOAT, 0, NULL,
     EF_BAD, 0},
    {"happiness", fldoff(nat_level[NAT_HLEV]), NSC_FLOAT, 0, NULL,
     EF_BAD, 0},
    {"relations", fldoff(nat_relate), NSC_HIDDEN, MAXNOC, NULL,
     EF_NATION_RELATIONS, NSC_EXTRA},
    /* mortals know there's contact (relations show), but not how strong */
    {"contacts", fldoff(nat_contact), NSC_UCHAR, MAXNOC, NULL,
     EF_BAD, NSC_DEITY | NSC_EXTRA},
    {"rejects", fldoff(nat_rejects), NSC_UCHAR, MAXNOC, NULL,
     EF_NATION_REJECTS, NSC_EXTRA | NSC_BITS},
    {"flags", fldoff(nat_flags), NSC_LONG, 0, NULL,
     EF_NATION_FLAGS, NSC_BITS},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr nat_ca[sizeof(cou_ca) / sizeof(*cou_ca)];
/* initialized by nsc_init() */

struct castr realm_ca[] = {
#define CURSTR struct realmstr
    /* uid is encoded in cnum, realm */
    {"timestamp", fldoff(r_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, NSC_EXTRA},
    {"cnum", fldoff(r_cnum), NSC_NATID, 0, NULL, EF_NATION, NSC_CONST},
    {"realm", fldoff(r_realm), NSC_USHORT, 0, NULL, EF_BAD, NSC_CONST},
    {"xl", fldoff(r_xl), NSC_XCOORD, 0, NULL, EF_BAD, 0},
    {"xh", fldoff(r_xh), NSC_XCOORD, 0, NULL, EF_BAD, 0},
    {"yl", fldoff(r_yl), NSC_YCOORD, 0, NULL, EF_BAD, 0},
    {"yh", fldoff(r_yh), NSC_YCOORD, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr game_ca[] = {
#define CURSTR struct gamestr
    /* no need for uid */
    {"timestamp", fldoff(game_timestamp), NSC_TIME, 0, NULL,
     EF_BAD, NSC_EXTRA},
    {"upd_disable", fldoff(game_upd_disable), NSC_CHAR, 0, NULL,
     EF_BAD, 0},
    {"down", fldoff(game_down), NSC_CHAR, 0, NULL, EF_BAD, 0},
    {"turn", fldoff(game_turn), NSC_SHORT, 0, NULL, EF_BAD, 0},
    {"tick", fldoff(game_tick), NSC_SHORT, 0, NULL, EF_BAD, NSC_DEITY},
    {"rt", fldoff(game_rt), NSC_TIME, 0, NULL, EF_BAD, NSC_DEITY},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr intrchr_ca[] = {
#define CURSTR struct sctintrins
    /* no need for uid as long as it's not referenced from other tables */
    {"name", fldoff(in_name), NSC_STRING, 0, NULL, EF_BAD, NSC_CONST},
    {"lcms", fldoff(in_lcms), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"hcms", fldoff(in_hcms), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"dcost", fldoff(in_dcost), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"mcost", fldoff(in_mcost), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {"enable", fldoff(in_enable), NSC_UCHAR, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr rpt_ca[] = {
#define CURSTR struct rptstr
    {"uid", fldoff(r_uid), NSC_CHAR, 0, NULL, EF_NEWS_CHR, 0},
    {"newstory", fldoff(r_newstory), NSC_STRING, NUM_RPTS, NULL,
     EF_BAD, 0},
    {"good_will", fldoff(r_good_will), NSC_INT, 0, NULL, EF_BAD, 0},
    {"newspage", fldoff(r_newspage), NSC_INT, 0, NULL,
     EF_PAGE_HEADINGS, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr update_ca[] = {
    {"time", 0, NSC_TIME, 0, NULL, EF_BAD, 0},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
};

struct castr empfile_ca[] = {
#define CURSTR struct empfile
    {"uid", fldoff(uid), NSC_INT, 0, NULL, EF_TABLE, 0},
    {"name", fldoff(name), NSC_STRING, 0, NULL, EF_BAD, NSC_CONST},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr symbol_ca[] = {
#define CURSTR struct symbol
    /*
     * value is is const because it has to match what is compiled into
     * the server.  name is const because clients key on it.
     */
    {"value", fldoff(value), NSC_INT, 0, NULL, EF_BAD, NSC_CONST},
    {"name", fldoff(name), NSC_STRING, 0, NULL, EF_BAD, NSC_CONST},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

struct castr mdchr_ca[] = {
#define CURSTR struct castr
    /* no need for uid */
    /* name must come first, clients may rely on it */
    {"name", fldoff(ca_name), NSC_STRING, 0, NULL, EF_BAD, NSC_CONST},
    {"type", fldoff(ca_type), NSC_CHAR, 0, NULL, EF_META_TYPE, NSC_CONST},
    {"flags", fldoff(ca_flags), NSC_UCHAR, 0, NULL,
     EF_META_FLAGS, NSC_CONST | NSC_BITS},
    {"len", fldoff(ca_len), NSC_USHORT, 0, NULL, EF_BAD, NSC_CONST},
    {"table", fldoff(ca_table), NSC_INT, 0, NULL, EF_BAD, NSC_CONST},
    {NULL, 0, NSC_NOTYPE, 0, NULL, EF_BAD, 0}
#undef CURSTR
};

void
nsc_init(void)
{
    static struct castr version_ca0 = {
	"version", 0, NSC_STRINGY, sizeof(PACKAGE_STRING), NULL, EF_BAD, 0
    };
    static struct castr version_ca1 = {
	"maxnoc", 0, NSC_LONG, 0, nsc_ver_maxnoc, EF_BAD, 0
    };
    static struct castr *ca;
    struct keymatch *kp;
    int n, i;
    unsigned flags;

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
	flags = nat_ca[i].ca_flags;
	if (flags & NSC_EXTRA)
	    flags &= ~NSC_EXTRA;
	else
	    flags |= NSC_DEITY;
	nat_ca[i].ca_flags = flags;
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
