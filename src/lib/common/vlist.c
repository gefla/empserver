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
 *  vlist.c: manage variable lists
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include "misc.h"
#include "var.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "product.h"
#include "optlist.h"
#include "common.h"
#include "gen.h"

static int freeslot(u_char *vec, register u_char *end);

int
vl_find(register int vtype, u_char *typevec, u_short *amtvec, int nelem)
{
    register u_char *vp;
    register u_short *ap;
    register u_char *endp;

    if (vtype < 0 || vtype > V_MAX)
	return -1;
    vp = typevec;
    ap = amtvec;
    endp = vp + nelem;
    for (; vp < endp; vp++, ap++) {
	if (*vp == vtype)
	    return (int)*ap;
    }
    return 0;
}

int
vl_set(register int vtype, u_int amt, u_char *typevec, u_short *amtvec,
       u_char *nvp, int max)
{
    register u_char *vp;
    register u_char *endp;
    register u_short *ap;
    int n;

    if (vtype < 0 || vtype > V_MAX)
	return 0;
    if (amt > 65536) {
	logerror("vl_set: bad amt %d (%x), vtype %d\n", (int)amt, amt,
		 vtype);
	return 0;
    }
    vp = typevec;
    ap = amtvec;
    endp = vp + *nvp;
    for (; vp < endp; vp++, ap++) {
	if (*vp == vtype)
	    break;
    }
    if (vp == endp) {
	if (amt == 0) {
	    /* deleting, but not present */
	    return 0;
	}
	if (*nvp == max) {
	    if (isdel(vtype) || isdist(vtype))
		return -1;
	    /* replace any del or dst entries */
	    if ((n = freeslot(typevec, endp)) < 0)
		return -1;
	    vp = &typevec[n];
	    ap = &amtvec[n];
	} else {
	    /* add at end */
	    (*nvp)++;
	}
	*vp = vtype;
	*ap = amt;
    } else {
	if (amt != 0) {
	    /* altering; just change value */
	    *ap = amt;
	    return 1;
	}
	(*nvp)--;
	if (vp < endp - 1) {
	    /* if not last element, copy last to current */
	    *ap = amtvec[*nvp];
	    *vp = typevec[*nvp];
	}
    }
    return 1;
}

int
vl_damage(register int pct, register u_char *typevec,
	  register u_short *amtvec, register int nelem)
{
    register int i;
    register int lose;
    extern double people_damage;

    if (pct >= 100)
	return 0;
    if (pct <= 0)
	return nelem;
    for (i = 0; i < nelem; i++) {
	if (!isitem(typevec[i]))
	    continue;
	if (opt_SUPER_BARS && typevec[i] == V_BAR)
	    continue;
	lose = roundavg((double)amtvec[i] * pct * 0.01);
	if (typevec[i] == V_CIVIL ||
	    typevec[i] == V_MILIT || typevec[i] == V_UW)
	    lose = ldround(people_damage * lose, 1);
	if ((amtvec[i] -= lose) + 1 == 0) {
	    nelem--;
	    if (i == nelem)
		break;
	    typevec[i] = typevec[nelem];
	    amtvec[i] = amtvec[nelem];
	    i--;
	}
    }
    return nelem;
}

/*
 * extract all "mask" items from the variable list
 * caller must pass a pointer to an aray of I_MAX+1,
 * or else bad things will happen.
 */
int
vl_getvec(register u_char *src_type, register u_short *src_amt,
	  register int src_nv, register int class, register int *dst_amt)
{
    register int n;
    int count;
    int item;

    for (n = 0; n < I_MAX + 1; n++)
	dst_amt[n] = 0;
    for (count = 0, n = 0; n < src_nv; n++) {
	if ((src_type[n] & VT_TYPE) != class)
	    continue;
	item = src_type[n] & ~VT_TYPE;
	if (item > I_MAX) {
	    logerror("vl_getvec: bad I-type %d (vtype %d)\n",
		     item, src_type[n]);
	    continue;
	}
	dst_amt[item] = src_amt[n];
	count++;
    }
    return count;
}

/*
 * Copy the vec into the variable list.  All items zero in
 * the vec will be deleted from the vlist, and all items
 * present in the vec will be added to the vlist.
 */
int
vl_setvec(register u_char *type, register u_short *amt, u_char *nvp,
	  int max, register int class, register int *vec)
{
    register int nv;
    register int n;
    register int vec_n;

    nv = *nvp;
    vec_n = 1;
    n = 0;
    while (n < nv) {
	if ((type[n] & VT_TYPE) != class) {
	    n++;
	    continue;
	}
	/* find non-zero vec entry to overwrite current slot */
	for (; vec_n <= I_MAX; vec_n++)
	    if (vec[vec_n] != 0)
		break;
	if (vec_n > I_MAX) {
	    /* no more elements left; delete */
	    nv--;
	    if (n < nv) {
		/* copy last entry over current entry */
		type[n] = type[nv];
		amt[n] = amt[nv];
	    }
	} else {
	    /* use new (unused) entry */
	    type[n] = vec_n | class;
	    amt[n] = vec[vec_n];
	    vec_n++;
	    n++;
	}
    }
    *nvp = nv;
    if (n >= nv && vec_n > I_MAX)
	return 1;
    /* free slots at end; copy rest of vec into the vlist */
    for (; vec_n <= I_MAX && nv < max; vec_n++) {
	if (vec[vec_n] == 0)
	    continue;
	type[nv] = vec_n | class;
	amt[nv] = vec[vec_n];
	nv++;
    }
    *nvp = nv;
    if (vec_n <= I_MAX && (class == VT_ITEM || class == VT_COND)) {
	/*
	 * still stuff left; make free slots out of deliveries
	 * and distributes and stuff 'em in (only for item or cond)
	 */
	for (; vec_n <= I_MAX; vec_n++) {
	    if (vec[vec_n] == 0)
		continue;
	    if ((n = freeslot(type, &type[max])) < 0) {
		logerror("vl_setvec: no free slots left\n");
		return 0;
	    }
	    logerror("vl_setvec: replacing type %d amt %d\n",
		     type[n], amt[n]);
	    type[n] = vec_n | class;
	    amt[n] = vec[vec_n];
	}
    }
    if (nv == max) {
	/* checking for overflow */
	while (vec_n <= I_MAX) {
	    if (vec[vec_n])
		break;
	}
	if (vec_n <= I_MAX) {
	    logerror("vl_setvec: no space for i-type #%d (%d)\n",
		     vec_n, vec[vec_n]);
	    return 0;
	}
    }
    return 1;
}

/*
 * make a free slot; deliveries and distributions
 * are fair game for us.
 */
static
    int
freeslot(u_char *vec, register u_char *end)
{
    register u_char *vp;

    for (vp = vec; vp < end; vp++) {
	if (isdel(*vp) || isdist(*vp))
	    return vp - vec;
    }
    return -1;
}
