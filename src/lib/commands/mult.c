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
 *  mult.c: Set per-nation list of price multipliers
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "nat.h"
#include "file.h"
#include "xy.h"
#include "nsc.h"
#include "commands.h"
#include "optlist.h"
#include <math.h>		/* bailey@math-cs.kent.edu */

static void multsingle(natid us, natid them, struct natstr *natp);

int
mult(void)
{
    struct nstr_item ni;
    struct natstr nat;
    int nats;

    if (!opt_MARKET) {
	pr("The market is disabled.\n");
	return RET_FAIL;
    }
    pr("The mult command is no longer used.\n");
    if (!snxtitem(&ni, EF_NATION, player->argp[1]))
	return RET_SYN;
    if (commread() < 0) {
	pr("Unable to read commodity file; get help!\n");
	return RET_SYS;
    }
    nats = 0;
    while (!player->aborted && nxtitem(&ni, (s_char *)&nat)) {
	if ((nat.nat_stat & STAT_NORM) == 0)
	    continue;
	multsingle(player->cnum, (natid)ni.cur, &nat);
	nats++;
    }
    pr("%d nation multipliers changed\n", nats);
    return RET_OK;
}

/*
 * Set the multipler for a single country.
 */
static void
multsingle(natid us, natid them, struct natstr *natp)
{
    extern double minmult;
    extern double maxmult;
    double price;
    s_char *p;
    s_char prompt[128];
    s_char buf[1024];

    sprintf(prompt, "%s (%7.3f) : ", natp->nat_cnam, multread(us, them));
    p = getstarg(player->argp[2], prompt, buf);
    if (p == 0 || *p == 0)
	return;
    if ((price = atof(p)) == 0.0)
	return;
#if defined(HUGE)
    if ((price == HUGE) || (price > 1000000.0))	/* Inf causes overflow. */
#else
    if (price > 1000000.0)
#endif
	price = 1000000.0;	/* bailey@math-cs.kent.edu */
    /*
     * no free lunches!
     */
    if (price <= minmult)
	price = minmult;
    if (price >= maxmult)
	price = maxmult;
    if (commread() < 0) {
	pr("Unable to re-read commodity file; get help!\n");
	return;
    }
    multset(them, price);
    if (commwrite() < 0) {
	pr("Unable to write out commodity file; get help!\n");
    }
}
