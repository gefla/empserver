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
 *  info.c: display an info page
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Mike Wise, 1997 - added apropos and case insensitivity
 *     Doug Hay, 1998
 *     Steve McClure, 1998-2000
 */

#include "misc.h"
#include "player.h"
#ifdef Rel4
#include <string.h>
#endif /* Rel4 */
#include <stdio.h>
#include <sys/stat.h>
#if !defined(_WIN32)
#include <dirent.h>
#else
#include <windows.h>
#endif
#ifdef solaris
#include <unistd.h>
#endif
#include "commands.h"

#if 0
static int fileselect(struct dirent *dp);
static int printdir(void);
#endif

static
s_char *
lowerit(s_char *buf, int n, s_char *orig)


{				/* converts a string to lower case */
    /* lower case output buffer */
    /* size of output buffer */
    /* input strig */
    int i;
    s_char *tmp;
    tmp = buf;
    memset(buf, 0, n);
    for (i = 0; i < n && *orig; i++) {
	*tmp++ = tolower(*orig++);
    }
    return buf;
}

static
    int
strnccmp(s_char *s1, s_char *s2, int n)
{
    int i;
    char c1, c2;
    for (i = 0; i < n && *s1 && s2; i++) {
	c1 = tolower(*s1++);
	c2 = tolower(*s2++);
	if (c1 > c2)
	    return 1;
	else if (c1 < c2)
	    return -1;
    }
    return 0;
}

#if !defined(_WIN32)

int
info(void)
{
    extern int errno;
    extern s_char *infodir;
#if (!defined linux) && (!defined FBSD) && (!defined __linux__) && (!defined __ppc__)
    extern s_char *sys_errlist[];
#endif
    s_char buf[255];
    FILE *fp;
    s_char *bp;
    struct stat statb;
    struct dirent *dp;
    s_char filename[1024];
    DIR *info_dp;

    if (player->argp[1] == 0 || !*player->argp[1])
	bp = "TOP";
#if 0
    /* Note that we generate an "all" file now, which contains
     * this list, so we don't do printdir. */
    else if (!strcmp(player->argp[1], "all"))
	return printdir();
#endif
    /*
     * don't let sneaky people go outside the info directory
     */
    else if (NULL != (bp = rindex(player->argp[1], '/')))
	bp++;
    else
	bp = player->argp[1];
    sprintf(filename, "%s/%s", infodir, bp);
    fp = fopen(filename, "r");
    if (fp == NULL) {
	int len = strlen(bp);
	/* may be a "partial" request.  */
	info_dp = opendir(infodir);
	if (info_dp == 0) {
	    pr("Can't open info dir \"%s\"\n", infodir);
	    return RET_SYS;
	}
	rewinddir(info_dp);
	while ((dp = readdir(info_dp)) != 0 && fp == 0) {
	    if (strnccmp(bp, dp->d_name, len) != 0)
		continue;
	    sprintf(filename, "%s/%s", infodir, dp->d_name);
	    fp = fopen(filename, "r");
	}
	closedir(info_dp);
	if (fp == NULL) {
	    pr("Sorry, there is no info on %s\n", bp);
	    return RET_FAIL;
	}
    }
    if (fstat(fileno(fp), &statb) < 0) {
	pr("Cannot read info page for \"%s\" (%s)\n",
	   dp->d_name, sys_errlist[errno]);
	fclose(fp);
	return RET_SYS;
    }
    if ((statb.st_mode & S_IFREG) == 0) {
	pr("There is no available information on \"%s\"\n", dp->d_name);
	fclose(fp);
	return RET_FAIL;
    }
    pr("Information on:  %s    Last modification date: %s",
       bp, ctime(&statb.st_mtime));
    while (fgets(buf, sizeof(buf), fp) != 0)
	pr("%s", buf);
    (void)fclose(fp);
    return RET_OK;
}

#if 0
static int biggest;

static
    int
