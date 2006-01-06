/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  rea.c: Read telegrams
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Doug Hay, 1998
 *     Steve McClure, 1998-2000
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "nat.h"
#include "file.h"
#include "tel.h"
#include "commands.h"
#include "optlist.h"
#include "match.h"

#include <stdio.h>
#include <fcntl.h>
#if !defined(_WIN32)
#include <sys/file.h>
#endif

int
rea(void)
{
    static s_char *telnames[] = {
	/* must follow TEL_ defines in tel.h */
	"Telegram", "Announcement", "BULLETIN", "Production Report"
    };
    register s_char *p;
    register s_char *mbox;
    s_char mbox_buf[256];	/* Maximum path length */
    struct telstr tgm;
    FILE *telfp;
    int teles;
    int size;
    unsigned int nbytes;
    char buf[1024];
    char msgbuf[4096];		/* UTF-8 */
    int lasttype;
    int lastcnum;
    time_t lastdate;
    int header;
    int filelen;
    s_char kind[80];
    int n;
    int num = player->cnum;
    struct natstr *np = getnatp(player->cnum);
    time_t now;
    time_t then;
    time_t delta;
    int first = 1;
    int readit;
    int may_delete = 1; /* may messages be deleted? */

    memset(kind, 0, sizeof(kind));
    now = time(NULL);

    if (*player->argp[0] == 'w') {
	strcpy(kind, "announcement");
	if (player->argp[1] && isdigit(*player->argp[1])) {
	    delta = days(atoi(player->argp[1]));
	    then = now - delta;
	    may_delete = 0;
	} else
	    then = np->nat_annotim;
	mbox = annfil;
    } else {
	strcpy(kind, "telegram");
	if (player->god && player->argp[1] &&
	    (mineq(player->argp[1], "yes") == ME_MISMATCH) &&
	    (mineq(player->argp[1], "no") == ME_MISMATCH)) {
	    if ((n = natarg(player->argp[1], NULL)) < 0)
		return RET_SYN;
	    num = n;
	    may_delete = 0;
	}
	mbox = mailbox(mbox_buf, num);
	clear_telegram_is_new(player->cnum);
    }

    if ((telfp = fopen(mbox, "rb+")) == 0) {
	logerror("telegram file %s", mbox);
	return RET_FAIL;
    }
    teles = 0;
    fseek(telfp, 0L, SEEK_SET);
    size = fsize(fileno(telfp));
  more:
    lastdate = 0;
    lastcnum = -1;
    lasttype = -1;
    while (fread(&tgm, sizeof(tgm), 1, telfp) == 1) {
	readit = 1;
	if (tgm.tel_length < 0) {
	    logerror("bad telegram file header in %s", mbox);
	    break;
	}
	if (tgm.tel_type < 0 || tgm.tel_type > TEL_LAST) {
	    pr("Bad telegram header.  Skipping telegram...\n");
	    readit = 0;
	    goto skip;
	}
	if (*kind == 'a') {
	    if (!player->god && (getrejects(tgm.tel_from, np) & REJ_ANNO)) {
		readit = 0;
		goto skip;
	    }
	    if (tgm.tel_date < then) {
		readit = 0;
		goto skip;
	    }
	}
	if (first && *kind == 'a') {
	    pr("\nAnnouncements since %s", ctime(&then));
	    first = 0;
	}
	header = 0;
	if (tgm.tel_type != lasttype || tgm.tel_from != lastcnum)
	    header++;
	if (abs((int)(tgm.tel_date - (long)lastdate)) > TEL_SECONDS)
	    header++;
	if (header) {
	    pr("\n> ");
	    lastcnum = tgm.tel_from;
	    lasttype = tgm.tel_type;
	    pr("%s ", telnames[(int)tgm.tel_type]);
	    if ((tgm.tel_type == TEL_NORM) ||
		(tgm.tel_type == TEL_ANNOUNCE) ||
		(tgm.tel_type == TEL_BULLETIN))
		pr("from %s, (#%d)", cname(tgm.tel_from), tgm.tel_from);
	    pr("  dated %s", ctime(&tgm.tel_date));
	    lastdate = tgm.tel_date;
	}
	teles++;
      skip:
	while (tgm.tel_length > 0) {
	    nbytes = tgm.tel_length;
	    if (nbytes > sizeof(msgbuf) - 1)
		nbytes = sizeof(msgbuf) - 1;
	    (void)fread(msgbuf, sizeof(s_char), nbytes, telfp);
	    msgbuf[nbytes] = 0;
	    if (readit)
		uprnf(msgbuf);
	    tgm.tel_length -= nbytes;
	}
    }
    p = NULL;
    if (teles > 0 && player->cnum == num && may_delete) {
	pr("\n");
	if (teles == 1) {
	    if (chance(0.25))
		p = "Forget this one? ";
	    else
		p = "Shall I burn it? ";
	} else {
	    if (chance(0.25))
		p = "Into the shredder, boss? ";
	    else
		p = "Can I throw away these old love letters? ";
	}
	p = getstarg(player->argp[1], p, buf);
	if (p && *p == 'y') {
	    if ((filelen = fsize(fileno(telfp))) > size) {
		pr("Wait a sec!  A new %s has arrived...\n", kind);
		/* force stdio to re-read tel file */
		(void)fflush(telfp);
		(void)fseek(telfp, (long)size, SEEK_SET);
		size = filelen;
		now = time(NULL);
		goto more;
	    }
	    if (*kind == 'a') {
		np->nat_annotim = now;
		putnat(np);
	    } else {
		/* Here, we just re-open the file for "w" only,
		   and that will wipe the file clean automatically */
		(void)fclose(telfp);
		telfp = fopen((char *)mbox, "wb");
	    }
	}
    }
    if (teles <= 0) {
	if (player->cnum == num)
	    pr("No %ss for you at the moment...\n", kind);
	else
	    pr("No %ss for %s at the moment...\n", kind, cname(num));
    }
    (void)fclose(telfp);
    if (np->nat_flags & NF_INFORM) {
	pr_inform(player, "\n");
	np->nat_tgms = 0;
	putnat(np);
    }
    return RET_OK;
}
