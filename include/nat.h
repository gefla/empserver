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
 *  nat.h: Definitions for things having to do with nations
 *
 *  Known contributors to this file:
 *     Thomas Ruschak
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 *     Ron Koenderink, 2005-2008
 *     Markus Armbruster, 2005-2008
 */

#ifndef NAT_H
#define NAT_H

#include <time.h>
#include "types.h"

#define NATID_BAD 255

#define	MAXNOR		50	/* max # realms */

/* Nation status */
enum nat_status {
    /*
     * Don't change order without checking inequality comparisons and
     * array initializers!
     */
    STAT_UNUSED,		/* not in use */
    STAT_NEW,			/* just initialized */
    STAT_VIS,			/* visitor */
    STAT_SANCT,			/* still in sanctuary */
    STAT_ACTIVE,		/* active (sanctuary broken) */
    STAT_GOD			/* deity powers */
};


/*
 * TODO
 *
 * One of (r_cnum, r_realm) and r_uid is redundant, provided MAXNOR is known.
 *
 * The only user of r_cnum and r_realm appears to be xdump.
 * If we had working virtual selectors, we could remove r_cnum and r_realm.
 */
struct realmstr {
    /* initial part must match struct empobj */
    short ef_type;
    short r_uid;		/* realm table index */
    unsigned r_seqno;
    time_t r_timestamp;		/* Last time this realm was touched */
    natid r_cnum;		/* country number */
    /* end of part matching struct empobj */
    unsigned short r_realm;	/* realm number */
    coord r_xl, r_xh;		/* horizontal bounds */
    coord r_yl, r_yh;		/* vertical bounds */
};

struct natstr {
    /* initial part must match struct empobj */
    short ef_type;
    short nat_uid;		/* equals nat_cnum */
    unsigned nat_seqno;
    time_t nat_timestamp;
    natid nat_cnum;		/* our country number */
    /* end of part matching struct empobj */
    enum nat_status nat_stat;
    char nat_cnam[20];		/* country name */
    char nat_pnam[20];		/* representative */
    char nat_hostaddr[32];	/* host addr of last user */
    char nat_hostname[512];	/* hostname of last user, may be empty */
    char nat_userid[32];	/* userid of last user, may be empty */
    coord nat_xcap, nat_ycap;	/* cap location in abs coords */
    coord nat_xorg, nat_yorg;	/* origin location in abs coords */
    signed char nat_update;	/* Want an update or not. */
    unsigned short nat_tgms;	/* # of telegrams to be announced */
    unsigned short nat_ann;	/* # of annos pending */
    int nat_timeused;		/* number of seconds used today */
    short nat_btu;		/* bureaucratic time units */
    short nat_access;		/* The tick when BTUs were last updated */
    long nat_reserve;		/* military reserves */
    long nat_money;		/* moola */
    time_t nat_last_login;	/* time of last login, 0 menas never */
    time_t nat_last_logout;	/* time of last logout, 0 means never */
    time_t nat_newstim;		/* date news last read */
    time_t nat_annotim;		/* date annos last read */
    float nat_level[4];		/* technology, etc */
    unsigned char nat_relate[MAXNOC];
    unsigned char nat_contact[MAXNOC];
    unsigned char nat_rejects[MAXNOC];
    long nat_flags;		/* nation flags */
};

	/* nation relation codes */
#define	AT_WAR		0
#define SITZKRIEG       1
#define MOBILIZATION    2
#define	HOSTILE		3
#define	NEUTRAL		4
#define FRIENDLY        5
#define	ALLIED		6

	/* nation reject codes */
#define	REJ_TELE	bit(0)	/* dont allow telegrams to be sent */
#define	REJ_TREA	bit(1)	/* dont allow treaties to be offered */
#define	REJ_ANNO	bit(2)	/* don't receive announcements */
#define	REJ_LOAN	bit(3)	/* don't allow loans to be offered */

#define	NAT_TLEV	0
#define	NAT_RLEV	1
#define	NAT_ELEV	2
#define	NAT_HLEV	3

/*
 * Number of updates contact lasts for various ways of making contact.
 * These are only useful with option LOSE_CONTACT option, which
 * implies option HIDDEN.
 */
/* Planes spotting and being spotted */
#define FOUND_FLY	3
/* Lookout */
#define FOUND_LOOK	5
/* Spies and ground combat */
#define FOUND_SPY	6
/* Communication */
#define FOUND_TELE	3
/* Coastwatch and skywatch */
#define FOUND_COAST	3

extern char *relates[];

/* procedures relating to nation stuff */

#define putnat(p) ef_write(EF_NATION, (p)->nat_cnum, (p))
#define getnatp(n) ((struct natstr *)ef_ptr(EF_NATION, (n)))

#define getrealm(r, n, p) ef_read(EF_REALM, ((r) + ((n) * MAXNOR)), (p))
#define putrealm(p) ef_write(EF_REALM, (p)->r_uid, (p))

extern double tfact(natid cn, double mult);
extern double techfact(int level, double mult);

extern char *cname(natid n);
extern char *relatename(struct natstr *np, natid other);
extern char *rejectname(struct natstr *np, natid other);
extern char *natstate(struct natstr *np);
extern int getrel(struct natstr *np, natid them);
extern int getrejects(natid them, struct natstr *np);
extern int getcontact(struct natstr *np, natid them);
extern void putrel(struct natstr *np, natid them, int relate);
extern void putreject(struct natstr *np, natid them, int how, int what);
extern void putcontact(struct natstr *np, natid them, int contact);
extern void agecontact(struct natstr *np);
extern int influx(struct natstr *np);

extern struct natstr *nat_reset(struct natstr *, enum nat_status, coord, coord);
extern int check_nat_name(char *cname);

extern int grant_btus(struct natstr *, int );

/* nation flags */
#define NF_INFORM	bit(0)	/* Inform me of telegrams right away */
#define NF_FLASH	bit(1)	/* Allow other players to flash me (sicko :) */
#define NF_BEEP		bit(2)	/* Make beeping sounds when appropriate */
#define NF_COASTWATCH	bit(3)	/* Turn auto-coastwatch on */
#define NF_SONAR	bit(4)	/* Turn auto-sonar on */
#define NF_TECHLISTS    bit(5)	/* Sort lists by tech not type */
#define NF_SACKED       bit(6)	/* Capital was sacked, and hasn't been reset yet */

#endif
