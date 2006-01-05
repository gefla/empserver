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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  product.h: Definitions for things having to do with products
 * 
 *  Known contributors to this file:
 * 
 */

#ifndef PRODUCT_H
#define PRODUCT_H

#include "item.h"

/*
 * Maximum number of product constituents.
 * Beware, some output formats rely on MAXPRCON <= 3!
 */
enum { MAXPRCON = 3 };

struct pchrstr {
    int p_uid;
    i_type p_ctype[MAXPRCON];	/* constituent types */
    u_short p_camt[MAXPRCON];	/* constituent amounts */
    i_type p_type;		/* product item type, or I_NONE if level */
    int p_level;		/* level index (NAT_?LEV), or -1 if item */
    int p_cost;			/* dollars / product unit */
    int p_nrndx;		/* index into sect of natural resource */
    int p_nrdep;		/* depletion as a % of resource used */
    int p_nlndx;		/* index (NAT_?LEV) affecting production */
    int p_nlmin;		/* minimum lvl required */
    int p_nllag;		/* lag, mul by (lvl-nlmin)/(lvl-nlmin+nllag) */
    int p_effic;		/* process efficiency, mult by p_effic/100 */
    char *p_name;		/* name of product */
    char *p_sname;		/* short (7 char or less) name of product */
};

#define	P_SHELL	1
#define	P_GUN	2
#define	P_PETROL	3
#define	P_IRON	4
#define	P_DUST	5
#define	P_BAR	6
#define	P_FOOD	7
#define	P_OIL	8
#define	P_LCM	9
#define	P_HCM	10
#define	P_TLEV	11
#define	P_RLEV	12
#define	P_ELEV	13
#define	P_HLEV	14
#define P_URAN	15
#define P_MDUST	16

extern struct pchrstr pchr[P_MDUST + 2];

#endif
