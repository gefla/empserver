/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  cons.c: Consider a loan or treaty
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "loan.h"
#include "nat.h"
#include "treaty.h"
#include "news.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

/*
 * Things common to a loan or treaty.
 */
struct ltcomstr {
    int type;			/* EF_LOAN or EF_TREATY */
    int num;			/* number */
    s_char *name;		/* "loan" or "treaty" */
    s_char *Name;		/* "Loan" or "Treaty" */
    natid proposer;		/* country offering */
    natid proposee;		/* country offered to */
    natid mailee;		/* who gets mail about it */
    s_char op;			/* 'a', 'd', or 'p' */
    union {
	struct lonstr l;	/* the loan */
	struct trtstr t;	/* the treaty */
    } u;
};

static int cons_choose(struct ltcomstr *ltcp);
static int treaty_accept(struct ltcomstr *ltcp);
static int treaty_decline(struct ltcomstr *ltcp);
static int loan_accept(struct ltcomstr *ltcp);
static int loan_decline(struct ltcomstr *ltcp);
static int postpone(struct ltcomstr *ltcp);
static void accpt(struct ltcomstr *ltcp);
static void decline(struct ltcomstr *ltcp);
static void late(struct ltcomstr *ltcp);
static void prev_signed(struct ltcomstr *ltcp);

int
cons(void)
{
    int rv;
    struct ltcomstr ltc;

    rv = cons_choose(&ltc);
    if (rv != RET_OK)
	return rv;

    switch (ltc.op) {
    case 'a':
	rv = (ltc.type == EF_TREATY) ? treaty_accept(&ltc)
	    : loan_accept(&ltc);
	break;
    case 'd':
	rv = (ltc.type == EF_TREATY) ? treaty_decline(&ltc)
	    : loan_decline(&ltc);
	break;
    case 'p':
	rv = postpone(&ltc);
	break;
    default:
	pr("Bad operation %c from cons_choose; get help!\n", ltc.op);
	break;
    };

    return rv;
}

/*
 * Choose whether we want to accept, decline, or postpone a
 * loan or treaty.  Put all the goodies in ltcp, and return
 * RET_OK if all goes well, and anything else on error.
 */
static int
cons_choose(struct ltcomstr *ltcp)
{
    s_char *p;
    struct lonstr *lp;
    struct trtstr *tp;
    s_char prompt[128];
    s_char buf[1024];

    memset(ltcp, 0, sizeof(*ltcp));
    if (getstarg(player->argp[1], "loan or treaty? ", buf) == 0)
	return RET_SYN;
    ltcp->type = ef_byname(buf);
    switch (ltcp->type) {
    case EF_TREATY:
	if (!opt_TREATIES) {
	    pr("Treaties are not enabled.\n");
	    return RET_FAIL;
	}
	ltcp->name = "treaty";
	ltcp->Name = "Treaty";
	break;
    case EF_LOAN:
	if (!opt_LOANS) {
	    pr("Loans are not enabled.\n");
	    return RET_FAIL;
	}
	ltcp->name = "loan";
	ltcp->Name = "Loan";
	break;
    default:
	pr("You must specify \"loan\" or \"treaty\".\n");
	return RET_SYN;
    }
    sprintf(prompt, "%s number? ", ltcp->Name);
    if ((ltcp->num = onearg(player->argp[2], prompt)) < 0)
	return RET_SYN;
    if (!ef_read(ltcp->type, ltcp->num, &ltcp->u) ||
	!(ltcp->type == EF_TREATY
	  ? distrea(ltcp->num, &ltcp->u.t)
	  : disloan(ltcp->num, &ltcp->u.l))) {
	pr("%s #%d is not being offered to you!\n", ltcp->Name, ltcp->num);
	return RET_SYN;
    }
    switch (ltcp->type) {
    case EF_LOAN:
	lp = &ltcp->u.l;
	if (lp->l_status == LS_SIGNED) {
	    pr("That loan has already been accepted!\n");
	    return RET_FAIL;
	}
	ltcp->proposer = lp->l_loner;
	ltcp->proposee = lp->l_lonee;
	break;
    case EF_TREATY:
	tp = &ltcp->u.t;
	if (tp->trt_status == TS_SIGNED) {
	    pr("That treaty has already been accepted!\n");
	    return RET_FAIL;
	}
	ltcp->proposer = tp->trt_cna;
	ltcp->proposee = tp->trt_cnb;
	break;
    }
    ltcp->mailee = (ltcp->proposer == player->cnum)
	? ltcp->proposee : ltcp->proposer;
    while ((p =
	    getstarg(player->argp[3], "Accept, decline or postpone? ",
		     buf)) && *p) {
	if (*p == 'a' || *p == 'd' || *p == 'p')
	    break;
	player->argp[3] = 0;
    }
    if (p == 0 || *p == 0)
	return RET_SYN;
    ltcp->op = *p;
    return RET_OK;
}

