/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  main.c: client main function
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 */

#ifdef _NTSDK
#undef _NTSDK
#endif

#include "misc.h"
#include "proto.h"
#include "queue.h"
#include "ioqueue.h"
#include "tags.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef _WIN32
#include <pwd.h>
#endif
#include <signal.h>
#include <errno.h>
#include <time.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#else
#include <windows.h>
#include <winsock.h>
#include <conio.h>
#include <io.h>
#endif

#ifdef _WIN32
HANDLE hStdIn;
#endif

#define	RETRY	3

int interrupt;
int sock;

static void intr(int sig);


int
main(int ac, char **av)
{
#ifdef _WIN32
    WSADATA WsaData;
    int err;
    fd_set readfds;
    struct timeval tm;
    DWORD stdinmode;
    SECURITY_ATTRIBUTES security;
    int bRedirected = 0;
    char unamebuf[128];
#else
    fd_set mask;
    fd_set savemask;
    int retry = 0;
#endif
    struct ioqueue server;
    char *argv[128];
    int i, j;
    char *ptr;
    char *auxout_fname;
    FILE *auxout_fp;
    struct sockaddr_in sin;
    int n;
    char *cname;
    char *pname;
    char *uname;
    int send_kill = 0;

#ifdef _WIN32
    err = WSAStartup(0x0101, &WsaData);
    if (err == SOCKET_ERROR) {
	printf("WSAStartup Failed\n");
	return FALSE;
    }
#else
    FD_ZERO(&mask);
    FD_ZERO(&savemask);
#endif
    memset(argv, 0, sizeof(argv));
    saveargv(ac, av, argv);
    auxout_fname = NULL;
    auxout_fp = NULL;
    for (i = j = 1; i < ac; ++i) {
	ptr = argv[i];
	if (strcmp(ptr, "-2") == NULL) {
	    if (i + 1 >= ac) {
		fprintf(stderr, "-2: Missing filename!\n");
		exit(1);
	    }
	    auxout_fname = argv[i + 1];
	    ++i;
	    continue;
	} else if (strcmp(ptr, "-k") == NULL) {
	    send_kill = 1;
	    continue;
	}
	argv[j] = argv[i];
	++j;
    }
    ac = j;
    if (auxout_fname && (auxout_fp = fopen(auxout_fname, "a")) == NULL) {
	fprintf(stderr, "Unable to open %s for append\n", auxout_fname);
	exit(1);
    }
    getsose();
    if (!hostport(getenv("EMPIREPORT"), &sin) &&
	!hostport("empire", &sin) && !hostport(empireport, &sin)) {
	fprintf(stderr, "No empire port\n");
	exit(1);
    }
    if (!hostaddr(getenv("EMPIREHOST"), &sin) &&
	!hostaddr(empirehost, &sin)) {
	fprintf(stderr, "No empire host\n");
	exit(1);
    }
    if ((sock = hostconnect(&sin)) < 0)
	exit(1);
    cname = getenv("COUNTRY");
    if (ac > 1)
	cname = argv[1];
    pname = getenv("PLAYER");
    if (ac > 2)
	pname = argv[2];
    uname = getenv("LOGNAME");
    if (uname == NULL) {
#ifndef _WIN32
	struct passwd *pwd;

	pwd = getpwuid(getuid());
	if (pwd == NULL) {
	    fprintf(stderr, "You don't exist.  Go away\n");
	    exit(1);
	}
	uname = pwd->pw_name;
#else
	DWORD unamesize;

	unamesize = sizeof(unamebuf);
	if (GetUserName(unamebuf, &unamesize)) {
	    uname = unamebuf;
	    if ((unamesize <= 0 ) || (strlen(uname) <= 0))
		uname = "nobody";
	} else
	    uname = "nobody";
#endif
    }
    if (!login(sock, uname, cname, pname, send_kill)) {
	close(sock);
	exit(1);
    }
    ioq_init(&server, 2048);
    io_init();
#ifndef _WIN32
    FD_ZERO(&mask);
    FD_SET(0, &savemask);
    FD_SET(sock, &savemask);
#endif
    (void)signal(SIGINT, intr);
#ifndef _WIN32
    (void)signal(SIGPIPE, SIG_IGN);
    while (FD_ISSET(sock, &savemask)) {
	mask = savemask;
	n = select(sock + 1, &mask, NULL, NULL, NULL);
	if (interrupt) {
	    if (!handleintr(sock))
		break;
	    errno = 0;
	}
	if (n <= 0) {
	    if (errno == EINTR) {
		perror("select");
		(void)close(sock);
		FD_CLR(sock, &savemask);
	    }
	} else {
	    if (FD_ISSET(0, &mask)) {
		if (!termio(0, sock, auxout_fp)) {
		    if (retry++ >= RETRY) {
			FD_CLR(0, &savemask);
		    }
		} else {
		    retry = 0;
		}
	    }
	    if (FD_ISSET(sock, &mask)) {
		if (!serverio(sock, &server))
		    FD_CLR(sock, &savemask);
		else
		    servercmd(&server, auxout_fp);
	    }
	}
    }
#else
    bRedirected = 0;
    tm.tv_sec = 0;
    tm.tv_usec = 1000;

    if (!_isatty(_fileno(stdin)))
	bRedirected = 1;
    else {
	security.nLength = sizeof(SECURITY_ATTRIBUTES);
	security.lpSecurityDescriptor = NULL;
	security.bInheritHandle = TRUE;
	hStdIn = CreateFile("CONIN$",
			    GENERIC_READ | GENERIC_WRITE,
			    FILE_SHARE_READ | FILE_SHARE_WRITE,
			    &security, OPEN_EXISTING, (DWORD) NULL, NULL);
	
	if (hStdIn == INVALID_HANDLE_VALUE) {
	    printf("Error getting hStdIn.\n");
	    fflush(stdout);
	    exit(-3);
	}
	
	err = GetConsoleMode(hStdIn, &stdinmode);
	if (!err) {
	    printf("Error getting console mode.\n");
	    fflush(stdout);
	    exit(-4);
	} else {
	    stdinmode |= ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT;
	    err = SetConsoleMode(hStdIn, stdinmode);
	    if (!err) {
		printf("Error setting console mode.\n");
		fflush(stdout);
		exit(-5);
	    }
	}
    }
    while (1) {
	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);
	n = select(sock + 1, &readfds, NULL, NULL, (struct timeval *)&tm);
	if (interrupt) {
	    if (!handleintr(sock))
		break;
	    errno = 0;
	}
	if (n < 0) {
	    if (errno == EINTR) {
		errno = WSAGetLastError();
		perror("select");
		(void)close(sock);
		break;
	    }
	} else {
	    if (bRedirected == 1) {
		if (!termio(0, sock, auxout_fp))
		    bRedirected = -1;
	    } else if (bRedirected == 0) {
		if (WaitForSingleObject(hStdIn, 10) != WAIT_TIMEOUT) {
		    termio(-1, sock, auxout_fp);
		    FlushConsoleInputBuffer(hStdIn);
		}
	    }
	    if (FD_ISSET(sock, &readfds)) {
		if (!serverio(sock, &server))
		    break;
		else
		    servercmd(&server, auxout_fp);
	    }
	}
    }
    if (bRedirected == 0)
	CloseHandle(hStdIn);
#endif
    ioq_drain(&server);
    (void)close(sock);
    return 0;			/* Shut the compiler up */
}

static void
intr(int sig)
{
    interrupt++;
#ifdef _WIN32
    signal(SIGINT, intr);
#endif
#ifdef hpux
    signal(SIGINT, intr);
#endif
}
