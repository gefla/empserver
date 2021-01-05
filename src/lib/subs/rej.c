/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  rej.c: Various relations setting routines
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Markus Armbruster, 2005-2016
 */

#include <config.h>

#include "nat.h"
#include "news.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "update.h"

void
setrel(natid us, natid them, enum relations rel)
{
    struct relatstr *relp = getrelatp(us);
    struct natstr *mynp = getnatp(us);
    enum relations oldrel;
    char *whichway;
    int n_up = 0;
    int n_down = 0;
    char *addendum = NULL;

    if (CANT_HAPPEN(rel < AT_WAR))
	rel = AT_WAR;
    if (CANT_HAPPEN(rel > ALLIED))
	rel = ALLIED;
    if (CANT_HAPPEN(!relp || !mynp))
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
	cname(them), whichway, relations_string(rel));
    if (nat_accepts(them, us, REJ_TELE))
	mpr(them,
	    "Country %s has %s their relations with you to \"%s\"!\n",
	    prnat(mynp), whichway, relations_string(rel));

    relp->rel_relate[them] = rel;
    putrelat(relp);

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
    struct contactstr *conp = getcontactp(us);

    if (CANT_HAPPEN(!conp))
	return;

    if (CANT_HAPPEN(contact < 0))
	contact = 0;
    if (CANT_HAPPEN(contact > 255))
	contact = 255;

    if (!opt_LOSE_CONTACT)
	contact = !!contact;
    if (conp->con_contact[them] < contact) {
	conp->con_contact[them] = contact;
	putcontact(conp);
    }
}

void
setrej(natid us, natid them, int reject, enum rej_comm what)
{
    struct rejectstr *rejp = getrejectp(us);

    if (CANT_HAPPEN(!rejp))
	return;
    if (reject)
	rejp->rej_rejects[them] |= bit(what);
    else
	rejp->rej_rejects[them] &= ~bit(what);
    putreject(rejp);
}
