/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sona.c: Sonar from a sub (or other sonar-equipped ship)
 *
 *  Known contributors to this file:
 *     Jim Griffith, 1989
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "commands.h"
#include "map.h"
#include "optlist.h"
#include "path.h"
#include "plane.h"
#include "retreat.h"
#include "ship.h"

static int blankrow(char *);

int
sona(void)
{
    struct nstr_item ni, nit;
    struct sctstr sect;
    struct shpstr ship;
    struct shpstr targ;
    struct natstr *natp;
    struct mchrstr *mcp;
    struct mchrstr *tmcp;
    struct nstr_sect ns;
    int range;
    int visib, pingrange;
    int srange;
    int vrange;
    int dist;
    int x, y;
    int cx, cy;
    int changed = 0;
    int row;
    /* Where these are used are non-re-entrant, so we keep 'em around */
    static char **rad = NULL;
    static char *radbuf = NULL;
    static signed char **vis = NULL;
    static signed char *visbuf = NULL;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;
    if (!radbuf)
	radbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!visbuf)
	visbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!rad && radbuf) {
	rad = malloc(WORLD_Y * sizeof(char *));
	if (rad) {
	    for (x = 0; x < WORLD_Y; x++) {
		rad[x] = &radbuf[(WORLD_X + 1) * x];
	    }
	}
    }
    if (!vis && visbuf) {
	vis = malloc(WORLD_Y * sizeof(signed char *));
	if (vis) {
	    for (x = 0; x < WORLD_Y; x++) {
		vis[x] = &visbuf[(WORLD_X + 1) * x];
	    }
	}
    }
    if (!radbuf || !visbuf || !rad || !vis) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in sona\n");
	return RET_FAIL;
    }
    while (nxtitem(&ni, &ship)) {
	if (!player->owner)
	    continue;
	mcp = &mchr[(int)ship.shp_type];
	if (!(mcp->m_flags & M_SONAR))
	    continue;
	getsect(ship.shp_x, ship.shp_y, &sect);
	if (sect.sct_type != SCT_WATER)
	    continue;
	range = (int)techfact(ship.shp_tech, mcp->m_vrnge);
	srange = MIN(7, 7 * range * ship.shp_effic / 200);
	pr("%s at %s efficiency %d%%, max range %d\n",
	   prship(&ship),
	   xyas(ship.shp_x, ship.shp_y, player->cnum),
	   ship.shp_effic, srange);
	snxtsct_dist(&ns, ship.shp_x, ship.shp_y, srange);
	blankfill(radbuf, &ns.range, 1);
	while (nxtsct(&ns, &sect)) {
	    if (player->owner || sect.sct_type == SCT_WATER)
		rad[ns.dy][ns.dx] = dchr[sect.sct_type].d_mnem;
	    else {
		rad[ns.dy][ns.dx] = '?';
	    }
	}
	snxtsct_dist(&ns, ship.shp_x, ship.shp_y, srange);
	cx = deltx(&ns.range, ship.shp_x);
	cy = delty(&ns.range, ship.shp_y);
	while (nxtsct(&ns, &sect)) {
	    if (!line_of_sight(rad, cx, cy, ns.dx, ns.dy)) {
		rad[ns.dy][ns.dx] = ' ';
		continue;
	    }
	    if (ship.shp_tech >= 310 && sect.sct_type == SCT_WATER) {
		if (sect.sct_mines) {
		    pr("Sonar detects %d mines in %s!\n",
		       sect.sct_mines,
		       xyas(sect.sct_x, sect.sct_y, player->cnum));
		    rad[ns.dy][ns.dx] = 'X';
		}
	    }
	    changed |= map_set(player->cnum, sect.sct_x, sect.sct_y,
			       rad[ns.dy][ns.dx], 0);

	}
	memset(visbuf, 0, (WORLD_Y * (WORLD_X + 1)));
	snxtitem_dist(&nit, EF_SHIP, ship.shp_x, ship.shp_y, range);
	while (nxtitem(&nit, &targ)) {
	    if (targ.shp_own == player->cnum || targ.shp_own == 0)
		continue;
	    tmcp = &mchr[(int)targ.shp_type];
	    visib = shp_visib(&targ);
	    pingrange = MIN(7, MAX(visib, 10) * range / 10);
	    vrange = pingrange * ship.shp_effic / 200;
	    dist = mapdist(targ.shp_x, targ.shp_y, ship.shp_x, ship.shp_y);
	    pingrange = (MAX(pingrange, 2) * targ.shp_effic) / 100;
	    if (dist > pingrange)
		continue;
	    if (tmcp->m_flags & M_SONAR && targ.shp_own) {
		natp = getnatp(targ.shp_own);
		if (natp->nat_flags & NF_SONAR)
		    wu(0, targ.shp_own,
		       "Sonar ping from %s detected by %s!\n",
		       xyas(ship.shp_x, ship.shp_y,
			    targ.shp_own), prship(&targ));
		if (targ.shp_rflags & RET_SONARED) {
		    retreat_ship(&targ, 's');
		    putship(targ.shp_uid, &targ);
		}
	    }
	    if (dist > vrange)
		continue;
	    x = deltx(&ns.range, (int)targ.shp_x);
	    y = delty(&ns.range, (int)targ.shp_y);
	    if (rad[y][x] != dchr[SCT_WATER].d_mnem && rad[y][x] != 'X')
		continue;
	    if (tmcp->m_flags & M_SUB &&
		relations_with(targ.shp_own, player->cnum) < FRIENDLY) {
		if (mcp->m_vrnge + visib < 8)
		    pr("Sonar detects sub #%d @ %s\n",
		       targ.shp_uid,
		       xyas(targ.shp_x, targ.shp_y, player->cnum));
		else if (mcp->m_vrnge + visib < 10)
		    pr("Sonar detects %s @ %s\n",
		       prship(&targ),
		       xyas(targ.shp_x, targ.shp_y, player->cnum));
		else
		    pr("Sonar detects %s %s @ %s\n",
		       cname(targ.shp_own), prship(&targ),
		       xyas(targ.shp_x, targ.shp_y, player->cnum));
	    } else
		pr("Sonar detects %s %s @ %s\n",
		   cname(targ.shp_own), prship(&targ),
		   xyas(targ.shp_x, targ.shp_y, player->cnum));

	    if (visib > vis[y][x]) {
		vis[y][x] = visib;
		/* &~0x20 makes it a cap letter */
		rad[y][x] = (*mchr[(int)targ.shp_type].m_name) & ~0x20;
	    }
	}
	if (!player->argp[2]) {
	    rad[cy][cx] = '0';
	    for (row = 0; row < ns.range.height; row++)
		if (!blankrow(rad[row]))
		    pr("%s\n", rad[row]);
	}
	pr("\n");

    }
    if (changed)
	writemap(player->cnum);
    return RET_OK;
}

