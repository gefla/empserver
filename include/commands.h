/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  commands.h: Definitions used for command stuff
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "misc.h"
#include "plane.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "nsc.h"
#include "nat.h"
#include "queue.h"
#include "subs.h"
#include "gen.h"
#include "common.h"
#include "lost.h"
#include "map.h"
#include "update.h"

#include "prototypes.h"		/* must come at end, after defines and typedefs */

#endif /* _COMMANDS_H_ */
