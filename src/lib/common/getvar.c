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
 *  getvar.c: Routines for manipulating variable lists.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include "misc.h"
#include "var.h"
#include "file.h"
#include "common.h"

int
getvar(int vtype, s_char *sp, int ptype)
{
	u_char	*vtypep;
	u_short	*vamtp;
	u_char	*nvp;
	int	amt;

	if (ef_vars(ptype, sp, &nvp, &vtypep, &vamtp) < 0) {
		logerror("getvar: ptype %d has no vars", ptype);
		return 0;
	}
	amt = vl_find(vtype, vtypep, vamtp, (int) *nvp);
	if (amt < 0) {
		logerror("getvar: vl_find returns %d, vtype %d",
			amt, vtype);
		return 0;
	}
	return amt;
}

int
getvec(int class, int *vec, s_char *sp, int ptype)
{
	u_char	*vtypep;
	u_short	*vamtp;
	u_char	*nvp;
	int	nv;

	if (ef_vars(ptype, sp, &nvp, &vtypep, &vamtp) < 0) {
		logerror("getvec: ptype %d has no vars", ptype);
		return 0;
	}
	nv = vl_getvec(vtypep, vamtp, (int)*nvp, class, vec);
	if (nv < 0) {
		logerror("vl_getvec: returns %d, ptype %d\n",
			nv, ptype);
		return 0;
	}
	return nv;
}

int
putvar(int vtype, int amt, s_char *sp, int ptype)
{
	u_char	*vtypep;
	u_short	*vamtp;
	u_char	*nvp;
	int	maxv;

	if (vtype < 0 || vtype > V_MAX) {
		logerror("putvar: bad vtype %d\n", vtype);
		return 0;
	}
	if ((maxv = ef_vars(ptype, sp, &nvp, &vtypep, &vamtp)) < 0) {
		logerror("putvar: ptype %d has no vars", ptype);
		return 0;
	}
	if (amt < 0)
		amt = 0;
	return vl_set(vtype, (u_int)amt, vtypep, vamtp, nvp, maxv);
}

int
putvec(int class, int *vec, s_char *sp, int ptype)
{
	u_char	*vtypep;
	u_short	*vamtp;
	u_char	*nvp;
	int	maxv,x;

	if ((maxv = ef_vars(ptype, sp, &nvp, &vtypep, &vamtp)) < 0) {
		logerror("putvec: ptype %d has no vars", ptype);
		return 0;
	}
	for(x=0;x<I_MAX;x++)
		if (vec[x] < 0)
			vec[x] = 0;
	return vl_setvec(vtypep, vamtp, nvp, maxv, class, vec);
}
