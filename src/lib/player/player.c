/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  player.c: Main command loop for a player
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2008
 *     Ron Koenderink, 2004-2007
 */

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include "com.h"
#include "empio.h"
#include "empthread.h"
#include "file.h"
#include "journal.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "proto.h"
#include "prototypes.h"
#include "tel.h"


static int command(void);
static int status(void);

struct player *player;

void
player_main(struct player *p)
{
    struct natstr *natp;
    char buf[128];

    p->state = PS_PLAYING;
    player = p;
    time(&player->curup);
    update_timeused_login(player->curup);
    show_motd();
    if (init_nats() < 0) {
	pr("Server confused, try again later\n");
	return;
    }
    natp = getnatp(player->cnum);
    if (!may_play_now(natp, player->curup, 0))
	return;
    if (natp->nat_stat != STAT_VIS
	&& natp->nat_last_login
	&& (strcmp(natp->nat_hostaddr, player->hostaddr)
	    || strcmp(natp->nat_userid, player->userid))) {
	pr("Last connection from: %s", ctime(&natp->nat_last_login));
	pr("                  to: %s",
	   natp->nat_last_login <= natp->nat_last_logout
	   ? ctime(&natp->nat_last_logout) : "?");
	pr("                  by: %s@%s\n",
	   natp->nat_userid,
	   *natp->nat_hostname ? natp->nat_hostname : natp->nat_hostaddr);
    }
    strcpy(natp->nat_userid, player->userid);
    strcpy(natp->nat_hostname, player->hostname);
    strcpy(natp->nat_hostaddr, player->hostaddr);
    natp->nat_last_login = player->curup;
    putnat(natp);
    journal_login();
    if (natp->nat_flags & NF_INFORM && natp->nat_tgms > 0) {
	if (natp->nat_tgms == 1)
	    pr("You have a new telegram waiting ...\n");
	else
	    pr("You have %s new telegrams waiting ...\n",
	       numstr(buf, natp->nat_tgms));
	natp->nat_tgms = 0;
    }

    while (status() && command()) {
	player->aborted = player->eof;
	empth_yield();
    }
    /* #*# I put the following line in to prevent server crash -KHS */
    natp = getnatp(player->cnum);
    time(&natp->nat_last_logout);
    putnat(natp);
    update_timeused(natp->nat_last_logout);
    enforce_minimum_session_time();
    pr("Bye-bye\n");
    journal_logout();
}

static int
command(void)
{
    char *redir;		/* UTF-8 */
    char scanspace[1024];
    time_t now;

    if (getcommand(player->combuf) < 0)
	return 0;

    now = time(NULL);
    update_timeused(now);
    if (!may_play_now(getnatp(player->cnum), now, 1))
	return 0;

    if (parse(player->combuf, scanspace, player->argp, player->comtail,
	      &player->condarg, &redir) < 0) {
	pr("See \"info Syntax\"?\n");
    } else {
	if (dispatch(player->combuf, redir) < 0)
	    pr("Try \"list of commands\" or \"info\"\n");
    }
    return 1;
}

