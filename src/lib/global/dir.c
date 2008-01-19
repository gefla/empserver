/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  dir.c: Directional orientation descriptions.  Must agree with defines
 *         in path.h
 * 
 *  Known contributors to this file:
 */

#include <config.h>

#include "path.h"

/* must agree with DIR_ defines */
char dirch[] = {
    'h',			/* stop */
    'u',			/* up-right */
    'j',			/* right */
    'n',			/* down-right */
    'b',			/* down-left */
    'g',			/* left */
    'y',			/* up-left */
    'v',			/* view */
    'm',			/* map */
    '\0',
};

/* must agree with dirch[] and DIR_ defines */
int diroff[][2] = {
    {0, 0}, {1, -1}, {2, 0}, {1, 1}, {-1, 1}, {-2, 0}, {-1, -1}, {0, 0},
    {0, 0}
};

/* this maps a character from a to z into the diroff mappings.  It
   keeps us from having to loop if we don't want to */
signed char dirindex[] = {
    -1, 4, -1, -1, -1, -1, 5, 0, -1, 2, -1, -1, 8,
    3, -1, -1, -1, -1, -1, -1, 1, 7, -1, -1, 6, -1
};

/* must agree with dirch[] and DIR_ defines */
char *routech[][2] = {
    {"   ", "$ $"},
    {"  /", "$ /"},
    {"  >", "$ >"},
    {"  \\", "$ \\"},
    {"/  ", "/ $"},
    {"<  ", "< $"},
    {"\\  ", "\\ $"}
};
