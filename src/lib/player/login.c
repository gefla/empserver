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
 *  login.c: Allow the player to login
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2008
 *     Ron Koenderink, 2005-2009
 */

#include <config.h>

#include "com.h"
#include "empio.h"
#include "empthread.h"
#include "file.h"
#include "match.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "player.h"
#include "proto.h"
#include "prototypes.h"

static int client_cmd(void);
static int coun_cmd(void);
static int kill_cmd(void);
static int options_cmd(void);
static int pass_cmd(void);
static int play_cmd(void);
static int quit_cmd(void);
static int sanc_cmd(void);
static int user_cmd(void);

static struct cmndstr login_coms[] = {
    {"client client-id...", 0, client_cmd, 0, 0},
    {"coun country", 0, coun_cmd, 0, 0},
    {"kill", 0, kill_cmd, 0, 0},
    {"options option=value...", 0, options_cmd, 0, 0},
    {"pass password", 0, pass_cmd, 0, 0},
    {"play [user [country [password]]]", 0, play_cmd, 0, 0},
    {"quit", 0, quit_cmd, 0, 0},
    {"sanc", 0, sanc_cmd, 0, 0},
    {"user name", 0, user_cmd, 0, 0},
    {0, 0, 0, 0, 0}
};

/*ARGSUSED*/
void
player_login(void *ud)
{
    char buf[128];
    char space[128];
    int ac;
    int cmd;
    int res;

    player->proc = empth_self();

    pr_id(player, C_INIT, "Empire server ready\n");

    while (player->state != PS_SHUTDOWN) {
	io_output(player->iop, IO_WAIT);
	if (io_gets(player->iop, buf, sizeof(buf)) < 0) {
	    res = io_input(player->iop, IO_WAIT);
	    if (res <= 0) {
		if (res == 0 && !io_eof(player->iop))
		    pr_id(player, C_DATA, "idle connection terminated\n");
		break;
	    }
	    continue;
	}
	ac = parse(buf, space, player->argp, NULL, NULL, NULL);
	cmd = comtch(player->argp[0], login_coms, 0);
	if (cmd < 0) {
	    pr_id(player, C_BADCMD, "Command %s not found\n", player->argp[0]);
	    continue;
	}
	switch (login_coms[cmd].c_addr()) {
	case RET_OK:
	    break;
	case RET_FAIL:
	    break;
	case RET_SYN:
	    pr_id(player, C_BADCMD, "Usage %s\n", login_coms[cmd].c_form);
	    break;
	default:
	    break;
	}
    }
    player->state = PS_SHUTDOWN;
    if (!io_eof(player->iop)) {
	pr_id(player, C_EXIT, "so long...\n");
	io_noblocking(player->iop, 0);
	while (io_output(player->iop, IO_WAIT) > 0) ;
    }
    player_delete(player);
    empth_exit();
    /*NOTREACHED*/
}

static int
client_cmd(void)
{
    int i, sz;
    char *p, *end;

    if (!player->argp[1])
	return RET_SYN;

    p = player->client;
    end = player->client + sizeof(player->client) - 1;
    for (i = 1; player->argp[i]; ++i) {
	if (i > 1)
	    *p++ = ' ';
	sz = strlen(player->argp[i]);
	sz = MIN(sz, end - p);
	memcpy(p, player->argp[i], sz);
	p += sz;
    }
    *p = 0;
    pr_id(player, C_CMDOK, "talking to %s\n", player->client);
    return RET_OK;
}

static int
user_cmd(void)
{
    if (!player->argp[1])
	return RET_SYN;
    strncpy(player->userid, player->argp[1], sizeof(player->userid) - 1);
    player->userid[sizeof(player->userid) - 1] = '\0';
    pr_id(player, C_CMDOK, "hello %s\n", player->userid);
    return RET_OK;
}

static int
sanc_cmd(void)
{
    struct nstr_item ni;
    struct natstr nat;
    int first = 1;

    if (!opt_BLITZ) {
	pr_id(player, C_BADCMD, "Command %s not found\n", player->argp[0]);
	return RET_FAIL;
    }

    snxtitem_all(&ni, EF_NATION);
    while (nxtitem(&ni, &nat)) {
	if (nat.nat_stat != STAT_SANCT)
	    continue;
	if (first) {
	    pr_id(player, C_DATA,
		  "The following countries are still in sanctuary:\n");
	    first = 0;
	}
	pr_id(player, C_DATA, "%s\n", nat.nat_cnam);
    }
    if (first)
	pr_id(player, C_CMDOK, "There are no countries in sanctuary\n");
    else
	pr_id(player, C_CMDOK, "\n");
    return RET_OK;
}

static int
coun_cmd(void)
{
    natid cnum;

    if (!player->argp[1])
	return RET_SYN;
    if (natbyname(player->argp[1], &cnum) < 0) {
	pr_id(player, C_CMDERR, "country %s does not exist\n", player->argp[1]);
	return RET_FAIL;
    }
    player->cnum = cnum;
    player->authenticated = 0;
    pr_id(player, C_CMDOK, "country name %s\n", player->argp[1]);
    return 0;
}