static int
status(void)
{
    struct natstr *natp;
    int old_nstat;
    char buf[128];

    if (player->eof || player->state == PS_SHUTDOWN)
	return 0;
    natp = getnatp(player->cnum);
    if (player->dolcost > 100.0)
	pr("That just cost you $%.2f\n", player->dolcost);
    else if (player->dolcost < -100.0)
	pr("You just made $%.2f\n", -player->dolcost);
    if (player->dolcost != 0.0) {
	/*
	 * Hackish work around for a race condition in the nightly
	 * build's regression tests: sometimes the update starts right
	 * after the force command yields, sometimes a bit later.  If
	 * it is late, we use one random number here, for the bye,
	 * and throwing off the random sequence.
	 */
	natp->nat_money -= roundavg(player->dolcost);
	player->dolcost = 0.0;
    }

    old_nstat = player->nstat;
    player_set_nstat(player, natp);
    if ((old_nstat & MONEY) && !(player->nstat & MONEY))
	pr("You are now broke; industries are on strike.\n");
    if (!(old_nstat & MONEY) && (player->nstat & MONEY))
	pr("You are no longer broke!\n");

    time(&player->curup);
    update_timeused(player->curup);
    if (!may_play_now(natp, player->curup, 0))
	return 0;
    if (player->btused) {
	natp->nat_btu -= player->btused;
	player->btused = 0;
    }
    if (natp->nat_tgms > 0) {
	if (!(natp->nat_flags & NF_INFORM)) {
	    if (natp->nat_tgms == 1)
		pr("You have a new telegram waiting ...\n");
	    else
		pr("You have %s new telegrams waiting ...\n",
		   numstr(buf, natp->nat_tgms));
	    natp->nat_tgms = 0;
	}
    }
    if (natp->nat_ann > 0) {
	if (natp->nat_ann == 1)
	    pr("You have a new announcement waiting ...\n");
	else
	    pr("You have %s new announcements waiting ...\n",
	       numstr(buf, natp->nat_ann));
	natp->nat_ann = 0;
    }
    if (natp->nat_stat == STAT_ACTIVE && (player->nstat & CAP) == 0)
	pr("You lost your capital... better designate one (see info capital)\n");
    putnat(natp);
    return 1;
}

/*
 * XXX This whole mess should be redone; execute block should
 * start with "exec start", and should end with "exec end".
 * We'll wait until 1.2 I guess.
 */
int
execute(void)
{
    char buf[1024];
    int failed;
    char *p;
    char *redir;		/* UTF-8 */
    char scanspace[1024];

    failed = 0;

    if (player->comtail[1])
	p = player->comtail[1];
    else
	p = getstring("File? ", buf);
    if (p == NULL || *p == '\0')
	return RET_SYN;
    prexec(p);

    while (!failed && status()) {
	player->nstat &= ~EXEC;
	if (recvclient(buf, sizeof(buf)) < 0)
	    break;
	if (parse(buf, scanspace, player->argp, player->comtail,
		  &player->condarg, &redir) < 0) {
	    failed = 1;
	    continue;
	}
	pr("\nExecute : %s\n", buf);
	if (redir) {
	    pr("Execute : redirection not supported\n");
	    failed = 1;
	} else if (dispatch(buf, NULL) < 0)
	    failed = 1;
    }
    if (failed) {
	while (recvclient(buf, sizeof(buf)) >= 0) ;
    }

    pr("Execute : %s\n", failed ? "aborted" : "terminated");
    player->eof = 0;
    return RET_OK;
}

int
show_motd(void)
{
    FILE *motd_fp;
    struct telstr tgm;
    char buf[MAXTELSIZE + 1];	/* UTF-8 */

    if ((motd_fp = fopen(motdfil, "rb")) == NULL) {
	if (errno == ENOENT)
	    return RET_OK;
	else {
	    pr ("Could not open motd.\n");
	    logerror("Could not open motd (%s).\n", motdfil);
	    return RET_FAIL;
	}
    }
    if (fread(&tgm, sizeof(tgm), 1, motd_fp) != 1) {
	logerror("bad header on login message (motdfil)");
	fclose(motd_fp);
	return RET_FAIL;
    }
    if (tgm.tel_length >= (long)sizeof(buf)) {
	logerror("text length (%ld) is too long for login message (motdfil)", tgm.tel_length);
	fclose(motd_fp);
	return RET_FAIL;
    }
    if (fread(buf, tgm.tel_length, 1, motd_fp) != 1) {
	logerror("bad length %ld on login message", tgm.tel_length);
	fclose(motd_fp);
	return RET_FAIL;
    }
    buf[tgm.tel_length] = 0;
    uprnf(buf);
    fclose(motd_fp);
    return RET_OK;
}

int
quit(void)
{
    player->state = PS_SHUTDOWN;
    return RET_OK;
}

char *
praddr(struct player *p)
{
    return prbuf("%s@%s", p->userid,
		 *p->hostname ? p->hostname : p->hostaddr);
}
