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
 *  keyword.c: Find keywords in a file
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "keyword.h"
#include "gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "common.h"

struct kwtab {
    struct kwtab *next;
    s_char *name;
    s_char *text;
};

struct kwtab *kw_list;

int
kw_read(FILE * fp)
{
    register struct kwtab *kw;
    register struct kwtab *next;
    s_char buf[255];
    s_char *p;
    int n;

    for (kw = kw_list; kw != 0; kw = next) {
	next = kw->next;
	free(kw->name);
	free(kw->text);
	free(kw);
    }
    kw_list = 0;
    for (n = 0; fgets(buf, sizeof(buf), fp) != 0; n++) {
	/* Allow for comments.. any line starting with # */
	if (buf[0] == '#')
	    continue;
	p = rindex(buf, '\n');
	if (p != 0)
	    *p = 0;
	if ((p = index(buf, ':')) == 0) {
	    logerror("kw_read: Bad keyword line #%d\n", n);
	    return 0;
	}
	*p++ = 0;
	while (*p && isspace(*p))
	    p++;
	kw = (struct kwtab *)malloc(sizeof(*kw));
	kw->name = strcpy(malloc(strlen(buf) + 1), buf);
	kw->text = strcpy(malloc(strlen(p) + 1), p);
	kw->next = kw_list;
	kw_list = kw;
    }
    return n;
}

s_char *
kw_find(s_char *name)
{
    register struct kwtab *kw;

    for (kw = kw_list; kw != 0; kw = kw->next) {
	if (strcmp(kw->name, name) == 0)
	    return kw->text;
    }
    return 0;
}

#define CF_VALUE	1
#define CF_TIME		2
#define CF_TIMERANGE	3
#define CF_WEEKDAY	4

/*
 * destructive parse
 */
s_char *
kw_parse(int type, s_char *text, int *data)
{
    s_char *get_time(s_char *ptr, int *data);
    s_char *weekday(s_char *ptr, int *data);
    s_char *next;

    while (isspace(*text))
	text++;
    switch (type) {
    case CF_VALUE:
	*data = atoip(&text);
	break;
    case CF_TIME:
	text = get_time(text, &data[0]);
	break;
    case CF_TIMERANGE:
	if ((next = index(text, '-')) == 0)
	    return 0;
	next++;
	if ((text = get_time(text, &data[0])) == 0)
	    return 0;
	text = get_time(next, &data[1]);
	break;
    case CF_WEEKDAY:
	text = weekday(text, &data[0]);
	break;
    default:
	text = 0;
	break;
    }
    return text;
}

struct day {
    s_char *string;
    int day[7];
} day[] = {
    {
	"smtwtfs", {
    -1, 0, -1, 2, -1, 4, -1}}, {
	"uouehra", {
    0, 1, 2, 3, 4, 5, 6}}
};

s_char *
weekday(s_char *ptr, int *data)
{
    register s_char *string;
    register int c;
    register int n;

    c = *ptr++;
    if (isupper(c))
	c = tolower(c);
    string = day[0].string;
    for (n = 0; n < 7; n++) {
	if (string[n] != c)
	    continue;
	if (day[0].day[n] >= 0)
	    break;
	if (day[1].string[n] == *ptr)
	    break;
    }
    if (n == 7)
	return 0;
    *data = day[1].day[n];
    while (*ptr && !isspace(*ptr))
	ptr++;
    return ptr;
}


s_char *
get_time(s_char *ptr, int *data)
{
    int hour;
    int minute;

    if (!isdigit(*ptr))
	return 0;
    hour = atoip(&ptr);
    minute = 0;
    if (*ptr) {
	if (*ptr != ':')
	    return 0;
	ptr++;
	if (!isdigit(*ptr))
	    return 0;
	minute = atoip(&ptr);
    }
    *data = (hour * 60) + minute;
    return ptr;
}
