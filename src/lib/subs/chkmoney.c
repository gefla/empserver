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
 *  chkmoney.c: Some routines to check if a country will go broke
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "prototypes.h"

int
chkmoney(long int cost, long int cash, s_char *argp)
{
    s_char buf[1024];
    s_char *p;

    if (cash > 0 && cost > cash / 2) {
	pr("This operation will cost you $%ld, and you only have $%ld.\n",
	   cost, cash);
	if (cost > cash) {
	    pr("You will be broke with $%ld if you proceed with this command.\n", cash - cost);
	}
	p = getstarg(argp, "Are you sure that you want to do this? ", buf);
	if (p == 0 || *p != 'y')
	    return RET_SYN;
    }
    return 0;
}

int
check_cost(int looping, int cost, long int cash, int *warnedp,
	   s_char *argp)
{
    s_char buf[1024];
    s_char *p;

    if (looping && cash > 0 && player->dolcost + cost > cash
	&& *warnedp < 2) {
	*warnedp = 2;
	pr("You will go broke!  (it will cost $%d and you only have $%ld)\n", cost, cash - (long)player->dolcost);
	p = getstarg(argp, "Are you sure you wish to continue? ", buf);
	if (p == 0 || *p != 'y')
	    return 1;
    }
    player->dolcost += cost;
    if (looping && cash > 0 && player->dolcost > cash / 2 && *warnedp < 1) {
	*warnedp = 1;
	pr("WARNING.  You have just spent over half of your money.\n");
	pr("You started with $%ld and now you only have $%ld left\n", cash,
	   cash - (long)player->dolcost);
    }
    return 0;
}
