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
 *  parse.c: Parse an Empire command line
 * 
 *  Known contributors to this file:
 *     
 */

/*
 * parse empire command line, chop into argp
 * If values argpp and spacep passed, parse will use them.
 * otherwise, parse will use static space and global argp.
 * parse assumes that argpp is a char *buf[16], and that spacep
 * points to a buf of at least 256 bytes.
 */

#include <ctype.h>
#include "misc.h"
#include "gen.h"

int
parse(register s_char *buf, s_char **argpp, s_char **condp, s_char *space,
      s_char **redir)
{
    register s_char *bp2;
    register s_char *bp1 = space;
    register s_char **arg = argpp;
    int fs;
    int quoted;
    int argnum;

    if (space == 0)
	return -1;
    if (redir)
	*redir = 0;
    if (condp != 0)
	*condp = 0;
    for (argnum = 0; *buf && argnum < 100;) {
	while (isspace(*buf))
	    buf++;
	if (!*buf)
	    break;
	if (redir && (*buf == '>' || *buf == '|')) {
	    *redir = buf;
	    break;
	}
	quoted = 0;
	for (bp2 = bp1; *buf;) {
	    if (!quoted && isspace(*buf)) {
		buf++;
		break;
	    }
	    if (*buf == '"') {
		quoted = !quoted;
		buf++;
	    } else {
		*bp1++ = *buf++;
	    }
	}
	*bp1++ = 0;
	if (*bp2 == '?' && condp != 0) {
	    *condp = bp2 + 1;
	} else {
	    arg[argnum] = bp2;
	    argnum++;
	}
    }
    arg[argnum] = 0;
    for (fs = argnum + 1; fs < 16; fs++)
	arg[fs] = 0;
    return argnum;
}
