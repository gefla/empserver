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
 *  sect.c: Sector designation characteristics
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Jeff Bailey
 *     Thomas Ruschak, 1992
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <config.h>

#include "misc.h"
#include "sect.h"

/*
 * Table of sector designations
 * Initialized on startup from sect.config and deity custom config (if any).
 * Terminated by a sentinel with null d_name.
 */
struct dchrstr dchr[SCT_TYPE_MAX + 2];

/*
 * Table of infrastructure types
 * Initialized on startup from infra.config and deity custom config (if any).
 */
struct sctintrins intrchr[INT_DEF + 2];
