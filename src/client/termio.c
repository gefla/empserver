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
 *  termio.c: Various io functions
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#if defined(aix) || defined(hpux) || defined(sgi)
#include <sys/termio.h>
#elif defined(linux)
#include <unistd.h>
#include <termio.h>
#else
#ifndef _WIN32
#include <unistd.h>
#include <sgtty.h>
#else
#include <winsock.h>
#include <io.h>
#endif /* _WIN32 */
#endif
#include "misc.h"
#include "tags.h"

extern struct tagstruct *taglist;
extern s_char buf[4096];
extern s_char exec[];

#ifdef _WIN32
extern HANDLE hStdIn;
#endif

int sendeof(int sock);

int
termio(int fd, int sock, FILE *auxfi)
{
    s_char out[4096];
    int i, n;
    s_char *ptr;
    s_char *p, *q, *r, *s, *t;
    int nbytes;
    int numarg, prespace, exec_com, inarg, quoted, tagging;
    struct tagstruct *tag;
#ifdef _WIN32
    char c;
    INPUT_RECORD InpBuffer[2];
    int err;
#endif

    i = strlen(buf);
    p = buf + i;
#ifndef _WIN32
    n = read(fd, p, sizeof(buf) - i);
#else
/* The keyboard is sometimes generating both keydown and keyup
 * events for the same key.  Thus, we only want to grab keydown
 * events. */
    if (fd == -1) {
	err = PeekConsoleInput(hStdIn, InpBuffer, 1, &n);
	if (InpBuffer[0].EventType != KEY_EVENT) {
	    ReadConsoleInput(hStdIn, InpBuffer, 1, &n);
	    return 1;
	}
	if (!InpBuffer[0].Event.KeyEvent.bKeyDown) {
	    ReadConsoleInput(hStdIn, InpBuffer, 1, &n);
	    return 1;
	}
	c = InpBuffer[0].Event.KeyEvent.uChar.AsciiChar;

	if (c == 13)
	    c = 10;
	n = 1;
	p[0] = c;
	p[1] = '\0';
	if (c != 10)
	    ReadConsole(hStdIn, &p[0], sizeof(buf) - i, &n, NULL);
	else
	    putchar(c);
/* Strip off the CRLF to just LF */
	if (n > 1) {
	    if (p[n - 2] == 13 && p[n - 1] == 10) {
		p[n - 2] = 10;
		p[n - 1] = 0;
		n--;
	    }
	}
	FlushConsoleInputBuffer(hStdIn);
    } else {
	n = read(fd, p, sizeof(buf) - i);
    }
#endif
    if (n == 0) {
	sendeof(sock);
	return 0;
    }
    if (n < 0) {
	perror("read standard input");
	return 0;
    }
    n += i;
    ptr = buf;
    p = buf;
    q = out;
    r = out;
    numarg = 0;
    tagging = 0;
    inarg = 0;
    prespace = 1;
    quoted = 0;
    while (p < buf + n && q < out + 4000) {
	if (*p == '\n') {
	    if (tagging) {
		tag = (struct tagstruct *)malloc(sizeof(struct tagstruct));
		tag->item = (s_char *)malloc((1 + p - s) * sizeof(s_char));
		tag->next = taglist;
		taglist = tag;
		t = tag->item;
		while (s < p)
		    *t++ = *s++;
		*t = 0;
	    }
	    *q++ = *p++;
	    numarg = 0;
	    tagging = 0;
	    inarg = 0;
	    prespace = 1;
	    quoted = 0;
	    ptr = p;
	    r = q;
	} else if (tagging) {
	    *q++ = *p++;
	} else if (!quoted && isspace(*p)) {
	    *q++ = *p++;
	    prespace = 1;
	    if (numarg == 1 && exec_com && s > exec + 2) {
		tagging = 1;
		s = p;
	    }
	} else if (prespace && *p == '|') {
	    tagging = 1;
	    *q++ = *p++;
	    s = p;
	} else if (prespace && *p == '>') {
	    tagging = 1;
	    *q++ = *p++;
	    if (*p != '\n' && (*p == '!' || *p == '>'))
		*q++ = *p++;
	    s = p;
	} else {
	    prespace = 0;
	    if (*p == '"') {
		quoted = !quoted;
	    } else {
		if (!inarg && *p != '?') {
		    s = exec;
		    exec_com = 1;
		    numarg++;
		}
		inarg = 1;
		if (*s && *s++ != *p)
		    exec_com = 0;
	    }
	    *q++ = *p++;
	}
    }
    p = buf;
    while (ptr < buf + n)
	*p++ = *ptr++;
    *p = 0;
    ptr = out;
    n = r - out;
    if (auxfi) {
	fwrite(out, n, 1, auxfi);
    }
    while (n > 0) {
#ifndef _WIN32
	nbytes = write(sock, ptr, n);
#else
	nbytes = send(sock, ptr, n, 0);
#endif
	if (nbytes <= 0) {
#ifdef _WIN32
	    errno = WSAGetLastError();
#endif
	    perror("write server socket");
	    return 0;
	}
	ptr += nbytes;
	n -= nbytes;
    }
    return 1;
}

int
sendeof(int sock)
{
#ifndef _WIN32
    if (write(sock, "ctld\n", 5) < 5) {
#else
    if (send(sock, "ctld\n", 5, 0) < 5) {
#endif
	fprintf(stderr, "sendeof: EOF send failed\n");
	close(sock);
	return 0;
    }
    return 1;
}

int echomode = 1;

#if defined(hpux) || defined(aix) || defined (sgi) || defined(linux)
void
_noecho(int fd)
{
    struct termio io;

    echomode = 0;
    (void)ioctl(fd, TCGETA, &io);
    io.c_line |= ECHO;
    (void)ioctl(fd, TCSETA, &io);
}

void
_echo(int fd)
{
    struct termio io;

    if (echomode)
	return;
    (void)ioctl(fd, TCGETA, &io);
    io.c_line &= ~ECHO;
    (void)ioctl(fd, TCSETA, &io);
    echomode++;
}

#else
#ifndef _WIN32

void
_noecho(fd)
int fd;
{
    struct sgttyb sgbuf;

    echomode = 0;
    (void)ioctl(fd, TIOCGETP, &sgbuf);
    sgbuf.sg_flags &= ~ECHO;
    (void)ioctl(fd, TIOCSETP, &sgbuf);
}

void
_echo(fd)
int fd;
{
    struct sgttyb sgbuf;

    if (echomode)
	return;
    (void)ioctl(fd, TIOCGETP, &sgbuf);
    sgbuf.sg_flags |= ECHO;
    (void)ioctl(0, TIOCSETP, &sgbuf);
    echomode++;
}
#else
void
_noecho(fd)
int fd;
{
    echomode = 0;
}

void
_echo(fd)
int fd;
{
    echomode++;
}

#endif /* _WIN32 */
#endif
