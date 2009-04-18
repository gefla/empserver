/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2007-2009
 *     Ron Koenderink, 2007-2009
 */

#include <config.h>

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/select.h>
#include <unistd.h>
#else
#include <process.h>
#include <io.h>
#include "w32types.h"
#endif
#include "linebuf.h"
#include "misc.h"
#include "proto.h"
#include "ringbuf.h"
#include "secure.h"

#ifdef _WIN32
static CRITICAL_SECTION signal_critical_section;
static LPCRITICAL_SECTION signal_critical_section_ptr = NULL;

static unsigned char bounce_buf[RING_SIZE];
/*
 * Set bounce_empty to indicate bounce_buf is available for the stdin thread
 * to use.
 */
static HANDLE bounce_empty;
/*
 * Set bounce_full to indicate bounce_buf is contains data from the
 * stdin thread and is available for recv_input
 */
static HANDLE bounce_full;
/* Ctrl-C (SIGINT) was detected, generate EINTR for the w32_select() */
static HANDLE ctrl_c_event;
static int bounce_status, bounce_error;

struct sigaction {
    int sa_flags;
    void (*sa_handler)(int sig);
};

#define SIGPIPE -1
static void (*ctrl_handler)(int sig) = { SIG_DFL };

/*
 * Ctrl-C handler for emulating the SIGINT in WIN32
 */
static BOOL WINAPI
w32_signal_handler(DWORD ctrl_type)
{
    if (ctrl_type == CTRL_C_EVENT) {
	EnterCriticalSection(signal_critical_section_ptr);
	if (ctrl_handler != SIG_DFL) {
	    ctrl_handler(SIGINT);
	    LeaveCriticalSection(signal_critical_section_ptr);
	    SetEvent(ctrl_c_event);
	    return TRUE;
	} else
	    LeaveCriticalSection(signal_critical_section_ptr);
    }
    return FALSE;
}

/*
 * WIN32 equivalent for sigaction supports the following:
 * set handler for SIGINT using WIN32 Ctrl-C handler
 * reset handler SIGINT to SIG_DFL
 * ignore SIGPIPE
 */
static int
sigaction(int signal, struct sigaction *action, struct sigaction *oaction)
{
    assert(!oaction);
    assert(action);

    if (signal == SIGPIPE)
	assert(action->sa_handler == SIG_IGN);
    else {
	assert(signal == SIGINT && action->sa_handler != SIG_IGN);
	if (ctrl_handler == action->sa_handler)
	    return 0;
	if (signal_critical_section_ptr == NULL) {
	    signal_critical_section_ptr = &signal_critical_section;
	    InitializeCriticalSection(signal_critical_section_ptr);
	}
	EnterCriticalSection(signal_critical_section_ptr);
	if (!SetConsoleCtrlHandler(w32_signal_handler,
				   action->sa_handler != SIG_DFL)) {
	    errno = GetLastError();
	    LeaveCriticalSection(signal_critical_section_ptr);
	    return -1;
	}
	ctrl_handler = action->sa_handler;
	LeaveCriticalSection(signal_critical_section_ptr);
    }
    return 0;
}

/*
 * Read the stdin in WIN32 environment
 * WIN32 does not support select type function on console input
 * so the client uses a separate thread to read input
 */
static void
stdin_read_thread(void *dummy)
{
    for (;;) {
	switch (WaitForSingleObject(bounce_empty, INFINITE)) {
	case WAIT_FAILED:
	    bounce_status = -1;
	    bounce_error = GetLastError();
	    break;
	case WAIT_OBJECT_0:
	    bounce_status = _read(0, bounce_buf, sizeof(bounce_buf));
	    bounce_error = errno;
	    break;
	case WAIT_ABANDONED:
	    return;
	default:
	    assert(0);
	}
	SetEvent(bounce_full);
    }
}

/*
 * Initialize and start the stdin reading thread for WIN32
 */
static void
sysdep_stdin_init(void)
{
    bounce_empty = CreateEvent(NULL, FALSE, TRUE, NULL);
    bounce_full = CreateEvent(NULL, TRUE, FALSE, NULL);
    ctrl_c_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    _beginthread(stdin_read_thread, 0, NULL);
}

/*
 * This function uses to WaitForMultipleObjects to wait for both
 * stdin and socket reading or writing.
 * Stdin is treated special in WIN32.  Waiting for stdin is done
 * via a bounce_full event which is set in the stdin thread.
 * Execute command file reading is done via handle.
 * WaitForMultipleObjects will only respond with one object,
 * so an additonal select is also done to determine
 * which individual events are active.
 */
