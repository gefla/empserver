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
 *  swap.c: Swap two sectors on the map
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#include <stdio.h>
#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "nat.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

static void print_res(struct sctstr *);

/*
 * Syntax: swap <SECT> <SECT>
 */
int
swaps(void)
{
    struct sctstr secta, sectb;
    coord x, y;
    s_char buf[1024];
    s_char *p;

    if (!(p = getstarg(player->argp[1], "First sector : ", buf)) ||
	!sarg_xy(p, &secta.sct_x, &secta.sct_y) ||
	!getsect(secta.sct_x, secta.sct_y, &secta))
	return RET_SYN;
    print_res(&secta);
    if (!(p = getstarg(player->argp[2], "Second sector : ", buf)) ||
	!sarg_xy(p, &sectb.sct_x, &sectb.sct_y) ||
	!getsect(sectb.sct_x, sectb.sct_y, &sectb))
	return RET_SYN;
    print_res(&sectb);
    if (!confirm ("Are you sure these are the two sectors you wish to swap? "))
	return RET_FAIL;
    /* save x and y from secta */
    x = secta.sct_x;
    y = secta.sct_y;
    /* change the location of secta to that of sectb */
    secta.sct_x = sectb.sct_x;
    secta.sct_y = sectb.sct_y;
    secta.sct_dist_x = sectb.sct_x;
    secta.sct_dist_y = sectb.sct_y;
    /* change the location of sectb to where secta was */
    sectb.sct_x = x;
    sectb.sct_y = y;
    sectb.sct_dist_x = x;
    sectb.sct_dist_y = y;
    putsect(&secta);
    putsect(&sectb);
    pr("done\n");
    return RET_OK;
}

static void
print_res(struct sctstr *sp)
{
    pr("own   sect        eff  min gold fert oil uran\n");

    pr("%3d ", sp->sct_own);

    prxy("%4d,%-4d", sp->sct_x, sp->sct_y, player->cnum);
    pr(" %c", dchr[sp->sct_type].d_mnem);
    if (sp->sct_newtype != sp->sct_type)
	pr("%c", dchr[sp->sct_newtype].d_mnem);
    else
	pr(" ");
    pr("%4d%%", sp->sct_effic);
    pr(" %4d", sp->sct_min);
    pr("%5d", sp->sct_gmin);
    pr("%5d", sp->sct_fertil);
    pr("%4d", sp->sct_oil);
    pr("%5d", sp->sct_uran);
    pr("\n");
}
