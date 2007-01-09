/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  wantupd.h: Header used to check to see if an update is wanted
 *             and/or allowed.
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1991
 */

#ifndef WANTUPD_H
#define WANTUPD_H

/* Update policies */
#define  UDP_NORMAL	0
#define  UDP_TIMES	1
#define	 UDP_NOREG	3
#define  UDP_BLITZ	2
#define  UDP_MAX	3
#define  UDP_DEFAULT	UDP_NORMAL

/* Demand update policies */
#define  UDDEM_TMCHECK	0
#define  UDDEM_COMSET	1
#define  UDDEM_DISABLE	2
#define  UDDEM_MAX	2
#define  UDDEM_DEFAULT	UDDEM_DISABLE

#endif
