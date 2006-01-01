/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#include <config.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#if !defined(_WIN32)
#include <dirent.h>
#else
#include <windows.h>
#endif
#include "misc.h"
#include "player.h"
#include "commands.h"
#include "optlist.h"

static s_char *
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

#if !defined(_WIN32)

int
info(void)
{
    s_char buf[255];
    FILE *fp;
    s_char *name;
    s_char *tmp_name;
    struct stat statb;
    struct dirent *dp;
    s_char filename[1024];
    s_char last[256];
    DIR *info_dp;
    int nmatch = 0;
    int width = 0;
    char sep;
 
    name = player->argp[1];
    if (name) {
	/*
	 * don't let sneaky people go outside the info directory
	 */
	if (NULL != (tmp_name = strrchr(name, '/')))
	    name = tmp_name + 1;
    }
    if (!name || !*name)
	name = "TOP";
 
    snprintf(filename, sizeof(filename), "%s/%s", infodir, name);
    fp = fopen(filename, "r");
    if (fp == NULL) {
	/* may be a "partial" request.  */
	info_dp = opendir(infodir);
	if (info_dp == 0) {
	    pr("Can't open info dir\n");
	    logerror("Can't open info dir \"%s\"\n", infodir);
	    return RET_SYS;
	}

	while ((dp = readdir(info_dp)) != 0) {
	    if (strncasecmp(name, dp->d_name, strlen(name)) != 0)
		continue;
	    nmatch++;
	    if (nmatch == 1) {
		snprintf(last, sizeof(last), "%s", dp->d_name);
	    } else {
		if (nmatch == 2) {
		    pr("`%s' is ambiguous.  The following topics match:\n%s",
			name, last);
		    width = strlen(last);
		}
		width += 2 + strlen(dp->d_name);
		sep = ' ';
		if (width > 75) {
		    sep = '\n';
		    width = strlen(dp->d_name);
		}
		pr(",%c%s", sep, dp->d_name);
	    }
	}
	closedir(info_dp);
	if (nmatch == 0) {
	    pr("Sorry, there is no info on %s\n", name);
	    return RET_FAIL;
	} else if (nmatch > 1) {
	    pr(".\n");
	    return RET_FAIL;
	}
	snprintf(filename, sizeof(filename), "%s/%s", infodir,
		 last);
	fp = fopen(filename, "r");
	if (fp == NULL) {
	    pr("Error reading info file for %s\n", name);
	    logerror("Cannot open for \"%s\" info file (%s)",
		     filename, strerror(errno));
	    return RET_FAIL;
	}
    }
    if (fstat(fileno(fp), &statb) < 0) {
	pr("Error reading info file for %s\n", name);
	logerror("Cannot fstat for \"%s\" info file (%s)",
		 filename, strerror(errno));
	fclose(fp);
	return RET_SYS;
    }
    if ((statb.st_mode & S_IFREG) == 0) {
	pr("Error reading info file for %s\n", name);
	logerror("The info file \"%s\" is not regular file\n", filename);
	fclose(fp);
	return RET_SYS;
    }
    pr("Information on:  %s    Last modification date: %s",
       name, ctime(&statb.st_mtime));
    while (fgets(buf, sizeof(buf), fp) != 0)
	pr("%s", buf);
    (void)fclose(fp);
    return RET_OK;
}

