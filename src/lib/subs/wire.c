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
 *  wire.c: Write an announcement to the world
 * 
 *  Known contributors to this file:
 *    
 */

#include "misc.h"
#include <fcntl.h>
#if !defined(_WIN32)
#include <sys/uio.h>
#endif
#include "nat.h"
#include "tel.h"
#include "file.h"
#include "prototypes.h"

#if 0
int
typed_wire(natid from, natid to, s_char *message, int type)
{
    register s_char *bp;
    int len;
    struct telstr tel;
    struct natstr *np;
    struct iovec iov[2];
    int fd;
    s_char buf[1024];

    if ((np = getnatp(to)) == 0 ||
	((np->nat_stat & STAT_NORM) == 0 &&
	 (np->nat_stat & STAT_SANCT) == 0))
	return 0;
    if ((fd = open(wirebox(buf, to), O_WRONLY | O_APPEND, 0)) < 0) {
	logerror("telegram 'open' of %s (#%d) failed",
		 wirebox(buf, to), to);
	return 0;
    }
    tel.tel_from = from;
    (void)time(&tel.tel_date);
    bp = message;
    while (*bp++) ;
    len = bp - message;
    if (len >= MAXTELSIZE)
	len = MAXTELSIZE;
    message[len] = 0;
    tel.tel_length = len;
    tel.tel_type = type;
    iov[0].iov_base = (caddr_t)&tel;
    iov[0].iov_len = sizeof(tel);
    iov[1].iov_base = message;
    iov[1].iov_len = len;
    if (writev(fd, iov, 2) < iov[0].iov_len + iov[1].iov_len) {
	logerror("telegram 'write' to #%d failed", to);
    } else {
	np->nat_ann++;
	/* don't do putnat because of expense */
    }
    (void)close(fd);
    return 0;
}
#endif
