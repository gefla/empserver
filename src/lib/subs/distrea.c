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
 *  distrea.c: Display treaty information
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "treaty.h"
#include "nat.h"
#include "deity.h"
#include "file.h"
#include "prototypes.h"

int
distrea(int n, register struct trtstr *tp)
{
	register int i;
	int	acond;
	int	bcond;
	int	cond;
	time_t	now;

	if (tp->trt_status == TS_FREE)
		return 0;
	if (tp->trt_cna != player->cnum &&
	    tp->trt_cnb != player->cnum && !player->god)
		return 0;
	(void) time(&now);
	if (now > tp->trt_exp) {
		if (!ef_lock(EF_TREATY)) {
			pr("Can't lock treaty file; get help!\n");
			return 0;
		}
		tp->trt_status = TS_FREE;
		if (!puttre(n, tp)) {
			pr("Couldn't save treaty; get help!\n");
			(void) ef_unlock(EF_TREATY);
			return 0;
		}
		(void) ef_unlock(EF_TREATY);
		pr("Treaty #%d expired %s", n, ctime(&tp->trt_exp));
		return 0;
	}
	pr("\n      * * *  Empire Treaty #%d  * * *\n", n);
	if (tp->trt_status == TS_PROPOSED)
		pr("(proposed)\n");
	pr("between %s and ", cname(tp->trt_cna));
	pr("%s  expires %s", cname(tp->trt_cnb), ctime(&tp->trt_exp));
	pr("%24.24s terms", cname(tp->trt_cna));
	pr(" - %s terms\n", cname(tp->trt_cnb));
	for (i = 0; 0 != (cond = tchr[i].t_cond); i++) {
		acond = tp->trt_acond & cond;
		bcond = tp->trt_bcond & cond;
		if (acond | bcond) {
			if (acond)
				pr("%30s", tchr[i].t_name);
			else
				pr("%30s", "");
			if (bcond)
				pr(" - %s\n", tchr[i].t_name);
			else
				pr(" -\n");
		}
	}
	return 1;
}
