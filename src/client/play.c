/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  play.c: Playing the game
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "linebuf.h"
#include "misc.h"
#include "proto.h"
#include "ringbuf.h"
#include "secure.h"

#define EOF_COOKIE "ctld\n"
#define INTR_COOKIE "\naborted\n"

int input_fd;
static volatile sig_atomic_t send_intr; /* need to send INTR_COOKIE */

/*
 * Receive and process server output from SOCK.
 * Return number of characters received on success, -1 on error.
 */
static int
recv_output(int sock)
{
    /*
     * Read a chunk of server output and feed its characters into a
     * simple state machine.
     * Initial state is SCANNING_ID.
     * In state SCANNING_ID, buffer the character.  If it's a space,
     * decode the id that has been buffered, and enter state BUFFERING
     * or COPYING depending on its value.
     * In state BUFFERING, buffer the character.  If it's newline,
     * pass id and buffered text to servercmd(), then enter state
     * SCANNING_ID.
     * In state COPYING, pass the character to outch().  If it's
     * newline, enter state SCANNING_ID.
     */
    static enum {
	SCANNING_ID, BUFFERING, COPYING
    } state = SCANNING_ID;
    static int id;
    static struct lbuf lbuf;
    char buf[4096];
    ssize_t n;
    int i, ch, len;
    char *line, *end;

    n = read(sock, buf, sizeof(buf));
    if (n < 0)
	return -1;

    for (i = 0; i < n; i++) {
	ch = buf[i];
	switch (state) {
	case SCANNING_ID:
	    if (ch == '\n') {
		/* FIXME gripe unexpected! */
		lbuf_init(&lbuf);
		break;
	    }
	    if (ch != ' ') {
		lbuf_putc(&lbuf, ch);
		break;
	    }
	    line = lbuf_line(&lbuf);
	    id = strtol(line, &end, 16);
	    if (end == line || *end) {
		/* FIXME gripe bad id */
		id = -1;
	    }
	    lbuf_init(&lbuf);

	    switch (id) {
	    case C_PROMPT:
	    case C_FLUSH:
	    case C_EXECUTE:
	    case C_EXIT:
	    case C_FLASH:
	    case C_INFORM:
	    case C_PIPE:
	    case C_REDIR:
		state = BUFFERING;
		break;
	    default:
		/* unknown or unexpected id, treat like C_DATA */
	    case C_DATA:
		state = COPYING;
		break;
	    }
	    break;

	case BUFFERING:
	    len = lbuf_putc(&lbuf, ch);
	    if (len) {
		line = lbuf_line(&lbuf);
		servercmd(id, line, len);
		lbuf_init(&lbuf);
		state = SCANNING_ID;
	    }
	    break;

	case COPYING:
	    outch(ch);
	    if (ch == '\n')
		state = SCANNING_ID;
	}
    }

    return n;
}

/*
 * Receive command input from FD into INBUF.
 * Return 1 on receipt of input, zero on EOF, -1 on error.
 */
static int
recv_input(int fd, struct ring *inbuf)
{
    static struct lbuf cmdbuf;
    int n, i, ch;
    char *line;
    int res = 1;

    n = ring_from_file(inbuf, fd);
    if (n < 0)
	return -1;
    if (n == 0) {
	/* EOF on input */
	if (lbuf_len(&cmdbuf)) {
	    /* incomplete line */
	    ring_putc(inbuf, '\n');
	    n++;
	}
	/*
	 * Can't put EOF cookie into INBUF here, it may not fit.
	 * Leave it to caller.
	 */
	res = 0;
    }

    /* copy input to AUXFP etc. */
    for (i = -n; i < 0; i++) {
	ch = ring_peek(inbuf, i);
	assert(ch != EOF);
	if (lbuf_putc(&cmdbuf, ch)) {
	    line = lbuf_line(&cmdbuf);
	    if (auxfp)
		fputs(line, auxfp);
	    save_input(line);
	    lbuf_init(&cmdbuf);
	}
    }

    return res;
}

static void
intr(int sig)
{
    send_intr = 1;
#ifdef _WIN32
    signal(SIGINT, intr);
#endif
}

/*
 * Play on SOCK.
 * The session must be in the playing phase.
 * Return 0 when the session ended, -1 on error.
 */
int
play(int sock)
{
    /*
     * Player input flows from INPUT_FD through recv_input() into ring
     * buffer INBUF, which drains into SOCK.  This must not block.
     * Server output flows from SOCK into recv_output().  Reading SOCK
     * must not block.
     */
    struct sigaction sa;
    struct ring inbuf;		/* input buffer, draining to SOCK */
    int eof_fd0;		/* read fd 0 hit EOF? */
    int send_eof;		/* need to send EOF_COOKIE */
    fd_set rdfd, wrfd;
    int n;

    sa.sa_flags = 0;
    sa.sa_handler = intr;
    sigaction(SIGINT, &sa, NULL);
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);

    ring_init(&inbuf);
    eof_fd0 = send_eof = 0;

    for (;;) {
	FD_ZERO(&rdfd);
	FD_ZERO(&wrfd);

	/*
	 * Want to read player input only when we don't need to send
	 * cookies, and we haven't hit EOF on fd 0, and INBUF can
	 * accept some.
	 */
	if (!send_intr && !send_eof && !eof_fd0 && ring_space(&inbuf))
	    FD_SET(input_fd, &rdfd);
	/* Want to send player input only when we have something */
	if (send_intr || send_eof || ring_len(&inbuf))
	    FD_SET(sock, &wrfd);
	/* Always want to read server output */
	FD_SET(sock, &rdfd);

	n = select(MAX(input_fd, sock) + 1, &rdfd, &wrfd, NULL, NULL);
	if (n < 0) {
	    if (errno != EINTR) {
		perror("select");
		return -1;
	    }
	}

	if (send_eof
	    && ring_putm(&inbuf, EOF_COOKIE, sizeof(EOF_COOKIE) - 1) >= 0)
	    send_eof = 0;
	if (send_intr
	    && ring_putm(&inbuf, INTR_COOKIE, sizeof(INTR_COOKIE) - 1) >= 0)
	    send_intr = 0;

	if (n < 0)
	    continue;

	/* read player input */
	if (FD_ISSET(input_fd, &rdfd)) {
	    n = recv_input(input_fd, &inbuf);
	    if (n < 0) {
		perror("read stdin"); /* FIXME stdin misleading, could be execing */
		return -1;
	    }
	    if (n == 0) {
		/* EOF on input */
		send_eof = 1;
		if (input_fd) {
		    /* execute done, switch back to fd 0 */
		    input_fd = 0;
		} else {
		    /* stop reading input, drain socket ring buffers */
		    eof_fd0 = 1;
		    sa.sa_handler = SIG_DFL;
		    sigaction(SIGINT, &sa, NULL);
		}
	    }
	}

	/* send it to the server */
	if (FD_ISSET(sock, &wrfd)) {
	    n = ring_to_file(&inbuf, sock);
	    if (n < 0) {
		perror("write socket");
		return -1;
	    }
	}

	/* read server output and print it */
	if (FD_ISSET(sock, &rdfd)) {
	    n = recv_output(sock);
	    if (n < 0) {
		perror("read socket");
		return -1;
	    }
	    if (n == 0)
		return 0;
	    if (input_fd < 0) {
		/* execute failed */
		input_fd = 0;
		send_eof = 1;
	    }
	}
    }
}
