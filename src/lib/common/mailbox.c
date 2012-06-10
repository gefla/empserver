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
 *  mailbox.c: Mailbox file access
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2009-2011
 */

#include <config.h>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "optlist.h"
#include "tel.h"
#include "prototypes.h"

char *
mailbox(char *buf, natid cn)
{
    sprintf(buf, "%s/tel%d", teldir, cn);
    return buf;
}

/*
 * Create an empty telegram file named MBOX.
 * Return 0 on success, -1 on failure.
 */
int
mailbox_create(char *mbox)
{
    int fd;

    fd = creat(mbox, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd < 0 || close(fd) < 0) {
	logerror("Can't create mailbox %s (%s)", mbox, strerror(errno));
	return -1;
    }
    return 0;
}

/*
 * Read telegram header from FP into TEL.
 * MBOX is the file name, it is used for logging errors.
 * Return 1 on success, 0 on EOF, -1 on error.
 */
int
tel_read_header(FILE *fp, char *mbox, struct telstr *tel)
{
    size_t n;

    n = fread(tel, 1, sizeof(*tel), fp);
    if (n == 0 && feof(fp))
	return 0;
    if (n != sizeof(*tel)
	|| tel->tel_type > TEL_LAST || tel->tel_from > MAXNOC) {
	logerror("Mailbox %s corrupt: bad header", mbox);
	return -1;
    }
    return 1;
}

/*
 * Read telegram body from FP.
 * MBOX is the file name, it is used for logging errors.
 * TEL is the header.
 * Unless SINK is null, it is called like SINK(CHUNK, SZ, ARG) to
 * consume the body, chunk by chunk.  The chunks are UTF-8, and
 * CHUNK[SZ} is 0.  Reading fails when SINK() returns a negative
 * value.
 * Return 0 on success, -1 on failure.
 */
int
tel_read_body(FILE *fp, char *mbox, struct telstr *tel,
	      int (*sink)(char *, size_t, void *),
	      void *arg)
{
    char buf[4096];
    size_t left, sz;

    left = tel->tel_length;
    while (left) {
	sz = MIN(left, sizeof(buf) - 1);
	if (fread(buf, 1, sz, fp) != sz) {
	    logerror("Mailbox %s corrupt: can't read body", mbox);
	    return -1;
	}
	buf[sz] = 0;
	if (sink && sink(buf, sz, arg) < 0)
	    return -1;
	left -= sz;
    }
    return 0;
}
