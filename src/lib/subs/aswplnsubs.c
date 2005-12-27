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
 *  aswplnsubs.c: Various subroutines used for ASW planes
 * 
 *  Known contributors to this file:
 *  Ron Koenderink, 2004
 *     
 */

#include <config.h>

#include <stdlib.h>
#include "misc.h"
#include "file.h"
#include "ship.h"
#include "plane.h"
#include "prototypes.h"

int
on_shiplist(short uid, struct shiplist *head)
{
    struct shiplist *s;

    s = head;
    while (s != NULL) {
	if (s->uid == uid)
	    return 1;
	s = s->next;
    }
    return 0;
}

void
add_shiplist(short uid, struct shiplist **head)
{
    struct shiplist *s, *s2;

    s = *head;
    s2 = NULL;

    while (s != NULL) {
	if (s->uid == uid) {
	    return;
	}
	s2 = s;
	s = s->next;
    }

    s = malloc(sizeof(struct shiplist));
    if (s2 != NULL) 
	s2->next = s;
    else
	*head = s;
    s->uid = uid;
    s->next = NULL;
}

void
free_shiplist(struct shiplist **head)
{
    struct shiplist *s, *s2;

    s = *head;

    while (s != NULL) {
	s2 = s;
	s = s->next;
	free(s2);
    }
    *head = NULL;
}

void
print_shiplist(struct shiplist *head)
{
    struct shiplist *s;
    int first;
    struct mchrstr *mp;
    struct shpstr ship;

    s = head;
    first = 1;

    while (s != NULL) {
	getship(s->uid, &ship);
	mp = &mchr[(int)ship.shp_type];
	if (first) {
	    pr(" #          player->owner           eff        type\n");
	    first = 0;
	}
	pr("(#%3d) %10.10s  %12.12s  %s\n", ship.shp_uid,
	   cname(ship.shp_own), effadv(ship.shp_effic), prship(&ship));
	s = s->next;
    }
}
