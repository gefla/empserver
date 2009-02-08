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
 *  tel.h: Definitions for things having to do with telegrams
 *
 *  Known contributors to this file:
 *
 */

#ifndef TEL_H
#define TEL_H

#include <stdio.h>
#include <time.h>
#include "types.h"

#define	MAXTELSIZE	1024	/* doesn't apply to TEL_UPDATE */

#define TEL_NORM	0	/* normal */
#define TEL_ANNOUNCE	1	/* announcement */
#define TEL_BULLETIN	2	/* bulletin */
#define TEL_UPDATE	3	/* update message */
#define TEL_LAST        3

#define TEL_SECONDS 5		/* how many seconds before starting a new telegram */

struct telstr {
    natid tel_from;		/* sender */
    unsigned char tel_type;
    unsigned tel_length;	/* how long */
    time_t tel_date;		/* when sent */
};

extern char *mailbox(char *buf, natid cn);
extern int tel_read_header(FILE *, char *, struct telstr *);
extern int tel_read_body(FILE *, char *, struct telstr *,
			 int (*sink)(char *, size_t, void *),
			 void *);

#endif
