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
#include "bit.h"

#include <stdio.h>
#ifndef _WIN32
#include <pwd.h>
#endif
#include <signal.h>
#include <errno.h>
#include <time.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#else
#include <windows.h>
#include <winsock.h>
#include <conio.h>
#endif

#ifdef _WIN32
HANDLE hStdIn;
#endif

#define	RETRY	3

int interrupt;
int sock;

void saveargv();
void getsose();
int hostport();
int hostaddr();
int hostconnect();
int login();
void ioq_init();
void io_init();
void bit_copy();
int handleintr();
int termio();
int serverio();
void servercmd();
void ioq_drain();


#ifdef _WIN32
s_char *
index(str, c)
s_char *str;
char c;
{
    static s_char *p;

    p = str;
    while (p && *p) {
	if (*p == c)
	    return (s_char *)p;
	p++;
    }
    return (s_char *)0;
}

#endif

int
main(ac, av)
int ac;
s_char *av[];
{
#ifdef _WIN32
    WSADATA WsaData;
    int err;
    fd_set readfds;
    struct timeval tm;
    INPUT_RECORD InpBuffer[10];
    DWORD numevents;
    DWORD stdinmode;
    SECURITY_ATTRIBUTES security;
    void intr(void);
#endif
    extern s_char *getenv();
    extern s_char empireport[];
    extern s_char empirehost[];
    bit_fdmask mask;
    bit_fdmask savemask;
    struct ioqueue server;
    s_char *argv[128];
    int i, j;
    s_char *ptr;
    s_char *auxout_fname;
    FILE *auxout_fp;
#ifndef _WIN32
    struct passwd *pwd;
    void intr();
#endif
    struct sockaddr_in sin;
    int n;
    s_char *cname;
    s_char *pname;
    int retry = 0;
    int send_kill = 0;

#ifdef _WIN32
    err = WSAStartup(0x0101, &WsaData);
    if (err == SOCKET_ERROR) {
	printf("WSAStartup Failed\n");
	return FALSE;
    }
#else
    mask = bit_newfdmask();
    savemask = bit_newfdmask();
#endif
    memset(argv, 0, sizeof(argv));
    saveargv(ac, av, argv);
    auxout_fname = 0;
    auxout_fp = 0;
    for (i = j = 1; i < ac; ++i) {
	ptr = argv[i];
	if (strcmp(ptr, "-2") == 0) {
	    if (i + 1 >= ac) {
		fprintf(stderr, "-2: Missing filename!\n");
		exit(1);
	    }
	    auxout_fname = argv[i + 1];
	    ++i;
	    continue;
	} else if (strcmp(ptr, "-k") == 0) {
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
#ifndef _WIN32
    pwd = getpwuid(getuid());
    if (pwd == NULL) {
	fprintf(stderr, "You don't exist.  Go away\n");
	exit(1);
    }
#endif
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
    pname = getenv("PLAYER");
    if (ac > 1)
	cname = argv[1];
    if (ac > 2)
	pname = argv[2];
#ifndef _WIN32
    if (!login(sock, pwd->pw_name, cname, pname, send_kill)) {
#else
    if (!login(sock, "win-empcl2.8", cname, pname, send_kill)) {
#endif
	close(sock);
	exit(1);
    }
    ioq_init(&server, 2048);
    io_init();
    mask = bit_newfdmask();
#ifndef _WIN32
    BIT_SETB(0, savemask);
    BIT_SETB(sock, savemask);
#endif
    (void)signal(SIGINT, intr);
#ifndef _WIN32
    (void)signal(SIGPIPE, SIG_IGN);
    while (BIT_ISSETB(sock, savemask)) {
	bit_copy(savemask, mask);
	n = select(sock + 1, (fd_set *) mask, (fd_set *) 0, (fd_set *) 0,
		   (struct timeval *)0);
	if (interrupt) {
	    if (!handleintr(sock))
		break;
	    errno = 0;
	}
	if (n <= 0) {
	    if (errno == EINTR) {
		perror("select");
		(void)close(sock);
		BIT_CLRB(sock, savemask);
	    }
	} else {
	    if (BIT_ISSETB(0, mask)) {
		if (!termio(0, sock, auxout_fp)) {
		    if (retry++ >= RETRY) {
			BIT_CLRB(0, savemask);
		    }
		} else {
		    retry = 0;
		}
	    }
	    if (BIT_ISSETB(sock, mask)) {
		if (!serverio(sock, &server))
		    BIT_CLRB(sock, savemask);
		else
		    servercmd(&server, auxout_fp);
	    }
	}
    }
#else
    tm.tv_sec = 0;
    tm.tv_usec = 1000;

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
    }
    err = GetConsoleMode(hStdIn, &stdinmode);
    if (!err) {
	printf("Error getting console mode.\n");
	fflush(stdout);
    }
    stdinmode |= ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT;
    err = SetConsoleMode(hStdIn, stdinmode);
    if (!err) {
	printf("Error setting console mode.\n");
	fflush(stdout);
    }
    while (1) {
	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);
	n = select(sock + 1, &readfds, (fd_set *) 0, (fd_set *) 0,
		   (struct timeval *)&tm);
	if (n < 0) {
	    if (errno == EINTR) {
		errno = WSAGetLastError();
		perror("select");
		(void)close(sock);
		break;
	    }
	} else {
	    if (WaitForSingleObject(hStdIn, 10) != WAIT_TIMEOUT) {
		if (!termio(-1, sock, auxout_fp)) {
		    if (retry++ >= RETRY) {
			;
		    }
		} else {
		    retry = 0;
		}
		FlushConsoleInputBuffer(hStdIn);
	    }
	    if (FD_ISSET(sock, &readfds)) {
		if (!serverio(sock, &server))
		    break;
		else
		    servercmd(&server, auxout_fp);
	    }
	}
    }
    CloseHandle(hStdIn);
#endif
    ioq_drain(&server);
    (void)close(sock);
    exit(0);
    return 0;			/* Shut the compiler up */
}

void
#ifdef _WIN32
intr(void)
#else
intr()
#endif
{
    interrupt++;
#ifdef hpux
    signal(SIGINT, intr);
#endif
}
