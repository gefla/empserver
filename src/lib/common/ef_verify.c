/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ef_verify.c: Verify game configuration
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2006-2021
 */

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "empobj.h"
#include "misc.h"
#include "nsc.h"
#include "product.h"
#include "xdump.h"

static void verify_fail(int, int, struct castr *, int, char *, ...)
    ATTRIBUTE((format (printf, 5, 6)));

static void
verify_fail(int type, int row, struct castr *ca, int idx, char *fmt, ...)
{
    int base = empfile[type].base < 0 ? type : empfile[type].base;
    va_list ap;

    fprintf(stderr, "%s %s uid %d",
	    EF_IS_GAME_STATE(base) ? "File" : "Config",
	    ef_nameof(type), row);
    if (ca) {
	fprintf(stderr, " field %s", ca->ca_name);
	if (CA_IS_ARRAY(ca))
	    fprintf(stderr, "(%d)", idx);
    }
    fprintf(stderr, ": ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);
}

static int
verify_ca(int type)
{
    struct castr *ca = ef_cadef(type);
    int i;

    for (i = 0; ca && ca[i].ca_name; i++) {
	/*
	 * Virtual selectors can't be used in xundump, since we lack a
	 * setter to go with ca_get().
	 */
	if (CANT_HAPPEN(xundumpable(type)
			&& ca[i].ca_get
			&& ca[i].ca_dump <= CA_DUMP_CONST))
	    ca[i].ca_dump = CA_DUMP_ONLY;
    }
    return 0;
}

static int
verify_tabref(int type, int row, struct castr *ca, int idx, long val)
{
    int tabno = ca->ca_table;
    struct castr *ca_sym = ef_cadef(tabno);
    int i;

    if (ca->ca_flags & NSC_BITS) {
	/* symbol set */
	if (CANT_HAPPEN(ca_sym != symbol_ca))
	    return -1;
	for (i = 0; i < (int)sizeof(long) * 8; i++) {
	    if (val & (1UL << i)) {
		if (!symbol_by_value(1L << i, ef_ptr(tabno, 0))) {
		    verify_fail(type, row, ca, idx,
				"bit %d is not in symbol table %s",
				i, ef_nameof(tabno));
		    return -1;
		}
	    }
	}
    } else if (ca_sym == symbol_ca) {
	/* symbol */
	if (!symbol_by_value(val, ef_ptr(tabno, 0))) {
	    verify_fail(type, row, ca, idx,
			"value %ld is not in symbol table %s",
			val, ef_nameof(tabno));
	    return -1;
	}
    } else {
	/* table index */
	if (val >= ef_nelem(tabno) || val < -1) {
	    verify_fail(type, row, ca, idx,
			"value %ld indexes table %s out of bounds 0..%d",
			val, ef_nameof(tabno), ef_nelem(tabno));
	    return -1;
	}
	/* laziness: assumes TABNO is EFF_MEM */
	if (val >= 0 && !empobj_in_use(tabno, ef_ptr(tabno, val))) {
	    verify_fail(type, row, ca, idx,
			"value %ld refers to missing element of table %s",
			val, ef_nameof(tabno));
	    return -1;
	}
    }
    return 0;
}

static int
verify_row(int type, int row)
{
    struct castr *ca = ef_cadef(type);
    struct empobj *row_ref;
    int i, j, n;
    struct valstr val;
    int ret_val = 0;
    int flags = ef_flags(type);

    if (flags & EFF_MEM)
	row_ref = ef_ptr(type, row);
    else {
	row_ref = malloc(empfile[type].size);
	ef_read(type, row, row_ref);
    }

    if ((flags & EFF_TYPED) && !EF_IS_VIEW(type)) {
	if (row_ref->ef_type != type || row_ref->uid != row) {
	    verify_fail(type, row, NULL, 0, "header corrupt");
	    ret_val = -1;
	}
    }

    if (!empobj_in_use(type, row_ref))
	goto out;

    for (i = 0; ca && ca[i].ca_name; ++i) {
	if (ca[i].ca_get)
	    continue;		/* virtual */
	n = CA_ARRAY_LEN(&ca[i]);
	j = 0;
	do {
	    if (ca[i].ca_table == EF_BAD)
		continue;
	    nstr_mksymval(&val, &ca[i], j);
	    nstr_eval(&val, 0, row_ref, NSC_NOTYPE);
	    if (CANT_HAPPEN(val.val_type != NSC_LONG)) {
		ret_val = -1;
		continue;
	    }
	    if (ca[i].ca_table == type && i == 0) {
		/* UID */
		if (val.val_as.lng != row) {
		    verify_fail(type, row, &ca[i], j,
				"value is %ld instead of %d",
				val.val_as.lng, row);
		    ret_val = -1;
		}
	    } else {
		if (verify_tabref(type, row, &ca[i], j, val.val_as.lng) < 0)
		    ret_val = -1;
	    }
	} while (++j < n);
    }

out:
    if (!(flags & EFF_MEM))
	free(row_ref);
    return ret_val;
}

static int
verify_table(int type)
{
    int retval = 0;
    int i;

    verify_ca(type);
    for (i = 0; i < ef_nelem(type); i++)
	retval |= verify_row(type, i);
    return retval;
}

static int
verify_sectors(int may_put)
{
    int i;
    struct sctstr *sp;
    coord x, y;

    /* laziness: assumes sector file is EFF_MEM */
    for (i = 0; (sp = getsectid(i)); i++) {
	sctoff2xy(&x, &y, sp->sct_uid);
	if (sp->sct_x != x || sp->sct_y != y) {
	    sp->sct_x = x;
	    sp->sct_y = y;
	    if (may_put)
		putsect(sp);
	    verify_fail(EF_SECTOR, i, NULL, 0, "bogus coordinates (fixed)");
	}
    }
    return 0;
}

static int
verify_planes(int may_put)
{
    int retval = 0;
    int i;
    struct plnstr *pp;

    /* laziness: assumes plane file is EFF_MEM */
    for (i = 0; (pp = getplanep(i)); i++) {
	if (pp->pln_own) {
	    if (pp->pln_flags & PLN_LAUNCHED
		&& (plchr[pp->pln_type].pl_flags & (P_M | P_O)) != P_O) {
		pp->pln_flags &= ~PLN_LAUNCHED;
		/* FIXME missile should be destroyed instead */
		if (may_put)
		    putplane(i, pp);
		verify_fail(EF_PLANE, i, NULL, 0, "stuck in the air (fixed)");
	    }
	    if (pp->pln_ship >= 0 && pp->pln_land >= 0) {
		verify_fail(EF_PLANE, i, NULL, 0, "on two carriers");
		retval = -1;
	    }
	} else {
	    if (pp->pln_ship >= 0 || pp->pln_land >= 0) {
		pp->pln_ship = pp->pln_land = -1;
		if (may_put)
		    putplane(i, pp);
		verify_fail(EF_PLANE, i, NULL, 0,
			    "ghost stuck on carrier (fixed)");
	    }
	}
    }
    return retval;
}

static int
verify_lands(int may_put)
{
    int retval = 0;
    int i;
    struct lndstr *lp;

    /* laziness: assumes land file is EFF_MEM */
    for (i = 0; (lp = getlandp(i)); i++) {
	if (lp->lnd_own) {
	    if (lp->lnd_ship >= 0 && lp->lnd_land >= 0) {
		verify_fail(EF_LAND, i, NULL, 0, "on two carriers");
		retval = -1;
	    }
	} else {
	    if (lp->lnd_ship >= 0 || lp->lnd_land >= 0) {
		lp->lnd_ship = lp->lnd_land = -1;
		if (may_put)
		    putland(i, lp);
		verify_fail(EF_LAND, i, NULL, 0,
			    "ghost stuck on carrier (fixed)");
	    }
	}
    }
    return retval;
}

static int
verify_nukes(int may_put)
{
    int retval = 0;
    int i;
    struct nukstr *np;

    /* laziness: assumes nuke file is EFF_MEM */
    for (i = 0; (np = getnukep(i)); i++) {
	if (!np->nuk_own) {
	    if (np->nuk_plane >= 0) {
		np->nuk_plane = -1;
		if (may_put)
		    putnuke(i, np);
		verify_fail(EF_NUKE, i, NULL, 0,
			    "ghost stuck on carrier (fixed)");
	    }
	}
    }
    return retval;
}

static int
verify_ship_chr(void)
{
    int retval = 0;
    int i;

    for (i = 0; mchr[i].m_name; i++) {
	if (!mchr[i].m_name[0])
	    continue;
	if ((mchr[i].m_flags & M_DCH) && !mchr[i].m_glim) {
	    verify_fail(EF_SHIP_CHR, i, NULL, 0,
			"flag %s requires non-zero glim",
			symbol_by_value(M_DCH, ship_chr_flags));
	    retval = -1;
	}
	if (mchr[i].m_nplanes && !(mchr[i].m_flags & (M_MSL | M_FLY))) {
	    verify_fail(EF_SHIP_CHR, i, NULL, 0,
			"non-zero nplanes needs flag %s or %s",
			symbol_by_value(M_FLY, ship_chr_flags),
			symbol_by_value(M_MSL, ship_chr_flags));
	    retval = -1;
	}
    }
    return retval;
}

static int
verify_plane_chr(void)
{
    int retval = 0;
    int i, flags, accepted_flags;
    char buf[1024];

    for (i = 0; plchr[i].pl_name; i++) {
	if (!plchr[i].pl_name[0])
	    continue;
	flags = plchr[i].pl_flags;
	accepted_flags = P_V | P_K | P_L;
	if (flags & P_M) {
	    /* missile */
	    accepted_flags |= P_M | P_E;
	    if (flags & P_N)
		accepted_flags |=  P_N;
	    else if (flags & P_O)
		accepted_flags |=  P_O;
	    else if (flags & P_F)
		accepted_flags |=  P_F;
	    else
		accepted_flags |=  P_T | P_MAR;
	    if (!(flags & P_V)) {
		verify_fail(EF_PLANE_CHR, i, NULL, 0,
			    "flag %s requires flag %s",
			    symbol_by_value(P_M, plane_chr_flags),
			    symbol_by_value(P_V, plane_chr_flags));
		retval = -1;
	    }
	} else if (flags & P_O) {
	    /* satellite */
	    accepted_flags |= P_O | P_S | P_I;
	} else {
	    /* plane */
	    accepted_flags |= P_B | P_T | P_F | P_C | P_S | P_I | P_A | P_P
		| P_ESC | P_MINE | P_SWEEP;
	    if ((flags & (P_P | P_C)) == P_P) {
		verify_fail(EF_PLANE_CHR, i, NULL, 0,
			    "flag %s requires flag %s",
			    symbol_by_value(P_P, plane_chr_flags),
			    symbol_by_value(P_C, plane_chr_flags));
		retval = -1;
	    }
	}
	if (flags & ~accepted_flags) {
	    symbol_set_fmt(buf, sizeof(buf), flags & ~accepted_flags,
			   plane_chr_flags, ", ", 1);
	    verify_fail(EF_PLANE_CHR, i, NULL, 0,
			"invalid flag combination, can't have %s", buf);
	    retval = -1;
	}
    }
    return retval;
}

static int
verify_land_chr(void)
{
    int retval = 0;
    int i;

    for (i = 0; lchr[i].l_name; i++) {
	if (!lchr[i].l_name[0])
	    continue;
	if ((lchr[i].l_flags & L_SPY) && lchr[i].l_item[I_MILIT]) {
	    verify_fail(EF_LAND_CHR, i, NULL, 0,
			"flag %s requires zero milit",
			symbol_by_value(L_SPY, land_chr_flags));
	    retval = -1;
	}
    }
    return retval;
}

static int
verify_products(void)
{
    int retval = 0;
    int i;

    /* product makes either level or item, not both */
    for (i = 0; pchr[i].p_sname; i++) {
	if (!pchr[i].p_sname[0])
	    continue;
	if ((pchr[i].p_type >= 0) == (pchr[i].p_level >= 0)) {
	    verify_fail(EF_PRODUCT, i, NULL, 0,
			"level must be none or type -1");
	    retval = -1;
	}
    }
    return retval;
}

/*
 * Verify game configuration is sane.
 * Return 0 on success, -1 on failure.
 */
int
ef_verify_config(void)
{
    int retval = 0;
    int i;

    for (i = 0; i < EF_MAX; i++) {
	if (!EF_IS_GAME_STATE(i) && !EF_IS_VIEW(i))
	    retval |= verify_table(i);
    }

    /* Special checks */
    retval |= verify_ship_chr();
    retval |= verify_plane_chr();
    retval |= verify_land_chr();
    retval |= verify_products();
    return retval;
}

/*
 * Verify game state is sane.
 * Correct minor problems, but write corrections to backing files only
 * if @may_put is non-zero.
 * Return -1 if uncorrected problems remain, else 0.
 */
int
ef_verify_state(int may_put)
{
    int retval = 0;
    int i;

    for (i = 0; i < EF_MAX; i++) {
	if (EF_IS_GAME_STATE(i) || EF_IS_VIEW(i))
	    retval |= verify_table(i);
    }

    /* Special checks */
    retval |= verify_sectors(may_put);
    retval |= verify_planes(may_put);
    retval |= verify_lands(may_put);
    retval |= verify_nukes(may_put);
    return retval;
}
