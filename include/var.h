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
 *  var.h: various variable definitions
 * 
 *  Known contributors to this file:
 *     
 */

#ifndef _VAR_H_
#define _VAR_H_

#define	V_MAX		255

#define	PLG_HEALTHY	0
#define	PLG_DYING	1
#define	PLG_INFECT	2
#define	PLG_INCUBATE	3
#define	PLG_EXPOSED	4

#define	VT_ITEM		(1<<5)
#define VT_COND		(2<<5)
#define VT_DIST		(3<<5)
#define VT_DEL		(4<<5)
#define VT_SELL		(5<<5)
#define VT_SPARE1	(6<<5)
#define VT_SPARE2	(7<<5)
#define VT_TYPE		(7<<5)

#define isitem(x)	(((x) & VT_TYPE) == VT_ITEM)
#define iscond(x)	(((x) & VT_TYPE) == VT_COND)
#define isdist(x)	(((x) & VT_TYPE) == VT_DIST)
#define isdel(x)	(((x) & VT_TYPE) == VT_DEL)
#define issell(x)	(((x) & VT_TYPE) == VT_SELL)
#define	unitem(x)	((x) & (VT_ITEM -1))

#define	V_ITEM(x)	((x)|VT_ITEM)	/* a moveable, sellable(?) commodity */
#define V_COND(x)	((x)|VT_COND)	/* condition (plg time, etc) */
#define V_DIST(x)	((x)|VT_DIST)	/* distribution command */
#define V_DEL(x)	((x)|VT_DEL)	/* delivery command */
#define V_SELL(x)	((x)|VT_SELL)	/* make a sale */

#define I_CIVIL		1
#define I_MILIT		2
#define I_SHELL		3
#define	I_GUN		4
#define	I_PETROL	5
#define I_IRON		6
#define I_DUST		7
#define I_BAR		8
#define I_FOOD		9
#define I_OIL		10
#define	I_LCM		11
#define	I_HCM		12
#define I_UW		13
#define I_RAD		14
#define I_MAX		14

#define	C_MINE		1
#define C_PSTAGE	2
#define C_PTIME		3
#define C_REM		4
#define C_CHE		5
#define C_FALLOUT	6

#define V_MINE		V_COND(C_MINE)
#define V_PSTAGE	V_COND(C_PSTAGE)
#define V_PTIME		V_COND(C_PTIME)
#define V_REM		V_COND(C_REM)
#define V_CHE		V_COND(C_CHE)
#define V_FALLOUT	V_COND(C_FALLOUT)

/* should this be here?? */
#define CHE_MAX                 255
#define	get_che_cnum(x)		((x) >> 8)
#define set_che_cnum(x, cn)	((x) = ((x) & 0xff) | ((cn) << 8))
#define get_che_value(x)	((x) & 0xff)
#define set_che_value(x, n)	((x) = ((x) & 0xff00) | (n))

#define	V_CIVIL		V_ITEM(I_CIVIL)
#define	V_MILIT		V_ITEM(I_MILIT)
#define	V_SHELL		V_ITEM(I_SHELL)
#define	V_GUN		V_ITEM(I_GUN)
#define	V_PETROL	V_ITEM(I_PETROL)
#define	V_IRON		V_ITEM(I_IRON)
#define	V_DUST		V_ITEM(I_DUST)
#define	V_BAR		V_ITEM(I_BAR)
#define	V_FOOD		V_ITEM(I_FOOD)
#define	V_OIL		V_ITEM(I_OIL)
#define	V_LCM		V_ITEM(I_LCM)
#define	V_HCM		V_ITEM(I_HCM)
#define V_UW		V_ITEM(I_UW)
#define V_RAD		V_ITEM(I_RAD)

#define	V_CDEL		V_DEL(I_CIVIL)
#define	V_MDEL		V_DEL(I_MILIT)
#define	V_SDEL		V_DEL(I_SHELL)
#define	V_GDEL		V_DEL(I_GUN)
#define	V_PDEL		V_DEL(I_PETROL)
#define	V_IDEL		V_DEL(I_IRON)
#define	V_DDEL		V_DEL(I_DUST)
#define	V_BDEL		V_DEL(I_BAR)
#define	V_FDEL		V_DEL(I_FOOD)
#define	V_ODEL		V_DEL(I_OIL)
#define	V_LDEL		V_DEL(I_LCM)
#define	V_HDEL		V_DEL(I_HCM)
#define	V_UDEL		V_DEL(I_UW)
#define	V_RDEL		V_DEL(I_RAD)

#define	V_CDIST		V_DIST(I_CIVIL)
#define	V_MDIST		V_DIST(I_MILIT)
#define	V_SDIST		V_DIST(I_SHELL)
#define	V_GDIST		V_DIST(I_GUN)
#define	V_PDIST		V_DIST(I_PETROL)
#define	V_IDIST		V_DIST(I_IRON)
#define	V_DDIST		V_DIST(I_DUST)
#define	V_BDIST		V_DIST(I_BAR)
#define	V_FDIST		V_DIST(I_FOOD)
#define	V_ODIST		V_DIST(I_OIL)
#define	V_LDIST		V_DIST(I_LCM)
#define	V_HDIST		V_DIST(I_HCM)
#define	V_UDIST		V_DIST(I_UW)
#define	V_RDIST		V_DIST(I_RAD)

#endif /* _VAR_H_ */
