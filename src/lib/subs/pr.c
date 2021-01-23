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
 *  pr.c: Output to players
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986, 1989
 *     Steve McClure, 1998-2000
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2005-2012
 */

/*
 * Player output is fully buffered.  It can block only if the
 * receiving player is the current player and his last command doesn't
 * have the C_MOD flag.  Output to another player must not block
 * because that player could be gone when the printing thread wakes
 * up, and the code isn't prepared for that.  Output within C_MOD
 * command never blocks, so that such commands can print freely
 * without yielding the processor.
 *
 * Each line of output starts with an identification character
 * encoding the output ID, followed by space.  Ids less than 10 are
 * encoded as decimal digits, and larger IDs as lower case letters,
 * starting with 'a'.  Symbolic names for IDs are defined in proto.h.
 */

#include <config.h>

#include <stdarg.h>
#include <stdlib.h>
#include "empio.h"
#include "journal.h"
#include "misc.h"
#include "nat.h"
#include "player.h"
#include "proto.h"
#include "prototypes.h"
#include "update.h"
#include "xy.h"

static void pr_player(struct player *pl, int id, char *buf);
static void upr_player(struct player *pl, int id, char *buf);
static void outid(struct player *pl, int n);
static void player_output_some(void);

/*
 * Print to current player similar to printf().
 * Use printf-style @format with the optional arguments.
 * Note: `to print' without further qualifications means sending
 * C_DATA text.
 */
void
pr(char *format, ...)
{
    char buf[4096];
    va_list ap;

    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    if (player->flags & PF_UTF8)
	/* normal text needs to be converted to user text */
	upr_player(player, C_DATA, buf);
    else
	/* normal text and user text are identical */
	pr_player(player, C_DATA, buf);
}

/*
 * Print UTF-8 text @buf to current player.
 */
void
uprnf(char *buf)
{
    char *p;

    if (!(player->flags & PF_UTF8)) {
	p = malloc(strlen(buf) + 1);
	copy_utf8_to_ascii_no_funny(p, buf);
	pr_player(player, C_DATA, p);
	free(p);
    } else
	pr_player(player, C_DATA, buf);
}

/*
 * Send some text to @p with ID @id, line-buffered.
 * Format text to send using printf-style @format and optional
 * arguments.  It is assumed to be already user text.  Plain ASCII and
 * text received from the same player are fine, for anything else the
 * caller has to deal with output filtering.
 * If a partial line is buffered, terminate it with a newline first.
 */
void
pr_id(struct player *p, int id, char *format, ...)
{
    char buf[4096];
    va_list ap;

    if (p->curid >= 0) {
	io_puts(p->iop, "\n");
	journal_output(p, p->curid, "\n");
	p->curid = -1;
    }
    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    pr_player(p, id, buf);
}

/*
 * Send C_FLASH text to @pl.
 * Format text to send using printf-style @format and optional
 * arguments.  It is assumed to be UTF-8.
 * Initiate an output queue flush, but do not wait for it to complete.
 */
void
pr_flash(struct player *pl, char *format, ...)
{
    char buf[4096];		/* UTF-8 */
    va_list ap;

    if (pl->state != PS_PLAYING)
	return;
    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    if (!(pl->flags & PF_UTF8))
	copy_utf8_to_ascii_no_funny(buf, buf);
    pr_player(pl, C_FLASH, buf);
    io_output(pl->iop, 0);
}

/*
 * Send C_INFORM text to @pl.
 * Format text to send using printf-style @format and optional
 * arguments.  It is assumed to be plain ASCII.
 * Initiate an output queue flush, but do not wait for it to complete.
 */
void
pr_inform(struct player *pl, char *format, ...)
{
    char buf[4096];
    va_list ap;

    if (pl->state != PS_PLAYING)
	return;
    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    pr_player(pl, C_INFORM, buf);
    io_output(pl->iop, 0);
}

/*
 * Send C_FLASH text to everyone.
 * Format text to send using printf-style @format and optional
 * arguments.  It is assumed to be plain ASCII.
 * Prefix text it with a header suitable for broadcast from deity.
 * Initiate an output queue flush, but do not wait for it to complete.
 */
void
pr_wall(char *format, ...)
{
    time_t now;
    struct tm *tm;
    char buf[4096];		/* UTF-8 */
    int n;
    struct player *p;
    va_list ap;

    time(&now);
    tm = localtime(&now);
    n = sprintf(buf, "BROADCAST from %s @ %02d:%02d: ",
		getnatp(0)->nat_cnam, tm->tm_hour, tm->tm_min);

    va_start(ap, format);
    (void)vsprintf(buf + n, format, ap);
    va_end(ap);
    for (p = player_next(NULL); p; p = player_next(p)) {
	if (p->state != PS_PLAYING)
	    continue;
	pr_player(p, C_FLASH, buf);
	io_output(p->iop, 0);
    }
}