fileselect(struct dirent *dp)
{
    int l;
    if (*dp->d_name == '.')
	return 0;

    if ((l = strlen(dp->d_name)) > biggest)
	biggest = l;
    return 1;
}

#ifdef solaris
static
    int
alphasort(struct dirent *d1, struct dirent *d2)
{
    return strcmp(d1->d_name, d2->d_name);
}

static int
scandir(char *dir, struct dirent ***dpp, int (*select) (), int (*sort) ())
     /* directory to read */
     /* directory entry pointer pointer */
{
    DIR *dirp;
    int nsize;
    int nents;
    struct dirent *d, *tmp;

    d = (struct dirent *)malloc(sizeof(struct dirent) +
				pathconf(".", _PC_NAME_MAX) + 1);
    if ((dirp = opendir(dir)) == NULL)
	return -1;
    nsize = 100;
    *dpp = (struct dirent **)malloc(nsize * sizeof(struct dirent *));
    if ((*dpp) == NULL) {
	closedir(dirp);
	return -1;
    }
    nents = 0;
    while ((d = readdir_r(dirp, d)) != NULL) {
	if (select != NULL && !(*select) (d))
	    continue;
	tmp =
	    (struct dirent *)malloc(sizeof(struct dirent) +
				    strlen(d->d_name) + 1);
	if (tmp == NULL)
	    return -1;
	tmp->d_ino = d->d_ino;
	tmp->d_reclen = d->d_reclen;
	strcpy(tmp->d_name, d->d_name);
	if (nents > nsize) {
	    *dpp =
		(struct dirent **)realloc((char *)*dpp,
					  nsize * sizeof(struct dirent *));
	    if (*dpp == NULL)
		return -1;
	}
	(*dpp)[nents++] = tmp;
    }
    closedir(dirp);
    if (nents > 0 && sort != NULL)
	qsort(*dpp, nents, sizeof(struct dirent *), sort);
    return nents;
}
#endif

static int
printdir(void)
{
#if !defined(solaris) && !defined(ALPHA) && !defined(__linux__)
    extern int alphasort(const struct dirent *const *,
			 const struct dirent *const *);
#endif
    extern s_char *infodir;
    static time_t lastmodtime;
    static int number;
    static struct dirent **dp;
    struct stat st;
    int count;
    int i;
    int npl;

    if (stat(infodir, &st) == -1)
	return RET_FAIL;
    pr("Available topics are:\n");
    if (lastmodtime < st.st_mtime) {
	if (dp)
	    free((s_char *)dp);
	biggest = 0;
	number = scandir(infodir, &dp, fileselect, alphasort);
	lastmodtime = st.st_mtime;
    }
    count = 79 / (biggest + 1) - 1;
    for (i = 0, npl = 0; i < number; i++) {
	pr("%-*.*s%c", biggest, strlen(dp[i]->d_name),
	   dp[i]->d_name, npl == count ? '\n' : ' ');
	if (npl == count)
	    npl = 0;
	else
	    npl++;
    }
    if (npl != 0)
	pr("\n");
    return 0;
}
#endif

