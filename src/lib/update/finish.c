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
 *  finish.c: Finish the update
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1993
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2011
 */

#include <config.h>

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <sys/resource.h>
#include "distribute.h"
#include "path.h"
#include "update.h"

static void assemble_dist_paths(double *);

void
finish_sects(int etu)
{
    static double *import_cost;
    struct sctstr *sp;
    struct natstr *np;
    int n;
    struct rusage rus1, rus2;

    if (import_cost == NULL) {
	logerror("First update since reboot, allocating buffer\n");
	import_cost = malloc(WORLD_SZ() * sizeof(*import_cost));
	if (import_cost == NULL) {
	    logerror("malloc failed in finish_sects.\n");
	    return;
	}
    }

    logerror("delivering...\n");
    /* Do deliveries */
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (sp->sct_type == SCT_WATER)
	    continue;
	if (sp->sct_own == 0)
	    continue;
	np = getnatp(sp->sct_own);
	if (np->nat_money < 0)
	    continue;
	dodeliver(sp);
    }
    logerror("done delivering\n");

    logerror("assembling paths...\n");
    getrusage(RUSAGE_SELF, &rus1);

    /* First, enable the best_path cacheing */
    bp_enable_cachepath();

    /* Now assemble the paths */
    assemble_dist_paths(import_cost);

    /* Now disable the best_path cacheing */
    bp_disable_cachepath();

    /* Now, clear the best_path cache that may have been created */
    bp_clear_cachepath();

    getrusage(RUSAGE_SELF, &rus2);
    logerror("done assembling paths %g user %g system",
	     rus2.ru_utime.tv_sec + rus2.ru_utime.tv_usec / 1e6
	     - (rus1.ru_utime.tv_sec + rus1.ru_utime.tv_usec / 1e6),
	     rus2.ru_stime.tv_sec + rus2.ru_stime.tv_usec / 1e6
	     - (rus1.ru_stime.tv_sec + rus1.ru_stime.tv_usec / 1e6));

    logerror("exporting...");
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (!sp->sct_own)
	    continue;
	np = getnatp(sp->sct_own);
	if (np->nat_money < 0)
	    continue;
	dodistribute(sp, EXPORT, import_cost[n]);
    }
    logerror("done exporting\n");

    logerror("importing...");
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (!sp->sct_own)
	    continue;
	np = getnatp(sp->sct_own);
	if (np->nat_money < 0)
	    continue;
	dodistribute(sp, IMPORT, import_cost[n]);
	sp->sct_off = 0;
    }
    logerror("done importing\n");

}

#if (defined(USE_PATH_FIND) || defined(TEST_PATH_FIND)) && !defined(DIST_PATH_NO_REUSE)
static int
distcmp(const void *p, const void *q)
{
    int a = *(int *)p;
    int b = *(int *)q;
    struct sctstr *sp = (void *)empfile[EF_SECTOR].cache;
    int d;

    d = sp[b].sct_dist_y - sp[a].sct_dist_y;
    if (d)
	return d;
    d = sp[b].sct_dist_x - sp[a].sct_dist_x;
    if (d)
	return d;
    return b - a;
}
#endif

static void
assemble_dist_paths(double *import_cost)
{
    struct sctstr *sp;
    struct sctstr *dist;
    int n;
#if defined(USE_PATH_FIND) || defined(TEST_PATH_FIND)
    static int *job;
    int uid, i;
    coord dx = 1, dy = 0;	/* invalid */

#ifdef DIST_PATH_NO_REUSE
    for (uid = 0; NULL != (sp = getsectid(uid)); uid++) {
	import_cost[uid] = -1;
	if (sp->sct_dist_x == sp->sct_x && sp->sct_dist_y == sp->sct_y)
	    continue;
#else
    if (!job)
	job = malloc(WORLD_SZ() * sizeof(*job));

    n = 0;
    for (uid = 0; NULL != (sp = getsectid(uid)); uid++) {
	import_cost[uid] = -1;
	if (sp->sct_dist_x == sp->sct_x && sp->sct_dist_y == sp->sct_y)
	    continue;
	job[n++] = uid;
    }

#ifdef PATH_FIND_STATS
    printf("dist path reuse %zu bytes, %d/%d used\n",
	   WORLD_SZ() * sizeof(*job), n, WORLD_SZ());
#endif

    qsort(job, n, sizeof(*job), distcmp);

    for (i = 0; i < n; i++) {
	uid = job[i];
#endif	/* !DIST_PATH_NO_REUSE */
	sp = getsectid(uid);
	dist = getsectp(sp->sct_dist_x, sp->sct_dist_y);
	if (CANT_HAPPEN(!dist))
	    continue;
	if (sp->sct_own != dist->sct_own)
	    continue;
#ifdef DIST_PATH_NO_REUSE
	import_cost[uid] = path_find(sp->sct_dist_x, sp->sct_dist_y,
				     sp->sct_x, sp->sct_y, dist->sct_own,
				     MOB_MOVE);
#else
	if (sp->sct_dist_x != dx || sp->sct_dist_y != dy) {
	    dx = sp->sct_dist_x;
	    dy = sp->sct_dist_y;
	    path_find_from(dx, dy, dist->sct_own, MOB_MOVE);
	}
	import_cost[uid] = path_find_to(sp->sct_x, sp->sct_y);
#endif
    }
#endif	/* USE_PATH_FIND || TEST_PATH_FIND */
#if !defined(USE_PATH_FIND) || defined(TEST_PATH_FIND)
    char *path;
    double d;
    char buf[512];

    for (n = 0; NULL != (sp = getsectid(n)); n++) {
#ifdef TEST_PATH_FIND
	double new_imc = import_cost[n];
#endif
	import_cost[n] = -1;
	if ((sp->sct_dist_x == sp->sct_x) && (sp->sct_dist_y == sp->sct_y))
	    continue;
	dist = getsectp(sp->sct_dist_x, sp->sct_dist_y);
	if (CANT_HAPPEN(!dist))
	    continue;
	if (sp->sct_own != dist->sct_own)
	    continue;
	/* Now, get the best distribution path over roads */
	/* Note we go from the dist center to the sector.  This gives
	   us the import path for that sector. */
	path = BestDistPath(buf, dist, sp, &d);
	if (path)
	    import_cost[n] = d;
#ifdef TEST_PATH_FIND
	if (fabs(import_cost[n] - new_imc) >= 1e-6) {
	    printf("%d,%d <- %d,%d %d: old %g, new %g, %g off\n",
		   sp->sct_dist_x, sp->sct_dist_y,
		   sp->sct_x, sp->sct_y, MOB_MOVE,
		   import_cost[n], new_imc, import_cost[n] - new_imc);
	    printf("\told: %s\n", path);
	    d = path_find(sp->sct_dist_x, sp->sct_dist_y,
			  sp->sct_x, sp->sct_y, dist->sct_own, MOB_MOVE);
	    assert(d - new_imc < 1e-6);
	    path_find_route(buf, sizeof(buf),
			    sp->sct_dist_x, sp->sct_dist_y,
			    sp->sct_x, sp->sct_y);
	    printf("\tnew: %s\n", buf);
	}
#endif	/* TEST_PATH_FIND */
    }
#endif	/* !USE_PATH_FIND || TEST_PATH_FIND */
#if defined(USE_PATH_FIND) || defined(TEST_PATH_FIND)
    path_find_print_stats();
#endif
}
