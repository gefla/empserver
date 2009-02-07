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
 *  anno.c: Delete announcements older than ANNO_KEEP_DAYS
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Doug Hay, 1998
 *     Steve McClure, 2000
 *     Ron Koenderink, 2004
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
	logerror("can't open telegram file %s for writing",
		 tmp_filename);
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
	logerror("can't close temporary telegram file %s",
		 tmp_filename);
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
    int writeit;
    char message[MAXTELSIZE];	/* UTF-8 */
    int deleted = 0;
    int saved = 0;
    int first = 1;

    while (fread(&tgm, sizeof(tgm), 1, annfp) == 1) {
	writeit = 1;
	if (tgm.tel_length > MAXTELSIZE) {
	    logerror("bad telegram file header (length=%d)",
		     tgm.tel_length);
	    return 0;
	}
	if (tgm.tel_type > TEL_LAST) {
	    logerror("bad telegram file header (type=%d)", tgm.tel_type);
	    return 0;
	}

	if (first) {
	    first = 0;
	    if (tgm.tel_date >= expiry_time)
		return 0;
	}
	if (tgm.tel_date < expiry_time)
	    writeit = 0;

	if (writeit) {
	    if (fwrite(&tgm, sizeof(tgm), 1, tmpfp) != 1) {
		logerror("error writing header to temporary "
			 "telegram file %s", tmp_filename);
		return 0;
	    }
	    ++saved;
	} else
	    ++deleted;
	if (fread(message, 1, tgm.tel_length, annfp) != tgm.tel_length) {
	    logerror("error reading body from telegram file %s",
		     annfil);
	    return 0;
	}
	if (writeit) {
	    if (fwrite(message, 1, tgm.tel_length, tmpfp)
		!= tgm.tel_length) {
		logerror("error writing body to temporary telegram "
			 "file %s", tmp_filename);
		return 0;
	    }
	}
    }
    logerror("%d announcements deleted; %d announcements saved",
	     deleted, saved);
    return 1;
}
