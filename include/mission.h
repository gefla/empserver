/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mission.h: Definitions for things having to do with missions.
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#ifndef _MISSION_H_
#define _MISSION_H_

#include "queue.h"

#define	MI_NONE		0
#define	MI_INTERDICT	1
#define	MI_SUPPORT	2
#define	MI_RESERVE	3
#define MI_ESCORT	4
#define MI_SINTERDICT	5	/* sub interdiction */
#define MI_AIR_DEFENSE	6
#define MI_DSUPPORT	7
#define MI_OSUPPORT	8

struct genlist {
    struct emp_qelem queue;	/* list of units */
    int type;			/* type of unit */
    int x, y;			/* x,y it came from */
    s_char *cp;			/* pointer to desc of thing */
    s_char *thing;		/* thing's struct */
};

#define SECT_HARDTARGET (-92917)	/* a very low number that no-one might pick */

#endif /* _MISSION_H_ */
