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
 *  login.c: Log into an empire server
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2009
 */

#include <config.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "misc.h"
#include "proto.h"

#ifndef HAVE_GETPASS
#define getpass ersatz_getpass
static char *
ersatz_getpass(char *prompt)
{
    static char buf[128];
    char *p;
    size_t len;
#ifdef _WIN32
    DWORD mode;
    HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);

    if (GetConsoleMode(input_handle, &mode))
	SetConsoleMode(input_handle, mode & ~ENABLE_ECHO_INPUT);
    else
#endif
	printf("Note: your input is echoed to the screen\n");
    printf("%s", prompt);
    fflush(stdout);
    p = fgets(buf, sizeof(buf), stdin);
#ifdef _WIN32
    if (GetConsoleMode(input_handle, &mode))
	SetConsoleMode(input_handle, mode | ENABLE_ECHO_INPUT);
#endif
    if (!p)
	return NULL;
    len = strlen(p);
    if (p[len - 1] == '\n')
	p[len - 1] = 0;
    return p;
}
#endif

int
login(int s, char *uname, char *cname, char *cpass,
      int kill_proc, int utf8)
{
    char tmp[128];
    char buf[1024];
    char *ptr;
    char *p;
    int len, code;

    if (!expect(s, C_INIT, buf))
	return 0;
    sendcmd(s, "user", uname);
    if (!expect(s, C_CMDOK, buf)) {
	fprintf(stderr, "Server rejects your user name\n");
	return 0;
    }
    if (utf8) {
	sendcmd(s, "options", "utf-8");
	for (;;) {
	    code = recvline(s, buf);
	    if (code == C_CMDOK)
		break;
	    if (code != C_DATA) {
		fprintf(stderr, "Server doesn't support UTF-8\n");
		return 0;
	    }
	}
    }
    if (cname == NULL) {
	(void)printf("Country name? ");
	fflush(stdout);
	cname = fgets(tmp, sizeof(tmp), stdin);
	if (cname == NULL || *cname == 0)
	    return 0;
    }
    len = strlen(cname);
    if (cname[len-1] == '\n')
	cname[len-1] = 0;
    sendcmd(s, "coun", cname);
    if (!expect(s, C_CMDOK, buf)) {
	(void)fprintf(stderr, "No such country\n");
	return 0;
    }
    if (cpass == NULL) {
	cpass = getpass("Your name? ");
	if (cpass == NULL || *cpass == 0)
	    return 0;
    }
    (void)printf("\n");
    sendcmd(s, "pass", cpass);
    memset(cpass, 0, strlen(cpass));	/* for core dumps */
    if (!expect(s, C_CMDOK, buf)) {
	(void)fprintf(stderr, "Bad password\n");
	return 0;
    }
    if (kill_proc) {
	sendcmd(s, "kill", NULL);
	(void)printf("\n\t-=O=-\n");
	(void)expect(s, C_EXIT, buf);
	fprintf(stderr, "%s\n", buf);
	return 0;
    }
    sendcmd(s, "play", NULL);
    (void)printf("\n\t-=O=-\n");
    if (!expect(s, C_INIT, buf)) {
	fprintf(stderr, "%s\n", buf);
	return 0;
    }
    for (ptr = buf; !isspace(*ptr); ptr++) ;
    ptr++;
    p = strchr(ptr, '\n');
    if (p != NULL)
	*p = 0;
    if (atoi(ptr) != CLIENTPROTO) {
	printf("Empire client out of date; get new version!\n");
	printf("   this version: %d, current version: %d\n",
	       CLIENTPROTO, atoi(ptr));
    }
    fflush(stdout);
    return 1;
}
