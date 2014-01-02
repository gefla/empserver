/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  hours.c: Game hours determination; is it legal to play now?
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2010
 */

#include <config.h>

#include <ctype.h>
#include <time.h>
#include "misc.h"
#include "optlist.h"
#include "prototypes.h"

static char *weekday(char *str, int *wday);
static char *daytime_range(char *str, int *from_min, int *to_min);

/*
 * Is week day WDAY (Sunday is 0) allowed by restriction DAYS?
 * If DAYS is not empty, it lists the allowed week day names.  See
 * weekday() for syntax.
 */
int
is_wday_allowed(int wday, char *days)
{
    int wd;

    if (!days || !*days)
	return 1;

    while (NULL != (days = weekday(days, &wd)))
	if (wd == wday)
	    return 1;

    return 0;
}

/*
 * Is day time DTIME (minutes since midnight) allowed by restriction TIMES?
 * If TIMES is not empty, it lists the allowed day time ranges.  See
 * daytime_range() for syntax.
 */
int
is_daytime_allowed(int dtime, char *times)
{
    int from, to;

    if (!times || !*times)
	return 1;

    while (NULL != (times = daytime_range(times, &from, &to)))
	if (from <= dtime && dtime < to)
	    return 1;

    return 0;
}

/*
 * Can the game played at time T?
 */
int
gamehours(time_t t)
{
    struct tm *tm;

    tm = localtime(&t);
    if (!is_wday_allowed(tm->tm_wday, game_days))
	return 0;
    return is_daytime_allowed(60 * tm->tm_hour + tm->tm_min, game_hours);
}

/*
 * Parse weekday name in STR.
 * On success assign day number (Sunday is 0) to *WDAY and return
 * pointer to first character not parsed.
 * Else return NULL.
 * Abbreviated names are recognized, but not single characters.
 * Initial whitespace is ignored.
 */
static char *
weekday(char *str, int *wday)
{
    /*
     * strptime() format " %a" would do fine, but it's XPG and Windows
     * doesn't have it.  Besides, Empire accepts more abbreviations.
     */
    static char *day_name[7] = {
	"sunday", "monday", "tuesday", "wednesday",
	"thursday", "friday", "saturday"
    };
    int i, j;

    for (; isspace(*str); ++str) ;

    for (i = 0; i < 7; ++i) {
	j = 0;
	while (str[j] && tolower(str[j]) == day_name[i][j])
	    ++j;
	if (j > 1) {
	    *wday = i;
	    return str + j;
	}
    }

    return NULL;
}

/*
 * Parse day time in STR.
 * On success store minutes since midnight in *MIN and return pointer
 * to first character not parsed.
 * Else return NULL.
 * Time format is HOUR:MINUTE.  Initial whitespace is ignored.
 */
static char *
daytime(char *str, int *min)
{
    /*
     * strptime() format " %H:%M" would do fine, but it's XPG and
     * Windows doesn't have it.
     */
    char *end;
    unsigned long h, m;

    h = strtoul(str, &end, 10);
    if (end == str || h > 24)
	return NULL;

    if (*end++ != ':')
	return NULL;

    str = end;
    m = strtoul(str, &end, 10);
    if (end == str || m > 59)
	return NULL;
    else if (h == 24 && m != 0)
	return NULL;

    *min = 60 * h + m;
    return end;
}

/*
 * Parse a day time range in STR.
 * On success store minutes since midnight in *FROM and *TO, return
 * pointer to first character not parsed.
 * Else return NULL.
 * Format is HOUR:MINUTE-HOUR:MINUTE.  Initial whitespace is ignored.
 */
static char *
daytime_range(char *str, int *from_min, int *to_min)
{
    char *end;

    end = daytime(str, from_min);
    if (!end)
	return NULL;
    while (isspace(*end)) ++end;
    if (*end++ != '-')
	return NULL;
    return daytime(end, to_min);
}
