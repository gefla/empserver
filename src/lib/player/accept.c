/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  accept.c: Keep track of people logged in
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Markus Armbruster, 2005-2012
 */

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "empio.h"
#include "empthread.h"
#include "file.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "power.h"
#include "proto.h"
#include "prototypes.h"

static struct emp_qelem Players;
static int player_socket;
static size_t player_addrlen;

void
player_init(void)
{
    emp_initque(&Players);
    init_player_commands();

    player_socket = tcp_listen(*listen_addr ? listen_addr : NULL,
			       loginport, &player_addrlen);
}

struct player *
player_new(int s)
{
    struct player *lp;

    lp = malloc(sizeof(struct player));
    if (!lp)
	return NULL;
    memset(lp, 0, sizeof(struct player));
    if (s >= 0) {
	/* real player, not dummy created by update and market update */
	lp->iop = io_open(s, IO_READ | IO_WRITE, IO_BUFSIZE);
	if (!lp->iop) {
	    free(lp);
	    return NULL;
	}
	emp_insque(&lp->queue, &Players);
	lp->cnum = NATID_BAD;
	lp->may_sleep = PLAYER_SLEEP_FREELY;
	lp->curid = -1;
	time(&lp->curup);
    }
    return lp;
}

struct player *
player_delete(struct player *lp)
{
    struct player *back;

    if (lp->iop) {
	/* it's a real player */
	io_close(lp->iop, player->curup + minutes(max_idle));
	lp->iop = NULL;
    }
    back = (struct player *)lp->queue.q_back;
    if (back)
	emp_remque(&lp->queue);
    free(lp);
    /* XXX may need to free bigmap here */
    return back;
}

struct player *
player_next(struct player *lp)
{
    if (!lp)
	lp = (struct player *)Players.q_forw;
    else
	lp = (struct player *)lp->queue.q_forw;
    if (&lp->queue == &Players)
	return NULL;
    return lp;
}

struct player *
player_prev(struct player *lp)
{
    if (!lp)
	lp = (struct player *)Players.q_back;
    else
	lp = (struct player *)lp->queue.q_back;
    if (&lp->queue == &Players)
	return NULL;
    return lp;
}

/*
 * Return player in state PS_PLAYING for CNUM.
 */
struct player *
getplayer(natid cnum)
{
    struct emp_qelem *qp;
    struct player *pl;

    for (qp = Players.q_forw; qp != &Players; qp = qp->q_forw) {
	pl = (struct player *)qp;
	if (pl->cnum == cnum && pl->state == PS_PLAYING)
	    return pl;
    }

    return NULL;
}

/*ARGSUSED*/
void
player_accept(void *unused)
{
    struct sockaddr *sap;
    void *inaddr;
    int s = player_socket;
    struct player *np;
    socklen_t len;
    int ns;
    int set = 1;
    int stacksize;
    char buf[128];
#ifdef RESOLVE_IPADDRESS
    struct hostent *hostp;
#endif

    /* auto sockaddr_storage would be simpler, but less portable */
    sap = malloc(player_addrlen);

    while (1) {
	empth_select(s, EMPTH_FD_READ, NULL);
	len = player_addrlen;
	ns = accept(s, sap, &len);
	/* FIXME accept() can block on some systems (RST after select() reports ready) */
	if (ns < 0) {
	    logerror("new socket accept");
	    continue;
	}
	/* FIXME SO_KEEPALIVE is useless, player_kill_idle() strikes long before */
	(void)setsockopt(ns, SOL_SOCKET, SO_KEEPALIVE, &set, sizeof(set));
	np = player_new(ns);
	if (!np) {
	    logerror("can't create player for fd %d", ns);
	    close(ns);
	    continue;
	}
#ifdef HAVE_GETADDRINFO
	inaddr = sap->sa_family == AF_INET
	    ? (void *)&((struct sockaddr_in *)sap)->sin_addr
	    : (void *)&((struct sockaddr_in6 *)sap)->sin6_addr;
	/* Assumes that if you got getaddrinfo(), you got inet_ntop() too */
	if (!inet_ntop(sap->sa_family, inaddr,
		       np->hostaddr, sizeof(np->hostaddr))) {
	    logerror("inet_ntop() failed: %s", strerror(errno));
	    player_delete(np);
	    continue;
	}
#else
	inaddr = &((struct sockaddr_in *)sap)->sin_addr;
	strcpy(np->hostaddr, inet_ntoa(*(struct in_addr *)inaddr));
#endif
#ifdef RESOLVE_IPADDRESS
	hostp = gethostbyaddr(inaddr, player_addrlen, sap->sa_family);
	if (NULL != hostp)
	    strcpy(np->hostname, hostp->h_name);
#endif /* RESOLVE_IPADDRESS */
	/* FIXME ancient black magic; figure out true stack need */
	stacksize = 100000
/* budget */  + MAX(WORLD_SZ() * sizeof(int) * 7,
/* power */ MAXNOC * sizeof(struct powstr));
	sprintf(buf, "Conn%d", ns);
	empth_create(player_login, stacksize, 0, buf, np);
    }
}
