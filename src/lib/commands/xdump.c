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
 *  xdump.c: Experimental extended dump
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004
 */

#include <stddef.h>
#include "misc.h"
#include "file.h"
#include "match.h"
#include "news.h"
#include "nsc.h"
#include "optlist.h"

/*
 * Dump everything under the sun
 *
 * Static game data (configuration):
 * - Item characteristics: ichr[]
 * - Land unit characteristics: lchr[]
 * - Nuke characteristics: nchr[]
 * - Plane characteristics: plchr[]
 * - Product characteristics: pchr[]
 * - Sector designation characteristics: dchr[]
 * - Sector infrastructure characteristics: intrchr[]
 * - Ship characteristics: mchr[]
 * Less important:
 * - News item characteristics: rpt[] (TODO)
 * - News page headings: page_headings[] (TODO)
 * - Treaty clause characteristics: tchr[]
 * - Commands: coms[] (TODO)
 * - Options: Options[]
 * - Configuration: configkeys[]
 *
 * Dynamic game data:
 * - Sectors: EF_SECTOR, sect_ca[] (already have dump)
 * - Land units: EF_LAND, land_ca[] (already have ldump)
 * - Lost: EF_LOST, lost_ca[] (already have lost)
 * - Nukes: EF_NUKE, nuke_ca[] (already have ndump)
 * - Planes: EF_PLANE, plane_ca[] (already have pdump)
 * - Ships: EF_SHIP, ship_ca[] (already have sdump)
 * - News: EF_NEWS, news_ca[]
 * - Treaties: EF_TREATY, treaty_ca[]
 * - Power: EF_POWER
 * - Nations: EF_NATION, nat_ca[]
 * - Loans: EF_LOAN, loan_ca[]
 * - Map: EF_MAP (TODO)
 * - Bmap: EF_BMAP (TODO)
 * - Market: EF_COMM, commodity_ca[]
 */

/* FIXME document dump format */
/* FIXME don't dump stuff that's useless due to options */

/* Selector descriptors for characteristics tables */
/* FIXME belongs into src/lib/global/ */

/* Return nsc_type for a signed integer with the same size as TYPE.  */
#define NSC_SITYPE(type)				\
    (sizeof(type) == 1 ? NSC_CHAR			\
     : sizeof(type) == sizeof(short) ? NSC_SHORT	\
     : sizeof(type) == sizeof(int) ? NSC_INT		\
     : sizeof(type) == sizeof(long) ? NSC_LONG		\
     : 1/0)

static struct castr dchr_ca[] = {
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
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

static struct castr ichr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct ichrstr, i_name), "name"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_mnem), "mnem"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_vtype), "vtype"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_value), "value"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_sell), "sell"},
    {NSC_INT, 0, 0, offsetof(struct ichrstr, i_lbs), "lbs"},
    {NSC_INT, 0, NUMPKG, offsetof(struct ichrstr, i_pkg), "pkg"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

static struct castr intrchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct sctintrins, in_name), "name"},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_lcms), "lcms"},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_hcms), "hcms"},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_dcost), "dcost"},
    {NSC_UCHAR, 0, 0, offsetof(struct sctintrins, in_mcost), "mcost"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

static struct castr rpt_ca[] = {
    {NSC_STRING, 0, NUM_RPTS, offsetof(struct rptstr, r_newstory), "newstory"},
    {NSC_INT, 0, 0, offsetof(struct rptstr, r_good_will), "good_will"},
    {NSC_INT, 0, 0, offsetof(struct rptstr, r_newspage), "newspage"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

static struct castr tchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct tchrstr, t_name), "name"},
    {NSC_INT, 0, 0, offsetof(struct tchrstr, t_cond), "cond"},
    {NSC_NOTYPE, 0, 0, 0, NULL}
};

