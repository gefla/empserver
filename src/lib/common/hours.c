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
 *  hours.c: Game hours determination; is it legal to play now?
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Doug Hay, 1998
 *     Steve McClure, 1998
 */

#include <errno.h>
#include <stdio.h>
#include "misc.h"
#include "nat.h"
#include "tel.h"
#include "proto.h"
#include "com.h"
#include "deity.h"
#include "keyword.h"
#include "file.h"
#include "common.h"

#if defined(Rel4) || defined(_WIN32)
#include <time.h>
#else
#include <sys/time.h>
#endif /* Rel4 */

/*
 * returns true if game can be played now.
 * Sets the number of minutes until the hours
 * function must be re-called.
 */
int
gamehours(time_t now, int *hour)
{
    extern s_char *game_days, *game_hours;
    extern struct tm *localtime(const time_t *);
    register s_char *bp;
    register struct tm *tm;
    int day;
    int curtime;
    int okday[7];
    int tomorrow;

    tm = localtime(&now);
    curtime = tm->tm_min + tm->tm_hour * 60;
    bp = game_days;
    if (*bp != 0) {
	for (day = 0; day < 7; day++)
	    okday[day] = 0;
	while (NULL != (bp = kw_parse(CF_WEEKDAY, bp, &day)))
	    okday[day] = 1;
    } else {
	for (day = 0; day < 7; day++)
	    okday[day] = 1;
    }
    if (!okday[tm->tm_wday])
	return 0;
    bp = game_hours;
    if (*bp != 0) {
	while (NULL != (bp = kw_parse(CF_TIMERANGE, bp, hour)))
	    if (curtime >= hour[0] && curtime < hour[1])
		break;
	if (bp == 0)
	    return 0;
    } else {
	hour[0] = 0;
	hour[1] = 24 * 60;
    }
    tomorrow = tm->tm_wday + 1;
    if (tomorrow >= 7)
	tomorrow = 0;
    return 1;
}