void
plane_sona(struct emp_qelem *plane_list, int x, int y,
	   struct shiplist **head)
{
    struct plnstr *pp;
    struct plchrstr *pcp;
    struct mchrstr *tmcp;
    struct shpstr *targ, s;
    struct natstr *natp;
    struct emp_qelem *qp;
    struct emp_qelem *next;
    struct plist *ip;
    struct sctstr sect;
    int found = 0;
    int range, i, vis;
    int pingrange;
    int vrange;
    int dist;

    getsect(x, y, &sect);
    if ((sect.sct_type != SCT_WATER) && (sect.sct_type != SCT_HARBR))
	return;
    for (qp = plane_list->q_forw; qp != plane_list; qp = next) {
	next = qp->q_forw;
	ip = (struct plist *)qp;
	pp = &ip->plane;
	pcp = ip->pcp;
	if (!(pcp->pl_flags & P_A))	/* if it isn't an ASW plane */
	    continue;
	range = (int)techfact(pp->pln_tech, (100.0 - pln_acc(pp)) / 10.0);
	for (i = 0; getship(i, &s); i++) {
	    targ = &s;
	    if (targ->shp_own == pp->pln_own || targ->shp_own == 0)
		continue;
	    if (on_shiplist(targ->shp_uid, *head))
		continue;
	    tmcp = &mchr[(int)targ->shp_type];
	    if (!(tmcp->m_flags & M_SUB))
		continue;
	    if (roll(100) > pln_identchance(pp, shp_hardtarget(targ),
					    EF_SHIP))
		continue;
	    vis = shp_visib(targ);
	    pingrange = MAX(vis, 10) * range / 10;
	    vrange = pingrange * (pp->pln_effic / 200.0);
	    dist = mapdist(targ->shp_x, targ->shp_y, x, y);
	    pingrange = (MAX(pingrange, 2) * targ->shp_effic);
	    pingrange = roundavg(pingrange / 100.0);
	    if (dist > pingrange)
		continue;
	    if (tmcp->m_flags & M_SONAR && targ->shp_own) {
		natp = getnatp(targ->shp_own);
		if (natp->nat_flags & NF_SONAR)
		    wu(0, targ->shp_own,
		       "Sonar ping from %s detected by %s!\n",
		       xyas(x, y, targ->shp_own), prship(targ));
	    }
	    if ((dist > vrange))
		continue;
	    add_shiplist(targ->shp_uid, head);
	    if (!found) {
		pr("Sonar contact in %s\n", xyas(x, y, player->cnum));
		found = 1;
	    }
	    if (relations_with(targ->shp_own, pp->pln_own) < FRIENDLY &&
		roll(100) > pln_identchance(pp, shp_hardtarget(targ),
					    EF_SHIP))
		if (roll(100) > pln_identchance(pp, shp_hardtarget(targ),
						EF_SHIP))
		    pr("sub #%d %s\n", targ->shp_uid,
		       xyas(targ->shp_x, targ->shp_y, player->cnum));
		else
		    pr("%s %s\n", prship(targ),
		       xyas(targ->shp_x, targ->shp_y, player->cnum));
	    else
		pr("%s %s @ %s\n", cname(targ->shp_own), prship(targ),
		   xyas(targ->shp_x, targ->shp_y, player->cnum));
	}
    }
    if (found)
	pr("\n");
}

