/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2017, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  servercmd.c: Change the state depending on the command from the server.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1998
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2005-2017
 */

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "misc.h"
#include "proto.h"
#include "secure.h"

int eight_bit_clean;
FILE *auxfp;
int restricted;

static FILE *redir_fp;
static int redir_is_pipe;
static int executing;

static void doredir(char *p);
static void dopipe(char *p);
static int doexecute(char *p);

int
servercmd(int code, char *arg, int len)
{
    static int nmin, nbtu, fd;
    static char the_prompt[1024];
    static char teles[64];

    switch (code) {
    case C_PROMPT:
	if (sscanf(arg, "%d %d", &nmin, &nbtu) != 2) {
	    fprintf(stderr, "Warning: server sent malformed prompt %s",
		    arg);
	}
	snprintf(the_prompt, sizeof(the_prompt), "[%d:%d] Command : ",
		 nmin, nbtu);
	if (redir_fp) {
	    if (redir_is_pipe)
		(void)pclose(redir_fp);
	    else
		(void)fclose(redir_fp);
	    redir_fp = NULL;
	}
	outch('\n');
	prompt(code, the_prompt, teles);
	executing = 0;
	break;
    case C_FLUSH:
	snprintf(the_prompt, sizeof(the_prompt), "%.*s", len - 1, arg);
	prompt(code, the_prompt, teles);
	break;
    case C_EXECUTE:
	fd = doexecute(arg);
	if (fd >= 0)
	    executing = 1;
	return fd;
    case C_EXIT:
	printf("Exit: %s", arg);
	if (auxfp)
	    fprintf(auxfp, "Exit: %s", arg);
	break;
    case C_FLASH:
	printf("\n%s", arg);
	if (auxfp)
	    fprintf(auxfp, "\n%s", arg);
	break;
    case C_INFORM:
	if (arg[0] != '\n') {
	    snprintf(teles, sizeof(teles), "(%.*s) ", len - 1, arg);
	    if (!redir_fp) {
		outch('\n');
		putchar('\07');
		prompt(code, the_prompt, teles);
	    }
	} else
	    teles[0] = 0;
	break;
    case C_PIPE:
	dopipe(arg);
	break;
    case C_REDIR:
	doredir(arg);
	break;
    default:
	assert(0);
	break;
    }

    return 0;
}

static char *
fname(char *s)
{
    char *beg, *end;

    for (beg = s; isspace(*(unsigned char *)beg); beg++) ;
    for (end = beg; !isspace(*(unsigned char *)end); end++) ;
    *end = 0;
    return beg;
}

static int
common_authorized(char *arg, char *attempt)
{
    if (restricted) {
	fprintf(stderr, "Can't %s in restricted mode\n", attempt);
	return 0;
    }

    if (executing) {
	fprintf(stderr, "Can't %s in a batch file\n", attempt);
	return 0;
    }
    return 1;
}

static int
redir_authorized(char *arg, char *attempt)
{
    if (redir_fp) {
	fprintf(stderr, "Warning: dropped conflicting %s %s",
		attempt, arg);
	return 0;
    }

    if (!seen_input(arg)) {
	fprintf(stderr, "Warning: server attempted to %s %s",
		attempt, arg);
	return 0;
    }

    return common_authorized(arg, attempt);
}

static int
exec_authorized(char *arg)
{
    if (!seen_exec_input(arg)) {
	fprintf(stderr,
		"Warning: server attempted to execute batch file %s", arg);
	return 0;
    }

    return common_authorized(arg, "execute batch file");
}

static void
doredir(char *p)
{
    int mode;
    int fd;

    if (!redir_authorized(p, "redirect to file"))
	return;
    if (*p++ != '>') {
	fprintf(stderr, "Warning: dropped weird redirection %s", p);
	return;
    }

    mode = O_WRONLY | O_CREAT;
    if (*p == '>') {
	mode |= O_APPEND;
	p++;
    } else if (*p == '!') {
	mode |= O_TRUNC;
	p++;
    } else
	mode |= O_EXCL;

    p = fname(p);
    if (*p == 0) {
	fprintf(stderr, "Redirection lacks a file name\n");
	return;
    }

    redir_is_pipe = 0;
    fd = open(p, mode, 0666);
    redir_fp = fd < 0 ? NULL : fdopen(fd, "w");
    if (!redir_fp) {
	fprintf(stderr, "Can't redirect to %s: %s\n",
		p, strerror(errno));
    }
}

static void
dopipe(char *p)
{
    if (!redir_authorized(p, "pipe to shell command"))
	return;
    if (*p++ != '|') {
	fprintf(stderr, "Warning: dropped weird pipe %s", p);
	return;
    }

    for (; *p && isspace(*p); p++) ;
    if (*p == 0) {
	fprintf(stderr, "Redirection lacks a command\n");
	return;
    }

    /* strip newline */
    p[strlen(p) - 1] = 0;

    redir_is_pipe = 1;
    errno = 0;
    if ((redir_fp = popen(p, "w")) == NULL) {
	fprintf(stderr, "Can't redirect to pipe %s%s%s\n",
		p, errno ? ": " : "", errno ? strerror(errno) : "");
    }
}

static int
doexecute(char *p)
{
    int fd;

    if (!exec_authorized(p))
	return -1;

    p = fname(p);
    if (*p == 0) {
	fprintf(stderr, "Need a file to execute\n");
	return -1;
    }

    if ((fd = open(p, O_RDONLY)) < 0) {
	fprintf(stderr, "Can't open batch file %s: %s\n",
		p, strerror(errno));
	return -1;
    }

    return fd;
}

void
outch(char c)
{
    if (auxfp)
	putc(c, auxfp);
    if (redir_fp)
	putc(c, redir_fp);
    else if (eight_bit_clean) {
	if (c == 14)
	    putso();
	else if (c == 15)
	    putse();
	else
	    putchar(c);
    } else if (c & 0x80) {
	putso();
	putchar(c & 0x7f);
	putse();
    } else
	putchar(c);
}
