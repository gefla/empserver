/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  head.c: Print headlines of the Empire News
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2013
 */

#include <config.h>

#include "commands.h"
#include "news.h"

struct histstr {
    int h_past;
    int h_recent;
};

static char *head_meanwhile(int val);
static void head_describe(struct histstr *, int, char *, char *);
static int head_printscoop(struct histstr (*hist)[MAXNOC], natid ano,
			   natid vno);
static int head_findscoop(struct histstr (*hist)[MAXNOC],
			  natid maxcnum, natid *ano, natid *vno);

int
head(void)
{
    struct histstr *hp;
    natid maxcnum;
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
    int i, n;

    (void)time(&now);
    natp = getnatp(player->cnum);
    if (player->argp[1] && *player->argp[1]) {
	news_per = days(atoi(player->argp[1]));
	if (news_per > days(3))
	    news_per = days(3);
    } else
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
	if (!news.nws_vrb || CANT_HAPPEN(news.nws_vrb > N_MAX_VERB))
	    continue;
	news_age = now - news.nws_when;
	if (news_age > news_per)
	    continue;
	if (news.nws_ano == news.nws_vno)
	    continue;
	if ((i = rpt[(int)news.nws_vrb].r_good_will) == 0)
	    continue;
	if (news_age > news_per / 2)
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
	head_printscoop(hist, actor, victim);
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
    struct histstr *hp;
    int severity;

    hp = &hist[ano][vno];
    severity = abs(hp->h_past) > abs(hp->h_recent) ? 1 : 0;
    severity += (hp->h_past >= 0) ? 2 : 0;
    severity += (hp->h_recent >= 0) ? 4 : 0;
    head_describe(hp, severity, cname(ano), cname(vno));
    pr("\n");
    hp->h_past = 0;
    hp->h_recent = 0;
    return severity;
}

static char *
head_meanwhile(int val)
{
    static char *meanwhile[4] = {
	"Meanwhile", "On the other hand", "At the same time", "Although"
    };
    return meanwhile[val % 4];
}

static void
head_describe(struct histstr *hp, int what, char *aname, char *vname)
{
    switch (what) {
    case 0:
	if (hp->h_recent > hp->h_past / 2)
	    pr("Bad relations between %s and %s worsen!", aname, vname);
	else
	    pr("Carnage wrought by %s on %s continues unabated!",
	       aname, vname);
	break;
    case 1:
	if (hp->h_recent < -16)
	    pr("%s agression against %s has lessened slightly",
	       aname, vname);
	else
	    pr("Peace talks may occur between %s & %s", aname, vname);
	break;
    case 2:
	if (hp->h_recent < -16) {
	    if (hp->h_past > 0)
		pr(" ! WAR !  Reversal of prior %s -- %s relations",
		   aname, vname);
	    else if (hp->h_recent >= -25)
		pr("VIOLENCE ERUPTS! -- %s wages war on %s", aname, vname);
	    else
		pr("%s wreaks havoc on %s!", aname, vname);
	} else
	    pr("Breakdown in communication between %s & %s", aname, vname);
	break;
    case 3:
	pr("FLASH!    %s turns on former ally, %s!", aname, vname);
	break;
    case 4:
	pr("%s \"makes friends\" with %s", aname, vname);
	break;
    case 5:
	if (hp->h_past >= -25)
	    pr("%s seems to have forgotten earlier disagreement with %s",
	       aname, vname);
	else
	    pr("Tensions ease as %s attacks on %s seem at an end",
	       aname, vname);
	break;
    case 6:
	pr("%s good deeds further growing alliance with %s", aname, vname);
	break;
    case 7:
	if (hp->h_recent - hp->h_past < 12)
	    pr("Honeymoon appears to be over between %s & %s",
	       aname, vname);
	else
	    pr("Friendly relations between %s & %s have cooled",
	       aname, vname);
	break;
    }
}

/*
 * returns 9 if no scoops were found
 * Pretty strange.
 */
static int
head_findscoop(struct histstr (*hist)[MAXNOC], natid maxcnum,
	       natid *ano, natid *vno)
{
    struct histstr *hp;
    int i, j, k, scoop;
    natid actor, victim;

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