static int
w32_select(int nfds, fd_set *rdfd, fd_set *wrfd, fd_set *errfd, struct timeval* time)
{
    HANDLE handles[3];
    SOCKET sock;
    int inp, sockfd, result, s_result, num_handles;
    struct timeval tv_time = {0, 0};
    fd_set rdsock, wrsock;

    switch (rdfd->fd_count) {
    case 1:
	inp = -1;
	sockfd = rdfd->fd_array[0];
	break;
    case 2:
	inp = rdfd->fd_array[0];
	sockfd = rdfd->fd_array[1];
	break;
    default:
	assert(0);
    }
    sock = W32_FD_TO_SOCKET(sockfd);

    assert(wrfd->fd_count == 0
	   || (wrfd->fd_count == 1 && wrfd->fd_array[0] == (SOCKET)sockfd));
    assert(inp < 0 || inp == input_fd);

    num_handles = 0;
    handles[num_handles++] = ctrl_c_event;
    if (inp >= 0)
	handles[num_handles++]
	    = inp ? (HANDLE)_get_osfhandle(inp) : bounce_full;
    /* always wait on the socket */
    handles[num_handles++] = WSACreateEvent();

    if (wrfd->fd_count > 0)
	WSAEventSelect(sock, handles[num_handles - 1],
		       FD_READ | FD_WRITE | FD_CLOSE);
    else
	WSAEventSelect(sock, handles[num_handles - 1],
		       FD_READ | FD_CLOSE);

    result = WaitForMultipleObjects(num_handles, handles, 0, INFINITE);
    if (result < 0) {
	errno = GetLastError();
	WSACloseEvent(handles[num_handles - 1]);
	return -1;
    }
    WSACloseEvent(handles[num_handles - 1]);

    if (result == WAIT_OBJECT_0) {
	errno = EINTR;
	return -1;
    }

    FD_ZERO(&rdsock);
    FD_ZERO(&wrsock);
    FD_SET(sock, &rdsock);
    if (wrfd->fd_count)
	FD_SET(sock, &wrsock);
    s_result = select(sock + 1, &rdsock, &wrsock, NULL, &tv_time);

    if (s_result < 0) {
	w32_set_winsock_errno();
	return s_result;
    }

    if (!FD_ISSET(sock, &rdsock))
	FD_CLR((SOCKET)sockfd, rdfd);
    if (!FD_ISSET(sock, &wrsock))
	FD_CLR((SOCKET)sockfd, wrfd);
    if (inp >= 0 && result == WAIT_OBJECT_0 + 1)
	s_result++;
    else
	FD_CLR((SOCKET)inp, rdfd);

    return s_result;
}

/*
 * Read input from the user either stdin or from file.
 * For stdin, read from bounce_buf which filled by the stdin thread
 * otherwise use the regular ring_from_file.
 */
static int
w32_ring_from_file_or_bounce_buf(struct ring *r, int fd)
{
    int i, res;

    if (fd)
        return ring_from_file(r, fd);

    if (bounce_status < 0) {
        errno = bounce_error;
        res = bounce_status;
    } else {
        for (i = 0; i < bounce_status; i++) {
            if (ring_putc(r, bounce_buf[i]) == EOF) {
                /* more work to do, hold on to bounce_buf */
                memmove(bounce_buf, bounce_buf + i, bounce_status - i);
                bounce_status -= i;
                return i;
            }
        }
        res = i;
    }

    ResetEvent(bounce_full);
    SetEvent(bounce_empty);
    return res;
}
#define ring_from_file w32_ring_from_file_or_bounce_buf
#define read(sock, buffer, buf_size) \
	w32_recv((sock), (buffer), (buf_size), 0)
#define select(nfds, rd, wr, error, time) \
	w32_select((nfds), (rd), (wr), (error), (time))
#define sigemptyset(mask) ((void)0)
#else
#define sysdep_stdin_init() ((void)0)
#endif

#define EOF_COOKIE "ctld\n"
#define INTR_COOKIE "\naborted\n"

int input_fd;
int send_eof;				/* need to send EOF_COOKIE */
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
    char *line;

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
	    lbuf_putc(&lbuf, ch);
	    if (ch != ' ')
		break;
	    line = lbuf_line(&lbuf);
	    id = parseid(line);
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
	if (ch != '\r' && lbuf_putc(&cmdbuf, ch)) {
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
    fd_set rdfd, wrfd;
    int n;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = intr;
    sigaction(SIGINT, &sa, NULL);
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);

    ring_init(&inbuf);
    eof_fd0 = send_eof = send_intr = 0;
    input_fd = 0;
    sysdep_stdin_init();

    for (;;) {
	FD_ZERO(&rdfd);
	FD_ZERO(&wrfd);

	/*
	 * Want to read player input only when we don't need to send
	 * cookies, and INPUT_FD is still open, and INBUF can accept
	 * some.
	 */
	if (!send_intr && !send_eof && input_fd >= 0 && ring_space(&inbuf))
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
	    send_eof--;
	if (send_intr
	    && ring_putm(&inbuf, INTR_COOKIE, sizeof(INTR_COOKIE) - 1) >= 0) {
	    send_intr = 0;
	    if (input_fd) {
		/* execute aborted, switch back to fd 0 */
		close(input_fd);
		input_fd = eof_fd0 ? -1 : 0;
	    }
	}

	if (n < 0)
	    continue;

	/* read player input */
	if (input_fd >= 0 && FD_ISSET(input_fd, &rdfd)) {
	    n = recv_input(input_fd, &inbuf);
	    if (n < 0) {
		perror("read stdin"); /* FIXME stdin misleading, could be execing */
		n = 0;
	    }
	    if (n == 0) {
		/* EOF on input */
		send_eof++;
		if (input_fd) {
		    /* execute done, switch back to fd 0 */
		    close(input_fd);
		    input_fd = eof_fd0 ? -1 : 0;
		} else {
		    /* stop reading input, drain socket ring buffers */
		    eof_fd0 = 1;
		    input_fd = -1;
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
	}
    }
}
