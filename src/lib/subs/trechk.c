/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  trechk.c: Check to see if an actor has violated a treaty.
 * 
 *  Known contributors to this file: 
 *      Steve McClure, 1998-1999
 */

/*
 * Check to see if an actor has a treaty forbidding a given action,
 * and if so, see if the actor wishes to break the treaty.
 * All applicable treaties are checked.  All treaties must be broken
 * if the action is to be allowed.  Propsed treaties are warned about,
 * but no mention of broken pending treaties are made in the news.
 */

#include <config.h>

#include "file.h"
#include "nat.h"
#include "news.h"
#include "optlist.h"
#include "prototypes.h"
#include "treaty.h"

int
trechk(natid actor, natid victim, int provision)
{
    natid cn;
    char buf[1024];
    int news_verb;
    int involved[MAXNOC];
    struct trtstr treaty;
    time_t now;
    int conditions;
    natid other;
    int broken;
    int applied;
    struct nstr_item nstr;

    if (!opt_TREATIES)
	return 1;
    (void)time(&now);
    broken = 0;
    applied = 0;
    for (cn = 0; cn < MAXNOC; cn++)
	involved[cn] = 0;
    snxtitem_all(&nstr, EF_TREATY);
    while (nxtitem(&nstr, &treaty)) {
	if (treaty.trt_status == TS_FREE)
	    continue;
	if (treaty.trt_exp < now)
	    continue;
	if (actor == treaty.trt_cna) {
	    conditions = treaty.trt_acond;
	    other = treaty.trt_cnb;
	} else if (actor == treaty.trt_cnb) {
	    conditions = treaty.trt_bcond;
	    other = treaty.trt_cna;
	} else
	    continue;
	if ((conditions & provision) == 0)
	    continue;
	if (victim != other) {
	    switch (provision) {
		/* These are violations no matter who the victim is */
	    case NEWSHP:
	    case NEWLND:
	    case NEWNUK:
	    case NEWPLN:
	    case TRTENL:
		break;
	    default:
		/* The rest are only violations against the victim */
		continue;
	    }
	}
	/* treaty applies to actor */
	applied++;
	pr("This action is in contravention of ");
	if (treaty.trt_status == TS_PROPOSED)
	    pr("pending ");
	pr(" treaty #%d (with %s)\n", nstr.cur, cname(other));
	getstring("Do you wish to go ahead anyway? [yn] ", buf);
	if (*buf == 'n' || *buf == 'N')
	    broken = 0;
	else
	    broken = 1;
	if (treaty.trt_status == TS_SIGNED)
	    involved[other]++;
    }
    if (applied > 0) {
	news_verb = N_HONOR_TRE;
	if (broken > 0)
	    news_verb = N_VIOL_TRE;
	for (cn = 0; cn < MAXNOC; cn++)
	    if (involved[cn] > 0)
		nreport(actor, news_verb, cn, 1);
    }
    if (applied && !broken) {
	/*
	 * if any treaty applied, and none were broken
	 * the intended action is NOT performed.
	 */
	return 0;
    }
    return 1;
}