int
apro(void)
{
    FILE *fp;
    s_char *name, *lbp;
    s_char *fbuf;
    s_char *lbuf;
    struct dirent *dp;
    s_char filename[1024];
    DIR *info_dp;
    long nf, nhf, nl, nlhl, nhl, nll;
    int alreadyhit;
    int lhitlim;
    struct stat statb;

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
    if (info_dp == NULL) {
	pr("Can't open info dir \n");
	logerror("Can't open info dir \"%s\"", infodir);
	return RET_SYS;
    }

    fbuf = malloc(256);
    lbuf = malloc(256);
    lbp = malloc(256);

    /*
     *  lower case search string into lbp
     */
    name = player->argp[1];
    lowerit(lbp, 256, name);

    /*
     *  search
     */
    nf = nhf = nl = nhl = 0;
    while ((dp = readdir(info_dp)) != 0) {
	snprintf(filename, sizeof(filename), "%s/%s", infodir,
	    dp->d_name);
	fp = fopen(filename, "r");
	alreadyhit = 0;
	nll = nlhl = 0;
	if (fp != NULL) {
	    if (fstat(fileno(fp), &statb) < 0) {
		logerror("Cannot stat for \"%s\" info file (%s)",
			 filename, strerror(errno));
		fclose(fp);
		continue;
	    }
	    if ((statb.st_mode & S_IFREG) == 0) {
		logerror("The info file \"%s\" is not regular file\n",
			 filename);
		fclose(fp);
		continue;
	    }
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
	pr("Limit of %d lines exceeded\n", lhitlim);
    }
    pr("Found %s in %ld of %ld files and in %ld of %ld lines\n",
       name, nhf, nf, nhl, nl);
    return RET_OK;
}

#else

int
info(void)
{
    s_char buf[255];
    FILE *fp;
    s_char *name;
    s_char *tmp_name;
    s_char filename[1024];
    s_char last[256];
    int nmatch = 0;
    int width = 0;
    char sep;

    name = player->argp[1];
    if (name) {
	/*
	 * don't let sneaky people go outside the info directory
	 */
	if (NULL != (tmp_name = strrchr(name, '/')))
	    name = tmp_name + 1;
	if (NULL != (tmp_name = strrchr(name, '\\')))
	    name = tmp_name + 1;
	if (NULL != (tmp_name = strrchr(name, ':')))
	    name = tmp_name + 1;
    }
    if (!name || !*name)
	name = "TOP";

    _snprintf(filename, sizeof(filename) - 1, "%s\\%s", infodir, name);
    fp = fopen(filename, "rb");
    if (fp == NULL) {
	/* may be a "partial" request.  */
	HANDLE hDir;
	WIN32_FIND_DATA fData;
	strcat(filename, "*");
	hDir = FindFirstFile(filename, &fData);
	if (hDir == INVALID_HANDLE_VALUE) {
	    switch (GetLastError()) {
	    case ERROR_FILE_NOT_FOUND:
		pr("Sorry, there is no info on %s\n", name);
		return RET_FAIL;
		break;
	    case ERROR_PATH_NOT_FOUND:
		pr("Can't open info dir\n");
		logerror("Can't open info dir \"%s\"", infodir);
		break;
	    default:
		pr("Error reading info dir\n");
		logerror("Error (%lu) reading info dir(%s)\\file(%s)",
		    GetLastError(), infodir, filename);
	    }
	    return RET_SYS;
	}
	do {
	    if ((fData.dwFileAttributes != (DWORD)-1) &&
		((fData.dwFileAttributes == FILE_ATTRIBUTE_NORMAL) ||
		 (fData.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE) ||
		 (fData.dwFileAttributes == FILE_ATTRIBUTE_READONLY)) &&
		(strncasecmp(name, fData.cFileName, strlen(name)) == 0)) {
		nmatch++;
		if (nmatch == 1) {
		    _snprintf(last, sizeof(last), "%s", fData.cFileName);
		} else {
		    if (nmatch == 2) {
			pr("`%s' is ambiguous.  The following topics match:\n%s",
			    name, last);
			width = strlen(last);
		    }
		    width += 2 + strlen(fData.cFileName);
		    sep = ' ';
		    if (width > 75) {
			sep = '\n';
			width = strlen(fData.cFileName);
		    }
		    pr(",%c%s", sep, fData.cFileName);
		}
	    }
	} while (FindNextFile(hDir, &fData));
	FindClose(hDir);
	if (nmatch == 0) {
	    pr("Sorry, there is no info on %s\n", name);
	    return RET_FAIL;
	} else if (nmatch > 1) {
	    pr(".\n");
	    return RET_FAIL;
	}
	_snprintf(filename, sizeof(filename), "%s/%s",
		  infodir, last);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
	    pr("Error reading info file for %s\n", name);
	    logerror("Cannot open for \"%s\" info file (%s)",
		     filename, strerror(errno));
	    return RET_FAIL;
	}
    }
    else {
	DWORD fAttrib = GetFileAttributes(filename);
	if ((fAttrib == (DWORD)-1) || /* INVALID_FILE_ATTRIBUTES */
	    ((fAttrib != FILE_ATTRIBUTE_NORMAL) &&
	     (fAttrib != FILE_ATTRIBUTE_ARCHIVE) &&
	     (fAttrib != FILE_ATTRIBUTE_READONLY))) {
	    pr("Error reading info file for %s\n", name);
	    logerror("The info file \"%s\" is not regular file\n",
		     filename);
	    fclose(fp);
	    return RET_SYS;
	}
    }

    pr("Information on:  %s", name);
    while (fgets(buf, sizeof(buf), fp) != 0)
	pr("%s", buf);
    (void)fclose(fp);
    return RET_OK;
}

int
apro(void)
{
    HANDLE hDir;
    WIN32_FIND_DATA fData;
    FILE *fp;
    s_char *name, *lbp;
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

    _snprintf(filename, sizeof(filename), "%s\\*",infodir);
    hDir = FindFirstFile(filename, &fData);
    if (hDir == INVALID_HANDLE_VALUE) {
	if (GetLastError() == ERROR_PATH_NOT_FOUND) {
	    pr("Can't open info dir\n");
	    logerror("Can't open info dir \"%s\"", infodir);
	} else {
	    pr("Error reading info dir\n");
	    logerror("Error (%lu) reading info dir(%s)\\file(%s)",
		GetLastError(), infodir, filename);
	}
	return RET_SYS;
    }

    fbuf = malloc(256);
    lbuf = malloc(256);
    lbp = malloc(256);

    /*
     *  lower case search string into lbp
     */
    name = player->argp[1];
    lowerit(lbp, 256, name);

    /*
     *  search
     */
    nf = nhf = nl = nhl = 0;
    do {
	if ((fData.dwFileAttributes != (DWORD)-1) &&
	    ((fData.dwFileAttributes == FILE_ATTRIBUTE_NORMAL) ||
	     (fData.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE) ||
	     (fData.dwFileAttributes == FILE_ATTRIBUTE_READONLY))) {
	    _snprintf(filename, sizeof(filename), "%s\\%s", infodir,
		      fData.cFileName);
	    fp = fopen(filename, "rb");
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
	}
    } while (FindNextFile(hDir, &fData));
    FindClose(hDir);

    free(fbuf);
    free(lbuf);
    free(lbp);

    if ((nhl) > lhitlim) {
	pr("Limit of %d lines exceeded\n", lhitlim);
    }
    pr("Found %s in %ld of %ld files and in %ld of %ld lines\n",
       name, nhf, nf, nhl, nl);
    return RET_OK;
}

#endif