/*
 * Accept a loan.  If the offering country has too little money,
 * leave him $100 left and offer the rest.  Return RET_OK on
 * success, anything else on error.
 */
static int
loan_accept(struct ltcomstr *ltcp)
{
    struct lonstr *lp;
    struct natstr *lender;
    struct nstr_item nstr;
    struct lonstr loan;

    lp = &ltcp->u.l;
    if (ltcp->proposee != player->cnum) {
	pr("%s %d is still pending.\n", ltcp->Name, ltcp->num);
	return RET_OK;
    }
    if (!getloan(ltcp->num, lp)) {
	pr("loan_accept: can't read loan; get help!\n");
	return RET_SYS;
    }
    if (lp->l_status == LS_FREE) {	/* other guy retratcted already */
	late(ltcp);
	return RET_OK;
    }
    if (lp->l_status == LS_SIGNED) {	/* already signed somehow */
	prev_signed(ltcp);
	return RET_OK;
    }
    /* check to see if a loan already exists */
    snxtitem_all(&nstr, EF_LOAN);
    while (nxtitem(&nstr, (s_char *)&loan)) {
	if (loan.l_status == LS_SIGNED && loan.l_lonee == lp->l_loner
	    && (loan.l_loner == lp->l_lonee)) {
	    pr("He already owes you money - make him repay his loan!\n");
	    return RET_OK;
	}
    }
    lender = getnatp(ltcp->proposer);
    if (lender->nat_money < lp->l_amtdue) {	/* other guy is poor */
	lp->l_amtdue = lender->nat_money - 100;
	pr("%s no longer has the funds.\n", cname(ltcp->proposer));
	if (lp->l_amtdue <= 0)
	    return RET_FAIL;
	pr("You may borrow $%ld at the same terms.\n", lp->l_amtdue);
    }
    lender->nat_money -= lp->l_amtdue;
    putnat(lender);
    player->dolcost -= lp->l_amtdue;
    lp->l_amtpaid = 0;
    (void)time(&lp->l_lastpay);
    lp->l_duedate = lp->l_ldur * 86400 + lp->l_lastpay;
    lp->l_status = LS_SIGNED;
    if (!putloan(ltcp->num, (s_char *)lp)) {
	pr("Problem writing lp->to disk; get help!\n");
	return RET_FAIL;
    }
    accpt(ltcp);
    pr("You are now $%ld richer (sort of).\n", lp->l_amtdue);
    return RET_OK;
}

/*
 * Declne a loan.  Return RET_OK on success, anything else on error.
 */
static int
loan_decline(struct ltcomstr *ltcp)
{
    struct lonstr *lp;

    lp = &ltcp->u.l;
    if (!getloan(ltcp->num, lp)) {
	pr("Decline: can't read loan; get help!\n");
	return RET_SYS;
    }
    /* loan got accepted somehow between now and last time we checked */
    if (lp->l_status == LS_SIGNED) {
	late(ltcp);
	return RET_OK;
    }
    lp->l_status = LS_FREE;
    if (!putloan(ltcp->num, lp)) {
	pr("loan_decline: can't write loan; get help!\n");
	return RET_SYS;
    }
    decline(ltcp);
    return RET_OK;
}

/*
 * Accept a treaty.  Return RET_OK on success, anything else on error.
 */
