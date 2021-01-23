/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  rea.c: Read telegrams
 *
 *  Known contributors to this file:
 *     Dave Pare
 *     Doug Hay, 1998
 *     Steve McClure, 1998-2000
 *     Ron Koenderink, 2005-2007
 *     Markus Armbruster, 2009-2016
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include "chance.h"
#include "commands.h"
#include "match.h"
#include "misc.h"
#include "optlist.h"
#include "tel.h"

static int print_sink(char *, size_t, void *);

int
c_read(void)
{
    static char *telnames[] = {
	/* must follow TEL_ defines in tel.h */
	"Telegram", "Announcement", "BULLETIN", "Production Report"
    };
    char *p;
    char *mbox;
    char mbox_buf[256];		/* Maximum path length */
    struct telstr tgm;
    FILE *telfp;
    int teles, need_hdr;
    char buf[1024];
    char *kind;
    int n, res;
    int num = player->cnum;
    struct natstr *np = getnatp(player->cnum);
    time_t now;
    time_t then;
    time_t delta;
    int may_delete = 1;

    now = time(NULL);

    if (*player->argp[0] == 'w') {
	kind = "announcement";
	if (player->argp[1] && isdigit(*player->argp[1])) {
	    delta = days(atoi(player->argp[1]));
	    then = now - delta;
	    may_delete = 0;
	} else
	    then = np->nat_annotim;
	mbox = annfil;
    } else {
	kind = "telegram";
	if (player->god && player->argp[1] &&
	    (mineq(player->argp[1], "yes") == ME_MISMATCH) &&
	    (mineq(player->argp[1], "no") == ME_MISMATCH)) {
	    if ((n = natarg(player->argp[1], NULL)) < 0)
		return RET_SYN;
	    num = n;
	    may_delete = 0;
	}
	mbox = mailbox(mbox_buf, num);
    }

    if (!(telfp = fopen(mbox, "rb"))) {
	logerror("telegram file %s", mbox);
	return RET_FAIL;
    }
    teles = 0;
    need_hdr = 1;
    for (;;) {
	res = tel_read_header(telfp, mbox, &tgm);
    more:
	if (res <= 0)
	    break;
	if (*kind == 'a') {
	    if ((!nat_accepts(player->cnum, tgm.tel_from, REJ_ANNO))
		|| tgm.tel_date < then) {
		res = tel_read_body(telfp, mbox, &tgm, NULL, NULL);
		if (res < 0)
		    break;
		continue;
	    }
	}
	if (!teles && *kind == 'a')
	    pr("\nAnnouncements since %s", ctime(&then));
	if (need_hdr || !tgm.tel_cont) {
	    pr("\n> ");
	    pr("%s ", telnames[tgm.tel_type]);
	    if ((tgm.tel_type == TEL_NORM) ||
		(tgm.tel_type == TEL_ANNOUNCE) ||
		(tgm.tel_type == TEL_BULLETIN))
		pr("from %s, (#%d)", cname(tgm.tel_from), tgm.tel_from);
	    pr("  dated %s", ctime(&tgm.tel_date));
	}
	teles++;
	need_hdr = 0;
	res = tel_read_body(telfp, mbox, &tgm, print_sink, NULL);
	if (res < 0)
	    break;
    }
    if (res < 0) {
	pr("\n> Mailbox corrupt, tell the deity.\n");
	may_delete = 0;
    }

    if (*kind == 'a')
	np->nat_ann = 0;
    else {
	np->nat_tgms = 0;
	if (np->nat_flags & NF_INFORM) {
	    pr_inform(player, "\n");
	}
    }
    putnat(np);

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
	    if (*kind == 'a') {
		np->nat_annotim = now;
		putnat(np);
	    } else {
		/* force stdio to re-read tel file */
		fflush(telfp);
		fseek(telfp, 0, SEEK_CUR);
		res = tel_read_header(telfp, mbox, &tgm);
		if (res != 0) {
		    pr("Wait a sec!  A new %s has arrived...\n", kind);
		    need_hdr = 1;
		    goto more;
		}
		/* Here, we just re-open the file for "w" only,
		   and that will wipe the file clean automatically */
		(void)fclose(telfp);
		telfp = fopen(mbox, "wb");
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
    return RET_OK;
}

/*
 * Print first telegram in file @fname.
 */
int
show_first_tel(char *fname)
{
    FILE *fp;
    int ret;
    struct telstr tgm;

    if ((fp = fopen(fname, "rb")) == NULL) {
	if (errno == ENOENT)
	    return 0;
	else {
	    logerror("Could not open %s.\n", fname);
	    return -1;
	}
    }

    ret = tel_read_header(fp, fname, &tgm);
    if (ret < 0)
	goto out;
    ret = tel_read_body(fp, fname, &tgm, print_sink, NULL);

out:
    fclose(fp);
    return ret;
}

static int
print_sink(char *chunk, size_t sz, void *arg)
{
    uprnf(chunk);
    return 0;
}
