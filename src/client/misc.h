/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 *     Markus Armbruster, 2004-2020
 */

#ifndef MISC_H
#define MISC_H

#include <stdio.h>

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

extern char empirehost[];
extern char empireport[];

/* expect.c */
int recvline(int s, char *buf);
int parseid(char *);
int expect(int s, int match, char *buf);
void sendcmd(int s, char *cmd, char *arg);

/* getpass.c */
char *get_password(const char *);

/* host.c */
int tcp_connect(char *, char *);

/* login.c */
int login(int s, char *uname, char *cname, char *cpass, int kill_proc, int);

/* play.c */
int play(int, char *);
void prompt(int, char *, char *);

/* servcmd.c */
extern int eight_bit_clean;
extern FILE *auxfp;
extern int restricted;
int servercmd(int, char *, int);
void outch(char);

/* termlib.c */
#ifdef HAVE_CURSES_TERMINFO
void getsose(void);
void putso(void);
void putse(void);
#else  /* !HAVE_CURSES_TERMINFO */
#define getsose() ((void)0)
#define putso() ((void)0)
#define putse() ((void)0)
#endif	/* !HAVE_CURSES_TERMINFO */

/* Compatibility gunk for Windows */
#ifdef _MSC_VER
#define pclose _pclose
#define popen _popen
#define snprintf _snprintf
#endif

#endif
