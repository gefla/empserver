/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nat.h: Definitions for things having to do with nations
 *
 *  Known contributors to this file:
 *     Thomas Ruschak
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 *     Ron Koenderink, 2005-2008
 *     Markus Armbruster, 2005-2016
 */

#ifndef NAT_H
#define NAT_H

#include "file.h"
#include "types.h"

#define NATID_BAD 255

#define MAXNOR		50	/* max # realms */

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
    signed ef_type: 8;
    unsigned r_seqno: 12;
    unsigned r_generation: 12;
    int r_uid;			/* realm table index */
    time_t r_timestamp;		/* Last time this realm was touched */
    natid r_cnum;		/* country number */
    /* end of part matching struct empobj */
    unsigned short r_realm;	/* realm number */
    coord r_xl, r_xh;		/* horizontal bounds */
    coord r_yl, r_yh;		/* vertical bounds */
};

struct natstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned nat_seqno: 12;
    unsigned nat_generation: 12;
    int nat_uid;		/* equals nat_cnum */
    time_t nat_timestamp;
    natid nat_cnum;		/* our country number */
    /* end of part matching struct empobj */
    enum nat_status nat_stat;
    int nat_flags;		/* nation flags */
    char nat_cnam[20];		/* country name */
    char nat_pnam[20];		/* representative */
    char nat_hostaddr[46];	/* host addr of last user */
    char nat_userid[32];	/* userid of last user, may be empty */
    coord nat_xcap, nat_ycap;	/* cap location in abs coords */
    coord nat_xorg, nat_yorg;	/* origin location in abs coords */
    signed char nat_update;	/* Want an update or not. */
    unsigned short nat_tgms;	/* # of telegrams to be announced */
    unsigned short nat_ann;	/* # of annos pending */
    int nat_timeused;		/* number of seconds used today */
    short nat_btu;		/* bureaucratic time units */
    short nat_access;		/* The tick when BTUs were last updated */
    int nat_reserve;		/* military reserves */
    int nat_money;		/* moola */
    time_t nat_last_login;	/* time of last login, 0 menas never */
    time_t nat_last_logout;	/* time of last logout, 0 means never */
    time_t nat_newstim;		/* date news last read */
    time_t nat_annotim;		/* date annos last read */
    float nat_level[4];		/* technology, etc */
    unsigned char nat_relate[MAXNOC]; /* enum relations */
};

#define NAT_TLEV	0
#define NAT_RLEV	1
#define NAT_ELEV	2
#define NAT_HLEV	3

/* Diplomatic relations */
enum relations {
    /*
     * Don't change order without checking inequality comparisons and
     * array initializers!
     */
    AT_WAR,
    HOSTILE,
    NEUTRAL,
    FRIENDLY,
    ALLIED
};

/*
 * Number of updates contact lasts for various ways of making contact.
 * These are only useful with option LOSE_CONTACT, which implies
 * option HIDDEN.
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

struct contactstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned con_seqno: 12;
    unsigned con_generation: 12;
    int con_uid;
    time_t con_timestamp;
    /* end of part matching struct empobj */
    unsigned char con_contact[MAXNOC];
};

/* Kinds of communication players can reject */
enum rej_comm {
    REJ_TELE,			/* dont allow telegrams to be sent */
    REJ_ANNO,			/* don't receive announcements */
    REJ_LOAN			/* don't allow loans to be offered */
};

struct rejectstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned rej_seqno: 12;
    unsigned rej_generation: 12;
    int rej_uid;
    time_t rej_timestamp;
    /* end of part matching struct empobj */
    unsigned char rej_rejects[MAXNOC];
};

/* procedures relating to nation stuff */

#define putnat(p) ef_write(EF_NATION, (p)->nat_cnum, (p))
#define getnatp(n) ((struct natstr *)ef_ptr(EF_NATION, (n)))

#define getcontact(n, p) ef_read(EF_CONTACT, (n), (p))
#define putcontact(p) ef_write(EF_CONTACT, (p)->con_uid, (p))
#define getcontactp(n) ((struct contactstr *)ef_ptr(EF_CONTACT, (n)))

#define getreject(n, p) ef_read(EF_REJECT, (n), (p))
#define putreject(p) ef_write(EF_REJECT, (p)->rej_uid, (p))
#define getrejectp(n) ((struct rejectstr *)ef_ptr(EF_REJECT, (n)))

#define getrealm(r, n, p) ef_read(EF_REALM, ((r) + ((n) * MAXNOR)), (p))
#define putrealm(p) ef_write(EF_REALM, (p)->r_uid, (p))

/* src/lib/common/tfact.c */
extern double tfact(natid cn, double mult);
extern double techfact(int level, double mult);

/* src/lib/common/nat.c */
extern char *cname(natid n);
extern char *natstate(struct natstr *np);
extern enum relations getrel(struct natstr *np, natid them);
extern enum relations relations_with(natid, natid);
extern char *relations_string(enum relations);
extern int nat_accepts(natid, natid, enum rej_comm);
extern int in_contact(natid, natid);
extern void agecontact(struct natstr *np);
extern int influx(struct natstr *np);
extern struct natstr *nat_reset(struct natstr *, natid, char *, char *,
				enum nat_status);

/* src/lib/subs/natsub.c */
extern int check_nat_name(char *, natid);
extern char *prnat(struct natstr *);
extern char *prnatid(natid);

/* src/lib/common/btu.c */
extern int grant_btus(struct natstr *, int);

/* src/lib/subs/rej.c */
extern void setrel(natid, natid, enum relations);
extern void setcont(natid, natid, int);
extern void setrej(natid, natid, int, enum rej_comm);

/* nation flags */
#define NF_INFORM	bit(0)	/* Inform me of telegrams right away */
#define NF_FLASH	bit(1)	/* Allow other players to flash me (sicko :) */
#define NF_BEEP		bit(2)	/* Make beeping sounds when appropriate */
#define NF_COASTWATCH	bit(3)	/* Turn auto-coastwatch on */
#define NF_SONAR	bit(4)	/* Turn auto-sonar on */
#define NF_TECHLISTS	bit(5)	/* Sort lists by tech not type */
#define NF_SACKED	bit(6)	/* Capital was sacked, and hasn't been reset yet */

#endif
