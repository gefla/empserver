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
 *  tags.c: save redir, execute and pipe info to protect against tampering
 *          by the deity.
 * 
 *  Known contributors to this file:
 *     John Yockey, 1998
 */

#include "misc.h"
#include "tags.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

struct tagstruct *taglist;

void
io_init(void)
{
    taglist = NULL;
}

char *
gettag(char *p)
{
    struct tagstruct *tag1, *tag2;

    if (taglist == NULL)
	return NULL;
    tag1 = taglist;
    if (!strncmp(tag1->item, p, strlen(tag1->item))) {
	p = tag1->item;
	taglist = taglist->next;
	free(tag1);
	return p;
    }
    while (tag1->next != NULL) {
	tag2 = tag1->next;
	if (!strncmp(tag2->item, p, strlen(tag2->item))) {
	    p = tag2->item;
	    tag1->next = tag2->next;
	    free(tag2);
	    return p;
	}
	tag1 = tag1->next;
    }
    return NULL;
}