int
apro(void)
{
    extern s_char *infodir;
    FILE *fp;
    s_char *bp, *lbp;
    s_char *fbuf;
    s_char *lbuf;
    struct dirent *dp;
    s_char filename[1024];
    DIR *info_dp;
    long nf, nhf, nl, nlhl, nhl, nll;
    int alreadyhit;
    int lhitlim;

    if (player->argp[1] == 0 || !*player->argp[1]) {
	pr("Apropos what?\n");
	return RET_FAIL;
    }

    lhitlim = 100;
    if (player->argp[2]) {
	lhitlim = atoi(player->argp[2]);
	if (lhitlim <= 0)
	    lhitlim = 100;
    }

    info_dp = opendir(infodir);
    if (info_dp == 0) {
	pr("Can't open info dir \"%s\"\n", infodir);
	return RET_SYS;
    }

    fbuf = (s_char *)malloc(256);
    lbuf = (s_char *)malloc(256);
    lbp = (s_char *)malloc(256);

    /*
     *  lower case search string into lbp
     */
    bp = player->argp[1];
    lowerit(lbp, 256, bp);

    /*
     *  search
     */
    nf = nhf = nl = nhl = 0;
    rewinddir(info_dp);
    while ((dp = readdir(info_dp)) != 0) {
	sprintf(filename, "%s/%s", infodir, dp->d_name);
	fp = fopen(filename, "r");
	alreadyhit = 0;
	nll = nlhl = 0;
	if (fp != NULL) {
	    while (fgets(fbuf, 256, fp)) {
		lowerit(lbuf, 256, fbuf);
		if (strstr(lbuf, lbp)) {
		    if (!alreadyhit) {
			pr("*** %s ***\n", dp->d_name);
			alreadyhit = 1;
			nhf++;
		    }
		    fbuf[74] = '\n';
		    fbuf[75] = 0;
		    pr("   %s", fbuf);
		    nlhl++;
		    /*
		     * break if too many lines
		     */
		    if ((nhl + nlhl) > lhitlim)
			break;
		}
		nll++;
	    }
	    fclose(fp);
	}
	nhl += nlhl;
	nl += nll;
	nf++;
	if (nhl > lhitlim)
	    break;
    }
    closedir(info_dp);

    free(fbuf);
    free(lbuf);
    free(lbp);

    if ((nhl) > lhitlim) {
	pr("Limit of %ld lines exceeded\n", lhitlim);
    }
    pr("Found %s in %ld of %ld files and in %ld of %ld lines\n",
       bp, nhf, nf, nhl, nl);
    return RET_OK;
}

#else

int
info(void)
{
    extern int errno;
    extern s_char *infodir;
    s_char buf[255];
    FILE *fp;
    s_char *bp;
    s_char *bp2;
    s_char filename[1024];

    if (player->argp[1] == 0 || !*player->argp[1])
	bp = "TOP";
#if 0
    /* Note that we generate an "all" file now, which contains
     * this list, so we don't do printdir. */
    else if (!strcmp(player->argp[1], "all"))
	return printdir();
#endif
    else {
	/*
	 * don't let sneaky people go outside the info directory
	 */
	bp = player->argp[1];
	if (NULL != (bp2 = rindex(bp, '/')))
	    bp = ++bp2;
	if (NULL != (bp2 = rindex(bp, '\\')))
	    bp = ++bp2;
	if (NULL != (bp2 = rindex(bp, ':')))
	    bp = ++bp2;
	if (!*bp)
	    bp = "TOP";
    }

    strncpy(filename, infodir, sizeof(filename) - 2);
    strcat(filename, "//");
    strncat(filename, bp, sizeof(filename) - 1 - strlen(filename));
    fp = fopen(filename, "r");
    if (fp == NULL) {
	/* may be a "partial" request.  */
	HANDLE hDir;
	WIN32_FIND_DATA fData;
	int len = strlen(bp);
	strncat(filename, "*", sizeof(filename) - 1 - strlen(filename));
	hDir = FindFirstFile(filename, &fData);
	if (hDir == INVALID_HANDLE_VALUE) {
	    pr("Can't open info dir \"%s\"\n", infodir);
	    return RET_SYS;
	}
	do {
	    if ((fData.dwFileAttributes == FILE_ATTRIBUTE_NORMAL) &&
		(strnccmp(bp, fData.cFileName, len) == 0)) {
		strncpy(filename, infodir, sizeof(filename) - 2);
		strcat(filename, "//");
		strncat(filename, fData.cFileName,
			sizeof(filename) - 1 - strlen(filename));
		fp = fopen(filename, "r");
	    }
	} while (!fp && FindNextFile(hDir, &fData));
	FindClose(hDir);
	if (fp == NULL) {
	    pr("Sorry, there is no info on %s\n", bp);
	    return RET_FAIL;
	}
    }
    pr("Information on:  %s", bp);
    while (fgets(buf, sizeof(buf), fp) != 0)
	pr("%s", buf);
    (void)fclose(fp);
    return RET_OK;
}

