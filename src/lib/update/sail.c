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
 *  sail.c: Sail ships during the update
 * 
 *  Known contributors to this file:
 *     Doug Hay
 *     Robert Forsman
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include "misc.h"
#include "var.h"
#include "sect.h"
#include "path.h"
#include "ship.h"
#include "var.h"
#include "news.h"
#include "file.h"
#include "nat.h"
#include "xy.h"
#include "nsc.h"
#include "update.h"
#include "subs.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>

static void fltp_to_list(struct fltheadstr *, struct emp_qelem *);

static void
cost_ship(struct shpstr *sp, struct fltelemstr *ep, struct fltheadstr *fp)
{
    double mobcost;
    int howfar;

    mobcost = 0.0;
    if (sp->shp_effic > 0) {
	mobcost = sp->shp_effic * sp->shp_speed * 0.01;
	mobcost = 480.0 / (mobcost * (1 + (50 + sp->shp_tech) /
				      (double)(200 + sp->shp_tech)));
    }
/* the next two lines are not necessary since shp_mobquota is unsigned
and therefore cannot be less than 0. 
	if (sp->shp_mobquota<0)
		sp->shp_mobquota=0;
*/

    howfar = 0;
    if (mobcost > 0) {
	howfar = (int)sp->shp_mobil - (int)sp->shp_mobquota;
	howfar = ceil((howfar / mobcost));
    }
    if (howfar < 0)
	howfar = 0;
#ifdef SAILDEBUG
    wu(0, fp->own,
       "Ship #%d can move %d spaces on mobility %d (cost/sect %f)\n",
       sp->shp_uid, howfar, sp->shp_mobil, mobcost);
#endif
    if ((unsigned int)howfar < fp->maxmoves)
	fp->maxmoves = howfar;

    ep->mobil = sp->shp_mobil;
    ep->mobcost = mobcost;
}

static int
sail_find_fleet(struct fltheadstr **head, struct shpstr *sp)
{
    struct fltheadstr *fltp;
    struct shpstr *ap;
    struct fltelemstr *this;
    int len = 0;
    int follow = -1;
    int stop;
    s_char *cp;

    if (sp->shp_own == 0)
	return (0);



    /* If this ship is following, find the head of the follow list. */
    for (ap = sp; ap; len++, ap = getshipp(follow)) {
	follow = ap->shp_follow;
	/* Not same owner */
	if (ap->shp_own != sp->shp_own) {
	    wu(0, sp->shp_own,
	       "Ship #%d, following #%d, which you don't own.\n",
	       sp->shp_uid, ap->shp_uid);
	    return (0);
	}
	/* Not a follower. */
	if (ap->shp_path[0] != 'f')
	    break;
	/* Following itself */
	if (follow == ap->shp_uid || follow == sp->shp_uid)
	    break;
    }
    if (!ap) {
	wu(0, sp->shp_own,
	   "Ship #%d, following #%d, which you don't own.\n",
	   sp->shp_uid, follow);
	return (0);
    }

    /* This should prevent infinite loops. */
    if (len >= 10) {
	wu(0, sp->shp_own,
	   "Ship #%d, too many follows (circular follow?).\n",
	   sp->shp_uid);
	return (0);
    }

    for (stop = 0, cp = ap->shp_path; (!stop) && (*cp); cp++) {
	switch (*cp) {
	case 'y':
	case 'u':
	case 'g':
	case 'j':
	case 'b':
	case 'n':
	case 'h':
	case 't':
	    break;
	default:
	    stop = 1;
	}
    }

    /* we found a non-valid char in the path. */
    if (*cp) {
	wu(0, ap->shp_own, "invalid char '\\%03o' in path of ship %d\n",
	   (unsigned char)*cp, ap->shp_uid);
	*cp = 0;
    }

    /* if this ship is not sailing anywhere then ignore it. */
    if (!*ap->shp_path)
	return (0);

    /* Find the fleet structure we belong to. */
    for (fltp = (*head); (fltp && fltp->leader != follow);
	 fltp = fltp->next) ;

    if (!fltp) {
	fltp = (struct fltheadstr *)malloc(sizeof(*fltp));
	memset(fltp, 0, sizeof(*fltp));

	/* Fix the links. */
	fltp->next = (*head);
	*head = fltp;

	/* Set the leader. */
	fltp->leader = ap->shp_uid;
	fltp->real_q = LEADER_REAL;
	fltp->x = ap->shp_x;
	fltp->y = ap->shp_y;
	fltp->own = ap->shp_own;
	fltp->maxmoves = 500;
    }

    /* If the fleet is not in the same sector as us, no go. */
    if ((fltp->x != sp->shp_x) || (fltp->y != sp->shp_y)) {
	wu(0, sp->shp_own,
	   "Ship %d not in same sector as its sailing fleet\n",
	   sp->shp_uid);
	fltp->real_q = LEADER_WRONGSECT;
	return (0);
    }

    this = (struct fltelemstr *)malloc(sizeof(*this));
    memset(this, 0, sizeof(*this));
    this->num = sp->shp_uid;
    this->own = sp->shp_own;
    this->next = fltp->head;
    fltp->head = this;
    cost_ship(sp, this, fltp);

    return 1;
}

