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
 *  ef_verify.c: Verify game configuration
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 */

#include <config.h>

#include <stdio.h>
#include <time.h>

#include "prototypes.h"
#include "file.h"
#include "nsc.h"

static int
verify_row(int type, int row)
{
    struct castr *ca = ef_cadef(type);
    void *row_ref;
    int i, j, k, n;
    struct castr *ca_sym;
    struct valstr val;
    int ret_val = 0; 
    int in_mem = (ef_flags(type) & EFF_MEM) != 0; 
 
    if (in_mem)
	row_ref = ef_ptr(type, row); 
    else {
	row_ref = malloc(empfile[type].size); 
	ef_read(type, row, row_ref);
    }

    for (i = 0; ca[i].ca_name; ++i) {
	if (ca[i].ca_flags & NSC_EXTRA)
	    continue;
	n = ca[i].ca_type != NSC_STRINGY ? ca[i].ca_len : 0;
	j = 0;
	do {
	    if (ca[i].ca_table == EF_BAD)
		continue;
	    val.val_type = ca[i].ca_type;
	    val.val_cat = NSC_OFF;
	    val.val_as.sym.off = ca[i].ca_off;
	    val.val_as.sym.idx = j;
	    nstr_exec_val(&val, 0, row_ref, NSC_NOTYPE);
	    if (val.val_type != NSC_LONG && val.val_type != NSC_TYPEID)
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
			if (!symbol_by_value(1L << k, ef_ptr(ca[i].ca_table, 0))) {
			    fprintf(stderr,
				    "bit %d not found in symbol table %s "
				    "when verify table %s row %d field %s\n",
				    k, ef_nameof(ca[i].ca_table),
				    ef_nameof(type), row + 1, ca[i].ca_name);
			    ret_val = -1;
			}
		}
	    } else if (ca[i].ca_table == type && i == 0) {
		/* uid */
		if (val.val_as.lng != row) {
		    fprintf(stderr,
			    "table %s row %d field %s is %ld instead of %d\n",
			    ef_nameof(type), row + 1, ca[i].ca_name,
			    val.val_as.lng, row);
		    ret_val = -1;
		}

	    } else if (ca_sym == symbol_ca) {
		/* symbol */
		if (val.val_as.lng > -1) {
		    if (!symbol_by_value(val.val_as.lng,
					 ef_ptr(ca[i].ca_table,0))) {
			fprintf(stderr, "value %ld not found in "
				"symbol table %s when verify table %s "
				"row %d field %s\n",
				val.val_as.lng,
				ef_nameof(ca[i].ca_table),
				ef_nameof(type), row + 1,
				ca[i].ca_name);
			ret_val = -1;
		    }
		}
	    } else {
		/* table index */
		if (val.val_as.lng >= ef_nelem(ca[i].ca_table) ||
		    val.val_as.lng < -2) {
		    fprintf(stderr, "Table index %ld to table %s "
			    "out of range, nelements %d for table %s "
			    "row %d field %s\n",
			    val.val_as.lng, ef_nameof(ca[i].ca_table),
			    ef_nelem(ca[i].ca_table), ef_nameof(type), 
			    row + 1, ca[i].ca_name);
		    ret_val = -1;
		}
	    }
	} while (++j < n);
    } 
    if (!in_mem)
        free(row_ref);
    return ret_val;
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
	for (i = 0; i < ef_nelem(ep->uid); i++) {
	    retval += verify_row(ep->uid, i);
	}
    }
    return retval;
}
