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
 *  login.c: Allow the player to login
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 2000
 */

#ifdef Rel4
#include <string.h>
#endif /* Rel4 */
#include "prototypes.h"
#include "misc.h"
#include "proto.h"
#include "player.h"
#include "com.h"
#include "empthread.h"
#include "empio.h"
#include "nsc.h"
#include "nat.h"
#include "optlist.h"
#include "file.h"
#include "subs.h"
#include "common.h"
#include "gen.h"

#ifdef aix
#include <unistd.h>
#endif /* aix */
#if !defined(_WIN32)
#include <netinet/in.h>
#endif
#include <errno.h>

struct cmndstr login_coms[];


/*ARGSUSED*/
void
player_login(void *ud)
{
	s_char	buf[128];
	s_char	space[512];
	s_char	*av[64];
	int	ac;
	int	cmd;

	player->proc = empth_self();

	pr_id(player, C_INIT, "Empire server ready\n");

	while (!io_eof(player->iop) && !io_error(player->iop)) {
		io_output(player->iop, IO_WAIT);
		if (io_gets(player->iop, buf, sizeof(buf)) < 0) {
			io_input(player->iop, IO_WAIT);
			continue;
		}
		ac = parse(buf, av, 0, space, 0);
		cmd = comtch(av[0], login_coms, 0, 0);
		if (cmd < 0) {
			pr_id(player, C_BADCMD, "Command %s not found\n", av[0]);
			continue;
		}
		switch (login_coms[cmd].c_addr(player, ac, av)) {
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
		if (player->state >= PS_SHUTDOWN)
			break;
	}
	player->state = PS_SHUTDOWN;
	if (!io_eof(player->iop)) {
		(void) pr_player(player, C_EXIT, "so long...\n");
		io_noblocking(player->iop, 0);
		while (io_output(player->iop, IO_WAIT) > 0)
			;
	}
	player_delete(player); 
	empth_exit();
	/*NOTREACHED*/
}

int
client_cmd(struct player *player, int ac, char **av)
{
	int i;

	if (ac < 1 || av[1] == '\0')
		return RET_SYN;

	for (i = 1; i < ac; ++i) {
		if (i > 1)
			(void) strncat(player->client, " ", sizeof(player->client)-1);
		(void) strncat(player->client, av[i], sizeof(player->client)-1);
	}
	player->client[sizeof(player->client)-1] = '\0';
	pr_id(player, C_CMDOK, "talking to %s\n", player->client);
	return RET_OK;
}

int
user_cmd(struct player *player, int ac, char **av)
{
	if (ac < 1 || av[1] == '\0')
		return RET_SYN;
	(void) strncpy(player->userid, av[1], sizeof(player->userid)-1);
	player->userid[sizeof(player->userid)-1] = '\0';
	pr_id(player, C_CMDOK, "hello %s\n", player->userid);
	return RET_OK;
}

int
sanc_cmd(struct player *player, int ac, char **av)
{
	struct	nstr_item ni;
	struct	natstr nat;
	int	first = 1;

	if (!opt_BLITZ) {
		pr_id(player, C_BADCMD, "Command %s not found\n", av[0]);
		return RET_FAIL;
	}

	snxtitem_all(&ni, EF_NATION);
	while (nxtitem(&ni, (s_char *)&nat)) {
		if (nat.nat_stat != (STAT_INUSE|STAT_SANCT))
			continue;
		if (first) {
			pr_id(player, C_DATA, "The following countries are still in sanctuary:\n");
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

int
coun_cmd(struct player *player, int ac, char **av)
{
	natid	cnum;

	if (ac < 1 || av[1] == '\0')
		return RET_SYN;
	if (natbyname(av[1], &cnum) < 0) {
		pr_id(player, C_CMDERR, "country %s does not exist\n",av[1]);
		return 0;
	}
	player->cnum = cnum;
	player->validated = 0;
	pr_id(player, C_CMDOK, "country name %s\n", av[1]);
	return 0;
}

int
pass_cmd(struct player *player, int ac, char **av)
{
	if (ac < 1 || av[1] == '\0')
		return RET_SYN;
	if (player->cnum == 255) {
		pr_id(player, C_CMDERR, "need country first\n");
		return RET_FAIL;
	}
	if (!natpass(player->cnum, av[1])) {
		pr_id(player, C_CMDERR, "password bad, logging entry\n");
		logerror("%s tried country #%d with %s",
			 praddr(player), player->cnum, av[1]);
		return RET_FAIL;
	}
	player->validated++;
	pr_id(player, C_CMDOK, "password ok\n");
	logerror("%s using country #%d", praddr(player), player->cnum);
	return RET_OK;
}

/*ARGSUSED*/
int
play_cmd(struct player *player, int ac, char **av)
{
	extern char *banfil;
	struct	player *other;
	natid	cnum;
	struct natstr *natp;

	if (ac == 4) {
		(void) strncpy(player->userid, av[1], sizeof(player->userid)-1);
		player->userid[sizeof(player->userid)-1] = '\0';
		player->validated = 0;
		if (natbyname(av[2], &cnum) < 0) {
			pr_id(player, C_CMDERR, "country %s does not exist\n", av[2]);
			return 0;
		}
		if (!natpass(cnum, av[3])) {
			pr_id(player, C_CMDERR, "password bad, logging entry\n");
			logerror("%s tried country #%d with %s",
				 praddr(player), cnum, av[3]);
			return RET_FAIL;
		}
		player->cnum = cnum;
		player->validated++;
	}
	if (player->cnum == 255 || !player->validated) {
		pr_id(player, C_CMDERR, "need country and password\n");
		return RET_FAIL;
	}
	if ((other = player_find_other(player, (natid)player->cnum)) != 0) {
	        natp = getnatp(player->cnum);
		if (natp->nat_stat != VIS) {
		  pr_id(player, C_EXIT, "country in use by %s\n",
			praddr(other));
		} else {
		  pr_id(player, C_EXIT, "country in use\n");
		}
		return RET_FAIL;
	}
	if (match_user(banfil, player)) {
		logerror("Attempted login by BANNED host %s",
			 praddr(player));
		pr("Your login has been banned from this game\n");
		io_shutdown(player->iop, IO_READ);
		return RET_FAIL;
	}
	player_main(player);
	player->state = PS_SHUTDOWN;
	return RET_OK;
}

/*ARGSUSED*/
int
kill_cmd(struct player *player, int ac, char **av)
{
	struct	player *other;

	if (player->cnum == 255 || !player->validated) {
		pr_id(player, C_CMDERR, "need country and password\n");
		return RET_FAIL;
	}
	/* XXX find cnum other than us */
	if ((other = player_find_other(player, player->cnum)) == 0) {
		pr_id(player, C_EXIT, "country not in use\n");
		return RET_FAIL;
	}
	io_shutdown(other->iop, IO_READ|IO_WRITE);
	pr_id(player, C_EXIT, "closed socket of offending job\n");
	return RET_OK;
}

/*ARGSUSED*/
int
list_cmd(struct player *player, int ac, char **av)
{
	struct	player *lp;
	int	first = 1;

	if (player->cnum != 0 || !player->validated) {
		pr_id(player, C_CMDERR, "Permission denied\n");
		return 0;
	}
	for (lp = player_next(0); lp != 0; lp = player_next(lp)) {
		if (first) {
			pr_id(player, C_DATA, "user@host\tcountry\tpid\n");
			first = 0;
		}
		pr_id(player, C_DATA, "%s\t%d\n", praddr(lp), lp->cnum);
	}
	if (first == 0)
		pr_id(player, C_DATA, "\n");
	return 0;
}

/*ARGSUSED*/
int
quit_cmd(struct player *player, int ac, char **av)
{
	pr_id(player, C_EXIT, "so long\n");
	io_shutdown(player->iop, IO_READ);
	return RET_OK;
}

struct cmndstr login_coms[] = {
	{ "list",				0, list_cmd, 0, 0 },
	{ "client clientname [version info]", 0, client_cmd, 0, 0 },
	{ "user username",		0, user_cmd, 0, 0 },
	{ "sanc",				0, sanc_cmd, 0, 0 },
	{ "coun countryname",		0, coun_cmd, 0, 0 },
	{ "pass password",		0, pass_cmd, 0, 0 },
	{ "play [user country pass]",	0, play_cmd, 0, 0 },
	{ "quit ",			0, quit_cmd, 0, 0 },
	{ "kill {self}",			0, kill_cmd, 0, 0 },
	{ 0,				0, 0, 0, 0 }
};
