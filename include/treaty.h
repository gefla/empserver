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
 *  treaty.h: Definitions for treaties
 * 
 *  Known contributors to this file:
 *      Steve McClure, 1998
 *     
 */

#ifndef _TREATY_H_
#define _TREATY_H_

struct trtstr {
    natid trt_cna;		/* proposer */
    natid trt_cnb;		/* acceptor */
    s_char trt_status;		/* treaty status */
    s_char trt_fill;
    short trt_acond;		/* conditions for proposer */
    short trt_bcond;		/* conditions for accepter */
    float trt_bond;		/* amount of bond involved (unused) */
    time_t trt_exp;		/* expiration date */
};

struct tchrstr {
    int t_cond;			/* bit to indicate this clause */
    s_char *t_name;		/* description of clause */
};

#define TS_FREE		0
#define TS_PROPOSED	1
#define TS_SIGNED	2

	/* treaty clauses */
#define	LNDATT	bit(0)		/* no attacks on land units */
#define	SEAATT	bit(1)		/* no attacks on ships */
#define	SEAFIR	bit(2)		/* no shelling ships */
#define	LANATT	bit(3)		/* no attacks on sectors */
#define	LANFIR	bit(4)		/* no shelling sectors */
#define	NEWSHP	bit(5)		/* no new ships */
#define	NEWNUK	bit(6)		/* no new nuclear weapons */
#define	NEWPLN	bit(7)		/* no new planes */
#define NEWLND  bit(8)		/* no new land units */
#define	TRTENL	bit(9)		/* no enlistment */
#define SUBFIR  bit(10)		/* no depth-charging submarines */

/* global treaty variables */

#define gettre(n, p) \
	ef_read(EF_TREATY, n, (caddr_t)p)
#define puttre(n, p) \
	ef_write(EF_TREATY, n, (caddr_t)p)
#define gettrep(n) \
	(struct trtstr *) ef_ptr(EF_TREATY, n)

extern struct tchrstr tchr[];

#endif /* _TREATY_H_ */
