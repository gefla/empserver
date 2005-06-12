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
 *  head.c: Print headlines of the Empire News
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "news.h"
#include "nat.h"
#include "file.h"
#include "xy.h"
#include "nsc.h"
#include "commands.h"

struct histstr {
    int h_past;
    int h_recent;
};

static s_char *head_meanwhile(int val);
static s_char *head_describe(struct histstr *hp, int what);
static int head_printscoop(struct histstr (*hist)[MAXNOC], natid ano,
			   natid vno);
static int head_findscoop(struct histstr (*hist)[MAXNOC],
			  register natid maxcnum, natid *ano, natid *vno);

int
head(void)
{
    register int i;
    register struct histstr *hp;
    register natid maxcnum;
    time_t now;
    int severity;
    int scoop;
    time_t news_per;
    time_t news_age;
    struct histstr hist[MAXNOC][MAXNOC];
    struct natstr *natp;
    struct nwsstr news;
    natid actor;
    natid victim;
    struct nstr_item nstr;
    int n;

    (void)time(&now);
    natp = getnatp(player->cnum);
    if (player->argp[1] != 0 && *player->argp[1] != 0) {
	news_per = days(atoi(player->argp[1]));
	if (news_per > days(3))
	    news_per = days(3);
    } else
	/* replaced the following line with the one under it to
	 * fix headlines which never change.
	 * 3/3/90 bailey@math-cs.kent.edu
	 */
/*		news_per = natp->nat_newstim; */
	news_per = now - natp->nat_newstim;
    pr("\n        -=[  EMPIRE NEWS  ]=-\n");
    pr("::::::::::::::::::::::::::::::::::::::::::::::::::\n");
    pr("!       \"All the news that fits, we print.\"      !\n");
    pr("::::::::::::::::::::::::::::::::::::::::::::::::::\n");
    pr("       %s", ctime(&now));
    pr("\n");
    memset(hist, 0, sizeof(hist));
    snxtitem_all(&nstr, EF_NEWS);
    maxcnum = 0;
    while (nxtitem(&nstr, &news)) {
	news_age = now - news.nws_when;
	if (news_age > news_per)
	    continue;
	if (news.nws_ano == news.nws_vno)
	    continue;
	if ((i = rpt[(int)news.nws_vrb].r_good_will) == 0)
	    continue;
	if (news_age > (news_per / 2))
	    hist[news.nws_ano][news.nws_vno].h_past += i;
	else
	    hist[news.nws_ano][news.nws_vno].h_recent += i;
	if (maxcnum < news.nws_ano)
	    maxcnum = news.nws_ano;
	if (maxcnum < news.nws_vno)
	    maxcnum = news.nws_vno;
    }
    for (n = 0; n < 5; n++) {
	if ((scoop = head_findscoop(hist, maxcnum, &actor, &victim)) < 10)
	    break;
	severity = head_printscoop(hist, actor, victim);
	hp = &hist[actor][victim];
	severity = hp->h_recent - hp->h_past;
	if (severity <= -scoop / 2 || severity >= scoop / 2) {
	    pr("\t%s\n", head_meanwhile(severity));
	    (void)head_printscoop(hist, victim, actor);
	}
    }
    if (n <= 1)
	pr("\nRelative calm prevails.\n");
    return RET_OK;
}

static int
head_printscoop(struct histstr (*hist)[MAXNOC], natid ano, natid vno)
{
    register struct histstr *hp;
    int severity;

    hp = &hist[ano][vno];
    severity = abs(hp->h_past) > abs(hp->h_recent) ? 1 : 0;
    severity += (hp->h_past >= 0) ? 2 : 0;
    severity += (hp->h_recent >= 0) ? 4 : 0;
    pr(head_describe(hp, severity), cname(ano), cname(vno));
    pr("\n");
    hp->h_past = 0;
    hp->h_recent = 0;
    return severity;
}

static s_char *
head_meanwhile(int val)
{
    switch (val & 03) {
    case 0:
	return "Meanwhile";
    case 1:
	return "On the other hand";
    case 2:
	return "At the same time";
    case 3:
	return "Although";
    }
    /*NOTREACHED*/
    return "";
}

static s_char *
head_describe(struct histstr *hp, int what)
{
    s_char *cp;

    cp = 0;
    switch (what) {
    case 0:
	if (hp->h_recent > hp->h_past / 2)
	    cp = "Bad relations between %s and %s worsen!";
	else
	    cp = "Carnage wrought by %s on %s continues unabated!";
	break;
    case 1:
	if (hp->h_recent < -16)
	    cp = "%s agression against %s has lessened slightly";
	else
	    cp = "Peace talks may occur between %s & %s";
	break;
    case 2:
	if (hp->h_recent < -16) {
	    if (hp->h_past > 0)
		cp = " ! WAR !  Reversal of prior %s -- %s relations";
	    else if (hp->h_recent >= -25)
		cp = "VIOLENCE ERUPTS! -- %s wages war on %s";
	    else
		cp = "%s wreaks havoc on %s!";
	} else
	    cp = "Breakdown in communication between %s & %s";
	break;
    case 3:
	cp = "FLASH!    %s turns on former ally, %s!";
	break;
    case 4:
	cp = "%s \"makes friends\" with %s";
	break;
    case 5:
	if (hp->h_past >= -25)
	    cp = "%s seems to have forgotten earlier disagreement with %s";
	else
	    cp = "Tensions ease as %s attacks on %s seem at an end";
	break;
    case 6:
	cp = "%s good deeds further growing alliance with %s";
	break;
    case 7:
	if (hp->h_recent - hp->h_past < 12)
	    cp = "Honeymoon appears to be over between %s & %s";
	else
	    cp = "Friendly relations between %s & %s have cooled";
	break;
    }
    return cp;
}

/*
 * returns 9 if no scoops were found
 * Pretty strange.
 */
static int
head_findscoop(struct histstr (*hist)[MAXNOC], register natid maxcnum,
	       natid *ano, natid *vno)
{
    register struct histstr *hp;
    register int i;
    register int j;
    register int k;
    int scoop;
    natid actor;
    natid victim;

    scoop = 9;
    actor = 0;
    victim = 0;
    for (i = 1; i < maxcnum; i++) {
	for (j = 1; j < maxcnum; j++) {
	    hp = &hist[i][j];
	    k = abs(hp->h_recent / 2);
	    if (k > scoop) {
		scoop = k;
		actor = (natid)i;
		victim = (natid)j;
	    }
	    k = abs(hp->h_recent - hp->h_past);
	    if (k > scoop) {
		scoop = k;
		actor = (natid)i;
		victim = (natid)j;
	    }
	}
    }
    *ano = actor;
    *vno = victim;
    return scoop;
}
