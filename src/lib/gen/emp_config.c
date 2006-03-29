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
 *  emp_config.c: Allows config file to control server config. from a file
 * 
 *  Known contributors to this file:
 *     Julian Onions, 1995
 *     Steve McClure, 1998-2000
 */

/*
 * STILL TO DO
 *
 * Change other constants - such as MAXNOC etc.
 * Just requires variables to be assigned, then dynamic allocation in
 * a few places.  Some checks needed in the server to check the world
 * hasn't changed size etc.
 */

#include <config.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "misc.h"
#include "file.h"
#include "optlist.h"
#include "gen.h"

/* Dummy one */
static int emp_config_dummy;

/* things that can be changed */
struct keymatch configkeys[] = {
#define	EMP_CONFIG_C_OUTPUT
#include "econfig-spec.h"
#undef	EMP_CONFIG_C_OUTPUT
};

static struct keymatch *keylookup(s_char *key, struct keymatch tbl[]);
static void set_dirs(char *);

/*
 * read in empire configuration
 */
int
emp_config(char *file)
{
    FILE *fp;
    char scanspace[1024];
    char *av[128];
    char buf[1024];
    struct keymatch *kp;
    int lno = 0;
    int errors = 0;
    int i;

    if (!file)
	file = dflt_econfig;
    errno = 0;
    if ((fp = fopen(file, "r")) == NULL) {
	if (file == dflt_econfig && errno == ENOENT)
	    goto done;
	fprintf(stderr, "Can't open %s for reading (%s)\n",
		file, strerror(errno));
	return -1;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
	++lno;
	for (i = 0; buf[i] && isspace(buf[i]); ++i) ;
	if (!buf[i] || buf[i] == '#')
	    continue;
	if (parse(buf, av, NULL, scanspace, NULL) < 0) {
	    fprintf(stderr, "%s:%d: Can't parse line %s", file, lno, buf);
	    errors = 1;
	    continue;
	}
	if ((kp = keylookup(av[0], configkeys)) == NULL) {
	    fprintf(stderr, "%s:%d: Unknown config key %s\n",
		    file, lno, av[0]);
	    errors = 1;
	    continue;
	}
	if (av[1] == NULL) {
	    fprintf(stderr, "%s:%d: Config key %s needs a value\n",
		    file, lno, av[0]);
	    errors = 1;
	    continue;
	}
	i = 2;
	switch (kp->km_type) {
	case NSC_INT:
	    *(int *)kp->km_data = atoi(av[1]);
	    break;
	case NSC_FLOAT:
	    *(float *)kp->km_data = atof(av[1]);
	    break;
	case NSC_DOUBLE:
	    *(double *)kp->km_data = atof(av[1]);
	    break;
	case NSC_LONG:
	    *(long *)kp->km_data = atol(av[1]);
	    break;
	case NSC_STRING:
	    if (kp->km_flags & KM_ALLOC)
		free(*(char **)kp->km_data);
	    *(char **)kp->km_data = strdup(av[1]);
	    kp->km_flags |= KM_ALLOC;
	    break;
	default:
	    assert(0);
	}
	if (av[i] != NULL) {
	    fprintf(stderr, "%s:%d: Junk after value of config key %s\n",
		    file, lno, av[0]);
	    errors = 1;
	}
    }

    fclose(fp);

done:
    WORLD_X &= ~1;		/* force even */
    set_dirs(file);

    return -errors;
}

/* find the key in the table */
static struct keymatch *
keylookup(register s_char *command, struct keymatch *tbl)
{
    register struct keymatch *kp;

    if (command == 0 || *command == 0)
	return 0;
    for (kp = tbl; kp->km_key != 0; kp++) {
	if (strcmp(kp->km_key, command) == 0)
	    return kp;
    }
    return NULL;
}

static void
set_dirs(char *econfig)
{
    char *slash;
    char *cwd = getcwd(NULL, 0);

#ifdef _WIN32
    econfig = _fullpath(NULL, econfig, 0);
    slash = strrchr(econfig, '\\');
    configdir = malloc(slash - econfig + 1);
    memcpy(configdir, econfig, slash - econfig);
    configdir[slash - econfig] = 0;
#else
    if ((slash = strrchr(econfig, '/'))) {
	configdir = malloc(slash - econfig + 1);
	memcpy(configdir, econfig, slash - econfig);
	configdir[slash - econfig] = 0;
    } else
	configdir = strdup(cwd);

    if (configdir[0] != '/') {
	char *tmp = configdir;
	size_t len = strlen(cwd);

	configdir = malloc(len + 1 + strlen(tmp) + 1);
	sprintf(configdir, "%s/%s", cwd, tmp);
	free(tmp);
    }
#endif /* !_WIN32 */

    free(cwd);
}

void
print_config(FILE *fp)
{
    struct keymatch *kp;

    fprintf(fp, "# Empire Configuration File:\n");
    for (kp = configkeys; kp->km_key; kp++) {
	if (kp->km_comment) {
	    if (kp->km_comment[0] != '\n' && kp->km_comment[0] != '#')
		fprintf(fp, "\n# ");
	    fprintf(fp, "%s\n", kp->km_comment);
	}
	if (!kp->km_key[0])
	    continue;
	switch (kp->km_type) {
	case NSC_STRING:
	    fprintf(fp, "%s \"%s\"\n", kp->km_key, *(char **)kp->km_data);
	    break;
	case NSC_INT:
	    fprintf(fp, "%s %d\n", kp->km_key, *(int *)kp->km_data);
	    break;
	case NSC_FLOAT:
	    fprintf(fp, "%s %g\n", kp->km_key, *(float *)kp->km_data);
	    break;
	case NSC_DOUBLE:
	    fprintf(fp, "%s %g\n", kp->km_key, *(double *)kp->km_data);
	    break;
	case NSC_LONG:
	    fprintf(fp, "%s %ld\n", kp->km_key, *(long *)kp->km_data);
	    break;
	default:
	    assert(0);
	}
    }

    fprintf(fp, "\n");
}
