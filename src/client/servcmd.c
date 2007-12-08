/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  servercmd.c: Change the state depending on the command from the server.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1998
 *     Ron Koenderink, 2005
 *     Markus Armbruster, 2005-2007
 */

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "misc.h"
#include "proto.h"
#include "secure.h"

int eight_bit_clean;
FILE *auxfp;

static FILE *redir_fp;
static int redir_is_pipe;
static int executing;
static size_t input_to_forget;

static void prompt(int, char *, char *);
static void doredir(char *p);
static void dopipe(char *p);
static int doexecute(char *p);

void
servercmd(int code, char *arg, int len)
{
    static int nmin, nbtu, fd;
    static char the_prompt[1024];
    static char teles[64];

    switch (code) {
    case C_PROMPT:
	if (sscanf(arg, "%d %d", &nmin, &nbtu) != 2) {
	    fprintf(stderr, "prompt: bad server prompt %s\n", arg);
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
	if (input_to_forget) {
	    forget_input(input_to_forget);
	    input_to_forget = 0;
	}
	prompt(code, the_prompt, teles);
	executing = 0;
	break;
    case C_FLUSH:
	snprintf(the_prompt, sizeof(the_prompt), "%.*s", len - 1, arg);
	prompt(code, the_prompt, teles);
	break;
    case C_EXECUTE:
	fd = doexecute(arg);
	if (fd < 0)
	    send_eof++;
	else {
	    input_fd = fd;
	    executing = 1;
	}
	break;
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
	    snprintf(teles, sizeof(teles), "(%.*s) ", len -1, arg);
	    if (!redir_fp) {
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
}

static void
prompt(int code, char *prompt, char *teles)
{
    char *nl;

    nl = code == C_PROMPT || code == C_INFORM ? "\n" : "";
    printf("%s%s%s", nl, teles, prompt);
    fflush(stdout);
    if (auxfp) {
	fprintf(auxfp, "%s%s%s", nl, teles, prompt);
	fflush(auxfp);
    }
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
redir_authorized(char *arg, char *attempt, int expected)
{
    size_t seen = seen_input(arg);

    if (executing) {
	fprintf(stderr, "Can't %s in a script\n", attempt);
	return 0;
    }

    if (!expected) {
	fprintf(stderr, "WARNING!  Server attempted to %s unexpectedly\n",
		attempt);
	return 0;
    }

    if (!seen || (input_to_forget && input_to_forget != seen)) {
	fprintf(stderr, "WARNING!  Server attempted to %s %s\n",
		attempt, arg);
	return 0;
    }
    input_to_forget = seen;
    return 1;
}

static void
doredir(char *p)
{
    int mode;
    int fd;

    if (!redir_authorized(p, "redirect to file", !redir_fp))
	return;
    if (*p++ != '>') {
	fprintf(stderr, "WARNING!  Weird redirection %s", p);
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
    if (!redir_authorized(p, "pipe to shell command", !redir_fp))
	return;
    if (*p++ != '|') {
	fprintf(stderr, "WARNING!  Weird pipe %s", p);
	return;
    }

    for (; *p && isspace(*p); p++) ;
    if (*p == 0) {
	fprintf(stderr, "Redirection lacks a command\n");
	return;
    }

    redir_is_pipe = 1;
    if ((redir_fp = popen(p, "w")) == NULL) {
	fprintf(stderr, "Can't redirect to pipe %s: %s\n",
		p, strerror(errno));
    }
}

static int
doexecute(char *p)
{
    int fd;

    if (!redir_authorized(p, "execute script file", 1))
	return -1;

    p = fname(p);
    if (*p == 0) {
	fprintf(stderr, "Need a file to execute\n");
	return -1;
    }

    if ((fd = open(p, O_RDONLY)) < 0) {
	fprintf(stderr, "Can't open execute file %s: %s\n",
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
