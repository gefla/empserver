/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  files.c: Create all the misc files
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#if defined(aix) || defined(linux) || defined(solaris)
#include <unistd.h>
#endif /* aix or linux */

#include <sys/types.h>
#include <fcntl.h>
#if !defined(_WIN32)
#include <sys/uio.h>
#include <sys/file.h>
#else
#include <direct.h>
#include "getopt.h"
#endif
#include <stdarg.h>
#include <stdio.h>

#include "options.h"
#include "misc.h"
#include "sect.h"
#include "nat.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "nuke.h"
#include "power.h"
#include "trade.h"
#include "file.h"
#include "tel.h"
#include "prototypes.h"
#include "optlist.h"

static void file_sct_init(coord x, coord y, s_char *ptr);

static void
print_usage(char *program_name)
{
    printf("Usage: %s -f -e econfig_file\n", program_name);
    printf("-f force mode\n");
}

int
main(int argc, char *argv[])
{
    s_char buf[255];
    s_char tbuf[512];
    s_char *filename;
    int x, y;
    struct natstr nat;
    struct sctstr sct;
    int i;
    s_char *map;
    int opt;
    char *config_file = NULL;
    int force = 0;

    while ((opt = getopt(argc, argv, "e:f")) != EOF) {
	switch (opt) {
	case 'e':
	    config_file = optarg;
	    break;
	case 'f':
	    force = 1;
	    break;
	default:
	    print_usage(argv[0]);
	    exit(-1);
	}
    }

    /* Try to use the existing data directory */
    if (config_file == NULL) {
	sprintf(tbuf, "%s/econfig", datadir);
	config_file = tbuf;
    }
    emp_config(config_file);
    empfile[EF_MAP].size = (WORLD_X * WORLD_Y) / 2;
    empfile[EF_BMAP].size = (WORLD_X * WORLD_Y) / 2;

    if (access(datadir, F_OK) < 0 && mkdir(datadir, 0750) < 0) {
	perror(datadir);
	printf("Can't make game directory\n");
	exit(1);
    }
    if (!force) {
    	printf("WARNING: this blasts the existing game in %s (if any)\n",
	   datadir);
    	printf("continue? ");
    	fgets(buf, sizeof(buf) - 1, stdin);
    	if (*buf != 'y' && *buf != 'Y')
  	    exit(1);
    }
    for (i = 0; i < EF_MAX; i++) {
	if (ef_open(i, O_RDWR | O_CREAT | O_TRUNC, 0) < 0) {
	    perror("ef_open");
	    exit(1);
	}
    }
    memset(&nat, 0, sizeof(nat));
    nat.ef_type = EF_NATION;
    if (nat.nat_cnam[0] == 0)
	strcpy(nat.nat_cnam, "POGO");
    if (nat.nat_pnam[0] == 0)
	strcpy(nat.nat_pnam, "peter");
    nat.nat_stat = STAT_INUSE | STAT_NORM | STAT_GOD | STAT_ABS;
    nat.nat_btu = 255;
    nat.nat_money = 123456789;
    nat.nat_cnum = 0;
    nat.nat_flags |= NF_FLASH;
    for (i = 0; i <= PRI_MAX; i++)
	nat.nat_priorities[i] = -1;
    putnat((&nat));
    printf("All praise to %s!\n", nat.nat_cnam);
    memset(&nat, 0, sizeof(nat));
    for (i = 1; i < MAXNOC; i++) {
	nat.ef_type = EF_NATION;
	nat.nat_cnum = i;
	putnat((&nat));
    }
    ef_close(EF_NATION);
    if (access(teldir, F_OK) < 0 && mkdir(teldir, 0750) < 0) {
	perror(teldir);
	printf("Can't make telegram directory\n");
	exit(1);
    }
    for (x = MAXNOC - 1; x >= 0; x--) {
	filename = mailbox(buf, x);
	close(creat(filename, 0600));
	chmod(filename, 0600);
    }
    close(creat(timestampfil, 0600));
    close(creat(annfil, 0600));
    chmod(infodir, 0750);
    chmod(datadir, 0770);
    chmod(teldir, 0770);

    /* create a zero-filled sector file */
    memset(&sct, 0, sizeof(sct));
    for (y = 0; y < WORLD_Y; y++) {
	for (x = 0; x < WORLD_X / 2; x++) {
	    file_sct_init(x * 2 + (y & 01), y, (s_char *)&sct);
	    putsect(&sct);
	}
    }
    map = (s_char *)calloc(WORLD_X * WORLD_Y / 2, sizeof(*map));
    for (i = 0; i < MAXNOC; i++) {
	ef_write(EF_MAP, i, map);
    }
    map = (s_char *)calloc(WORLD_X * WORLD_Y / 2, sizeof(*map));
    for (i = 0; i < MAXNOC; i++) {
	ef_write(EF_BMAP, i, map);
    }
    for (i = 0; i < EF_MAX; i++) {
	ef_close(i);
    }

    exit(0);
}

static void
file_sct_init(coord x, coord y, s_char *ptr)
{
    struct sctstr *sp = (struct sctstr *)ptr;

    sp->ef_type = EF_SECTOR;
    sp->sct_x = x;
    sp->sct_y = y;
    sp->sct_dist_x = x;
    sp->sct_dist_y = y;
}

void
logerror(s_char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

void
log_last_commands(void)
{
}
