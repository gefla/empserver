/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  loan.h: Definitions for loans
 *
 *  Known contributors to this file:
 *
 */

#ifndef LOAN_H
#define LOAN_H

#include "file.h"
#include "types.h"

#define MAXLOAN		100000
#define SECS_PER_DAY	(60*60*24)

struct lonstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned l_seqno: 12;
    unsigned l_generation: 12;
    int l_uid;
    time_t l_timestamp;
    /* end of part matching struct empobj */
    natid l_loner;		/* loan shark */
    natid l_lonee;		/* sucker */
    signed char l_status;	/* loan status */
    int l_irate;		/* interest rate */
    int l_ldur;			/* intended duration */
    int l_amtpaid;		/* amount paid so far */
    int l_amtdue;		/* amount still owed */
    time_t l_lastpay;		/* date of most recent payment */
    time_t l_duedate;		/* date after which interest doubles, etc */
};

#define LS_FREE		AGREE_FREE
#define LS_PROPOSED	AGREE_PROPOSED
#define LS_SIGNED	AGREE_SIGNED

extern double loan_owed(struct lonstr *loan, time_t paytime);

#define getloan(n, p) ef_read(EF_LOAN, (n), (p))
#define putloan(n, p) ef_write(EF_LOAN, (n), (p))
#define getloanp(n) ((struct lonstr *)ef_ptr(EF_LOAN, (n)))

#endif