static int
sail_nav_fleet(struct fltheadstr *fltp)
{
    struct fltelemstr *fe;
    struct shpstr *sp, ship;
    struct sctstr *sectp;
    int error = 0;
    s_char *s, *p;
    natid own;
    struct emp_qelem ship_list;
    int dir;

#ifdef SAILDEBUG
    switch (fltp->real_q) {
    case LEADER_VIRTUAL:
	s = "leaderless";
	break;
    case LEADER_REAL:
	s = "real";
	break;
    case LEADER_WRONGSECT:
	s = "scattered";
	break;
    default:
	s = "inconsistent";
    }
    wu(0, fltp->own,
       "Fleet lead by %d is %s, can go %d spaces\n  contains ships:",
       fltp->leader, s, fltp->maxmoves);
    for (fe = fltp->head; fe; fe = fe->next)
	wu(0, fltp->own, " %d", fe->num);
    wu(0, fltp->own, "\n");
#endif
    sectp = getsectp(fltp->x, fltp->y);
    switch (check_nav(sectp)) {
    case CN_NAVIGABLE:
	break;
    case CN_CONSTRUCTION:
    case CN_LANDLOCKED:
    default:
	wu(0, fltp->own, "Your fleet lead by %d is trapped by land.\n",
	   fltp->leader);
	return (0);
    }
    for (fe = fltp->head; fe; fe = fe->next) {
	sp = getshipp(fe->num);
	if (sp->shp_item[I_MILIT] == 0 && sp->shp_item[I_CIVIL] == 0) {
	    wu(0, fltp->own,
	       "   ship #%d (%s) is crewless and can't go on\n",
	       fe->num, cname(fe->own));
	    error = 1;
	}
    }
    if (error)
	return (0);
    sp = getshipp(fltp->leader);
    own = sp->shp_own;
    fltp_to_list(fltp, &ship_list);	/* hack -KHS 1995 */
    for (s = sp->shp_path; (*s) && (fltp->maxmoves > 0); s++) {
	dir = chkdir(*s, DIR_STOP, DIR_LAST);
	if (0 != (error = shp_nav_one_sector(&ship_list, dir, own, 0)))
	    fltp->maxmoves = 1;
	--(fltp->maxmoves);
    }
    shp_put(&ship_list, own);
    getship(sp->shp_uid, &ship);
    fltp->x = ship.shp_x;
    fltp->y = ship.shp_y;
    for (p = &ship.shp_path[0]; *s; p++, s++)
	*p = *s;
    *p = 0;
    putship(ship.shp_uid, &ship);
#ifdef SAILDEBUG
    if (sp->shp_path[0]) {
	wu(0, fltp->own,
	   "Fleet lead by #%d nav'd to %s, path left = %s\n",
	   fltp->leader, xyas(fltp->x, fltp->y, fltp->own), &sp->shp_path);
    } else
	wu(0, fltp->own,
	   "Fleet lead by #%d nav'd to %s, finished.\n",
	   fltp->leader, xyas(fltp->x, fltp->y, fltp->own));
    wu(0, sp->shp_own, "Ship #%d has %d mobility now.\n",
       fe->num, (int)fe->mobil);
#endif
    return 1;
}

void
sail_ship(natid cn)
{
    struct shpstr *sp;
    struct fltheadstr *head = 0;
    struct fltheadstr *fltp;
    int n;


    for (n = 0; NULL != (sp = getshipp(n)); n++)
	if (sp->shp_own == cn) {
	    sail_find_fleet(&head, sp);
	}

    /* see what the fleets fall out into */
    for (fltp = head; fltp; fltp = fltp->next) {
	sail_nav_fleet(fltp);
	wu(0, fltp->own, "Your fleet lead by ship #%d has reached %s.\n",
	   fltp->leader, xyas(fltp->x, fltp->y, fltp->own));
    }

    /* Free up the memory, 'cause I want to. */
    for (fltp = head; fltp != 0;) {
	struct fltelemstr *fe;
	struct fltheadstr *saveh;
	saveh = fltp->next;
	for (fe = fltp->head; fe != 0;) {
	    struct fltelemstr *saveel;
	    saveel = fe->next;
	    free(fe);
	    fe = saveel;
	}
	free(fltp);
	fltp = saveh;
    }
}

/* The following is a total hack by Ken Stevens to cut down dramatically on repeated code 1995 */

static void
fltp_to_list(struct fltheadstr *fltp, struct emp_qelem *list)
{
    struct fltelemstr *fe;
    struct mlist *mlp;
    struct shpstr *sp;

    emp_initque(list);
    for (fe = fltp->head; fe; fe = fe->next) {
	mlp = (struct mlist *)malloc(sizeof(struct mlist));
	sp = getshipp(fe->num);
	mlp->mcp = mchr + sp->shp_type;
	mlp->ship = *sp;
	mlp->mobil = fe->mobil;
	emp_insque(&mlp->queue, list);
    }
}
