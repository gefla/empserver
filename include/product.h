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
 *  product.h: Definitions for things having to do with products
 * 
 *  Known contributors to this file:
 * 
 */

#ifndef _PRODUCT_H_
#define _PRODUCT_H_

struct	pchrstr {
	u_char	p_nv;		/* number of constituents */
	u_char	p_vtype[MAXCHRNV];/* constituent types */
	u_short	p_vamt[MAXCHRNV]; /* constituent amounts */
	int	p_type;		/* vtype if product is a variable */
	int	p_level;	/* index (NAT_?LEV) if product is not a var */
	int	p_cost;		/* dollars / product unit */
	int	p_nrndx;	/* index into sect of natural resource */
	int	p_nrdep;	/* depletion as a % of resource used */
	int	p_nlndx;	/* index (NAT_?LEV) affecting production */
	int	p_nlmin;	/* minimum lvl required */
	int	p_nllag;	/* lag, mul by (lvl-nlmin)/(lvl-nlmin+nllag) */
	int	p_effic;	/* process efficiency, mult by p_effic/100 */
	s_char	*p_name;	/* name of product */
	s_char	*p_sname;	/* short (7char or less) name of product */
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

extern	struct pchrstr pchr[];

extern	int prd_maxno;

#endif /* _PRODUCT_H_ */
