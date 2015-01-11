/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  wu.c: Write a telegram to a user from another
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Markus Armbruster, 2005-2015
 */

#include <config.h>

#include <fcntl.h>
#include <stdarg.h>
#include <sys/uio.h>
#include <unistd.h>
#include "file.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "server.h"
#include "tel.h"

static struct telstr last_tel[MAXNOC];

void
clear_telegram_is_new(natid to)
{
    last_tel[to].tel_from = NATID_BAD;
}

/*
 * telegram_is_new counts new telegrams the same as read_telegrams in
 * lib/commands/mail.c and lib/commands/rea.c
 */

static int
telegram_is_new(natid to, struct telstr *tel)
{
    if (tel->tel_from != last_tel[to].tel_from
	|| tel->tel_type != last_tel[to].tel_type
	|| (tel->tel_type != TEL_UPDATE
	    && abs(tel->tel_date - last_tel[to].tel_date) > TEL_SECONDS)) {
	last_tel[to] = *tel;
	return 1;
    }
    return 0;
}

/*
 * Send a telegram from FROM to TO.
 * Format text to send using printf-style FORMAT and optional
 * arguments.  It is plain ASCII.
 * If running from the update, telegram type is TEL_UPDATE.
 * Else if FROM is a deity, type is TEL_BULLETIN.
 * Else it is TEL_NORM.
 * Return 0 on success, -1 on error.
 */
int
wu(natid from, natid to, char *format, ...)
{
    struct natstr *np;
    va_list ap;
    char buf[4096];

    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    np = getnatp(from);
    if (update_running)
	return typed_wu(from, to, buf, TEL_UPDATE);
    else if (np->nat_stat == STAT_GOD)
	return typed_wu(from, to, buf, TEL_BULLETIN);
    else
	return typed_wu(from, to, buf, TEL_NORM);
}

/*
 * Send a telegram from FROM to TO.
 * MESSAGE is the text to send, in UTF-8.
 * TYPE is the telegram type.
 * Return 0 on success, -1 on error.
 */
int
typed_wu(natid from, natid to, char *message, int type)
{
    size_t len;
    struct telstr tel;
    struct natstr *np;
    struct iovec iov[2];
    int fd;
    char box[1024];
    struct player *other;

    if (type == TEL_ANNOUNCE)
	strcpy(box, annfil);
    else
	mailbox(box, to);

    if (type != TEL_ANNOUNCE)
	if (!(np = getnatp(to)) || np->nat_stat < STAT_SANCT)
	    return -1;
#if !defined(_WIN32)
    if ((fd = open(box, O_WRONLY | O_APPEND, 0)) < 0) {
#else
    if ((fd = open(box, O_WRONLY | O_APPEND | O_BINARY, 0)) < 0) {
#endif
	logerror("telegram 'open' of %s failed", box);
	return -1;
    }

    memset(&tel, 0, sizeof(tel));
    tel.tel_from = from;
    (void)time(&tel.tel_date);
    len = strlen(message);
    CANT_HAPPEN(len && message[len - 1] != '\n');
    tel.tel_length = len;
    tel.tel_type = type;
    tel.tel_cont = !telegram_is_new(to, &tel);
    iov[0].iov_base = &tel;
    iov[0].iov_len = sizeof(tel);
    iov[1].iov_base = message;
    iov[1].iov_len = len;
    if (writev(fd, iov, 2) < (int)(iov[0].iov_len + iov[1].iov_len)) {
	logerror("telegram 'write' to %s failed", box);
	close(fd);
	return -1;
    }
    if (close(fd) < 0) {
	logerror("telegram 'write' to %s failed to close.", box);
	return -1;
    }

    if (type == TEL_ANNOUNCE) {
	for (to = 0; NULL != (np = getnatp(to)); to++) {
	    if (np->nat_stat < STAT_SANCT)
		continue;
	    if (!player->god && (getrejects(from, np) & REJ_ANNO))
		continue;
	    if (!np->nat_ann || !tel.tel_cont) {
		np->nat_ann++;
		putnat(np);
	    }
	}
    } else {
	if (!np->nat_tgms || !tel.tel_cont) {
	    np->nat_tgms++;
	    putnat(np);
	    if (np->nat_flags & NF_INFORM) {
		other = getplayer(to);
		if (other) {
		    if (np->nat_tgms == 1)
			pr_inform(other, "[new tele]\n");
		    else
			pr_inform(other, "[%d new teles]\n", np->nat_tgms);
		}
	    }
	}
    }

    return 0;
}
