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

#include <stdio.h>
#include <stdlib.h>		/* atoi free atol */
#include <string.h>

#include "misc.h"
#include "com.h"
#include "match.h"
#include "file.h"
#include "optlist.h"
#include "tel.h"
#include "gen.h"		/* parse */

/* for systems without strdup  */
#ifdef NOSTRDUP
extern char *strdup();
#endif /* NOSTRDUP */

/* Dummy one */
static int emp_config_dummy;

static void optstrset(struct keymatch *kp, s_char **av);
static void intset(struct keymatch *kp, s_char **av);
static void floatset(struct keymatch *kp, s_char **av);
static void doubleset(struct keymatch *kp, s_char **av);
static void longset(struct keymatch *kp, s_char **av);
static void optionset(struct keymatch *kp, s_char **av);
static void optiondel(struct keymatch *kp, s_char **av);
static void worldxset(struct keymatch *kp, s_char **av);

/* things that can be changed */
struct keymatch configkeys[] = {
#define	EMP_CONFIG_C_OUTPUT
#include "econfig-spec.h"
#undef	EMP_CONFIG_C_OUTPUT
};

static void fixup_files(void);
static struct keymatch *keylookup(s_char *key, struct keymatch tbl[]);


/*
 * read in empire configuration
 */
int
emp_config(char *file)
{
    FILE *fp;
    s_char scanspace[1024];
    s_char *av[65];
    char buf[BUFSIZ];
    struct keymatch *kp;

    if (file == NULL || (fp = fopen(file, "r")) == NULL) {
	fixup_files();
	return RET_OK;
    }
    while (fgets(buf, sizeof buf, fp) != NULL) {
	if (buf[0] == '#' || buf[0] == '\n')
	    continue;
	if (parse(buf, av, 0, scanspace, 0) < 0) {
	    fprintf(stderr, "Can't parse line %s", buf);
	    continue;
	}
	if ((kp = keylookup(av[0], configkeys)) != NULL) {
	    (*kp->km_func) (kp, av + 1);
	} else {
	    fprintf(stderr, "Unknown config key %s\n", av[0]);
	}
    }
    fclose(fp);
    fixup_files();

    return RET_OK;
}

struct otherfiles {
    s_char **files;
    char *name;
};

/* list of other well known files... -maybe tailor these oneday
 * anyway - meantime they are all relative to datadir */