static int
pass_cmd(void)
{
    if (!player->argp[1])
	return RET_SYN;
    if (player->cnum == NATID_BAD) {
	pr_id(player, C_CMDERR, "need country first\n");
	return RET_FAIL;
    }
    if (!natpass(player->cnum, player->argp[1])) {
	pr_id(player, C_CMDERR, "password bad, logging entry\n");
	logerror("%s tried country #%d with %s",
		 praddr(player), player->cnum, player->argp[1]);
	return RET_FAIL;
    }
    player->authenticated = 1;
    pr_id(player, C_CMDOK, "password ok\n");
    logerror("%s using country #%d", praddr(player), player->cnum);
    return RET_OK;
}

static int
options_cmd(void)
{
    /*
     * The option mechanism allows arbitrary string values, but so far
     * all options are flags in struct player.  Should be easy to
     * generalize if needed.
     */
    struct logoptstr {
	char *name;
	int val;
    };
    static struct logoptstr login_opts[] = {
	{ "utf-8", PF_UTF8 },
	{ NULL, 0 }
    };
    char **ap;
    char *p;
    int opt;
    unsigned i;

    if (!player->argp[1]) {
	for (i = 0; login_opts[i].name; ++i)
	    pr_id(player, C_DATA, "%s=%d\n",
		  login_opts[i].name,
		  (player->flags & login_opts[i].val) != 0);
	pr_id(player, C_CMDOK, "\n");
	return RET_OK;
    }

    for (ap = player->argp+1; *ap; ++ap) {
	p = strchr(*ap, '=');
	if (p)
	    *p++ = 0;
	opt = stmtch(*ap, login_opts,
		     offsetof(struct logoptstr, name),
		     sizeof(struct logoptstr));
	if (opt < 0) {
	    pr_id(player, C_BADCMD, "Option %s not found\n", *ap);
	    return RET_FAIL;
	}
	if (!p || atoi(p))
	    player->flags |= login_opts[opt].val;
	else
	    player->flags &= ~login_opts[opt].val;
    }

    pr_id(player, C_CMDOK, "Accepted\n");

    return RET_OK;
}

static int
may_play(void)
{
    struct natstr *np;

    if (player->cnum == NATID_BAD || !player->authenticated) {
	pr_id(player, C_CMDERR, "need country and password\n");
	return 0;
    }
    /* TODO strstr() cheesy, compare IP against IP/BITS ... */
    np = getnatp(player->cnum);
    if (np->nat_stat == STAT_GOD && *privip
	&& !strstr(privip, player->hostaddr)) {
	logerror("Deity login from untrusted host attempted by %s",
		 praddr(player));
	logerror("To allow this, add %s to econfig key privip",
		 player->hostaddr);
	pr_id(player, C_EXIT,
	      "Deity login not allowed from this IP!"
	      "  See log for help on how to allow it.\n");
	return 0;
    }
    return 1;
}

static int
play_cmd(void)
{
    struct player *other;
    natid cnum;
    struct natstr *natp;
    char **ap;
    char buf[128];

    ap = player->argp;
    if (*++ap) {
	strncpy(player->userid, *ap, sizeof(player->userid) - 1);
	player->userid[sizeof(player->userid) - 1] = '\0';
	player->authenticated = 0;
    }
    if (*++ap) {
	if (natbyname(*ap, &cnum) < 0) {
	    pr_id(player, C_CMDERR, "country %s does not exist\n", *ap);
	    return RET_FAIL;
	}
    }
    if (*++ap) {
	if (!natpass(cnum, *ap)) {
	    pr_id(player, C_CMDERR, "password bad, logging entry\n");
	    logerror("%s tried country #%d with %s",
		     praddr(player), cnum, *ap);
	    return RET_FAIL;
	}
	player->cnum = cnum;
	player->authenticated = 1;
    }
    if (!may_play())
	return RET_FAIL;
    other = getplayer((natid)player->cnum);
    if (other) {
	natp = getnatp(player->cnum);
	if (natp->nat_stat != STAT_VIS) {
	    pr_id(player, C_EXIT, "country in use by %s\n", praddr(other));
	} else {
	    pr_id(player, C_EXIT, "country in use\n");
	}
	return RET_FAIL;
    }
    snprintf(buf, sizeof(buf), "Play#%d", player->cnum);
    empth_set_name(empth_self(), buf);
    logerror("%s logged in as country #%d", praddr(player), player->cnum);
    pr_id(player, C_INIT, "%d\n", CLIENTPROTO);
    player_main(player);
    logerror("%s logged out, country #%d", praddr(player), player->cnum);
    player->state = PS_SHUTDOWN;
    return RET_OK;
}

static int
kill_cmd(void)
{
    struct player *other;

    if (!may_play())
	return RET_FAIL;
    other = getplayer(player->cnum);
    if (!other) {
	pr_id(player, C_EXIT, "country not in use\n");
	return RET_FAIL;
    }
    logerror("%s killed country #%d", praddr(player), player->cnum);
    io_shutdown(other->iop, IO_READ | IO_WRITE);
    pr_id(player, C_EXIT, "closed socket of offending job\n");
    return RET_OK;
}

static int
quit_cmd(void)
{
    pr_id(player, C_EXIT, "so long\n");
    io_shutdown(player->iop, IO_READ);
    return RET_OK;
}