/*
 * Send @id text @buf to @pl, line-buffered.
 * @buf is user text.
 * If a partial line with different ID is buffered, terminate it with
 * a newline first.
 */
static void
pr_player(struct player *pl, int id, char *buf)
{
    char *p;
    char *bp;
    int len;

    journal_output(pl, id, buf);

    bp = buf;
    while (*bp != '\0') {
	if (pl->curid != -1 && pl->curid != id) {
	    io_puts(pl->iop, "\n");
	    pl->curid = -1;
	}
	if (pl->curid == -1)
	    outid(pl, id);
	p = strchr(bp, '\n');
	if (p != NULL) {
	    len = (p - bp) + 1;
	    io_write(pl->iop, bp, len);
	    bp += len;
	    pl->curid = -1;
	} else {
	    len = io_puts(pl->iop, bp);
	    bp += len;
	}
    }

    if (player == pl)
	player_output_some();
}

/*
 * Send @id text @buf to @pl, line-buffered.
 * This function translates from normal text to user text.
 * If a partial line with different ID is buffered, terminate it with
 * a newline first.
 */
static void
upr_player(struct player *pl, int id, char *buf)
{
    char *bp;
    int standout = 0;
    char printbuf[2];
    char ch;

    journal_output(pl, id, buf);

    printbuf[0] = '\0';
    printbuf[1] = '\0';

    bp = buf;
    while ((ch = *bp++)) {
	if (pl->curid != -1 && pl->curid != id) {
	    io_puts(pl->iop, "\n");
	    pl->curid = -1;
	}
	if (pl->curid == -1)
	    outid(pl, id);

	if (ch & 0x80) {
	    if (standout == 0) {
		printbuf[0] = 0x0e;
		io_puts(pl->iop, printbuf);
		standout = 1;
	    }
	    ch &= 0x7f;
	} else {
	    if (standout == 1) {
		printbuf[0] = 0x0f;
		io_puts(pl->iop, printbuf);
		standout = 0;
	    }
	}
	if (ch == '\n') {
	    io_write(pl->iop, &ch, 1);
	    pl->curid = -1;
	} else {
	    printbuf[0] = ch;
	    io_puts(pl->iop, printbuf);
	}
    }

    if (player == pl)
	player_output_some();
}

/*
 * Send ID @n to @pl.
 * This runs always at the beginning of a line.
 */
static void
outid(struct player *pl, int n)
{
    char buf[3];

    if (CANT_HAPPEN(n > C_LAST))
	n = C_DATA;

    if (n >= 10)
	buf[0] = 'a' - 10 + n;
    else
	buf[0] = '0' + n;
    buf[1] = ' ';
    buf[2] = '\0';
    io_puts(pl->iop, buf);
    pl->curid = n;
}

static void
player_output_some(void)
{
    time_t deadline = player_io_deadline(player, 1);

    while (io_output_if_queue_long(player->iop, deadline) > 0)
	;
}

/*
 * Send redirection request @redir to the current player.
 * @redir is UTF-8, but non-ASCII characters can occur only if the
 * player sent them.  Therefore, it is also user text.
 */
void
prredir(char *redir)
{
    pr_id(player, *redir == '>' ? C_REDIR : C_PIPE, "%s\n", redir);
}

/*
 * Send script execute request @file to the current player.
 * @file is UTF-8, but non-ASCII characters can occur only if the
 * player sent them.  Therefore, it is also user text.
 */
void
prexec(char *file)
{
    pr_id(player, C_EXECUTE, "%s\n", file);
}

/*
 * Send a command prompt to the current player.
 */
void
prprompt(int min, int btu)
{
    pr_id(player, C_PROMPT, "%d %d\n", min, btu);
}

/*
 * Prompt for a line of non-command input.
 * Send C_FLUSH prompt @prompt to the current player.
 * Read a line of input into @buf[@size] and convert it to ASCII.
 * This may block for input, yielding the processor.  Flush buffered
 * output when blocking, to make sure player sees the prompt.
 * Return number of bytes in @buf[], not counting the terminating 0,
 * or -1 on error.
 */
int
prmptrd(char *prompt, char *buf, int size)
{
    int r;

    if (CANT_HAPPEN(!prompt))
	prompt = "? ";

    pr_id(player, C_FLUSH, "%s\n", prompt);
    if ((r = recvclient(buf, size)) < 0)
	return r;
    time(&player->curup);
    if (*buf == 0)
	return 1;
    if (player->flags & PF_UTF8)
	return copy_utf8_to_ascii_no_funny(buf, buf);
    return copy_ascii_no_funny(buf, buf);
}

/*
 * Prompt for a line of non-command, UTF-8 input.
 * Send C_FLUSH prompt @prompt to the current player.
 * Read a line of input into @buf[@size], replacing funny characters by
 * '?'.  The result is UTF-8.
 * This may block for input, yielding the processor.  Flush buffered
 * output when blocking, to make sure player sees the prompt.
 * Return number of bytes in @buf[], not counting the terminating 0,
 * or -1 on error.
 */
