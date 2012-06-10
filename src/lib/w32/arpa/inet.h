/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  arpa/inet.h: POSIX networking for WIN32
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2007
 */

#ifndef ARPA_INET_H
#define ARPA_INET_H

#include "sys/socket.h"

extern const char *inet_ntop(int af, const void *src, char *dst,
			     socklen_t cnt);

#endif /* ARPA_INET_H */