static struct castr mchr_ca[] = {
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

static struct castr pchr_ca[] = {
    {NSC_STRING, 0, 0, offsetof(struct pchrstr, p_name), "name"},
    {NSC_STRING, 0, 0, offsetof(struct pchrstr, p_sname), "sname"},
    {NSC_UCHAR, 0, MAXPRCON, offsetof(struct pchrstr, p_ctype), "ctype"},
    {NSC_USHORT, 0, MAXPRCON, offsetof(struct pchrstr, p_camt), "camt"},
    {NSC_INT, 0, 0, offsetof(struct pchrstr, p_type), "type"},
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

static struct castr plchr_ca[] = {
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

static struct castr lchr_ca[] = {
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

static struct castr nchr_ca[] = {
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

/* Characteristics table meta data */
struct camap {
    char *name;			/* name for lookup */
    struct castr *ca;		/* selector descriptors */
    void *chr;			/* characteristics table */
    size_t size;		/* size of characteristics table element */
};

/* Table of characteristics tables */
static struct camap chr_camap[] = {
    {"sect chr", dchr_ca, dchr, sizeof(dchr[0])},
    {"ship chr", mchr_ca, mchr, sizeof(mchr[0])},
    {"plane chr", plchr_ca, plchr, sizeof(plchr[0])},
    {"land chr", lchr_ca, lchr, sizeof(lchr[0])},
    {"nuke chr", nchr_ca, nchr, sizeof(nchr[0])},
#if 0
    /* FIXME rpt[] lacks sentinel, xdchr() doesn't terminate */
    {"news chr", rpt_ca, rpt, sizeof(rpt[0])},
#endif
    {"treaty chr", tchr_ca, tchr, sizeof(tchr[0])},
    {"item", ichr_ca, ichr, sizeof(ichr[0])},
    {"infrastructure", intrchr_ca, intrchr, sizeof(intrchr[0])},
    {"product", pchr_ca, pchr, sizeof(pchr[0])},
    {NULL, NULL, NULL, 0}
};

/*
 * Search chr_camap[] for element named NAME, return its index.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 */
static int
chridx_by_name(char *name)
{
    return stmtch(name, chr_camap, offsetof(struct camap, name),
		  sizeof(chr_camap[0]));
}

/*
 * Evaluate a attribute of an object into VAL, return VAL.
 * TYPE is the attribute's type.
 * PTR points to the context object.
 * The attribute is stored there at offset OFF + IDX * S, where S is
 * its size.
 */
static struct valstr *
xdeval(struct valstr *val, nsc_type type, void *ptr, ptrdiff_t off, int idx)
{
    val->val_type = type;
    val->val_cat = NSC_OFF;
    val->val_as_type = -1;
    val->val_as.sym.off = off;
    val->val_as.sym.idx = idx;
    nstr_exec_val(val, player->cnum, ptr, NSC_NOTYPE);
    return val;			/* FIXME nstr_exec_val() should return VAL */
}

/* Dump VAL prefixed with SEP, return " ".  */
static char *
xdprval(struct valstr *val, char *sep)
{
    unsigned char *s, *e, *l;

    switch (val->val_type) {
    case NSC_TYPEID:
    case NSC_LONG:
	pr("%s%ld", sep, val->val_as.lng);
	break;
    case NSC_DOUBLE:
	pr("%s%#g", sep, val->val_as.dbl);
	break;
    case NSC_STRING:
	s = (unsigned char *)val->val_as.str.base;
	if (s) {
	    pr("%s\"", sep);
	    l = s + val->val_as.str.maxsz;
	    for (;;) {
		for (e=s; e<l && *e != '"' && *e != '\\' && isgraph(*e); ++e) ;
		pr("%.*s", (int)(e-s), s);
		if (e < l && *e)
		    pr("\\%03o", *e++);
		else
		    break;
		s = e;
	    }
	    prnf("\"");
	} else
	    pr("%snil", sep);
	break;
    default:
	CANT_HAPPEN("Bad VAL type");
	pr("0");
    }
    return " ";
}

/*
 * Dump field values of a context object.
 * CA[] describes fields.
 * PTR points to context object.
 */
static void
xdflds(struct castr ca[], void *ptr)
{
    int i, j, n;
    struct valstr val;
    char *sep = "";

    for (i = 0; ca[i].ca_name; ++i) {
	if (ca[i].ca_flags & NSC_DEITY && !player->god)
	    continue;
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	n = ca[i].ca_type != NSC_STRINGY ? ca[i].ca_len : 0;
	j = 0;
	do {
	    xdeval(&val, ca[i].ca_type, ptr, ca[i].ca_off, j);
	    sep = xdprval(&val, sep);
	} while (++j < n);
    }
}

/* Dump field names; CA[] describes fields.  */
static void
xdfldnam(struct castr ca[])
{
    int i;
    char *sep = "";

    for (i = 0; ca[i].ca_name; ++i) {
	if (ca[i].ca_flags & NSC_DEITY && !player->god)
	    continue;
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	pr("%s%s", sep, ca[i].ca_name);
	if (ca[i].ca_len && ca[i].ca_type != NSC_STRINGY)
	    pr(" %d", ca[i].ca_len);
	sep = " ";
    }
}

/* Dump first line of header for dump NAME.  */
static void
xdhdr1(char *name)
{
    pr("XDUMP %s %ld\n", name, (long)time(NULL));
}

/* Dump header for dump NAME with fields described by CA[].  */
static void
xdhdr(char *name, struct castr ca[])
{
    xdhdr1(name);
    xdfldnam(ca);
    pr("\n");
}

/* Dump footer for a dump that dumped N objects.  */
static void
xdftr(int n)
{
    pr("dumped %d\n", n);
}

/*
 * Dump items of type TYPE selected by ARG.
 * Return RET_OK on success, RET_SYN on error.
 */
static int
xditem(int type, char *arg)
{
    struct castr *ca;
    struct nstr_item ni;
    int n;
    char buf[2048];		/* FIXME buffer size? */

    ca = ef_cadef(type);
    if (!ca)
	return RET_SYN;

    if (!snxtitem(&ni, type, arg))
	return RET_SYN;

    xdhdr(ef_nameof(type), ca);

    n = 0;
    while (nxtitem(&ni, buf)) {
	if (!player->owner)
	    continue;
	++n;
	xdflds(ca, buf);
	pr("\n");
    }

    xdftr(n);

    return RET_OK;
}

/*
 * Dump characteristics described by chr_camap[IDX].
 * Return RET_OK on success, RET_SYN if IDX < 0.
 */
static int
xdchr(int chridx)
{
    struct camap *cm;
    char *p;
    struct valstr val;
    int n;

    if (chridx < 0)
	return RET_SYN;
    cm = &chr_camap[chridx];

    xdhdr(cm->name, cm->ca);

    n = 0;
    for (p = cm->chr; ; p += cm->size) {
	val.val_type = cm->ca[0].ca_type;
	val.val_cat = NSC_OFF;
	val.val_as_type = -1;
	val.val_as.sym.off = cm->ca[0].ca_off;
	val.val_as.sym.idx = 0;
	nstr_exec_val(&val, player->cnum, p, NSC_STRING);
	if (!val.val_as.str.base || !*val.val_as.str.base)
	    break;
	++n;
	xdflds(cm->ca, p);
	pr("\n");
    }

    xdftr(n);

    return RET_OK;
}

/* Dump Options[], return RET_OK.  */
static int
xdopt(void)
{
    int i;
    char *sep;

    xdhdr1("options");

    sep = "";
    for (i = 0; Options[i].opt_key; ++i) {
	pr("%s%s", sep, Options[i].opt_key);
	sep = " ";
    }
    pr("\n");
    
    sep = "";
    for (i = 0; Options[i].opt_key; ++i) {
	pr("%s%d", sep, *Options[i].opt_valuep);
	sep = " ";
    }
    pr("\n");

    return RET_OK;
}

static int
xdver(void)
{
    struct keymatch *kp;
    char *sep;
    struct valstr val;

    xdhdr1("version");

    sep = "";
    for (kp = configkeys; kp->km_key; ++kp) {
	if (kp->km_type != NSC_NOTYPE && !(kp->km_flags & KM_INTERNAL)) {
	    pr("%s%s", sep, kp->km_key);
	    sep = " ";
	}
    }
    pr("\n");
    
    sep = "";
    for (kp = configkeys; kp->km_key; ++kp) {
	if (kp->km_type != NSC_NOTYPE && !(kp->km_flags & KM_INTERNAL)) {
	    xdeval(&val, kp->km_type, kp->km_data, 0, 0);
	    sep = xdprval(&val, sep);
	}
    }
    pr("\n");

    return RET_OK;
}

/* Experimental extended dump command */
int
xdump(void)
{
    char *p;
    char buf[1024];
    int type;

    p = getstarg(player->argp[1], "What? ", buf);
    if (!p)
	return RET_SYN;

    type = ef_byname(p);
    if (type >= 0) {
	return xditem(type, player->argp[2]);
    } else if (!strncmp(p, "chr", strlen(p)) && player->argp[2]) {
	return xdchr(chridx_by_name(player->argp[2]));
    } else if (!strncmp(p, "opt", strlen(p))) {
	return xdopt();
    } else if (!strncmp(p, "ver", strlen(p))) {
	return xdver();
    }

    return RET_SYN;
}