/*
 * line_of_sight() - is there a "straight" all water path from (x,y) to (tx,ty)
 * Ken & Irina Stevens, 1995
 */

#define DOT(ax,ay,bx,by) ((ax)*(bx) + (ay)*(by))
#define LEN(x,y) ((x)*(x) + (y)*(y))
#define DIST(ax,ay,bx,by) LEN(bx - ax, by -ay)

int
line_of_sight(char **rad, int ax, int ay, int bx, int by)
{
    int dxn = XNORM(bx - ax);
    int dyn = YNORM(by - ay);
    int dx = dxn > WORLD_X / 2 ? dxn - WORLD_X : dxn;
    int dy = dyn > WORLD_Y / 2 ? dyn - WORLD_Y : dyn;
    int dlen = LEN(dx, dy);
    int n;
    int cx = 0;
    int cy = 0;
    int tx, ty;			/* test point */
    double cd_dist = dlen;	/* closest distance from c to vector d */
    double md_dist;		/* minimum distance from t to vector d */
    double td_dist;		/* distance from t to vector d */
    double td_proj;		/* the projection of t onto vector d */
    int closest;		/* index of closest */
    int blocked = 0;
    struct sctstr *sectp;

    while (cd_dist) {
	if (blocked)
	    return 0;
	md_dist = 100;		/* will always be <= 2 */
	closest = -1;
	for (n = 1; n <= 6; ++n) {	/* Directions */
	    tx = cx + diroff[n][0];
	    ty = cy + diroff[n][1];
	    if (DIST(tx, ty, dx, dy) >= cd_dist)
		continue;
	    td_proj = (double)DOT(tx, ty, dx, dy) / dlen;
	    td_dist = DIST(tx, ty, td_proj * dx, td_proj * dy);
	    if (td_dist < md_dist) {
		md_dist = td_dist;
		closest = n;
	    }
	}
	if (CANT_HAPPEN(closest < 0))
	    return 0;
	cx = cx + diroff[closest][0];
	cy = cy + diroff[closest][1];
	if (rad) {
	    blocked = (rad[YNORM(ay + cy)][XNORM(ax + cx)]
		       != dchr[SCT_WATER].d_mnem);
	} else {
	    sectp = getsectp((ax + cx), (ay + cy));
	    if (sectp) {
		if (sectp->sct_type == SCT_WATER ||
		    sectp->sct_type == SCT_BSPAN) {
		    blocked = 0;
		} else {
		    blocked = 1;
		}
	    }
	}
	cd_dist = DIST(cx, cy, dx, dy);
    }
    return 1;
}

static int
blankrow(char *s)
{
    while (*s) {
	if (*s != ' ')
	    return 0;
	++s;
    }
    return 1;
}
