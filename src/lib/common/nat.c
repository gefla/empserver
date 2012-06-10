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
 *  nat.c: Misc. accesses on the nation file
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2006-2011
 */

#include <config.h>

#include <string.h>
#include "file.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "sect.h"
#include "tel.h"

char *relates[] = {
    /* must follow nation relation defines in nat.h */
    "At War", "Hostile", "Neutral", "Friendly", "Allied"
};

char *
cname(natid n)
{
    struct natstr *np;

    if (!(np = getnatp(n)))
	return NULL;
    return np->nat_cnam;
}

char *
relatename(struct natstr *np, natid other)
{
    return relates[getrel(np, other)];
}

char *
rejectname(struct natstr *np, natid other)
{
    static char *rejects[] = {
	/* must follow reject flags defined in nat.h */
	"  YES  YES  YES  YES",
	"  NO   YES  YES  YES",
	"  YES  NO   YES  YES",
	"  NO   NO   YES  YES",
	"  YES  YES  NO   YES",
	"  NO   YES  NO   YES",
	"  YES  NO   NO   YES",
	"  NO   NO   NO   YES",
	"  YES  YES  YES  NO ",
	"  NO   YES  YES  NO ",
	"  YES  NO   YES  NO ",
	"  NO   NO   YES  NO ",
	"  YES  YES  NO   NO ",
	"  NO   YES  NO   NO ",
	"  YES  NO   NO   NO ",
	"  NO   NO   NO   NO "
    };

    return rejects[getrejects(other, np)];
}

char *
natstate(struct natstr *np)
{
    static char *stnam[] = {
	/* must match nat_status */
	"FREE", "VISITOR", "VISITOR", "SANCTUARY", "ACTIVE", "DEITY"
    };
    return stnam[np->nat_stat];
}

/* This returns the relations that np has with them */
int
getrel(struct natstr *np, natid them)
{
    return np->nat_relate[them];
}

/*
 * Return relations US has with THEM.
 * Countries are considered allied to themselves.
 */
int
relations_with(natid us, natid them)
{
    return us == them ? ALLIED : getrel(getnatp(us), them);
}

int
getrejects(natid them, struct natstr *np)
{
    return np->nat_rejects[them];
}

void
agecontact(struct natstr *np)
{
    int them;

    for (them = 1; them < MAXNOC; ++them) {
	if (them != np->nat_cnum && np->nat_contact[them]) {
	    --np->nat_contact[them];
	}
    }
}

int
getcontact(struct natstr *np, natid them)
{
    return np->nat_contact[them];
}

void
putrel(struct natstr *np, natid them, int relate)
{
    np->nat_relate[them] = relate;
}

void
putreject(struct natstr *np, natid them, int how, int what)
{
    if (how)
	np->nat_rejects[them] |= what;
    else
	np->nat_rejects[them] &= ~what;
}

void
putcontact(struct natstr *np, natid them, int contact)
{
    if (CANT_HAPPEN(contact < 0))
	contact = 0;
    if (CANT_HAPPEN(contact > 255))
	contact = 255;

    if (np->nat_contact[them] < contact)
	np->nat_contact[them] = contact;
}

int
influx(struct natstr *np)
{
    struct sctstr sect;

    getsect(np->nat_xcap, np->nat_ycap, &sect);
    if (sect.sct_own != np->nat_cnum ||
	(sect.sct_type != SCT_CAPIT && sect.sct_type != SCT_MOUNT &&
	 sect.sct_type != SCT_SANCT) ||
	(np->nat_flags & NF_SACKED))
	return 1;
    else
	return 0;
}

/*
 * Initialize NATP for country #CNUM in status STAT.
 * STAT must be STAT_UNUSED, STAT_NEW, STAT_VIS or STAT_GOD.
 * Also wipe realms and telegrams.
 */
struct natstr *
nat_reset(struct natstr *natp, natid cnum, char *name, char *rep,
	  enum nat_status stat)
{
    struct realmstr newrealm;
    char buf[1024];
    int i;

    ef_blank(EF_NATION, cnum, natp);
    natp->nat_stat = stat;
    strncpy(natp->nat_cnam, name, sizeof(natp->nat_cnam) - 1);
    strncpy(natp->nat_pnam, rep, sizeof(natp->nat_pnam) - 1);
    if (stat == STAT_GOD)
	natp->nat_money = 123456789;
    for (i = 0; i < MAXNOR; i++) {
	ef_blank(EF_REALM, i + cnum * MAXNOR, &newrealm);
	putrealm(&newrealm);
    }
    mailbox_create(mailbox(buf, cnum));
    /* FIXME natp->nat_ann = #annos */
    natp->nat_level[NAT_HLEV] = start_happiness;
    natp->nat_level[NAT_RLEV] = start_research;
    natp->nat_level[NAT_TLEV] = start_technology;
    natp->nat_level[NAT_ELEV] = start_education;
    for (i = 0; i < MAXNOC; i++)
	natp->nat_relate[i] = NEUTRAL;
    natp->nat_flags =
	NF_FLASH | NF_BEEP | NF_COASTWATCH | NF_SONAR | NF_TECHLISTS;
    return natp;
}
