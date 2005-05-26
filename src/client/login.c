/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

int
login(int s, char *uname, char *cname, char *cpass, int kill_proc)
{
    char tmp[128];
    char buf[1024];
    char *ptr;
    char *p;
    int len;

    if (!expect(s, C_INIT, buf))
	return 0;
    (void)sendcmd(s, "user", uname);
    if (!expect(s, C_CMDOK, buf))
	return 0;
    if (cname == NULL) {
	(void)printf("Country name? ");
	cname = fgets(tmp, sizeof(tmp), stdin);
	if (cname == NULL || *cname == 0)
	    return 0;
    }
    len = strlen(cname);
    if (cname[len-1] == '\n')
	cname[len-1] = 0;
    (void)sendcmd(s, "coun", cname);
    if (!expect(s, C_CMDOK, buf)) {
	(void)fprintf(stderr, "empire: no such country\n");
	return 0;
    }
    if (cpass == NULL) {
#ifndef _WIN32
	cpass = getpass("Your name? ");
	if (cpass == NULL || *cpass == 0)
	    return 0;
#else
	printf("Note: This is echoed to the screen\n");
	printf("Your name? ");
	cpass = fgets(tmp, sizeof(tmp), stdin);
	if (cpass == NULL || *cpass == 0)
	    return 0;
	len = strlen(cpass);
	if (cname[len-1] == '\n')
	    cname[len-1] = 0;
#endif
    }
    (void)printf("\n");
    (void)sendcmd(s, "pass", cpass);
    memset(cpass, 0, strlen(cpass));	/* for core dumps */
    if (!expect(s, C_CMDOK, buf)) {
	(void)fprintf(stderr, "Bad password\n");
	return 0;
    }
    if (kill_proc) {
	(void)sendcmd(s, "kill", NULL);
	(void)printf("\n\t-=O=-\n");
	(void)expect(s, C_EXIT, buf);
	fprintf(stderr, "%s\n", buf);
	return 0;
    }
    (void)sendcmd(s, "play", NULL);
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
    return 1;
}
