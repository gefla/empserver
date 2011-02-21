/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  proto.h: Protocol for client
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#ifndef PROTO_H
#define PROTO_H

/*
 * Protocol version.
 * Bump this if the protocol changes in a way that is not backwards
 * compatible.  This breaks all existing clients.  If you are tempted
 * to do that, please consider using session options to make your
 * change backwards compatible.
 */
#define CLIENTPROTO	2

#define C_CMDOK		0x0
#define C_DATA		0x1
#define C_INIT		0x2
#define C_EXIT		0x3
#define C_FLUSH		0x4
#define C_NOECHO	0x5
#define C_PROMPT	0x6
#define C_ABORT		0x7
#define C_REDIR		0x8
#define C_PIPE		0x9
#define C_CMDERR	0xA
#define C_BADCMD	0xB
#define C_EXECUTE	0xC
#define C_FLASH		0xD
#define C_INFORM	0xE
#define C_LAST		0xE

#endif