#if 0
static int
printdir(void)
{
    extern s_char *infodir;
    HANDLE hDir;
    WIN32_FIND_DATA fData;
    int count;
    int i;
    int npl;
    s_char filename[1024];

    strncpy(filename, infodir, sizeof(filename) - 3);
    strcat(filename, "//*");

    hDir = FindFirstFile(filename, &fData);
    if (hDir == INVALID_HANDLE_VALUE) {
	return RET_FAIL;
    }

    pr("Available topics are:\n");
    do {
	if (fData.dwFileAttributes == FILE_ATTRIBUTE_NORMAL) {
	    /* Yes, we could do multi-column work here. */
	    pr("%s\n", fData.cFileName);
	}
    } while (FindNextFile(hDir, &fData));
    FindClose(hDir);

    return 0;
}
#endif

int
apro(void)
{
    extern s_char *infodir;
    HANDLE hDir;
    WIN32_FIND_DATA fData;
    FILE *fp;
    s_char *bp, *lbp;
    s_char *fbuf;
    s_char *lbuf;
    s_char filename[1024];
    long nf, nhf, nl, nlhl, nhl, nll;
    int alreadyhit;
    int lhitlim;

    if (player->argp[1] == 0 || !*player->argp[1]) {
	pr("Apropos what?\n");
	return RET_FAIL;
    }

    lhitlim = 100;
    if (player->argp[2]) {
	lhitlim = atoi(player->argp[2]);
	if (lhitlim <= 0)
	    lhitlim = 100;
    }

    strncpy(filename, infodir, sizeof(filename) - 3);
    strcat(filename, "//*");
    hDir = FindFirstFile(filename, &fData);
    if (hDir == INVALID_HANDLE_VALUE) {
	return RET_FAIL;
    }

    fbuf = (s_char *)malloc(256);
    lbuf = (s_char *)malloc(256);
    lbp = (s_char *)malloc(256);

    /*
     *  lower case search string into lbp
     */
    bp = player->argp[1];
    lowerit(lbp, 256, bp);

    /*
     *  search
     */
    nf = nhf = nl = nhl = 0;
    do {
	strncpy(filename, infodir, sizeof(filename) - 3);
	strcat(filename, "//");
	strncat(filename, fData.cFileName,
		sizeof(filename) - 1 - strlen(filename));
	fp = fopen(filename, "r");
	alreadyhit = 0;
	nll = nlhl = 0;
	if (fp != NULL) {
	    while (fgets(fbuf, 256, fp)) {
		lowerit(lbuf, 256, fbuf);
		if (strstr(lbuf, lbp)) {
		    if (!alreadyhit) {
			pr("*** %s ***\n", fData.cFileName);
			alreadyhit = 1;
			nhf++;
		    }
		    fbuf[74] = '\n';
		    fbuf[75] = 0;
		    pr("   %s", fbuf);
		    nlhl++;
		    /*
		     * break if too many lines
		     */
		    if ((nhl + nlhl) > lhitlim)
			break;
		}
		nll++;
	    }
	    fclose(fp);
	}
	nhl += nlhl;
	nl += nll;
	nf++;
	if (nhl > lhitlim)
	    break;
    } while (FindNextFile(hDir, &fData));
    FindClose(hDir);

    free(fbuf);
    free(lbuf);
    free(lbp);

    if ((nhl) > lhitlim) {
	pr("Limit of %ld lines exceeded\n", lhitlim);
    }
    pr("Found %s in %ld of %ld files and in %ld of %ld lines\n",
       bp, nhf, nf, nhl, nl);
    return RET_OK;
}

#endif
