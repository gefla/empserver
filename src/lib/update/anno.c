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
 *  anno.c: Delete announcements older than ANNO_KEEP_DAYS
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Doug Hay, 1998
 *     Steve McClure, 2000
 */

#include "misc.h"
#include "tel.h"
#include <stdio.h>
#include <fcntl.h>
#if !defined(_WIN32)
#include <sys/file.h>
#include <unistd.h>
#endif
#include "update.h"
#include "optlist.h"
#include "common.h"

void
delete_old_announcements(void)
{
    time_t now;
    time_t old;
    struct telstr tgm;
    FILE *oldfp;
    int tmpfd;
    s_char tmp_filename[1024];
    int writeit;
    s_char message[MAXTELSIZE];
    int deleted = 0;
    int saved = 0;
    int length;
    int nbytes;
    int first = 1;

    time(&now);
    old = now - days(ANNO_KEEP_DAYS);
    logerror("Deleting annos older than %s", ctime(&old));

#if !defined(_WIN32)
    if ((oldfp = fopen(annfil, "r+")) == 0) {
#else
    if ((oldfp = fopen(annfil, "r+b")) == 0) {
#endif
	logerror("can't read telegram file %s", annfil);
	return;
    }
    sprintf(tmp_filename, "%s.tmp", annfil);
#if !defined(_WIN32)
    if ((tmpfd =
	 open(tmp_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
#else
    if ((tmpfd =
	 open(tmp_filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,
	      0666)) < 0) {
#endif
	logerror("can't write telegram file %s", tmp_filename);
	return;
    }
    while (fread((s_char *)&tgm, sizeof(tgm), 1, oldfp) == 1) {
	writeit = 1;
	if (tgm.tel_length < 0) {
	    logerror("bad telegram file header (length)");
	    return;
	}
	if (tgm.tel_type < 0 || tgm.tel_type > TEL_LAST) {
	    logerror("bad telegram file header (type)");
	    writeit = 0;
	}
	if (first) {
	    first = 0;
	    if (tgm.tel_date >= old) {
		fclose(oldfp);
		return;
	    }
	}

	if (tgm.tel_date < old) {
	    writeit = 0;
	}

	if (writeit) {
	    if (write(tmpfd, &tgm, sizeof(tgm)) < (int)sizeof(tgm)) {
		logerror("error writing to ann.tmp");
		return;
	    }
	    ++saved;
	} else {
	    ++deleted;
	}
	length = tgm.tel_length;
	while (length > 0) {
	    nbytes = length;
	    if (nbytes > (int)sizeof(message))
		nbytes = sizeof(message);
	    (void)fread(message, sizeof(s_char), nbytes, oldfp);
	    if (writeit) {
		if (write(tmpfd, message, nbytes) < nbytes) {
		    logerror("Error writing to ann.tmp");
		    return;
		}
	    }
	    length -= nbytes;
	}
    }
    logerror("%d announcements deleted; %d announcements saved",
	     deleted, saved);
    fclose(oldfp);
    close(tmpfd);
    unlink(annfil);
    rename(tmp_filename, annfil);
}
