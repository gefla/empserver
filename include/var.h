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

#define	PLG_HEALTHY	0
#define	PLG_DYING	1
#define	PLG_INFECT	2
#define	PLG_INCUBATE	3
#define	PLG_EXPOSED	4

#define	V_ITEM(x)	(x)

#define I_NONE		0
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

#endif /* _VAR_H_ */
