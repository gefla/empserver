/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  accept.c: Keep track of people logged in
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 */

#include "prototypes.h"
#include "misc.h"
#include "proto.h"
#include "empthread.h"
#include "player.h"
#include "file.h"
#include "empio.h"
#include "power.h"
#include "gen.h"
#include "optlist.h"

#if !defined(_WIN32)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#else
#include <winsock2.h>
#endif
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

struct emp_qelem Players;

void
player_init(void)
{
    emp_initque(&Players);
    init_player_commands();
}

struct player *
player_new(int s, struct sockaddr_in *sin)
{
    struct player *lp;
#ifdef RESOLVE_IPADDRESS
    struct hostent *hostp;
#endif

    lp = (struct player *)malloc(sizeof(struct player));
    if (!lp)
      return NULL;
    memset(lp, 0, sizeof(struct player));
    if (sin) {
	/* real player, not dummy created by update and market update */
	lp->iop = io_open(s,
			  IO_READ | IO_WRITE | IO_NBLOCK,
			  IO_BUFSIZE, 0, 0);
	if (!lp->iop) {
	    free(lp);
	    return NULL;
	}
	emp_insque(&lp->queue, &Players);
	strcpy(lp->hostaddr, inet_ntoa(sin->sin_addr));
#ifdef RESOLVE_IPADDRESS
	if (NULL !=
	    (hostp =
	     gethostbyaddr((char *)&sin->sin_addr, sizeof(sin->sin_addr),
			   AF_INET)))
	    strcpy(lp->hostname, hostp->h_name);
#endif /* RESOLVE_IPADDRESS */
	lp->cnum = 255;
	lp->curid = -1;
	time(&lp->curup);
    }
    return lp;
}

struct player *
player_delete(struct player *lp)
{
    struct player *back;

    back = (struct player *)lp->queue.q_back;
    if (back)
	emp_remque(&lp->queue);
    if (lp->iop) {
	/* it's a real player */
	io_close(lp->iop);
	lp->iop = 0;
    }
    free((s_char *)lp);
    /* XXX may need to free bigmap here */
    return back;
}

struct player *
player_next(struct player *lp)
{
    if (lp == 0)
	lp = (struct player *)Players.q_forw;
    else
	lp = (struct player *)lp->queue.q_forw;
    if (&lp->queue == &Players)
	return 0;
    return lp;
}

struct player *
player_prev(struct player *lp)
{
    if (lp == 0)
	lp = (struct player *)Players.q_back;
    else
	lp = (struct player *)lp->queue.q_back;
    if (&lp->queue == &Players)
	return 0;
    return lp;
}

struct player *
getplayer(natid cnum)
{
    register struct emp_qelem *qp;

    for (qp = Players.q_forw; qp != &Players; qp = qp->q_forw)
	if (((struct player *)qp)->cnum == cnum)
	    return (struct player *)qp;

    return 0;
}

struct player *
player_find_other(struct player *us, register natid cnum)
{
    register struct emp_qelem *qp;

    for (qp = Players.q_forw; qp != &Players; qp = qp->q_forw)
	if (((struct player *)qp)->cnum == cnum &&
	    ((struct player *)qp != us) &&
	    (((struct player *)qp)->state == PS_PLAYING))
	    return (struct player *)qp;


    return 0;
}

void
player_wakeup_all(natid cnum)
{
    register struct player *lp;

    if (NULL != (lp = getplayer(cnum)))
	player_wakeup(lp);
}

void
player_wakeup(struct player *pl)
{
    if (pl->waiting)
	empth_wakeup(pl->proc);
}

/*ARGSUSED*/
void
player_accept(void *unused)
{
    struct sockaddr_in sin;
    struct servent *sp;
    int s;
    short port;
    int val;
    struct player *np;
    int len;
    int ns;
    int set = 1;
    int stacksize;
    char buf[128];

    player_init();
    sp = getservbyname("empire", "tcp");
    if (sp == 0)
	port = htons(atoi(loginport));
    else
	port = sp->s_port;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = port;
    sin.sin_family = AF_INET;
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	logerror("inet socket create");
	exit(1);
    }
    val = 1;
#if !(defined(__linux__) && defined(__alpha__))
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val))
	< 0) {
	logerror("inet socket setsockopt SO_REUSEADDR (%d)", errno);
	exit(1);
    }
#else
    logerror("Alpha/Linux?  You don't support SO_REUSEADDR yet, do you?\n");
#endif
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	logerror("inet socket bind");
	exit(1);
    }
#ifdef LISTENMAXCONN		/* because someone in linux world didn't want to use
				 * SOMAXCONN as defined in the header files... */
    if (listen(s, LISTENMAXCONN) < 0) {
	logerror("inet socket listen");
	exit(1);
    }
#else
    if (listen(s, SOMAXCONN) < 0) {
	logerror("inet socket listen");
	exit(1);
    }
#endif
    while (1) {
	empth_select(s, EMPTH_FD_READ);
	len = sizeof(sin);
	ns = accept(s, (struct sockaddr *)&sin, &len);
	if (ns < 0) {
	    logerror("new socket accept");
	    continue;
	}
	(void)setsockopt(ns, SOL_SOCKET, SO_KEEPALIVE,
			 (char *)&set, sizeof(set));
	np = player_new(ns, &sin);
	if (!np) {
	    logerror("can't create player for fd %d", ns);
 	    close(ns);
 	    continue;
 	}
	/* XXX may not be big enough */
	stacksize = 100000
/* budget */  + max(WORLD_X * WORLD_Y / 2 * sizeof(int) * 7,
/* power */ MAXNOC * sizeof(struct powstr));
	sprintf(buf, "Player (fd #%d)", ns);
	empth_create(PP_PLAYER, player_login, stacksize,
		     0, buf, "Empire player", np);
    }
}
