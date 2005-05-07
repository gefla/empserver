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
 *  servercmd.c: Change the state depending on the command from the server.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "proto.h"
#include "queue.h"
#include "ioqueue.h"
#include "tags.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#if !defined(_WIN32)
#include <unistd.h>
#else
#include <io.h>
#endif

char num_teles[64];
static char the_prompt[1024];
static int mode;
static int nbtu;
static int nmin;
FILE *redir_fp;
FILE *pipe_fp;
int exec_fd;

static void prompt(FILE *auxfi);
static void doredir(char *p);
static void dopipe(char *p);
static void doexecute(char *p, FILE *auxfi);
static void output(int code, char *buf, FILE *auxfi);
static void screen(char *buf);

void
servercmd(struct ioqueue *ioq, FILE *auxfi)
{
    char buf[1024];
    char *p;
    int code;

    while (ioq_gets(ioq, buf, sizeof(buf))) {
	p = buf;
	while (*p && !isspace(*p))
	    p++;
	*p++ = 0;
	if (isalpha(*buf))
	    code = 10 + (*buf - 'a');
	else
	    code = *buf - '0';
	switch (code) {
	case C_PROMPT:
	    if (sscanf(p, "%d %d", &nmin, &nbtu) != 2) {
		fprintf(stderr, "prompt: bad server prompt %s\n", p);
	    }
	    mode = code;
	    sprintf(the_prompt, "[%d:%d] Command : ", nmin, nbtu);
	    prompt(auxfi);
	    break;
	case C_REDIR:
	    doredir(p);
	    break;
	case C_PIPE:
	    dopipe(p);
	    break;
	case C_FLUSH:
	    mode = code;
	    sprintf(the_prompt, "%s", p);
	    prompt(auxfi);
	    break;
	case C_EXECUTE:
	    doexecute(p, auxfi);
	    break;
	case C_INFORM:
	    if (*p) {
		p[strlen(p) - 1] = '\0';
		sprintf(num_teles, "(%s) ", p + 1);
		if (!redir_fp && !pipe_fp && !exec_fd) {
		    putchar('\07');
		    prompt(NULL);
		}
	    } else
		*num_teles = '\0';
	    break;
	default:
	    output(code, p, auxfi);
	    break;
	}
    }
}

static void
prompt(FILE *auxfi)
{
    if (mode == C_PROMPT) {
	if (redir_fp) {
	    (void)fclose(redir_fp);
	    redir_fp = NULL;
	} else if (pipe_fp) {
#ifndef _WIN32
	    (void)pclose(pipe_fp);
#endif
	    pipe_fp = NULL;
	} else if (exec_fd > 0) {
	    close(exec_fd);
	    close(0);
	    exec_fd = -1;
	    open("/dev/tty", O_RDONLY, 0);
	}
    }
    if (mode == C_PROMPT)
	printf("\n");
    printf("%s%s", num_teles, the_prompt);
    (void)fflush(stdout);
    if (auxfi) {
	fprintf(auxfi, "\n%s%s", num_teles, the_prompt);
	(void)fflush(auxfi);
    }
}

/*
 * opens redir_fp if successful
 */
static void
doredir(char *p)
{
    char *how;
    char *name;
    char *tag;
    int mode;
    int fd;

    if (redir_fp) {
	(void)fclose(redir_fp);
	redir_fp = NULL;
    }
    how = p++;
    if (*p && ((*p == '>') || (*p == '!')))
	p++;
    tag = gettag(p);
    while (*p && isspace(*p))
	p++;
    name = p;
    while (*p && !isspace(*p))
	p++;
    *p = 0;
    if (tag == NULL) {
	fprintf(stderr, "WARNING!  Server redirected output to file %s\n",
		name);
	return;
    }
    mode = O_WRONLY | O_CREAT;
    if (how[1] == '>')
	mode |= O_APPEND;
    else if (how[1] == '!')
	mode |= O_TRUNC;
    else
	mode |= O_EXCL;
    if (*name == 0) {
	fprintf(stderr, "Null file name after redirect\n");
	free(tag);
	return;
    }
    if ((fd = open(name, mode, 0600)) < 0) {
	fprintf(stderr, "Redirect open failed\n");
	perror(name);
    } else {
	redir_fp = fdopen(fd, "w");
    }
    free(tag);
}

