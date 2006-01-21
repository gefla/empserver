/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  finish.c: Finish the update
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1993
 *     Steve McClure, 1998
 */

#include <config.h>

#include "misc.h"
#include "sect.h"
#include "nat.h"
#include "file.h"
#include "xy.h"
#include "path.h"
#include "distribute.h"
#include "update.h"
#include "common.h"
#include "optlist.h"

/* Used for building up distribution info */
struct distinfo {
    s_char *path;		/* path to take */
    double imcost;		/* import cost */
    double excost;		/* export cost */
};

/* This is our global buffer of distribution pointers.  Note that
 * We only malloc this once, and never again (until reboot time
 * of course :) ) We do clear it each and every time. */
struct distinfo *g_distptrs = (struct distinfo *)0;

/* Note that even though we malloc and save the path, it is never
 * used.  Thus, this option.  If you want to malloc and save every
 * path and then free when done, just enable this.  Or, if the
 * dodistribute ever uses the path for something other than checking
 * to see that a path exists, enable this */
/* #define SAVE_FINISH_PATHS */

#ifndef SAVE_FINISH_PATHS
static s_char *finish_path = "h";	/* Placeholder indicating path exists */
#endif /* SAVE_FINISH_PATHS */

static void assemble_dist_paths(struct distinfo *distptrs);
static s_char *ReversePath(s_char *path);

void
finish_sects(int etu)
{
    struct sctstr *sp;
    struct natstr *np;
    int n;
    struct distinfo *infptr;

    if (g_distptrs == (struct distinfo *)0) {
	logerror("First update since reboot, allocating buffer\n");
	/* Allocate the information buffer */
	g_distptrs = (struct distinfo *)(malloc((WORLD_X * WORLD_Y) *
						sizeof(struct distinfo)));
	if (g_distptrs == (struct distinfo *)0) {
	    logerror("malloc failed in finish_sects.\n");
	    return;
	}

	logerror("Allocated '%lu' bytes '%d' indices\n",
		 (unsigned long)(WORLD_X * WORLD_Y * sizeof(struct distinfo)),
		 WORLD_X * WORLD_Y);
    }

    /* Wipe it clean */
    memset(g_distptrs, 0, ((WORLD_X * WORLD_Y) * sizeof(struct distinfo)));

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

    /* First, enable the best_path cacheing */
    bp_enable_cachepath();

    /* Now assemble the paths */
    assemble_dist_paths(g_distptrs);

    /* Now disable the best_path cacheing */
    bp_disable_cachepath();

    /* Now, clear the best_path cache that may have been created */
    bp_clear_cachepath();

    logerror("done assembling paths\n");

    logerror("exporting...");
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (sp->sct_type == SCT_WATER || sp->sct_own == 0)
	    continue;
	np = getnatp(sp->sct_own);
	if (np->nat_money < 0)
	    continue;
	/* Get the pointer */
	infptr = &g_distptrs[XYOFFSET(sp->sct_x, sp->sct_y)];
	dodistribute(sp, EXPORT,
		     infptr->path, infptr->imcost, infptr->excost);
    }
    logerror("done exporting\n");

    /* Note that we free the paths (if allocated) as we loop here */
    logerror("importing...");
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	/* Get the pointer (we do it first so we can free if needed) */
	infptr = &g_distptrs[XYOFFSET(sp->sct_x, sp->sct_y)];
	if (sp->sct_type == SCT_WATER || sp->sct_own == 0) {
#ifdef SAVE_FINISH_PATHS
	    if (infptr->path)
		free(infptr->path);
#endif /* SAVE_FINISH_PATHS */
	    continue;
	}
	np = getnatp(sp->sct_own);
	if (np->nat_money < 0) {
#ifdef SAVE_FINISH_PATHS
	    if (infptr->path)
		free(infptr->path);
#endif /* SAVE_FINISH_PATHS */
	    continue;
	}
	dodistribute(sp, IMPORT,
		     infptr->path, infptr->imcost, infptr->excost);
#ifdef SAVE_FINISH_PATHS
	if (infptr->path)
	    free(infptr->path);
#endif /* SAVE_FINISH_PATHS */
    }
    logerror("done importing\n");

}

static void
assemble_dist_paths(struct distinfo *distptrs)
{
    s_char *path, *p;
    double d;
    struct sctstr *sp;
    struct sctstr *dist;
    struct distinfo *infptr;
    int n;
    s_char buf[512];

    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if ((sp->sct_dist_x == sp->sct_x) && (sp->sct_dist_y == sp->sct_y))
	    continue;
	/* Set the pointer */
	infptr = &distptrs[XYOFFSET(sp->sct_x, sp->sct_y)];
	/* now, get the dist sector */
	dist = getsectp(sp->sct_dist_x, sp->sct_dist_y);
	if (dist == (struct sctstr *)0) {
	    logerror("Bad dist sect %d,%d for %d,%d !\n", sp->sct_dist_x,
		     sp->sct_dist_y, sp->sct_x, sp->sct_y);
	    continue;
	}
	/* Now, get the best distribution path over roads */
	/* Note we go from the dist center to the sector.  This gives
	   us the import path for that sector. */
	path = BestDistPath(buf, dist, sp, &d, MOB_ROAD);

	/* Now, we have a path */
	if (path != (s_char *)0) {
#ifdef SAVE_FINISH_PATHS
	    int len;
	    /* Here we malloc a buffer and save it */
	    len = strlen(path);
	    infptr->path = malloc(len);
	    if (infptr->path == (s_char *)0) {
		logerror("malloc failed in assemble_dist_path!\n");
		return;
	    }
#endif /* SAVE_FINISH_PATHS */
	    /* Save the import cost */
	    infptr->imcost = d;
	    /* Now, reverse the path */
	    p = ReversePath(path);
	    /* And walk the path back to the dist center to get the export
	       cost */
	    infptr->excost = pathcost(sp, p, MOB_ROAD);
#ifdef SAVE_FINISH_PATHS
	    memcpy(infptr->path, p, len);
#else
	    infptr->path = finish_path;
#endif /* SAVE_FINISH_PATHS */
	}
    }
}

static s_char *
ReversePath(s_char *path)
{
    s_char *patharray = "aucdefjhigklmyopqrstbvwxnz";
    static s_char new_path[512];
    int ind;

    if (path == (s_char *)0)
	return (s_char *)0;

    ind = strlen(path);
    if (ind == 0)
	return (s_char *)0;

    if (path[ind - 1] == 'h')
	ind--;

    new_path[ind--] = '\0';
    new_path[ind] = '\0';

    while (ind >= 0) {
	new_path[ind--] = patharray[*(path++) - 'a'];
    }

    return new_path;
}
