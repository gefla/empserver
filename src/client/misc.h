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
 *  misc.h: Misc client definitions
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#ifndef _MISC_H_
#define _MISC_H_

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#if !defined(aix) && !defined(sgi)
#ifndef ultrix			/* already defined in ultrix */
typedef char s_char;		/* change to signed char for aix */
#endif /* ultrix */
#else
typedef signed char s_char;
#endif /* !aix && !sgi */
typedef short coord;		/* also change NSC_COORD in nsc.h */

struct ioqueue;

extern s_char empirehost[];
extern s_char empireport[];
extern int interrupt;
extern s_char num_teles[];
extern int sock;
extern s_char *SO;
extern s_char *SE;

#ifdef _WIN32
HANDLE hStdIn;
#endif

void getsose(void);
int expect(int s, int match, s_char *buf);
int handleintr(int);
int hostaddr(s_char *name, struct sockaddr_in *addr);
int hostconnect(struct sockaddr_in *addr);
int hostport(s_char *name, struct sockaddr_in *addr);
int login(int s, s_char *uname, s_char *cname, s_char *cpass, int kill_proc);
void saveargv(int ac, s_char **src, s_char **dst);
void sendcmd(int s, int cmd, s_char *arg);
int sendeof(int sock);
void servercmd(struct ioqueue *ioq, FILE *auxfi);
int serverio(int s, struct ioqueue *ioq);
int termio(int fd, int sock, FILE *auxfi);

#endif
