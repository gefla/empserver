/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ef_verify.c: Verify game configuration
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2006-2008
 */

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "misc.h"
#include "nsc.h"
#include "plane.h"
#include "product.h"

static void verify_fail(int, int, struct castr *, int, char *, ...)
    ATTRIBUTE((format (printf, 5, 6)));

static void
verify_fail(int type, int row, struct castr *ca, int idx, char *fmt, ...)
{
    int i;
    va_list ap;

    /* Find base table of view, if any */
    for (i = 0; empfile[i].cache == empfile[type].cache; i++) ;

    fprintf(stderr, "%s %s uid %d",
	    EF_IS_GAME_STATE(i) ? "File" : "Config",
	    ef_nameof(type), row);
    if (ca) {
	fprintf(stderr, " field %s", ca->ca_name);
	if (ca->ca_type != NSC_STRINGY && ca->ca_len != 0)
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

    for (i = 0; ca[i].ca_name; i++) {
	/*
	 * Virtual selectors must be NSC_EXTRA, because xundump can't
	 * cope with them without setter methods.  Exception: if
	 * EFF_MEM is not set, xundump doesn't touch the table.
	 */
	if (CANT_HAPPEN((ef_flags(type) & EFF_MEM)
			&& ca[i].ca_get && !(ca[i].ca_flags & NSC_EXTRA)))
	    ca[i].ca_flags |= NSC_EXTRA;
    }
    return 0;
}

static int
verify_row(int type, int row)
{
    struct castr *ca = ef_cadef(type);
    struct emptypedstr *row_ref;
    int i, j, k, n;
    struct castr *ca_sym;
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

    for (i = 0; ca[i].ca_name; ++i) {
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	n = ca[i].ca_type != NSC_STRINGY ? ca[i].ca_len : 0;
	j = 0;
	do {
	    if (ca[i].ca_table == EF_BAD)
		continue;
	    nstr_mksymval(&val, &ca[i], j);
	    nstr_exec_val(&val, 0, row_ref, NSC_NOTYPE);
	    if (val.val_type != NSC_LONG)
		continue;
	    ca_sym = ef_cadef(ca[i].ca_table);
	    if (ca[i].ca_flags & NSC_BITS) {
		/* symbol set */
		if (CANT_HAPPEN(ca_sym != symbol_ca)) {
		    ret_val = -1;
		    continue;
		}
		for (k = 0; k < (int)sizeof(long) * 8; k++) {
		    if (val.val_as.lng & (1L << k))
			if (!symbol_by_value(1L << k,
					     ef_ptr(ca[i].ca_table, 0))) {
			    verify_fail(type, row, &ca[i], j,
					"bit %d is not in symbol table %s",
					k, ef_nameof(ca[i].ca_table));
			    ret_val = -1;
			}
		}
	    } else if (ca[i].ca_table == type && i == 0) {
		/* uid */
		/* Some files contain zeroed records, cope */
		/* TODO tighten this check */
		if (val.val_as.lng == 0)
		    continue;
		if (val.val_as.lng != row) {
		    verify_fail(type, row, &ca[i], j,
				"value is %ld instead of %d",
				val.val_as.lng, row);
		    ret_val = -1;
		}

	    } else if (ca_sym == symbol_ca) {
		/* symbol */
		if (!symbol_by_value(val.val_as.lng,
				     ef_ptr(ca[i].ca_table, 0))) {
		    verify_fail(type, row, &ca[i], j,
				"value %ld is not in symbol table %s",
				val.val_as.lng, ef_nameof(ca[i].ca_table));
		    ret_val = -1;
		}
	    } else {
		/* table index */
		if (val.val_as.lng >= ef_nelem(ca[i].ca_table)
		    || val.val_as.lng < -1) {
		    verify_fail(type, row, &ca[i], j,
			"value %ld indexes table %s out of bounds 0..%d",
			val.val_as.lng, ef_nameof(ca[i].ca_table),
			ef_nelem(ca[i].ca_table));
		    ret_val = -1;
		}
	    }
	} while (++j < n);
    } 
    if (!(flags & EFF_MEM))
        free(row_ref);
    return ret_val;
}

static void
pln_zap_transient_flags(void)
{
    int i;
    struct plnstr *pp;
    int dirty = 0;

    /* laziness: assumes plane file is EFF_MEM */
    for (i = 0; (pp = getplanep(i)) != NULL; i++) {
	if (pp->pln_flags & PLN_LAUNCHED
	    && (plchr[pp->pln_type].pl_flags & (P_M | P_O)) != P_O) {
	    pp->pln_flags &= ~PLN_LAUNCHED;
	    /* FIXME missile should be destroyed instead */
	    dirty = 1;
	    verify_fail(EF_PLANE, i, NULL, 0, "stuck in the air (fixed)");
	    /*
	     * Can't putplane() here, because pln_prewrite() crashes
	     * without a valid player.
	     */
	}
    }
    if (dirty)
	ef_flush(EF_PLANE);	/* pretty wasteful */
}

int
ef_verify()
{
    struct empfile *ep;
    int retval = 0;
    int i;

    for (ep = empfile; ep->name; ep++) {
	if (!ef_cadef(ep->uid))
	    continue;
	verify_ca(ep->uid);
	for (i = 0; i < ef_nelem(ep->uid); i++) {
	    retval += verify_row(ep->uid, i);
	}
    }

    /* Special checks */
    for (i = 0; pchr[i].p_sname; i++) {
	if ((pchr[i].p_type >= 0) == (pchr[i].p_level >= 0)) {
	    fprintf(stderr,
		"Config %s uid %d field level doesn't match field type\n",
		ef_nameof(EF_PRODUCT), i);
	    retval = -1;
	}
    }

    pln_zap_transient_flags();
    return retval;
}
