/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  news.c: Show current Empire news
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2016
 */

#include <config.h>

#include "chance.h"
#include "commands.h"
#include "news.h"
#include "optlist.h"

static void preport(struct nwsstr *np);
static int sectwon_cmp(const void *p, const void *q);

struct sectwon {
    natid ano, vno;
    unsigned short num;
};

int
news(void)
{
    struct natstr *natp;
    time_t now;
    int heading, page;
    time_t then;
    time_t delta;
    struct nwsstr nws;
    struct nstr_item nstr;
    int page_has_news[N_MAX_PAGE + 1];
    unsigned short sectors_taken[MAXNOC][MAXNOC];
    natid i, j;
    int k, n, diff;
    struct sectwon *sectwon;
    char num[128];
    char *verb;

    if (!snxtitem(&nstr, EF_NEWS, "*", NULL))
	return RET_SYN;
    memset(sectors_taken, 0, sizeof(sectors_taken));
    (void)time(&now);
    natp = getnatp(player->cnum);
    then = natp->nat_newstim;
    if (player->argp[1]) {
	/*
	 * We want to hide events before contact.  Proper solution
	 * would be to timestamp the contact.  Cheesy approximation:
	 * disable old news.
	 */
	if (opt_HIDDEN && !player->god) {
	    pr("Sorry, argument doesn't work with HIDDEN enabled\n");
	    return RET_FAIL;
	}
	delta = days(atoi(player->argp[1]));
	then = now - delta;
    }
    natp->nat_newstim = now;
    head();
    pr("\nThe details of Empire news since %s", ctime(&then));

    heading = 0;
    memset(page_has_news, 0, sizeof(page_has_news));
    page_has_news[0] = 1;

    for (page = 0; page <= N_MAX_PAGE; page++) {
	if (!page_has_news[page])
	    continue;
	snxtitem_rewind(&nstr);
	while (nxtitem(&nstr, &nws)) {
	    if (CANT_HAPPEN(nws.nws_vrb > N_MAX_VERB))
		continue;
	    if (nws.nws_when < then)
		continue;
	    if (CANT_HAPPEN(nws.nws_ntm <= 0))
		nws.nws_ntm = 1;
	    if (opt_HIDDEN) {
		if (!player->god &&
		    !(in_contact(player->cnum, nws.nws_ano) &&
		      in_contact(player->cnum, nws.nws_vno)))
		    continue;
	    }
	    page_has_news[rpt[nws.nws_vrb].r_newspage] = 1;
	    if (rpt[nws.nws_vrb].r_newspage != page)
		continue;
	    if (heading != page) {
		pr("\n\t ===  %s  ===\n", page_headings[page].name);
		heading = page;
	    }
	    if (nws.nws_vrb == N_WON_SECT ||
		nws.nws_vrb == N_AWON_SECT ||
		nws.nws_vrb == N_PWON_SECT)
		sectors_taken[nws.nws_ano][nws.nws_vno] += nws.nws_ntm;
	    preport(&nws);
	}
    }

    if (!heading) {
	pr("\nNo news at the moment...\n");
	return RET_OK;
    }

    n = 0;
    for (i = 0; i < MAXNOC; ++i) {
	for (j = 0; j < i; ++j)
	    n += !!(sectors_taken[i][j] - sectors_taken[j][i]);
    }
    sectwon = malloc(sizeof(*sectwon) * n);

    n = 0;
    for (i = 0; i < MAXNOC; ++i) {
	for (j = 0; j < i; ++j) {
	    diff = sectors_taken[i][j] - sectors_taken[j][i];
	    if (diff > 0) {
		sectwon[n].ano = i;
		sectwon[n].vno = j;
		sectwon[n].num = diff;
		n++;
	    } else if (diff < 0) {
		sectwon[n].ano = j;
		sectwon[n].vno = i;
		sectwon[n].num = -diff;
		n++;
	    }
	}
    }

    qsort(sectwon, n, sizeof(*sectwon), sectwon_cmp);

    if (n) {
	pr("\n\t ===  The Bottom Line   ==\n");
	for (k = 0; k < n; k++) {
	    if (sectwon[k].num == 1)
		verb = "stole";
	    else if (sectwon[k].num < 4)
		verb = "took";
	    else if (sectwon[k].num < 8)
		verb = "captured";
	    else
		verb = "seized";
	    numstr(num, sectwon[k].num);
	    pr("%s %s %s sector%s from %s\n",
	       cname(sectwon[k].ano), verb, num, splur(sectwon[k].num),
	       cname(sectwon[k].vno));
	}
    }

    free(sectwon);
    return RET_OK;
}

static void
preport(struct nwsstr *np)
{
    char *cp;
    int i;
    char buf[255];
    char num[128];
    char *ptr;

    cp = buf;
    sprintf(buf, "%-16.16s  ", ctime(&np->nws_when));
    cp += strlen(cp);
    ptr = numstr(num, np->nws_ntm);
    /*
     * vary the order of the printing of "%d times "
     */
    if (roll0(4) == 0 && np->nws_ntm > 1) {
	sprintf(cp, "%s times ", ptr);
	cp += strlen(cp);
	np->nws_ntm = 1;
    }
    strcpy(cp, cname(np->nws_ano));
    cp += strlen(cp);
    *cp++ = ' ';
    sprintf(cp, rpt[(int)np->nws_vrb].r_newstory[roll0(NUM_RPTS)],
	    cname(np->nws_vno));
    cp += strlen(cp);
    if (np->nws_ntm != 1) {
	sprintf(cp, " %s times", ptr);
	cp += strlen(cp);
    }
    if (cp - buf > 80) {
	for (i = 80; --i > 60;)
	    if (buf[i] == ' ')
		break;
	buf[i] = '\0';
	pr("%s\n\t\t  %s\n", buf, &buf[i + 1]);
    } else {
	pr("%s\n", buf);
    }
    np->nws_ntm = 0;
    return;
}

static int
sectwon_cmp(const void *p, const void *q)
{
    const struct sectwon *a = p, *b = q;
    int cmp;

    cmp = b->num - a->num;
    if (cmp)
       return cmp;
    cmp = b->ano - a->ano;
    if (cmp)
       return cmp;
    return b->vno - a->vno;
}
