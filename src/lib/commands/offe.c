/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  offe.c: Offer a loan or treaty
 *
 *  Known contributors to this file:
 *     Pat Loney, 1992
 *     Steve McClure, 1996
 */

#include <config.h>

#include "commands.h"
#include "loan.h"
#include "optlist.h"
#include "treaty.h"

static int do_treaty(void);
static int do_loan(void);

int
offe(void)
{
    char *cp;
    char buf[1024];

    cp = getstarg(player->argp[1], "loan or treaty? ", buf);
    if (!cp || !*cp)
	return RET_SYN;

    switch (*cp) {
    case 'l':
	if (!opt_LOANS) {
	    pr("Loans are not enabled.\n");
	    return RET_FAIL;
	}
	return do_loan();
    case 't':
	if (!opt_TREATIES) {
	    pr("Treaties are not enabled.\n");
	    return RET_FAIL;
	}
	return do_treaty();
    default:
	pr("You must specify \"loan\" as there are no treaties.\n");
	return RET_SYN;
    }
}

static int
do_treaty(void)
{
    char *cp;
    int ourcond, theircond;
    struct symbol *tfp;
    struct trtstr trty;
    struct nstr_item nstr;
    natid recipient;
    time_t now;
    int j, n;
    struct natstr *natp;
    char prompt[128];
    char buf[1024];

    if ((n = natarg(player->argp[2], "Treaty offered to? ")) < 0)
	return RET_SYN;
    recipient = n;
    if (recipient == player->cnum) {
	pr("You can't sign a treaty with yourself!\n");
	return RET_FAIL;
    }
    natp = getnatp(recipient);
    if (player->cnum && (getrejects(player->cnum, natp) & REJ_TREA)) {
	pr("%s is rejecting your treaties.\n", cname(recipient));
	return RET_SYN;
    }
    pr("Terms for %s:\n", cname(recipient));
    theircond = 0;
    for (tfp = treaty_flags; tfp && tfp->name; tfp++) {
	sprintf(prompt, "%s? ", tfp->name);
	if (!(cp = getstring(prompt, buf)))
	    return RET_FAIL;
	if (*cp == 'y')
	    theircond |= tfp->value;
    }
    pr("Terms for you:\n");
    ourcond = 0;
    for (tfp = treaty_flags; tfp && tfp->name; tfp++) {
	sprintf(prompt, "%s? ", tfp->name);
	if (!(cp = getstring(prompt, buf)))
	    return RET_FAIL;
	if (*cp == 'y')
	    ourcond |= tfp->value;
    }
    if (ourcond == 0 && theircond == 0) {
	pr("Treaties with no clauses aren't very useful, boss!\n");
	return RET_SYN;
    }
    cp = getstring("Proposed treaty duration? (days) ", buf);
    if (!cp)
	return RET_FAIL;
    j = atoi(cp);
    if (j <= 0) {
	pr("Bad treaty duration.\n");
	return RET_SYN;
    }
    (void)time(&now);
    snxtitem_all(&nstr, EF_TREATY);
    while (nxtitem(&nstr, &trty)) {
	if (trty.trt_status == TS_FREE) {
	    break;
	}
    }
    ef_blank(EF_TREATY, nstr.cur, &trty);
    trty.trt_acond = ourcond;
    trty.trt_bcond = theircond;
    trty.trt_status = TS_PROPOSED;
    trty.trt_cna = player->cnum;
    trty.trt_cnb = recipient;
    trty.trt_exp = j * SECS_PER_DAY + now;
    if (!puttre(nstr.cur, &trty)) {
	logerror("do_treaty: can't write treaty");
	pr("Couldn't save treaty; get help.\n");
	return RET_FAIL;
    }
    wu(0, recipient, "Treaty #%d proposed to you by %s\n",
       nstr.cur, cname(player->cnum));
    pr("You have proposed treaty #%d\n", nstr.cur);
    return RET_OK;
}

static int
do_loan(void)
{
    int amt, irate, dur, maxloan;
    struct nstr_item nstr;
    struct natstr *natp;
    struct lonstr loan;
    natid recipient;
    int n;
    char prompt[128];

    if ((n = natarg(player->argp[2], "Lend to? ")) < 0)
	return RET_SYN;
    recipient = n;
    if (recipient == player->cnum) {
	pr("You can't loan yourself money!\n");
	return RET_FAIL;
    }
    natp = getnatp(recipient);
    if (player->cnum && (getrejects(player->cnum, natp) & REJ_LOAN)) {
	pr("%s is rejecting your loans.\n", cname(recipient));
	return RET_SYN;
    }
    natp = getnatp(player->cnum);
    if (natp->nat_money + 100 > MAXLOAN)
	maxloan = MAXLOAN;
    else
	maxloan = natp->nat_money - 100;
    if (maxloan < 0) {
	pr("You don't have enough money to loan!\n");
	return RET_FAIL;
    }
    sprintf(prompt, "Size of loan for country #%d? (max %d) ",
	    recipient, maxloan);
    amt = onearg(player->argp[3], prompt);
    if (amt <= 0)
	return RET_FAIL;
    if (amt > MAXLOAN) {
	pr("You can only loan $%d at a time.\n", MAXLOAN);
	return RET_FAIL;
    }
    if (amt > maxloan) {
	pr("You can't afford that much.\n");
	return RET_FAIL;
    }
    dur = onearg(player->argp[4], "Duration? (days, max 7) ");
    if (dur <= 0)
	return RET_FAIL;
    if (dur > 7)
	return RET_FAIL;
    irate = onearg(player->argp[5], "Interest rate? (from 5 to 25%) ");
    if (irate > 25)
	return RET_FAIL;
    if (irate < 5)
	return RET_FAIL;
    snxtitem_all(&nstr, EF_LOAN);
    while (nxtitem(&nstr, &loan)) {
	if ((loan.l_status == LS_SIGNED) && (loan.l_lonee == player->cnum)
	    && (loan.l_loner == recipient)) {
	    pr("You already owe HIM money - how about repaying your loan?\n");
	    return RET_FAIL;
	}
    }
    snxtitem_all(&nstr, EF_LOAN);
    while (nxtitem(&nstr, &loan)) {
	if (loan.l_status == LS_FREE)
	    break;
    }
    ef_blank(EF_LOAN, nstr.cur, &loan);
    loan.l_loner = player->cnum;
    loan.l_lonee = recipient;
    loan.l_status = LS_PROPOSED;
    loan.l_irate = MIN(irate, 127);
    loan.l_ldur = MIN(dur, 127);
    loan.l_amtpaid = 0;
    loan.l_amtdue = amt;
    (void)time(&loan.l_lastpay);
    loan.l_duedate = loan.l_ldur * SECS_PER_DAY + loan.l_lastpay;
    if (!putloan(nstr.cur, &loan)) {
	logerror("do_loan: can't save loan");
	pr("Couldn't save loan; get help!\n");
	return RET_FAIL;
    }
    pr("You have offered loan %d\n", nstr.cur);
    wu(0, recipient, "Country #%d has offered you a loan (#%d)\n",
       player->cnum, nstr.cur);
    return RET_OK;
}
