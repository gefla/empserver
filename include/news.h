/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  news.h: Definitions for Empire News
 * 
 *  Known contributors to this file:
 *     Dave Pare
 *     Thomas Ruschak
 *     Steve McClure
 */

/*
 *	News codes for new options are not #ifdefed, since defining maxverb
 *	would be a pain, and it doesn't hurt to leave them in --ts
 */

#ifndef NEWS_H
#define NEWS_H

#include <time.h>
#include "nsc.h"

struct nwsstr {
    /* initial part must match struct empobj */
    short ef_type;
    short nws_uid;
    time_t nws_timestamp;
    /* end of part matching struct empobj */
    natid nws_ano;		/* "actor" country # */
    unsigned char nws_vrb;	/* action (verb) */
    natid nws_vno;		/* "victim" country # */
    signed char nws_ntm;	/* number of times */
    time_t nws_when;		/* time of action */
};

#define	NUM_RPTS	2	/* number of story alternates */

struct rptstr {
    signed char r_uid;
    int r_good_will;		/* how "nice" the action is */
    int r_newspage;		/* which page this item belongs on */
    char *r_newstory[NUM_RPTS];	/* texts for fmt( */
};

	/* news verbs */
#define	N_WON_SECT	1
#define	N_SCT_LOSE	2
#define	N_SPY_SHOT	3
#define	N_SENT_TEL	4
#define	N_SIGN_TRE	5
#define	N_MAKE_LOAN	6
#define	N_REPAY_LOAN	7
#define	N_MAKE_SALE	8
#define	N_OVFLY_SECT	9
#define	N_SCT_SHELL	10
#define	N_SHP_SHELL	11
#define	N_TOOK_UNOCC	12
#define	N_TORP_SHIP	13
#define	N_FIRE_BACK	14
#define	N_BROKE_SANCT	15
#define	N_SCT_BOMB	16
#define	N_SHP_BOMB	17
#define	N_BOARD_SHIP	18
#define	N_SHP_LOSE	19
/* unused		20 */
#define	N_SEIZE_SECT	21
#define	N_HONOR_TRE	22
#define	N_VIOL_TRE	23
/* unused		24 */
#define	N_HIT_MINE	25
#define	N_DECL_ALLY	26
/* unused		27 */
#define	N_DECL_WAR	28
#define	N_DIS_ALLY	29
#define	N_DIS_WAR	30
#define	N_OUT_PLAGUE	31
#define	N_DIE_PLAGUE	32
#define	N_NAME_CHNG	33
#define	N_DIE_FAMINE	34
/* unused		35 */
#define	N_DOWN_PLANE	36
#define	N_NUKE		37
#define N_FREEDOM_FIGHT	38
#define N_SHOOT_CIV	39
#define N_LAUNCH	40
#define N_SAT_KILL	41
#define N_GIFT		42
#define N_AIDS		43
#define N_HURTS		44
#define N_TAKE		45
#define N_NUKE_STOP     46
#define	N_SCT_MISS	47
#define	N_SHP_MISS	48
#define N_TRADE		49
#define N_PIRATE_TRADE	50
#define N_PIRATE_KEEP	51
#define	N_SHIP_TORP	52
#define N_SUB_BOMB	53
/* unused		54 */
#define N_UNIT_BOMB	55
#define	N_LHIT_MINE	56
#define	N_FIRE_F_ATTACK	57
#define	N_FIRE_L_ATTACK	58
#define	N_FIRE_S_ATTACK	59
#define N_SACK_CAP      60
#define N_UP_FRIENDLY   61
#define N_DOWN_FRIENDLY 62
#define N_UP_NEUTRAL    63
#define N_DOWN_NEUTRAL  64
#define N_UP_HOSTILE    65
#define N_DOWN_HOSTILE  66
#define N_SCT_SMISS     67
#define N_SHP_SMISS     68
#define N_START_COL     69
#define N_NUKE_SSTOP    70
#define N_LND_MISS      71
#define N_LND_SMISS     72
#define N_AWON_SECT	73
#define N_PWON_SECT	74
#define N_PARA_UNOCC	75
#define N_ALOSE_SCT	76
#define N_PLOSE_SCT	77
/* unused		78 */
/* unused		79 */
#define N_WELCH_DEAL    80
#define N_LND_LOSE      81
#define N_BOARD_LAND    82
#define	N_MAX_VERB	82

#define N_NOTUSED        0
#define N_FOR            1
#define N_FRONT          2
#define N_SEA            3
#define N_SKY            4
#define N_MISS           5
#define N_ARTY           6
#define N_ECON           7
#define N_COLONY         8
#define N_HOME           9
#define N_SPY           10
#define N_TELE          11
#define	N_MAX_PAGE	11

#define getnews(n, p) ef_read(EF_NEWS, (n), (p))
#define putnews(n, p) ef_write(EF_NEWS, (n), (p))
#define getnewsp(n) ((struct nwsstr *)ef_ptr(EF_NEWS, (n)))

extern struct rptstr rpt[N_MAX_VERB + 2];
extern struct symbol page_headings[N_MAX_PAGE + 2];

#endif
