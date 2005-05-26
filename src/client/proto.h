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
 *  proto.h: Protocol for client
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */
/*
 * The master copy of this is in the standard include directory.
 * A copy is placed in the client directory to allow client
 * to be compiled independently of the rest of the game.
 */

#define CLIENTPROTO	2	/* if new things are added, bump this */

#define	C_CMDOK		0x0
#define	C_DATA		0x1
#define	C_INIT		0x2
#define	C_EXIT		0x3
#define C_FLUSH		0x4
#define	C_NOECHO	0x5
#define C_PROMPT	0x6
#define	C_ABORT		0x7
#define C_REDIR		0x8
#define C_PIPE		0x9
#define	C_CMDERR	0xA
#define	C_BADCMD	0xB
#define C_EXECUTE	0xC
#define C_FLASH		0xD
#define C_INFORM        0xE
#define C_LAST          0xE
