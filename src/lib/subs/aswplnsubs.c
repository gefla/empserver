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
 *  aswplnsubs.c: Various subroutines used for ASW planes
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "ship.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "file.h"
#include "queue.h"
#include "plane.h"
#include <fcntl.h>
#include <ctype.h>
#include "prototypes.h"

int
have_looked(u_char uid, struct shiplook *head)
{
    struct shiplook *s;

    s = head;
    if (s->uid == -1)
	return 0;

    while (s != ((struct shiplook *)0)) {
	if (s->uid == uid)
	    return s->looked;
	s = s->next;
    }

    return 0;
}

int
have_found(u_char uid, struct shiplook *head)
{
    struct shiplook *s;

    s = head;
    if (s->uid == -1)
	return 0;

    while (s != ((struct shiplook *)0)) {
	if (s->uid == uid)
	    return s->found;
	s = s->next;
    }

    return 0;
}

void
set_have_looked(u_char uid, struct shiplook *head)
{
    struct shiplook *s, *s2;

    s = head;
    if (s->uid == -1) {
	s->uid = uid;
	s->looked = 1;
	s->found = 0;
	s->next = (struct shiplook *)0;
    }

    while (s != ((struct shiplook *)0)) {
	if (s->uid == uid) {
	    s->looked = 1;
	    return;
	}
	s2 = s;
	s = s->next;
    }

    s = (struct shiplook *)malloc(sizeof(struct shiplook));
    memset(s, 0, sizeof(struct shiplook));
    s2->next = s;
    s->uid = uid;
    s->looked = 1;
    s->next = (struct shiplook *)0;
}

void
set_have_found(u_char uid, struct shiplook *head)
{
    struct shiplook *s, *s2;

    s = head;
    if (s->uid == -1) {
	s->uid = uid;
	s->looked = 0;
	s->found = 1;
	s->next = (struct shiplook *)0;
    }


    while (s != ((struct shiplook *)0)) {
	if (s->uid == uid) {
	    s->found = 1;
	    return;
	}
	s2 = s;
	s = s->next;
    }

    s = (struct shiplook *)malloc(sizeof(struct shiplook));
    memset(s, 0, sizeof(struct shiplook));
    s2->next = s;
    s->uid = uid;
    s->found = 1;
    s->next = (struct shiplook *)0;
}

int
print_found(struct shiplook *head)
{
    struct shiplook *s;
    extern s_char *effadv(int);
    int first;
    struct mchrstr *mp;
    struct shpstr ship;

    s = head;
    first = 1;
    if (s->uid == -1)
	return 0;

    while (s != ((struct shiplook *)0)) {
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

    return 1;
}
