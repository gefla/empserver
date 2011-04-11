/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  treaty.h: Definitions for treaties
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#ifndef TREATY_H
#define TREATY_H

#include <time.h>
#include "types.h"

struct trtstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned trt_seqno: 12;
    unsigned trt_generation: 12;
    int trt_uid;
    time_t trt_timestamp;
    /* end of part matching struct empobj */
    natid trt_cna;		/* proposer */
    natid trt_cnb;		/* acceptor */
    signed char trt_status;	/* treaty status */
    char trt_fill;
    short trt_acond;		/* conditions for proposer */
    short trt_bcond;		/* conditions for accepter */
    time_t trt_exp;		/* expiration date */
};

#define TS_FREE		AGREE_FREE
#define TS_PROPOSED	AGREE_PROPOSED
#define TS_SIGNED	AGREE_SIGNED

	/* treaty clauses */
#define LNDATT	bit(0)		/* no attacks on land units */
#define SEAATT	bit(1)		/* no attacks on ships */
#define SEAFIR	bit(2)		/* no shelling ships */
#define LANATT	bit(3)		/* no attacks on sectors */
#define LANFIR	bit(4)		/* no shelling sectors */
#define NEWSHP	bit(5)		/* no new ships */
#define NEWNUK	bit(6)		/* no new nuclear weapons */
#define NEWPLN	bit(7)		/* no new planes */
#define NEWLND	bit(8)		/* no new land units */
#define TRTENL	bit(9)		/* no enlistment */
#define SUBFIR	bit(10)		/* no depth-charging submarines */

#define gettre(n, p) ef_read(EF_TREATY, (n), (p))
#define puttre(n, p) ef_write(EF_TREATY, (n), (p))
#define gettrep(n) ((struct trtstr *)ef_ptr(EF_TREATY, (n)))

#endif
