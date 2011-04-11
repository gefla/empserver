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
 *  anno.c: Delete announcements older than ANNO_KEEP_DAYS
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Doug Hay, 1998
 *     Steve McClure, 2000
 *     Ron Koenderink, 2004-2006
 *     Markus Armbruster, 2004-2010
 */

#include <config.h>

#if defined(_WIN32) && defined(__GNUC__)
#include <io.h>
#endif
#include <stdio.h>
#include <time.h>
#include "tel.h"
#include "update.h"

static int copy_and_expire(FILE *annfp, FILE *tmpfp,
			   char *tmp_filename, time_t expiry_time);
static int copy_sink(char *, size_t, void *);

void
delete_old_announcements(void)
{
    time_t now;
    time_t old;
    FILE *annfp;
    FILE *tmpfp;
    char tmp_filename[1024];
    int copy_file;

    if (anno_keep_days < 0)
	return;

    time(&now);
    old = now - days(anno_keep_days);
    logerror("Deleting annos older than %s", ctime(&old));

    if ((annfp = fopen(annfil, "rb")) == NULL) {
	logerror("can't open telegram file %s for reading", annfil);
	return;
    }
    sprintf(tmp_filename, "%s.tmp", annfil);
    if ((tmpfp = fopen(tmp_filename, "wb")) == NULL) {
	logerror("can't open telegram file %s for writing", tmp_filename);
	if (fclose(annfp) != 0)
	    logerror("can't close telegram file %s", annfil);
	return;
    }
    copy_file = copy_and_expire(annfp, tmpfp, tmp_filename, old);

    if (fclose(annfp) != 0) {
	logerror("can't close telegram file %s", annfil);
	copy_file = 0;
    }
    if (fclose(tmpfp) != 0) {
	logerror("can't close temporary telegram file %s", tmp_filename);
	copy_file = 0;
    }
#if defined(_WIN32)
    if (copy_file) {
	if (unlink(annfil) != 0) {
	    logerror("can't delete telegram file %s", annfil);
	    copy_file = 0;
	}
    }
#endif
    if (copy_file) {
	if (rename(tmp_filename, annfil) != 0)
	    logerror("can't move temporary telegram file %s "
		     "to telegram file %s", tmp_filename, annfil);
    } else {
	if (remove(tmp_filename) < 0)
	    logerror("can't delete telegram file %s", tmp_filename);
    }
}

static int
copy_and_expire(FILE *annfp, FILE *tmpfp, char *tmp_filename,
		time_t expiry_time)
{
    struct telstr tgm;
    int res, writeit;
    int deleted = 0;
    int saved = 0;
    int first = 1;

    while ((res = tel_read_header(annfp, annfil, &tgm)) > 0) {
	writeit = tgm.tel_date >= expiry_time;
	if (first) {
	    first = 0;
	    if (writeit)
		return 0;
	}
	if (writeit) {
	    if (fwrite(&tgm, sizeof(tgm), 1, tmpfp) != 1) {
		logerror("error writing header to temporary "
			 "telegram file %s", tmp_filename);
		return 0;
	    }
	    ++saved;
	} else
	    ++deleted;
	res = tel_read_body(annfp, annfil, &tgm,
			    writeit ? copy_sink : NULL, tmpfp);
	if (res < 0)
	    return 0;
    }

    if (res < 0)
	return 0;
    logerror("%d announcements deleted; %d announcements saved",
	     deleted, saved);
    return 1;
}

static int
copy_sink(char *chunk, size_t sz, void *fp)
{
    if (fwrite(chunk, 1, sz, fp) != sz) {
	logerror("error writing to %s.tmp", annfil);
	return -1;
    }
    return 0;
}
