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
 *  loan.h: Definitions for loans
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _LOAN_H_
#define _LOAN_H_

#define	MAXLOAN		100000
#define SECS_PER_DAY	(60*60*24)

struct	lonstr {
	short	ef_type;
	natid	l_loner;	/* loan shark */
	short	l_uid;
	natid	l_lonee;	/* sucker */
	s_char	l_status;	/* loan status */
	short	l_sell;		/* pointer to trade file */
	int	l_irate;	/* interest rate */
	int	l_ldur;		/* intended duration */
	long	l_amtpaid;	/* amount paid so far */
	long	l_amtdue;	/* amount still owed */
	time_t	l_lastpay;	/* date of most recent payment */
	time_t	l_duedate;	/* date after which interest doubles, etc */
};

#define LS_FREE		0
#define LS_PROPOSED	1
#define LS_SIGNED	2

#define getloan(n, lp) \
	ef_read(EF_LOAN, n, (caddr_t)lp)
#define putloan(n, lp) \
	ef_write(EF_LOAN, n, (caddr_t)lp)
#define getloanp(n) \
	(struct lonstr *) ef_ptr(EF_LOAN, n)

#endif /* _LOAN_H_ */
