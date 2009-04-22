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
 *  empio.h: Describes io pointers used in Empire
 *
 *  Known contributors to this file:
 *
 */

#ifndef EMPIO_H
#define EMPIO_H
#include <sys/time.h>

#define	IO_READ		0x1
#define IO_WRITE	0x2
#define	IO_EOF		0x10
#define IO_ERROR	0x40

#define IO_BUFSIZE	4096

#define IO_NOWAIT	0
#define IO_WAIT		1

extern struct iop *io_open(int, int, int, struct timeval);
extern void io_init(void);
extern void io_close(struct iop *);
extern int io_input(struct iop *, int);
extern int io_inputwaiting(struct iop *);
extern int io_outputwaiting(struct iop *);
extern int io_output(struct iop *, int);
extern int io_peek(struct iop *, char *, int);
extern int io_read(struct iop *, char *, int);
extern int io_write(struct iop *, char *, int, int);
extern int io_output_all(struct iop *);
extern int io_gets(struct iop *, char *, int);
extern int io_puts(struct iop *, char *);
extern int io_shutdown(struct iop *, int);
extern int io_error(struct iop *);
extern int io_eof(struct iop *);
extern int io_fileno(struct iop *);

#endif