int
uprmptrd(char *prompt, char *buf, int size)
{
    int r;

    if (CANT_HAPPEN(!prompt))
	prompt = "? ";

    pr_id(player, C_FLUSH, "%s\n", prompt);
    if ((r = recvclient(buf, size)) < 0)
	return r;
    time(&player->curup);
    if (*buf == 0)
	return 1;
    if (player->flags & PF_UTF8)
	return copy_utf8_no_funny(buf, buf);
    return copy_ascii_no_funny(buf, buf);
}

/*
 * Print the current time in ctime() format.
 */
void
prdate(void)
{
    time_t now;

    (void)time(&now);
    pr("%s", ctime(&now));
}

/*
 * Print coordinates @x,@y.
 * @format must be a printf-style format string that converts exactly
 * two int values.
 */
void
prxy(char *format, coord x, coord y)
{
    struct natstr *np;

    np = getnatp(player->cnum);
    pr(format, xrel(np, x), yrel(np, y));
}

/*
 * Sound the current player's bell.
 */
void
pr_beep(void)
{
    struct natstr *np = getnatp(player->cnum);

    if (np->nat_flags & NF_BEEP)
	pr("\07");
}

/*
 * Print complete lines to country @cn similar to printf().
 * Use printf-style @format with the optional arguments.  @format must
 * end with '\n'.
 * If @cn is zero, don't print anything.
 * Else, if @cn is the current player and we're not in the update,
 * print just like pr().  Else print into a bulletin.
 * Because printing like pr() requires normal text, and bulletins
 * require user text, only plain ASCII is allowed.
 */
void
mpr(int cn, char *format, ...)
{
    char buf[4096];
    va_list ap;

    CANT_HAPPEN(!format[0] || format[strlen(format) - 1] != '\n');
    if (!cn)
	return;
    va_start(ap, format);
    (void)vsprintf(buf, format, ap);
    va_end(ap);
    if (update_running || cn != player->cnum)
	wu(0, cn, "%s", buf);
    else
	pr_player(player, C_DATA, buf);
}

/*
 * Copy @src without funny characters to @dst.
 * Drop control characters, except for '\t'.
 * Replace non-ASCII characters by '?'.
 * Return length of @dst.
 * @dst must have space.  If it overlaps @src, then @dst <= @src must
 * hold.
 */
size_t
copy_ascii_no_funny(char *dst, char *src)
{
    char *p;
    unsigned char ch;

    p = dst;
    while ((ch = *src++)) {
	if ((ch < 0x20 && ch != '\t' && ch != '\n') || ch == 0x7f)
	    ;			/* ignore funny control */
	else if (ch > 0x7f)
	    *p++ = '?';		/* replace non-ASCII */
	else
	    *p++ = ch;
    }
    *p = 0;

    return p - dst;
}

/*
 * Copy UTF-8 @src without funny characters to @dst.
 * Drop control characters, except for '\t'.
 * FIXME Replace malformed UTF-8 sequences by '?'.
 * Return byte length of @dst.
 * @dst must have space.  If it overlaps @src, then @dst <= @src must
 * hold.
 */
size_t
copy_utf8_no_funny(char *dst, char *src)
{
    char *p;
    unsigned char ch;

    p = dst;
    while ((ch = *src++)) {
	/* FIXME do the right thing for malformed and overlong sequences */
	if ((ch < 0x20 && ch != '\t' && ch != '\n') || ch == 0x7f)
	    ;			/* ignore funny control */
	else
	    *p++ = ch;
    }
    *p = 0;

    return p - dst;
}

/*
 * Copy UTF-8 @src without funny characters to ASCII @dst.
 * Drop control characters, except for '\t'.
 * Replace non-ASCII characters by '?'.
 * Return length of @dst.
 * @dst must have space.  If it overlaps @src, then @dst <= @src must
 * hold.
 */
size_t
copy_utf8_to_ascii_no_funny(char *dst, char *src)
{
    char *p;
    unsigned char ch;

    p = dst;
    while ((ch = *src++)) {
	/* FIXME do the right thing for malformed and overlong sequences */
	if ((ch < 0x20 && ch != '\t' && ch != '\n') || ch == 0x7f)
	    ;			/* ignore funny control */
	else if (ch > 0x7f) {
	    *p++ = '?';		/* replace non-ASCII */
	    while ((*src & 0xc0) == 0x80)
		src++;
	} else
	    *p++ = ch;
    }
    *p = 0;

    return p - dst;
}

/*
 * Return byte-index of the @n-th UTF-8 character in UTF-8 string @s.
 * If @s doesn't have that many characters, return its length instead.
 */
int
ufindpfx(char *s, int n)
{
    int i = 0;

    while (n && s[i]) {
	if ((s[i++] & 0xc0) == 0xc0)
	    while ((s[i] & 0xc0) == 0x80)
		i++;
	--n;
    }
    return i;
}
