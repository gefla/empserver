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
 *  emp_config.c: Allows config file to control server config. from a file
 * 
 *  Known contributors to this file:
 *     Julian Onions, 1995
 *     Steve McClure, 1998-2000
 */

/*
 * STILL TO DO
 *
 * 1. Change other constants - such as Num Countries etc.
 *    Just requires variables to be assigned, then dynamic allocation in
 *    a few places. Some checks needed in the server to check the world
 *    hasn't changed size etc.
 * 2. Could look at loading in planes, units etc. Should be easy enough.
 *
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>		/* atoi free atol */
#include <string.h>

#include "misc.h"
#include "file.h"
#include "optlist.h"
#include "gen.h"		/* parse */

/* for systems without strdup  */
#ifdef NOSTRDUP
extern char *strdup();
#endif /* NOSTRDUP */

/* Dummy one */
static int emp_config_dummy;

/* things that can be changed */
struct keymatch configkeys[] = {
#define	EMP_CONFIG_C_OUTPUT
#include "econfig-spec.h"
#undef	EMP_CONFIG_C_OUTPUT
};

static void fixup_files(void);
static struct keymatch *keylookup(s_char *key, struct keymatch tbl[]);
static int set_option(const char *, int);

/*
 * read in empire configuration
 */
int
emp_config(char *file)
{
    FILE *fp;
    char scanspace[1024];
    char *av[65];
    char buf[BUFSIZ];
    struct keymatch *kp;
    int lno = 0;
    int errors = 0;
    int i;

    if (file == NULL) {
	fixup_files();
	return 0;
    }
    if ((fp = fopen(file, "r")) == NULL) {
	fprintf(stderr, "Can't open %s for reading (%s)\n",
		file, strerror(errno));
	return -1;
    }

    while (fgets(buf, sizeof buf, fp) != NULL) {
	++lno;
	for (i = 0; buf[i] && isspace(buf[i]); ++i) ;
	if (!buf[i] || buf[i] == '#')
	    continue;
	if (parse(buf, av, 0, scanspace, 0) < 0) {
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
	case NSC_NOTYPE:
	    for (i = 1; av[i]; ++i) {
		if (set_option(av[i], kp->km_key[0] != 'n') < 0) {
		    fprintf(stderr, "%s:%d: Unknown option %s\n",
			    file, lno, av[i]);
		    errors = 1;
		}
	    }
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
    fixup_files();
    WORLD_X &= ~1;		/* make even */

    return -errors;
}

struct otherfiles {
    char **files;
    char *name;
};

/* list of other well known files... -maybe tailor these oneday
 * anyway - meantime they are all relative to datadir */
static struct otherfiles ofiles[] = {
    {&motdfil, "motd"},
    {&downfil, "down"},
    {&disablefil, "disable"},
    {&banfil, "ban"},
    {&authfil, "auth"},
    {&annfil, "ann"},
    {&timestampfil, "timestamp"},
    {&teldir, "tel"},
    {&telfil, "tel/tel"},
    {NULL, NULL}
};

/* fix up the empfile struct to reference full path names */
static void
fixup_files(void)
{
    struct empfile *ep;
    struct otherfiles *op;
    s_char buf[1024];

    for (ep = empfile; ep < &empfile[EF_MAX]; ep++) {
	sprintf(buf, "%s/%s", datadir, ep->name);
	ep->file = strdup(buf);
    }

    for (op = ofiles; op->files; op++) {
	sprintf(buf, "%s/%s", datadir, op->name);
	*op->files = strdup(buf);
    }
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

void
print_config(FILE *fp)
{
    struct empfile *ep;
    struct option_list *op;
    struct otherfiles *ofp;
    struct keymatch *kp;

    fprintf(fp, "# Empire Configuration File:\n");
    for (kp = configkeys; kp->km_key; kp++) {
	if (kp->km_comment) {
	    if (kp->km_comment[0] != '\n')
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
	case NSC_NOTYPE:
	    for (op = Options; op->opt_key; op++)
		if (*op->opt_valuep != (kp->km_key[0] == 'n'))
		    fprintf(fp, "%s %s\n", kp->km_key, op->opt_key);
	    break;
	default:
	    assert(0);
	}
    }

    fprintf(fp, "\n");
    for (ep = empfile; ep < &empfile[EF_MAX]; ep++)
	fprintf(fp, "# File %s -> %s\n", ep->name, ep->file);
    for (ofp = ofiles; ofp->files; ofp++)
	fprintf(fp, "# File %s -> %s\n", ofp->name, *(ofp->files));

}


/* Set option S to value VAL; return 0 on success, -1 on failure.  */
static int
set_option(const char *s, int val)
{
    struct option_list *op;

    for (op = Options; op->opt_key; op++) {
	if (strcmp(op->opt_key, s) == 0) {
	    *op->opt_valuep = val;
	    return 0;
	}
    }
    return -1;
}