static int
treaty_accept(struct ltcomstr *ltcp)
{
    struct trtstr *tp;

    tp = &ltcp->u.t;
    if (ltcp->proposee != player->cnum) {
	pr("%s %d is still pending.\n", ltcp->Name, ltcp->num);
	return RET_OK;
    }
    if (!gettre(ltcp->num, tp)) {
	pr("Accept: can't read treaty; get help!\n");
	return RET_SYS;
    }
    if (tp->trt_status == TS_FREE) {	/* treaty offer withdrawn */
	late(ltcp);
	return RET_OK;
    }
    if (tp->trt_status == TS_SIGNED) {	/* somehow got accepted */
	prev_signed(ltcp);
	return RET_OK;
    }
    tp->trt_status = TS_SIGNED;
    if (!puttre(ltcp->num, tp)) {
	pr("Problem saving treaty; get help!\n");
	return RET_SYS;
    }
    accpt(ltcp);
    pr("Treaty in effect until %s", ctime(&tp->trt_exp));
    return RET_OK;
}

/*
 * Decline a treaty.  Return RET_OK on success, anything else on error.
 */
static int
treaty_decline(struct ltcomstr *ltcp)
{
    struct trtstr *tp;

    tp = &ltcp->u.t;
    if (!gettre(ltcp->num, tp)) {
	pr("Decline: can't read treaty; get help!\n");
	return RET_SYS;
    }
    /* treaty got signed somehow between now and last time we read it */
    if (tp->trt_status == TS_SIGNED) {
	late(ltcp);
	return RET_OK;
    }
    tp->trt_status = TS_FREE;
    if (!puttre(ltcp->num, tp)) {
	pr("Problem saving treaty; get help!\n");
	return RET_SYS;
    }
    decline(ltcp);
    return RET_OK;
}

/*
 * Postpone a treaty; always succeeds.
 */
static int
postpone(struct ltcomstr *ltcp)
{
    pr("%s %d is still pending.\n", ltcp->Name, ltcp->num);
    if (ltcp->proposee == player->cnum)
	wu(0, ltcp->proposer, "%s %d considered by %s\n",
	   ltcp->name, ltcp->num, cname(player->cnum));
    return RET_OK;
}

/*
 * Somebody tried to accept a loan/treaty that was retracted,
 * or to decline a loan/treaty they already signed.
 */
static void
late(struct ltcomstr *ltcp)
{
    pr("Too late; that %s %s!\n", ltcp->name,
       (ltcp->op == 'a') ? "is no longer being offered"
       : "has already been accepted");
}

/*
 * Loan or treaty was previously signed.
 */
static void
prev_signed(struct ltcomstr *ltcp)
{
    pr("%s #%d is already in effect.\n", ltcp->Name, ltcp->num);
}

/*
 * Post-processing after successful declination of loan or treaty.
 * Notify the folks involved.
 */
static void
decline(struct ltcomstr *ltcp)
{
    if (ltcp->proposee == player->cnum) {
	wu(0, ltcp->proposer, "%s %d refused by %s\n",
	   ltcp->Name, ltcp->num, cname(player->cnum));
	pr("%s %d refused.\n", ltcp->Name, ltcp->num);
    } else {
	wu(0, ltcp->proposee,
	   "%s offer %d retracted by %s\n",
	   ltcp->Name, ltcp->num, cname(player->cnum));
	pr("%s offer %d retracted.\n", ltcp->Name, ltcp->num);
    }
}

/*
 * Post-processing after successful acceptance of loan or treaty.
 * Notify the press, and the folks involved.
 * (Weird spelling is to avoid accept(2)).
 */
static void
accpt(struct ltcomstr *ltcp)
{
    if (ltcp->type == EF_LOAN)
	nreport(ltcp->proposer, N_MAKE_LOAN, player->cnum, 1);
    else
	nreport(player->cnum, N_SIGN_TRE, ltcp->mailee, 1);
    wu(0, ltcp->mailee, "%s #%d accepted by %s\n",
       ltcp->Name, ltcp->num, cname(player->cnum));
}