/*
 * opens "pipe_fp" if successful
 */
static void
dopipe(char *p)
{
    char *tag;

    if (*p == '|')
	p++;
    tag = gettag(p);
    while (*p && isspace(*p))
	p++;
    if (tag == NULL) {
	fprintf(stderr, "WARNING!  Server attempted to run: %s\n", p);
	return;
    }
    if (*p == 0) {
	fprintf(stderr, "Null program name after redirect\n");
	free(tag);
	return;
    }
#ifndef _WIN32
    if ((pipe_fp = popen(p, "w")) == NULL) {
#else
    if (1) {
#endif
	fprintf(stderr, "Pipe open failed\n");
	perror(p);
    }
    free(tag);
}

static void
doexecute(char *p, FILE *auxfi)
{
    int fd;
    char *tag;

    tag = gettag(p);
    while (*p && isspace(*p))
	p++;
    if (tag == NULL) {
	fprintf(stderr,
		"WARNING!  Server attempted unauthorized read of file %s\n",
		p);
	return;
    }
    if (p == NULL) {
	fprintf(stderr, "Null file to execute\n");
	free(tag);
	return;
    }
#if !defined(_WIN32)
    if ((fd = open(p, O_RDONLY, 0)) < 0) {
#else
    if ((fd = open(p, O_RDONLY | O_BINARY, 0)) < 0) {
#endif
	fprintf(stderr, "Can't open execute file\n");
	perror(p);
	free(tag);
	return;
    }
    /* copies 4k at a time to the socket */
    while (termio(fd, sock, auxfi))	/*do copy */
	;
    /* Some platforms don't send the eof (cntl-D) at the end of
       copying a file.  If emp_client hangs at the end of an
       execute, include the following line and notify wolfpack
       of the platform you are using.
       sendeof(sock);
     */
    close(fd);
    free(tag);
}

static void
output(int code, char *buf, FILE *auxfi)
{
    switch (code) {
    case C_NOECHO:
	/* not implemented; serve doesn't send it */
	break;
    case C_FLUSH:
	(void)fflush(stdout);
	if (auxfi)
	    (void)fflush(auxfi);
	break;
    case C_ABORT:
	printf("Aborted\n");
	if (auxfi)
	    fprintf(auxfi, "Aborted\n");
	break;
    case C_CMDERR:
    case C_BADCMD:
	printf("Error; ");
	if (auxfi)
	    fprintf(auxfi, "Error; ");
	break;
    case C_EXIT:
	printf("Exit: ");
	if (auxfi)
	    fprintf(auxfi, "Exit: ");
	break;
    case C_FLASH:
	printf("\n");
	break;
    default:
	break;
    }
    if (auxfi) {
	fprintf(auxfi, "%s", buf);
	if (code == C_FLUSH)
	    (void)fflush(auxfi);
	else
	    (void)putc('\n', auxfi);
    }

    if (redir_fp)
	fprintf(redir_fp, "%s\n", buf);
    else if (pipe_fp)
	fprintf(pipe_fp, "%s\n", buf);
    else {
	if (SO && SE)
	    screen(buf);
	else
	    fputs(buf, stdout);
	if (code == C_FLUSH)
	    (void)fflush(stdout);
	else
	    (void)putc('\n', stdout);
    }
}

static void
screen(char *buf)
{
    char *sop;
    char c;

    while ((c = *buf++)) {
	if (c & 0x80) {
	    for (sop = SO; putc(*sop, stdout); sop++) ;
	    (void)putc(c & 0x7f, stdout);
	    for (sop = SE; putc(*sop, stdout); sop++) ;
	} else
	    (void)putc(c, stdout);
    }
}
