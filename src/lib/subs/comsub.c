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
 *  comsub.c: Commodity read/write stuff
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "nat.h"
#include "prototypes.h"

#if !defined(_WIN32)
#include <sys/uio.h>
#endif

int	commf;

int	c_comm[MAXNOC][I_MAX+1];
float	c_price[MAXNOC][I_MAX+1];
float	c_mult[MAXNOC][MAXNOC];

#if !defined(_WIN32)
static	struct iovec commvec[3] = {
	{ (caddr_t)c_comm,	sizeof(c_comm) },
	{ (caddr_t)c_price,	sizeof(c_price) },
	{ (caddr_t)c_mult,	sizeof(c_mult) }
};
static	int nvec = sizeof(commvec)/sizeof(struct iovec);
#endif

/*
 * comm file is the list of "deity commodities"
 * lying around.  Perhaps a better way of doing this
 * might be at each update seeing what is on sale,
 * then buying it, then distributing things amongst the
 * people wanting the items.
 *
 * Of course that would be a gigantic pain...
 */

int
commread(void)
{
	int	n;

	(void) lseek(commf, (off_t) 0, 0);
#if !defined(_WIN32)
	if ((n = readv(commf, commvec, nvec)) !=
	    sizeof(c_comm) + sizeof(c_price) + sizeof(c_mult)) {
		logerror("commread: readv returns %d", n);
		return -1;
	}
#else
	if ((n = read(commf, c_comm, sizeof(c_comm))) != sizeof(c_comm)) {
		logerror("commread: read returns %d, not %d", n, sizeof(c_comm));
		return -1;
    }
	if ((n = read(commf, c_price, sizeof(c_price))) != sizeof(c_price)) {
		logerror("commread: read returns %d, not %d", n, sizeof(c_price));
		return -1;
    }
	if ((n = read(commf, c_mult, sizeof(c_mult))) != sizeof(c_mult)) {
		logerror("commread: read returns %d, not %d", n, sizeof(c_mult));
		return -1;
    }
#endif
	return 0;
}

int
commwrite(void)
{
#if defined(_WIN32)
	int n;
#endif
	(void) lseek(commf, (off_t) 0, 0);
#if !defined(_WIN32)
	if (writev(commf, commvec, nvec) != 
	    sizeof(c_comm) + sizeof(c_price) + sizeof(c_mult)) {
		logerror("commwrite: writev failed");
		return -1;
	}
#else
	if ((n = write(commf, c_comm, sizeof(c_comm))) != sizeof(c_comm)) {
		logerror("commwrite: write returns %d, not %d", n, sizeof(c_comm));
		return -1;
    }
	if ((n = write(commf, c_price, sizeof(c_price))) != sizeof(c_price)) {
		logerror("commwrite: write returns %d, not %d", n, sizeof(c_price));
		return -1;
    }
	if ((n = write(commf, c_mult, sizeof(c_mult))) != sizeof(c_mult)) {
		logerror("commwrite: write returns %d, not %d", n, sizeof(c_mult));
		return -1;
    }
#endif
	return 0;
}

int
commlock(void)
{
	return file_lock(commf);
}

int
communlock(void)
{
	return file_unlock(commf);
}

/*
 * returns amount of commodity, and price to the user
 */
int
commamt(natid trader, int product, float *priceval)
{
	*priceval = c_price[trader][product] * c_mult[trader][player->cnum];
	return c_comm[trader][product];
}

void
commset(natid trader, int product, int amt)
{
	c_comm[trader][product] += amt;
}

double
multread(natid trader, natid tradee)
{
	return c_mult[trader][tradee];
}

void
multset(natid tradee, float newmult)
{
	c_mult[player->cnum][tradee] = newmult;
}

void
commprice(int product, float *newprice)
{
	c_price[player->cnum][product] = *newprice;
}
