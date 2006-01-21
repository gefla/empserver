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
 *  parse.c: Parse an Empire command line
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "gen.h"

/*
 * Parse user command in BUF.
 * BUF is UTF-8.
 * Set ARG[0] to point to the command name.
 * Set ARG[1..N] to point to arguments, where N is the number of
 * arguments.  Set ARG[N+1..127] to NULL.
 * If *CONDP is not null, recognize conditional argument syntax, and
 * set *CONDP to the conditional argument if present, else NULL.
 * Command name and arguments are copied into SPACE[], whose size must
 * be at least strlen(BUF) + 1.
 * If *REDIR is not null, recognize redirection syntax, and set *REDIR
 * to UTF-8 redirection string if present, else NULL.
 * Return number of slots used in ARG[], or -1 on error.
 */
int
parse(char *buf, char **arg, char **condp, char *space, char **redir)
{
    char *bp2;
    char *bp1 = space;
    int fs;
    int quoted;
    int argnum;

    if (redir)
	*redir = 0;
    if (condp != NULL)
	*condp = NULL;
    for (argnum = 0; *buf && argnum < 127;) {
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
		if (*buf >= 0x20 && *buf <= 0x7e)
		    *bp1++ = *buf++;
		else
		    buf++;
	    }
	}
	*bp1++ = 0;
	if (*bp2 == '?' && condp != NULL) {
	    *condp = bp2 + 1;
	} else {
	    arg[argnum] = bp2;
	    argnum++;
	}
    }
    for (fs = argnum; fs < 128; fs++)
	arg[fs] = NULL;
    return argnum;
}
