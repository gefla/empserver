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
 * encoding the output id, followed by space.  Ids less than 10 are
 * encoded as decimal digits, and larger ids as lower case letters,
 * starting with 'a'.  Symbolic names for ids are defined in proto.h.
 */

#include <config.h>

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
#include "tel.h"
#include "server.h"
#include "prototypes.h"

static void pr_player(struct player *pl, int id, char *buf);
static void upr_player(struct player *pl, int id, char *buf);
static void outid(struct player *pl, int n);

/*
 * Print to current player similar to printf().
 * Use printf-style FORMAT with the optional arguments.
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
 * Print UTF-8 text BUF to current player.
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
 * Send some text to P with id ID, line-buffered.
 * Format text to send using printf-style FORMAT and optional
 * arguments.  It is assumed to be already user text.  Plain ASCII and
 * text received from the same player are fine, for anything else the
 * caller has to deal with output filtering.
 * If a partial line with different id is buffered, terminate it with
 * a newline first.
 */
void
pr_id(struct player *p, int id, char *format, ...)
{
    char buf[4096];
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

/*
 * Send C_FLASH text to PL.
 * Format text to send using printf-style FORMAT and optional
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
    io_output(pl->iop, IO_NOWAIT);
}

/*
 * Send C_INFORM text to PL.
 * Format text to send using printf-style FORMAT and optional
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
    io_output(pl->iop, IO_NOWAIT);
}

/*
 * Send C_FLASH text to everyone.
 * Format text to send using printf-style FORMAT and optional
 * arguments.  It is assumed to be plain ASCII.
 * Initiate an output queue flush, but do not wait for it to complete.
 */
void
pr_wall(char *format, ...)
{
    char buf[4096];		/* UTF-8 */
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

/*
 * Send ID text BUF to PL, line-buffered.
 * BUF is user text.
 * If a partial line with different id is buffered, terminate it with
 * a newline first.
 */
static void
pr_player(struct player *pl, int id, char *buf)
{
    char *p;
    char *bp;
    int len;

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
	    if ((pl->command && (pl->command->c_flags & C_MOD)) ||
		(player != pl))
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
 * Send ID text BUF to PL, line-buffered.
 * This function translates from normal text to user text.
 * If a partial line with different id is buffered, terminate it with
 * a newline first.
 */
static void
upr_player(struct player *pl, int id, char *buf)
{
    char *bp;
    int standout = 0;
    char printbuf[2];
    char ch;

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
	    if ((pl->command && (pl->command->c_flags & C_MOD)) ||
		(player != pl))
		io_write(pl->iop, &ch, 1, IO_NOWAIT);
	    else
		io_write(pl->iop, &ch, 1, IO_WAIT);
	    pl->curid = -1;
	} else {
	    printbuf[0] = ch;
	    io_puts(pl->iop, printbuf);
	}
    }
}

/*
 * Send id N to PL.
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

/*
 * Send redirection request REDIR to the current player.
 * REDIR is UTF-8, but non-ASCII characters can occur only if the
 * player sent them.  Therefore, it is also user text.
 */
void
prredir(char *redir)
{
    pr_id(player, *redir == '>' ? C_REDIR : C_PIPE, "%s\n", redir);
}

/*
 * Send script execute request FILE to the current player.
 * REDIR is UTF-8, but non-ASCII characters can occur only if the
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
 * Send C_FLUSH prompt PROMPT to the current player.
 * Read a line of input into BUF[SIZE] and convert it to ASCII.
 * This may block for input, yielding the processor.  Flush buffered
 * output when blocking, to make sure player sees the prompt.
 * Return number of bytes in BUF[], not counting the terminating 0,
 * or -1 on error.
 */
int
prmptrd(char *prompt, char *buf, int size)
{
    int r;

    /*
     * Each prompt must consume one line of input.  recvclient()
     * doesn't do that while player->aborted.
     */
    CANT_HAPPEN(player->aborted);

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
 * Send C_FLUSH prompt PROMPT to the current player.
 * Read a line of input into BUF[SIZE], replacing funny characters by
 * '?'.  The result is UTF-8.
 * This may block for input, yielding the processor.  Flush buffered
 * output when blocking, to make sure player sees the prompt.
 * Return number of bytes in BUF[], not counting the terminating 0,
 * or -1 on error.
 */
int
uprmptrd(char *prompt, char *buf, int size)
{
    int r;

    /* See prmptrd() */
    CANT_HAPPEN(player->aborted);

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
    pr(ctime(&now));
}

/*
 * Print coordinates X, Y for COUNTRY.
 * FORMAT must be a printf-style format string that converts exactly
 * two int values.
 */
void
prxy(char *format, coord x, coord y, natid country)
{
    char buf[255];
    struct natstr *np;

    np = getnatp(country);
    sprintf(buf, format, xrel(np, x), yrel(np, y));
    pr(buf);
}

/*
 * Print to country CN similar to printf().
 * Use printf-style FORMAT with the optional arguments.
 * Output is buffered until a newline arrives.
 * If CN is the current player and we're not in the update, print just
 * like pr().  Else print into a bulletin.
 * Because printing like pr() requires normal text, and bulletins
 * require user text, only plain ASCII is allowed.
 */
void
PR(int cn, char *format, ...)
{
    /* XXX should really do this on a per-nation basis */
    static char longline[MAXNOC][512];
    int newline;
    va_list ap;
    char buf[1024];

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

/*
 * Print the current time in ctime() format to country CN.
 * If CN is the current player and we're not in the update, print just
 * like prdate().  Else print into a bulletin.
 */
void
PRdate(natid cn)
{
    time_t now;

    (void)time(&now);
    PR(cn, ctime(&now));
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
 * Print to country CN similar to printf().
 * Use printf-style FORMAT with the optional arguments.
 * If CN is the current player and we're not in the update, print just
 * like pr().  Else print into a bulletin.
 * Because printing like pr() requires normal text, and bulletins
 * require user text, only plain ASCII is allowed.
 */
void
mpr(int cn, char *format, ...)
{
    char buf[4096];
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

/*
 * Copy SRC without funny characters to DST.
 * Drop control characters, except for '\t'.
 * Replace non-ASCII characters by '?'.
 * Return length of DST.
 * DST must have space.  If it overlaps SRC, then DST <= SRC must
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
	    *p++ = '?';	/* replace non-ASCII */
	else
	    *p++ = ch;
    }
    *p = 0;

    return p - dst;
}

/*
 * Copy UTF-8 SRC without funny characters to DST.
 * Drop control characters, except for '\t'.
 * FIXME Replace malformed UTF-8 sequences by '?'.
 * Return byte length of DST.
 * DST must have space.  If it overlaps SRC, then DST <= SRC must
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
 * Copy UTF-8 SRC without funny characters to ASCII DST.
 * Drop control characters, except for '\t'.
 * Replace non-ASCII characters by '?'.
 * Return length of DST.
 * DST must have space.  If it overlaps SRC, then DST <= SRC must
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
	    while ((*src++ & 0xc0) == 0x80) ;
	} else
	    *p++ = ch;
    }
    *p = 0;

    return p - dst;
}

/*
 * Return byte-index of the N-th UTF-8 character in UTF-8 string S.
 * If S doesn't have that many characters, return its length instead.
 */
int
ufindpfx(char *s, int n)
{
    int i = 0;

    while (n && s[i])
    {
	if ((s[i++] & 0xc0) == 0xc0)
            while ((s[i] & 0xc0) == 0x80)
		i++;
        --n;
    }
    return i;
}
