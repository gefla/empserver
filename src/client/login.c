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
 *  login.c: Log into an empire server
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "proto.h"

#include <ctype.h>
#include <stdio.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

int expect();
int atoi();
void sendcmd();

int
login(s, uname, cname, cpass, kill_proc)
int s;
s_char *uname;
s_char *cname;
s_char *cpass;
int kill_proc;
{
    s_char tmp[128];
    s_char buf[1024];
    s_char *ptr;
    s_char *p;

    if (!expect(s, C_INIT, buf))
	return 0;
    (void)sendcmd(s, USER, uname);
    if (!expect(s, C_CMDOK, buf))
	return 0;
    if (cname == 0) {
	(void)printf("Country name? ");
	cname = fgets(tmp, 128, stdin);
	if (cname == 0 || *cname == 0)
	    return 0;
    }
    if (cname[strlen(cname) - 1] == '\n')
	cname[strlen(cname) - 1] = 0;
    if (cname[strlen(cname) - 1] == '\r')
	cname[strlen(cname) - 1] = 0;
    (void)sendcmd(s, COUN, cname);
    if (!expect(s, C_CMDOK, buf)) {
	(void)fprintf(stderr, "empire: no such country\n");
	return 0;
    }
    if (cpass == 0) {
#ifndef _WIN32
	cpass = (s_char *)getpass("Your name? ");
#else
	printf("Note: This is echoed to the screen\n");
	printf("Your name? ");
	cpass = tmp;
	*cpass = 0;
	fgets(cpass, 128, stdin);
#endif
	if (cpass == 0 || *cpass == 0)
	    return 0;
    }
    (void)printf("\n");
    (void)sendcmd(s, PASS, cpass);
    bzero(cpass, strlen(cpass));	/* for core dumps */
    if (!expect(s, C_CMDOK, buf)) {
	(void)fprintf(stderr, "Bad password\n");
	return 0;
    }
    if (kill_proc) {
	(void)sendcmd(s, KILL, (s_char *)0);
	(void)printf("\n\t-=O=-\n");
	(void)expect(s, C_EXIT, buf);
	fprintf(stderr, "%s\n", buf);
	return 0;
    }
    (void)sendcmd(s, PLAY, (s_char *)0);
    (void)printf("\n\t-=O=-\n");
    if (!expect(s, C_INIT, buf)) {
	fprintf(stderr, "%s\n", buf);
	return 0;
    }
    for (ptr = buf; !isspace(*ptr); ptr++) ;
    ptr++;
    p = index(ptr, '\n');
    if (p != 0)
	*p = 0;
    if (atoi(ptr) != CLIENTPROTO) {
	printf("Empire client out of date; get new version!\n");
	printf("   this version: %d, current version: %d\n",
	       CLIENTPROTO, atoi(ptr));
    }
    return 1;
}