static struct otherfiles ofiles[] = {
    {&upfil, "up"},
    {&downfil, "down"},
    {&disablefil, "disable"},
    {&banfil, "ban"},
    {&authfil, "auth"},
    {&annfil, "ann"},
    {&timestampfil, "timestamp"},
    {&teldir, "tel"},
#if !defined(_WIN32)
    {&telfil, "tel/tel"},
#else
    {&telfil, "tel\\tel"},
#endif
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
#if !defined(_WIN32)
	sprintf(buf, "%s/%s", datadir, ep->name);
#else
	sprintf(buf, "%s\\%s", datadir, ep->name);
#endif
	ep->file = strdup(buf);
    }

    for (op = ofiles; op->files; op++) {
#if !defined(_WIN32)
	sprintf(buf, "%s/%s", datadir, op->name);
#else
	sprintf(buf, "%s\\%s", datadir, op->name);
#endif
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

/* worldx int setting function */
static void
worldxset(struct keymatch *kp, s_char **av)
{
    int *intptr = (int *)kp->km_data;

    if (*av == NULL || intptr == NULL)
	return;
    *intptr = atoi(*av);
    if (!((*intptr % 2) == 0)) {
	/* Must be div / 2, so subtract one */
	*intptr = *intptr - 1;
    }
}

/* generic int setting function */
static void
intset(struct keymatch *kp, s_char **av)
{
    int *intptr = (int *)kp->km_data;

    if (*av == NULL || intptr == NULL)
	return;
    *intptr = atoi(*av);
}

/* generic float set function */
static void
floatset(struct keymatch *kp, s_char **av)
{
    float *floatptr = (float *)kp->km_data;

    if (*av == NULL || floatptr == NULL)
	return;
    *floatptr = atof(*av);
}



/* generic string set function */
static void
optstrset(struct keymatch *kp, s_char **av)
{
    s_char **confstrp = (s_char **)kp->km_data;

    if (*av == NULL || confstrp == NULL)
	return;
    if (kp->km_flags & KM_ALLOC)
	free(*confstrp);
    *confstrp = strdup(*av);
    kp->km_flags |= KM_ALLOC;
}

/* generic double set function */
static void
doubleset(struct keymatch *kp, s_char **av)
{
    double *doublep = (double *)kp->km_data;

    if (*av == NULL || doublep == NULL)
	return;
    *doublep = atof(*av);
}

/* generic long set function */
static void
longset(struct keymatch *kp, s_char **av)
{
    long int *longp = (long int *)kp->km_data;

    if (*av == NULL || longp == NULL)
	return;
    *longp = atol(*av);
}

void
print_config(FILE * fp)
{
    struct empfile *ep;
    struct otherfiles *op;
    struct keymatch *kp;

    fprintf(fp, "# Empire Configuration File:\n");
    for (kp = configkeys; kp->km_key; kp++) {
	/* We print a few special things here */
	if (kp->km_comment) {
	    if (kp->km_comment[0]) {
		if (kp->km_comment[0] != '\n')
		    fprintf(fp, "\n# ");
		fprintf(fp, "%s\n", kp->km_comment);
	    }
	}
	if (!kp->km_key[0])
	    continue;
	if (kp->km_func == optstrset) {
	    fprintf(fp, "%s \"%s\"\n", kp->km_key,
		    *(s_char **)kp->km_data);
	} else if (kp->km_func == intset) {
	    fprintf(fp, "%s %d\n", kp->km_key, *(int *)kp->km_data);
	} else if (kp->km_func == worldxset) {
	    fprintf(fp, "%s %d\n", kp->km_key, *(int *)kp->km_data);
	} else if (kp->km_func == floatset) {
	    fprintf(fp, "%s %g\n", kp->km_key, *(float *)kp->km_data);
	} else if (kp->km_func == doubleset) {
	    fprintf(fp, "%s %g\n", kp->km_key, *(double *)kp->km_data);
	} else if (kp->km_func == longset) {
	    fprintf(fp, "%s %ld\n", kp->km_key, *(long *)kp->km_data);
	} else if (kp->km_func == optionset) {
	    struct option_list *op;

	    for (op = Options; op->opt_key; op++) {
		if (*op->opt_valuep)
		    fprintf(fp, "%s %s\n", kp->km_key, op->opt_key);
	    }
	} else if (kp->km_func == optiondel) {
	    struct option_list *op;

	    for (op = Options; op->opt_key; op++) {
		if (*op->opt_valuep == 0)
		    fprintf(fp, "%s %s\n", kp->km_key, op->opt_key);
	    }
	} else
	    fprintf(fp, "# Unknown format %s\n", kp->km_key);
    }

    fprintf(fp, "\n");
    for (ep = empfile; ep < &empfile[EF_MAX]; ep++)
	fprintf(fp, "# File %s -> %s\n", ep->name, ep->file);
    for (op = ofiles; op->files; op++)
	fprintf(fp, "# File %s -> %s\n", op->name, *(op->files));

}


/* add an option to the list */
static void
set_option(const char *s)
{
    struct option_list *op;

    for (op = Options; op->opt_key; op++) {
	if (strcmp(op->opt_key, s) == 0) {
	    *op->opt_valuep = 1;
	    return;
	}
    }
    fprintf(stderr, "Unknown option %s\n", s);
}

/* delete an option from the list */
static void
delete_option(const char *s)
{
    struct option_list *op;

    for (op = Options; op->opt_key; op++) {
	if (strcmp(op->opt_key, s) == 0) {
	    *op->opt_valuep = 0;
	    return;
	}
    }
    fprintf(stderr, "Unknown option %s\n", s);
}

/* config interface */
static void
optionset(struct keymatch *kp, s_char **av)
				/* unused - we have a well known global */
{
    char **cpp;

    for (cpp = (char **)av; *cpp; cpp++)
	set_option(*cpp);
}

/* config interface */
static void
optiondel(struct keymatch *kp, s_char **av)
				/* unused - we have a well known global */
{
    char **cpp;

    for (cpp = (char **)av; *cpp; cpp++)
	delete_option(*cpp);
}
