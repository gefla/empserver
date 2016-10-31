/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2006-2016
 */

#include <config.h>

#include <string.h>
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "sect.h"
#include "tel.h"

char *
cname(natid n)
{
    struct natstr *np;

    if (!(np = getnatp(n)))
	return NULL;
    return np->nat_cnam;
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

/*
 * Return relations @us has with @them.
 * Countries are considered allied to themselves.
 */
enum relations
relations_with(natid us, natid them)
{
    return us == them ? ALLIED : getnatp(us)->nat_relate[them];
}

char *
relations_string(enum relations rel)
{
    static char *relates[] = {
	/* must match enum relations */
	"At War", "Hostile", "Neutral", "Friendly", "Allied"
    };

    return relates[rel];
}

int
nat_accepts(natid us, natid them, enum rej_comm what)
{
    return getnatp(them)->nat_stat == STAT_GOD
	|| !(getrejectp(us)->rej_rejects[them] & bit(what));
}

void
agecontact(struct natstr *np)
{
    struct contactstr con;
    int them;

    getcontact(np->nat_cnum, &con);

    for (them = 1; them < MAXNOC; ++them) {
	if (them != np->nat_cnum && con.con_contact[them])
	    con.con_contact[them]--;
    }
}

int
in_contact(natid us, natid them)
{
    return getcontactp(us)->con_contact[them];
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
 * Initialize country #@cnum in status @stat.
 * @stat must be STAT_UNUSED, STAT_NEW, STAT_VIS or STAT_GOD.
 * Also wipe realms and telegrams.
 */
void
nat_reset(natid cnum, char *name, char *rep, enum nat_status stat)
{
    struct natstr nat;
    struct realmstr newrealm;
    char buf[1024];
    int i;

    ef_blank(EF_NATION, cnum, &nat);
    nat.nat_stat = stat;
    strncpy(nat.nat_cnam, name, sizeof(nat.nat_cnam) - 1);
    strncpy(nat.nat_pnam, rep, sizeof(nat.nat_pnam) - 1);
    if (stat == STAT_GOD)
	nat.nat_money = 123456789;
    for (i = 0; i < MAXNOR; i++) {
	ef_blank(EF_REALM, i + cnum * MAXNOR, &newrealm);
	putrealm(&newrealm);
    }
    mailbox_create(mailbox(buf, cnum));
    /* FIXME nat.nat_ann = #annos */
    nat.nat_level[NAT_HLEV] = start_happiness;
    nat.nat_level[NAT_RLEV] = start_research;
    nat.nat_level[NAT_TLEV] = start_technology;
    nat.nat_level[NAT_ELEV] = start_education;
    for (i = 0; i < MAXNOC; i++)
	nat.nat_relate[i] = NEUTRAL;
    nat.nat_flags =
	NF_FLASH | NF_BEEP | NF_COASTWATCH | NF_SONAR | NF_TECHLISTS;
    putnat(&nat);
}
