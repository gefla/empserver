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
 *  wu.c: Write a telegram to a user from another
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     
 */

#include <stdarg.h>
#include "misc.h"
#include <fcntl.h>
#if !defined(_WIN32)
#include <sys/uio.h>
#endif
#include "nat.h"
#include "tel.h"
#include "file.h"
#include "player.h"
#include "prototypes.h"

static struct telstr last_tel[MAXNOC];

void
clear_telegram_is_new(natid to)
{
    last_tel[to].tel_type = 0;
    last_tel[to].tel_from = 0;
    last_tel[to].tel_date = 0;
}

/*
 * telegram_is_new counts new telegrams the same as read_telegrams in 
 * lib/commands/mail.c and lib/commands/rea.c
 */

static int
telegram_is_new(natid to, struct telstr *tel)
{
    extern int update_pending;
    int is_new = 0;

    is_new |= tel->tel_type != last_tel[to].tel_type;
    is_new |= tel->tel_from != last_tel[to].tel_from;
    is_new |= !update_pending &&	/* sometimes updates take a long time */
	abs(tel->tel_date - last_tel[to].tel_date) > TEL_SECONDS;

    last_tel[to].tel_type = tel->tel_type;
    last_tel[to].tel_from = tel->tel_from;
    last_tel[to].tel_date = tel->tel_date;

    return is_new;
}

/*VARARGS*/
int
wu(natid from, natid to, s_char *format, ...)
{
    struct natstr *np;
    va_list ap;
    s_char buf[4096];
    extern int update_pending;

    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    np = getnatp(from);
    if (update_pending)
	return typed_wu(from, to, buf, TEL_UPDATE);
    else if (np->nat_stat & STAT_GOD)
	return typed_wu(from, to, buf, TEL_BULLETIN);
    else
	return typed_wu(from, to, buf, TEL_NORM);
}

int
typed_wu(natid from, natid to, s_char *message, int type)
{
    register s_char *bp;
    int len;
    struct telstr tel;
    struct natstr *np;
#if !defined(_WIN32)
    struct iovec iov[2];
#endif
    int fd;
    s_char box[1024];
    int notify = 0;
    int new_tele = 0;
    struct player *other;

    if (type == TEL_ANNOUNCE)
	strcpy(box, annfil);
    else
	mailbox(box, to);

    if (type != TEL_ANNOUNCE)
	if ((np = getnatp(to)) == 0 ||
	    ((np->nat_stat & STAT_NORM) == 0 &&
	     (np->nat_stat & STAT_SANCT) == 0)) {
	    return -1;
	}
#if !defined(_WIN32)
    if ((fd = open(box, O_WRONLY | O_APPEND, 0)) < 0) {
#else
    if ((fd = open(box, O_WRONLY | O_APPEND | O_BINARY, 0)) < 0) {
#endif
	logerror("telegram 'open' of %s (#%d) failed", box, to);
	return -1;
    }
    tel.tel_from = from;
    (void)time(&tel.tel_date);
    bp = message;
    while (*bp++) ;
    len = bp - message;
    if (len >= MAXTELSIZE)
	len = (MAXTELSIZE - 1);
    message[len] = 0;
    tel.tel_length = len;
    tel.tel_type = type;
#if !defined(_WIN32)
    iov[0].iov_base = (caddr_t)&tel;
    iov[0].iov_len = sizeof(tel);
    iov[1].iov_base = message;
    iov[1].iov_len = len;
    if (writev(fd, iov, 2) < (int)(iov[0].iov_len + iov[1].iov_len)) {
#else
    if ((write(fd, &tel, sizeof(tel)) != sizeof(tel)) ||
	(write(fd, message, len) != len)) {
#endif
	logerror("telegram 'write' to #%d failed", to);
    } else if (type == TEL_ANNOUNCE) {
	for (to = 0; NULL != (np = getnatp(to)); to++) {
	    if (!(np->nat_stat & STAT_NORM) &&
		!(np->nat_stat & STAT_SANCT))
		continue;
	    if (!player->god && (getrejects(from, np) & REJ_ANNO))
		continue;
	    notify = (np->nat_ann == 0);
	    np->nat_ann++;
	    putnat(np);
	    if (notify)
		player_wakeup_all(to);
	}
    } else {
	notify = (np->nat_tgms == 0);
	new_tele = telegram_is_new(to, &tel);
	np->nat_tgms += new_tele || notify;
	putnat(np);

	if (new_tele && np->nat_flags & NF_INFORM) {
	    if (NULL != (other = getplayer(to))) {
		if (np->nat_tgms == 1)
		    pr_inform(other, "[new tele]\n");
		else
		    pr_inform(other, "[%d new teles]\n", np->nat_tgms);
		player_wakeup_all(to);
	    }
	} else if (notify)
	    player_wakeup_all(to);
    }

    close(fd);
    return 0;
}
