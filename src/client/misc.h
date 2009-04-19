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
 *  misc.h: Misc client definitions
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#ifndef MISC_H
#define MISC_H

#include <stdio.h>

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

extern char empirehost[];
extern char empireport[];
extern int eight_bit_clean;
extern int input_fd;
extern int send_eof;
extern FILE *auxfp;

#ifdef HAVE_CURSES_TERMINFO
void getsose(void);
void putso(void);
void putse(void);
#else  /* !HAVE_CURSES_TERMINFO */
#define getsose() ((void)0)
#define putso() ((void)0)
#define putse() ((void)0)
#endif	/* !HAVE_CURSES_TERMINFO */

int recvline(int s, char *buf);
int parseid(char *);
int expect(int s, int match, char *buf);
int tcp_connect(char *, char *);
int login(int s, char *uname, char *cname, char *cpass, int kill_proc, int);
int play(int);
void sendcmd(int s, char *cmd, char *arg);
void servercmd(int, char *, int);
void outch(char);

#ifdef _MSC_VER
#define pclose _pclose
#define popen _popen
#define snprintf _snprintf
#endif

#endif
