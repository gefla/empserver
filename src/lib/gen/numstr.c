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
 *  numstr.c: Turn a number into a word
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include <config.h>

#include "prototypes.h"

char *
numstr(char *buf, int n)
{
    static char *numnames[] = {
	"zero", "one", "two", "three", "four", "five", "six",
	"seven", "eight", "nine", "ten", "eleven", "twelve",
	"thirteen", "fourteen", "fifteen", "sixteen",
	"seventeen", "eighteen", "nineteen",
    };
    static char *tennames[] = {
	"", "", "twenty", "thirty", "forty", "fifty",
	"sixty", "seventy", "eighty", "ninety", "hundred"
    };

    if (n > 100) {
	(void)strcpy(buf, "several");
    } else if (n < 0) {
	(void)strcpy(buf, "a negative number of");
    } else {
	if (n >= 20) {
	    (void)strcpy(buf, tennames[n / 10]);
	    if (n % 10) {
		(void)strcat(buf, "-");
		(void)strcat(buf, numnames[n % 10]);
	    }
	} else {
	    (void)strcpy(buf, numnames[n % 20]);
	}
    }
    return buf;
}

char *
effadv(int n)
{
    static char *effadv_list[] = {
	"minimally", "partially", "moderately", "completely",
    };

    if (n < 0)
	n = 0;
    if (n >= 100)
	n = 99;
    return effadv_list[n / 25];
}
