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
 *  pr.c: Use to do output to a player
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986, 1989 
 *     Steve McClure, 1998-2000
 */
/*
 * The pr routine historically arranged for nonbuffered i/o
 * because stdio didn't used to automatically flush stdout before
 * it read something from stdin.  Now pr() prepends an "output id"
 * in front of each line of text, informing the user interface
 * what sort of item it is seeing; prompt, noecho prompt,
 * more input data, etc.
 */

#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdarg.h>
#include "proto.h"
#include "misc.h"
#include "player.h"
#include "nat.h"
#include "empio.h"
#include "file.h"
#include "com.h"
#include "news.h"
#include "tel.h"
#include "server.h"
#include "prototypes.h"

static void outid(struct player *pl, int n);

/*VARARGS*/
void
pr(s_char *format, ...)
{
    s_char buf[4096];
    va_list ap;

    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    pr_player(player, C_DATA, buf);
}

void
prnf(s_char *buf)
{
    pr_player(player, C_DATA, buf);
}

/*VARARGS*/
void
pr_id(struct player *p, int id, s_char *format, ...)
{
    s_char buf[4096];
    va_list ap;

    if (p->curid >= 0) {
	io_puts(p->iop, "\n");
	p->curid = -1;
    }
    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    pr_player(p, id, buf);
}

void
pr_flash(struct player *pl, s_char *format, ...)
{
    s_char buf[4096];
    va_list ap;

    if (pl->state != PS_PLAYING)
	return;
    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    pr_player(pl, C_FLASH, buf);
    io_output(pl->iop, IO_NOWAIT);
}

void
pr_inform(struct player *pl, s_char *format, ...)
{
    s_char buf[4096];
    va_list ap;

    if (pl->state != PS_PLAYING)
	return;
    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    pr_player(pl, C_INFORM, buf);
    io_output(pl->iop, IO_NOWAIT);
}

void
pr_wall(s_char *format, ...)
{
    s_char buf[4096];
    struct player *p;
    va_list ap;

    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    for (p = player_next(0); p; p = player_next(p)) {
	if (p->state != PS_PLAYING)
	    continue;
	pr_player(p, C_FLASH, buf);
	io_output(p->iop, IO_NOWAIT);
    }
}

void
pr_player(struct player *pl, int id, s_char *buf)
{
    register s_char *p;
    register s_char *bp;
    register int len;

    bp = buf;
    while (*bp != '\0') {
	if (pl->curid != -1 && pl->curid != id) {
	    io_puts(pl->iop, "\n");
	    pl->curid = -1;
	}
	if (pl->curid == -1) {
	    outid(pl, id);
	}
	p = strchr(bp, '\n');
	if (p != 0) {
	    len = (p - bp) + 1;
	    if (pl->command && (pl->command->c_flags & C_MOD))
		io_write(pl->iop, bp, len, IO_NOWAIT);
	    else
		io_write(pl->iop, bp, len, IO_WAIT);
	    bp += len;
	    pl->curid = -1;
	} else {
	    len = io_puts(pl->iop, bp);
	    bp += len;
	}
    }
}

/*
 * highlighted characters have hex 80 or'ed in
 * with them to designate their highlightedness
 */
void
pr_hilite(s_char *buf)
{
    register s_char *bp;
    register s_char c;
    s_char *p;

    p = (s_char *)malloc(strlen(buf) + 1);
    strcpy(p, buf);
    for (bp = p; 0 != (c = *bp); bp++)
	if (isprint(c))
	    *bp |= 0x80;
    pr(p);
    free(p);
}

/*
 * output hex code + space
 */
static void
outid(struct player *pl, int n)
{
    s_char c;
    s_char buf[3];

    if (n > C_LAST) {
	logerror("outid: %d not valid code\n", n);
	return;
    }
    if (n >= 10)
	c = 'a' - 10 + n;
    else
	c = '0' + n;
    buf[0] = c;
    buf[1] = ' ';
    buf[2] = '\0';
    io_puts(pl->iop, buf);
    pl->curid = n;
}

void
prredir(s_char *redir)
{
    pr_id(player, *redir == '>' ? C_REDIR : C_PIPE, "%s\n", redir);
}

void
prexec(s_char *file)
{
    pr_id(player, C_EXECUTE, "%s\n", file);
}

void
prprompt(int min, int btu)
{
    pr_id(player, C_PROMPT, "%d %d\n", min, btu);
}

void
showvers(int vers)
{
    pr_id(player, C_INIT, "%d\n", vers);
}

int
prmptrd(s_char *prompt, s_char *str, int size)
{
    int r;

    pr_id(player, C_FLUSH, "%s\n", prompt);
    if ((r = recvclient(str, size)) < 0)
	return r;
    time(&player->curup);
    if (*str == 0)
	return 1;
    return strlen(str);
}

void
prdate(void)
{
    time_t now;

    (void)time(&now);
    pr(ctime(&now));
}

/*
 * print x,y formatting as country
 */
void
prxy(s_char *format, coord x, coord y, natid country)
{
    s_char buf[255];
    struct natstr *np;

    np = getnatp(country);
    sprintf(buf, format, xrel(np, x), yrel(np, y));
    pr(buf);
}

/*VARARGS*/
void
PR(int cn, s_char *format, ...)
{
    /* XXX should really do this on a per-nation basis */
    static s_char longline[MAXNOC][512];
    int newline;
    va_list ap;
    s_char buf[1024];

    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    newline = strrchr(buf, '\n') ? 1 : 0;
    strcat(longline[cn], buf);
    if (newline) {
	if (update_pending || (cn && cn != player->cnum))
	    typed_wu(0, cn, longline[cn], TEL_BULLETIN);
	else
	    pr_player(player, C_DATA, longline[cn]);
	longline[cn][0] = '\0';
    }
}

void
PRdate(natid cn)
{
    time_t now;

    (void)time(&now);
    PR(cn, ctime(&now));
}

void
pr_beep(void)
{
    struct natstr *np = getnatp(player->cnum);

    if (np->nat_flags & NF_BEEP)
	pr("\07");
}

void
mpr(int cn, s_char *format, ...)
{
    s_char buf[4096];
    va_list ap;

    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    if (cn) {
	if (update_pending || cn != player->cnum)
	    typed_wu(0, cn, buf, TEL_BULLETIN);
	else
	    pr_player(player, C_DATA, buf);
    }
}
