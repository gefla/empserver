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
 *  news.c: Show current Empire news
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "nat.h"
#include "news.h"
#include "file.h"
#include "xy.h"
#include "nsc.h"
#include "deity.h"
#include "commands.h"
#include "optlist.h"

static void preport(register struct nwsstr *np);

int
news(void)
{
    struct natstr *natp;
    time_t now;
    int page;
    time_t then;
    time_t delta;
    struct nwsstr nws;
    struct nstr_item nstr;
    int page_has_news[N_MAX_PAGE + 1];
    int there_is_news = 0;
    short sectors_taken[MAXNOC][MAXNOC];
    short sectors_delta;
    short max_delta = -1;
    short abs_delta;
    short k;
    int sectors_were_taken = 0;
    natid i, j;
    s_char num[128];
    s_char *verb;

    memset(page_has_news, 0, sizeof(page_has_news));
    memset(sectors_taken, 0, sizeof(sectors_taken));
    (void)head();
    (void)time(&now);
    natp = getnatp(player->cnum);
    then = natp->nat_newstim;
    if (!opt_HIDDEN) {
	if (player->argp[1] != 0 && isdigit(*player->argp[1])) {
	    delta = days(atoi(player->argp[1]));
	    then = now - delta;
	}
    }
    natp->nat_newstim = now;
/*	if (then < now - days(3))
		then = now - days(3);
*/
    snxtitem(&nstr, EF_NEWS, "*");
    pr("\nThe details of Empire news since %s", ctime(&then));
    while (nxtitem(&nstr, (s_char *)&nws)) {
	if (nws.nws_when < then)
	    continue;
	if (opt_HIDDEN) {
	    if (!player->god &&
		!(getcontact(getnatp(player->cnum), nws.nws_ano) &&
		  getcontact(getnatp(player->cnum), nws.nws_vno)))
		continue;
	}
	++page_has_news[rpt[(int)nws.nws_vrb].r_newspage];
	++there_is_news;
    }
    for (page = 1; page <= N_MAX_PAGE; page++) {
	if (!page_has_news[page])
	    continue;
	pr("\n\t ===  %s  ===\n", page_headings[page]);
	snxtitem_rewind(&nstr);
	while (nxtitem(&nstr, (s_char *)&nws)) {
	    if (rpt[(int)nws.nws_vrb].r_newspage != page)
		continue;
	    if (nws.nws_when < then)
		continue;
	    if (nws.nws_ntm == 0)
		nws.nws_ntm = 1;
	    if (opt_HIDDEN) {
		if (!player->god &&
		    !(getcontact(getnatp(player->cnum), nws.nws_ano) &&
		      getcontact(getnatp(player->cnum), nws.nws_vno)))
		    continue;
	    }
	    if (page == N_FRONT &&
		(nws.nws_vrb == N_WON_SECT ||
		 nws.nws_vrb == N_AWON_SECT ||
		 nws.nws_vrb == N_PWON_SECT)) {
		sectors_taken[nws.nws_ano][nws.nws_vno] += nws.nws_ntm;
		sectors_were_taken += nws.nws_ntm;
	    }
	    preport(&nws);
	}
    }
    if (sectors_were_taken) {
	for (i = 0; i < MAXNOC; ++i) {
	    for (j = 0; j < i; ++j) {
		sectors_delta = sectors_taken[i][j] - sectors_taken[j][i];
		if (max_delta < abs(sectors_delta))
		    max_delta = abs(sectors_delta);
	    }
	}
	pr("\n\t ===  The Bottom Line   ==\n");
	for (k = max_delta; k > 0; --k) {
	    for (i = 0; i < MAXNOC; ++i) {
		for (j = 0; j < i; ++j) {
		    sectors_delta = sectors_taken[i][j] -
			sectors_taken[j][i];
		    abs_delta = abs(sectors_delta);
		    if (abs_delta != k)
			continue;
		    if (abs_delta == 1)
			verb = "stole";
		    else if (abs_delta < 4)
			verb = "took";
		    else if (abs_delta < 8)
			verb = "captured";
		    else
			verb = "seized";
		    if (sectors_delta > 0) {
			numstr(num, abs_delta);
			pr("%s %s %s sector%s from %s\n", cname(i), verb,
			   num, splur(sectors_delta), cname(j));
		    } else if (sectors_delta < 0) {
			numstr(num, abs_delta);
			pr("%s %s %s sector%s from %s\n", cname(j), verb,
			   num, splur(-sectors_delta), cname(i));
		    }
		}
	    }
	}
    }
    if (!there_is_news)
	pr("\nNo news at the moment...\n");
    return 0;
}

static void
preport(register struct nwsstr *np)
{
    register s_char *cp;
    register int i;
    s_char buf[255];
    s_char num[128];
    s_char *ptr;

    cp = buf;
    sprintf(buf, "%-16.16s  ", ctime(&np->nws_when));
    cp += strlen(cp);
    ptr = numstr(num, np->nws_ntm);
    /*
     * vary the order of the printing of "%d times "
     */
    if ((random() & 3) == 0 && np->nws_ntm > 1) {
	sprintf(cp, "%s times ", ptr);
	cp += strlen(cp);
	np->nws_ntm = 1;
    }
    strcpy(cp, cname(np->nws_ano));
    cp += strlen(cp);
    *cp++ = ' ';
    if (np->nws_vrb < 1 || np->nws_vrb > N_MAX_VERB)
	np->nws_vrb = 0;
    sprintf(cp, rpt[(int)np->nws_vrb].r_newstory[random() % NUM_RPTS],
	    cname(np->nws_vno));
    cp += strlen(cp);
    if (np->nws_ntm != 1) {
	sprintf(cp, " %s times", ptr);
	cp += strlen(cp);
    }
    if (*buf >= 'a' && *buf <= 'z')
	*buf += 'A' - 'a';
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
