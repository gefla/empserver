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
 *  player.c: Main command loop for a player
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     
 */

#include "prototypes.h"
#include <string.h>
#include "misc.h"
#include "player.h"
#include "proto.h"
#include "com.h"
#include "nat.h"
#include "sect.h"
#include "file.h"
#include "proto.h"
#include "empio.h"
#include "empthread.h"
#include "tel.h"
#include "gen.h"
#include "subs.h"
#include "common.h"
#include "optlist.h"

#if !defined(_WIN32)
#include <unistd.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

static int status(void);

struct player *player;

void
player_main(struct player *p)
{
    struct natstr *natp;
    int secs;
    char buf[128];

    p->state = PS_PLAYING;
    player = p;
    time(&player->lasttime);
    time(&player->curup);
    show_motd();
    if (init_nats() < 0)
	return;
    natp = getnatp(player->cnum);
    if (player->god && !match_user(authfil, player)) {
	logerror("NON-AUTHed Login attempted by %s", praddr(player));
	pr("You're not a deity!\n");
	return;
    }
    if (!gamehours(player->curup)) {
	pr("Empire hours restriction in force\n");
	if ((natp->nat_stat & STAT_GOD) == 0)
	    return;
    }
    daychange(player->curup);
    if ((player->minleft = getminleft(player->curup, m_m_p_d)) <= 0) {
	pr("Time exceeded today\n");
	return;
    }
    if (natp->nat_stat != VIS
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

    time(&natp->nat_last_login);
    putnat(natp);
    if (natp->nat_flags & NF_INFORM && natp->nat_tgms > 0) {
	if (natp->nat_tgms == 1)
	    pr("You have a new telegram waiting ...\n");
	else
	    pr("You have %s new telegrams waiting ...\n",
	       numstr(buf, natp->nat_tgms));
	natp->nat_tgms = 0;
    }

    while (status()) {
	if (command() == 0 && !player->aborted)
	    break;
	player->aborted = 0;
	empth_yield();
    }
    /* #*# I put the following line in to prevent server crash -KHS */
    natp = getnatp(player->cnum);
    /*
     * randomly round up to the nearest minute,
     * charging at least 15 seconds.
     */
    time(&natp->nat_last_logout);
    secs = max(natp->nat_last_logout - player->lasttime, 15);
    natp->nat_minused += secs / 60;
    secs = secs % 60;
    if (chance(secs / 60.0))
	natp->nat_minused += 1;
    putnat(natp);
    pr("Bye-bye\n");
}

int
command(void)
{
    unsigned int x;
    char *redir;		/* UTF-8 */
    char scanspace[1024];

    if (getcommand(player->combuf) < 0)
	return 0;
    if (parse(player->combuf, player->argp, &player->condarg,
	      scanspace, &redir) < 0) {
	pr("See \"info Syntax\"?\n");
    } else {
	/* XXX don't use alarm; use a scavenger thread */
	/* DONT USE IT!!!! alarm and sleep may and dont work
	   together -- Sasha */
	/* alarm((unsigned int)60*60); 1 hour */
	if (player->condarg != NULL)
	    for (x = 0; x < strlen(player->condarg); x++)
		if (isupper(*(player->condarg + x)))
		    *(player->condarg + x) =
			tolower(*(player->condarg + x));
	if (dispatch(player->combuf, redir) < 0)
	    pr("Try \"list of commands\" or \"info\"\n");
    }
    return 1;
}

static int
status(void)
{
    struct natstr *natp;
    int minute;
    struct sctstr sect;
    char buf[128];

    if (player->state == PS_SHUTDOWN)
	return 0;
    natp = getnatp(player->cnum);
    if (io_error(player->iop) || io_eof(player->iop)) {
	putnat(natp);
	return 0;
    }
    player->visitor = (natp->nat_stat & (STAT_NORM | STAT_GOD)) == 0;
    if (player->dolcost != 0.0) {
	if (player->dolcost > 100.0)
	    pr("That just cost you $%.2f\n", player->dolcost);
	else if (player->dolcost < -100.0)
	    pr("You just made $%.2f\n", -player->dolcost);
	if (natp->nat_money < player->dolcost && !player->broke) {
	    player->broke = 1;
	    player->nstat &= ~MONEY;
	    pr("You are now broke; industries are on strike.\n");
	} else if (player->broke && natp->nat_money - player->dolcost > 0) {
	    player->broke = 0;
	    player->nstat |= MONEY;
	    pr("You are no longer broke!\n");
	}
	natp->nat_money -= roundavg(player->dolcost);
	player->dolcost = 0.0;
    } else {
	if (natp->nat_money < 0.0 && !player->broke) {
	    player->broke = 1;
	    player->nstat &= ~MONEY;
	    pr("You are now broke; industries are on strike.\n");
	}
	if (player->broke && natp->nat_money > 0) {
	    player->broke = 0;
	    player->nstat |= MONEY;
	    pr("You are no longer broke!\n");
	}
    }
    getsect(natp->nat_xcap, natp->nat_ycap, &sect);
    if (influx(natp))
	player->nstat &= ~CAP;
    else
	player->nstat |= CAP;
    player->ncomstat = player->nstat;
    time(&player->curup);
    minute = (player->curup - player->lasttime) / 60;
    if (minute > 0) {
	player->minleft -= minute;
	if (player->minleft <= 0) {
	    /*
	     * countdown timer "player->minleft" has expired.
	     * either day change, or hours restriction
	     */
	    daychange(player->curup);
	    if (!gamehours(player->curup)) {
		pr("Empire hours restriction in force\n");
		if ((natp->nat_stat & STAT_GOD) == 0) {
		    putnat(natp);
		    return 0;
		}
	    }
	    player->minleft = getminleft(player->curup, m_m_p_d);
	}
	player->lasttime += minute * 60;
	natp->nat_minused += minute;
    }
    if ((player->nstat & NORM) && natp->nat_minused > m_m_p_d) {
	pr("Max minutes per day limit exceeded.\n");
	player->ncomstat = VIS;
    }
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
    if (!player->visitor && !player->god && (player->nstat & CAP) == 0)
	pr("You lost your capital... better designate one\n");
    putnat(natp);
    if (gamedown() && !player->god) {
	pr("gamedown\n");
	return 0;
    }
    return 1;
}

/*
 * actually a command; redirection and piping ignored.
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
    redir = NULL;

    p = getstarg(player->argp[1], "File? ", buf);

    if (p == NULL || *p == '\0')
	return RET_SYN;

    /* FIXME should use raw argument here, to support UTF-8 file names */
    prexec(player->argp[1]);

    while (!failed && status()) {
	if (recvclient(buf, sizeof(buf)) < 0)
	    break;
	if (parse(buf, player->argp, &player->condarg,
		  scanspace, &redir) < 0) {
	    failed = 1;
	    continue;
	}
	if (redir == NULL)
	    pr("\nExecute : %s\n", buf);
	if (dispatch(buf, redir) < 0)
	    failed = 1;
    }
    if (failed) {
	while (recvclient(buf, sizeof(buf)) >= 0) ;
    }
    if (redir == NULL)
	pr("Execute : %s\n", failed ? "aborted" : "terminated");
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
	    return RET_SYS;
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
match_user(char *file, struct player *p)
{
    FILE *fp;
    int match = 0;
    char host[256];
    char user[256];

    if ((fp = fopen(file, "r")) == NULL) {
	/*logerror("Cannot find file %s", file); */
	return 0;
    }
    match = 0;
    while (!feof(fp) && !match) {
	if (fgets(host, sizeof(host), fp) == NULL)
	    break;
	if (host[0] == '#')
	    continue;
	if (fgets(user, sizeof(user), fp) == NULL)
	    break;
	host[strlen(host) - 1] = '\0';
	user[strlen(user) - 1] = '\0';
	if (strstr(p->userid, user) &&
	    (strstr(p->hostaddr, host) ||
	     strstr(p->hostname, host)))
	    ++match;
    }
    fclose(fp);
    return match;
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
