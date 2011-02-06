/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  rej.c: Various relations setting routines
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 */

#include <config.h>

#include "file.h"
#include "nat.h"
#include "news.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "server.h"

void
setrel(natid us, natid them, int rel)
{
    struct natstr *mynp = getnatp(us);
    struct natstr *themnp = getnatp(them);
    int oldrel;
    char *whichway;
    int n_up = 0;
    int n_down = 0;
    char *addendum = NULL;

    if (CANT_HAPPEN(rel < AT_WAR))
	rel = AT_WAR;
    if (CANT_HAPPEN(rel > ALLIED))
	rel = ALLIED;
    if (CANT_HAPPEN(!mynp || !themnp))
	return;
    if (us == them)
	return;
    oldrel = relations_with(us, them);
    if (oldrel == rel)
	return;
    if (rel > oldrel)
	whichway = "upgraded";
    else
	whichway = "downgraded";
    if (rel == ALLIED) {
	addendum = "Congratulations!";
	n_up = N_DECL_ALLY;
    } else if (rel == FRIENDLY) {
	n_up = N_UP_FRIENDLY;
	n_down = N_DOWN_FRIENDLY;
    } else if (rel == NEUTRAL) {
	n_up = N_UP_NEUTRAL;
	n_down = N_DOWN_NEUTRAL;
    } else if (rel == HOSTILE) {
	addendum = "Another cold war...";
	n_up = N_UP_HOSTILE;
	n_down = N_DOWN_HOSTILE;
    } else if (rel < HOSTILE) {
	addendum = "Declaration made (give 'em hell).";
	n_down = N_DECL_WAR;
    }

    if (addendum && us == player->cnum && !update_running)
	pr("%s\n", addendum);
    mpr(us, "Diplomatic relations with %s %s to \"%s\".\n",
	cname(them), whichway, relates[rel]);
    if (!(getrejects(us, themnp) & REJ_TELE))
	mpr(them,
	    "Country %s (#%d) has %s their relations with you to \"%s\"!\n",
	    cname(us), us, whichway, relates[rel]);

    putrel(mynp, them, rel);
    putnat(mynp);

    if (!player->god) {
	if (oldrel == ALLIED)
	    nreport(us, N_DIS_ALLY, them, 1);
	else if (oldrel < HOSTILE && rel >= HOSTILE)
	    nreport(us, N_DIS_WAR, them, 1);
	if (rel > oldrel)
	    nreport(us, n_up, them, 1);
	else
	    nreport(us, n_down, them, 1);
    }
    if (opt_HIDDEN)
	setcont(them, us, FOUND_TELE);
}

void
setcont(natid us, natid them, int contact)
{
    struct natstr *np = getnatp(us);

    if (CANT_HAPPEN(!np))
	return;
    putcontact(np, them, contact);
    putnat(np);
}

void
setrej(natid us, natid them, int how, int what)
{
    struct natstr *np = getnatp(us);

    if (CANT_HAPPEN(!np))
	return;
    putreject(np, them, how, what);
    putnat(np);
}
