/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  files.c: Create all the misc files
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <config.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#include "../lib/gen/getopt.h"
#else
#include <unistd.h>
#endif

#include "file.h"
#include "game.h"
#include "land.h"
#include "misc.h"
#include "nat.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"
#include "power.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "tel.h"
#include "trade.h"
#include "version.h"

static void file_sct_init(coord, coord, struct sctstr *ptr,
			  time_t timestamp);

static void
print_usage(char *program_name)
{
    printf("Usage: %s [OPTION]...\n"
	   "  -e CONFIG-FILE  configuration file\n"
	   "                  (default %s)\n"
	   "  -f              force overwrite of existing game\n"
	   "  -h              display this help and exit\n"
	   "  -v              display version information and exit\n",
	   program_name, dflt_econfig);
}

int
main(int argc, char *argv[])
{
    char buf[255];
    char *filename;
    int x, y;
    struct gamestr *game;
    struct natstr nat;
    struct realmstr realm;
    struct sctstr sct;
    int i, j;
    char *map;
    int opt;
    char *config_file = NULL;
    int force = 0;
    time_t current_time = time(NULL);

    while ((opt = getopt(argc, argv, "e:fhv")) != EOF) {
	switch (opt) {
	case 'e':
	    config_file = optarg;
	    break;
	case 'f':
	    force = 1;
	    break;
	case 'h':
	    print_usage(argv[0]);
	    exit(0);
	case 'v':
	    printf("%s\n\n%s", version, legal);
	    exit(0);
	default:
	    print_usage(argv[0]);
	    exit(1);
	}
    }

    if (emp_config(config_file) < 0)
	exit(1);

    ef_init();

    if (mkdir(gamedir, S_IRWXU | S_IRWXG) < 0 && errno != EEXIST) {
	perror(gamedir);
	printf("Can't make game directory\n");
	exit(1);
    }
    if (chdir(gamedir)) {
	fprintf(stderr, "Can't chdir to %s (%s)\n", gamedir, strerror(errno));
	exit(EXIT_FAILURE);
    }

    if (!force) {
    	printf("WARNING: this blasts the existing game in %s (if any)\n",
	       gamedir);
    	printf("continue? ");
    	fgets(buf, sizeof(buf), stdin);
    	if (*buf != 'y' && *buf != 'Y')
  	    exit(1);
    }
    for (i = 0; i < EF_MAX; i++) {
	if (!EF_IS_GAME_STATE(i))
	    continue;
	if (!ef_open(i, EFF_CREATE)) {
	    perror("ef_open");
	    exit(1);
	}
    }
    game = getgamep();
    memset(game, 0, sizeof(*game));
    game->ef_type = EF_GAME;
    putgame();
    memset(&nat, 0, sizeof(nat));
    nat.ef_type = EF_NATION;
    strcpy(nat.nat_cnam, "POGO");
    strcpy(nat.nat_pnam, "peter");
    nat.nat_stat = STAT_GOD;
    nat.nat_btu = 255;
    nat.nat_money = 123456789;
    nat.nat_cnum = 0;
    nat.nat_flags |= NF_FLASH;
    putnat((&nat));
    printf("All praise to %s!\n", nat.nat_cnam);
    memset(&nat, 0, sizeof(nat));
    for (i = 1; i < MAXNOC; i++) {
	nat.ef_type = EF_NATION;
	nat.nat_cnum = i;
	putnat((&nat));
    }
    memset(&realm, 0, sizeof(realm));
    realm.ef_type = EF_REALM;
    for (i = 0; i < MAXNOC; i++) {
	realm.r_cnum = i;
	for (j = 0; j < MAXNOR; j++) {
	    realm.r_realm = j;
	    realm.r_uid = (i * MAXNOR) + j;
	    realm.r_timestamp = current_time;
	    putrealm(&realm);
	}
    }
    if (mkdir(teldir, S_IRWXU | S_IRWXG) < 0 && errno != EEXIST) {
	perror(teldir);
	printf("Can't make telegram directory\n");
	exit(1);
    }
    for (x = MAXNOC - 1; x >= 0; x--) {
	filename = mailbox(buf, x);
	close(creat(filename, S_IRWUG));
    }
    close(creat(timestampfil, S_IRWUG));
    close(creat(annfil, S_IRWUG));

    /* create a zero-filled sector file */
    memset(&sct, 0, sizeof(sct));
    for (y = 0; y < WORLD_Y; y++) {
	for (x = 0; x < WORLD_X / 2; x++) {
	    file_sct_init(x * 2 + (y & 1), y, &sct, current_time);
	    putsect(&sct);
	}
    }
    map = calloc(WORLD_X * WORLD_Y / 2, sizeof(*map));
    for (i = 0; i < MAXNOC; i++) {
	ef_write(EF_MAP, i, map);
    }
    for (i = 0; i < MAXNOC; i++) {
	ef_write(EF_BMAP, i, map);
    }
    for (i = 0; i < EF_MAX; i++) {
	if (!EF_IS_GAME_STATE(i))
	    continue;
	ef_close(i);
    }

    exit(0);
}

static void
file_sct_init(coord x, coord y, struct sctstr *ptr, time_t timestamp)
{
    struct sctstr *sp = (struct sctstr *)ptr;

    sp->ef_type = EF_SECTOR;
    sp->sct_x = x;
    sp->sct_y = y;
    sp->sct_dist_x = x;
    sp->sct_dist_y = y;
    sp->sct_timestamp = timestamp;
    sp->sct_newtype = sp->sct_type = SCT_WATER;
    sp->sct_coastal = 1;
}
