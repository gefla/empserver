/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  dump.c: Dump sector information
 *
 *  Known contributors to this file:
 *     Jeff Wallace, 1989
 *     Steve McClure, 1997 (added timestamping)
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "path.h"

int
dump(void)
{
    struct sctstr sect;
    char dirstr[20];
    int nsect;
    int n, i;
    struct nstr_sect nstr;
    int field[128];
    struct natstr *np;
    time_t now;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    (void)strcpy(dirstr, ".      $");
    for (n = 1; n <= 6; n++)
	dirstr[n] = dirch[n];
    nsect = 0;
    np = getnatp(player->cnum);

    if (!player->argp[2]) {
	for (n = 1; n <= 80; n++)
	    field[n - 1] = n;
	field[n - 1] = 0;
    } else {
	n = 2;
	i = 0;
	while (player->argp[n]) {
	    if (!strcmp("des", player->argp[n])) {
		field[i++] = 1;
	    } else if (!strcmp("sdes", player->argp[n])) {
		field[i++] = 2;
	    } else if (!strcmp("eff", player->argp[n])) {
		field[i++] = 3;
	    } else if (!strcmp("mob", player->argp[n])) {
		field[i++] = 4;
	    } else if (!strcmp("*", player->argp[n])) {
		field[i++] = 5;
	    } else if (!strcmp("off", player->argp[n])) {
		field[i++] = 6;
	    } else if (!strcmp("min", player->argp[n])) {
		field[i++] = 7;
	    } else if (!strcmp("gold", player->argp[n])) {
		field[i++] = 8;
	    } else if (!strcmp("fert", player->argp[n])) {
		field[i++] = 9;
	    } else if (!strcmp("ocontent", player->argp[n])) {
		field[i++] = 10;
	    } else if (!strcmp("uran", player->argp[n])) {
		field[i++] = 11;
	    } else if (!strcmp("work", player->argp[n])) {
		field[i++] = 12;
	    } else if (!strcmp("avail", player->argp[n])) {
		field[i++] = 13;
	    } else if (!strcmp("terr", player->argp[n])) {
		field[i++] = 14;
	    } else if (!strcmp("civ", player->argp[n])) {
		field[i++] = 15;
	    } else if (!strcmp("mil", player->argp[n])) {
		field[i++] = 16;
	    } else if (!strcmp("uw", player->argp[n])) {
		field[i++] = 17;
	    } else if (!strcmp("food", player->argp[n])) {
		field[i++] = 18;
	    } else if (!strcmp("shell", player->argp[n])) {
		field[i++] = 19;
	    } else if (!strcmp("gun", player->argp[n])) {
		field[i++] = 20;
	    } else if (!strcmp("pet", player->argp[n])) {
		field[i++] = 21;
	    } else if (!strcmp("iron", player->argp[n])) {
		field[i++] = 22;
	    } else if (!strcmp("dust", player->argp[n])) {
		field[i++] = 23;
	    } else if (!strcmp("bar", player->argp[n])) {
		field[i++] = 24;
	    } else if (!strcmp("oil", player->argp[n])) {
		field[i++] = 25;
	    } else if (!strcmp("lcm", player->argp[n])) {
		field[i++] = 26;
	    } else if (!strcmp("hcm", player->argp[n])) {
		field[i++] = 27;
	    } else if (!strcmp("rad", player->argp[n])) {
		field[i++] = 28;
	    } else if (!strcmp("u_del", player->argp[n])) {
		field[i++] = 29;
	    } else if (!strcmp("f_del", player->argp[n])) {
		field[i++] = 30;
	    } else if (!strcmp("s_del", player->argp[n])) {
		field[i++] = 31;
	    } else if (!strcmp("g_del", player->argp[n])) {
		field[i++] = 32;
	    } else if (!strcmp("p_del", player->argp[n])) {
		field[i++] = 33;
	    } else if (!strcmp("i_del", player->argp[n])) {
		field[i++] = 34;
	    } else if (!strcmp("d_del", player->argp[n])) {
		field[i++] = 35;
	    } else if (!strcmp("b_del", player->argp[n])) {
		field[i++] = 36;
	    } else if (!strcmp("o_del", player->argp[n])) {
		field[i++] = 37;
	    } else if (!strcmp("l_del", player->argp[n])) {
		field[i++] = 38;
	    } else if (!strcmp("h_del", player->argp[n])) {
		field[i++] = 39;
	    } else if (!strcmp("r_del", player->argp[n])) {
		field[i++] = 40;
	    } else if (!strcmp("u_cut", player->argp[n])) {
		field[i++] = 41;
	    } else if (!strcmp("f_cut", player->argp[n])) {
		field[i++] = 42;
	    } else if (!strcmp("s_cut", player->argp[n])) {
		field[i++] = 43;
	    } else if (!strcmp("g_cut", player->argp[n])) {
		field[i++] = 44;
	    } else if (!strcmp("p_cut", player->argp[n])) {
		field[i++] = 45;
	    } else if (!strcmp("i_cut", player->argp[n])) {
		field[i++] = 46;
	    } else if (!strcmp("d_cut", player->argp[n])) {
		field[i++] = 47;
	    } else if (!strcmp("b_cut", player->argp[n])) {
		field[i++] = 48;
	    } else if (!strcmp("o_cut", player->argp[n])) {
		field[i++] = 49;
	    } else if (!strcmp("l_cut", player->argp[n])) {
		field[i++] = 50;
	    } else if (!strcmp("h_cut", player->argp[n])) {
		field[i++] = 51;
	    } else if (!strcmp("r_cut", player->argp[n])) {
		field[i++] = 52;
	    } else if (!strcmp("dist_x", player->argp[n])) {
		field[i++] = 53;
	    } else if (!strcmp("dist_y", player->argp[n])) {
		field[i++] = 54;
	    } else if (!strcmp("c_dist", player->argp[n])) {
		field[i++] = 55;
	    } else if (!strcmp("m_dist", player->argp[n])) {
		field[i++] = 56;
	    } else if (!strcmp("u_dist", player->argp[n])) {
		field[i++] = 57;
	    } else if (!strcmp("f_dist", player->argp[n])) {
		field[i++] = 58;
	    } else if (!strcmp("s_dist", player->argp[n])) {
		field[i++] = 59;
	    } else if (!strcmp("g_dist", player->argp[n])) {
		field[i++] = 60;
	    } else if (!strcmp("p_dist", player->argp[n])) {
		field[i++] = 61;
	    } else if (!strcmp("i_dist", player->argp[n])) {
		field[i++] = 62;
	    } else if (!strcmp("d_dist", player->argp[n])) {
		field[i++] = 63;
	    } else if (!strcmp("b_dist", player->argp[n])) {
		field[i++] = 64;
	    } else if (!strcmp("o_dist", player->argp[n])) {
		field[i++] = 65;
	    } else if (!strcmp("l_dist", player->argp[n])) {
		field[i++] = 66;
	    } else if (!strcmp("h_dist", player->argp[n])) {
		field[i++] = 67;
	    } else if (!strcmp("r_dist", player->argp[n])) {
		field[i++] = 68;
	    } else if (!strcmp("road", player->argp[n])) {
		field[i++] = 69;
	    } else if (!strcmp("rail", player->argp[n])) {
		field[i++] = 70;
	    } else if (!strcmp("defense", player->argp[n])) {
		field[i++] = 71;
	    } else if (!strcmp("fallout", player->argp[n])) {
		field[i++] = 72;
	    } else if (!strcmp("coast", player->argp[n])) {
		field[i++] = 73;
	    } else if (!strcmp("c_del", player->argp[n])) {
		field[i++] = 74;
	    } else if (!strcmp("m_del", player->argp[n])) {
		field[i++] = 75;
	    } else if (!strcmp("c_cut", player->argp[n])) {
		field[i++] = 76;
	    } else if (!strcmp("m_cut", player->argp[n])) {
		field[i++] = 77;
	    } else if (!strcmp("terr1", player->argp[n])) {
		field[i++] = 78;
	    } else if (!strcmp("terr2", player->argp[n])) {
		field[i++] = 79;
	    } else if (!strcmp("terr3", player->argp[n])) {
		field[i++] = 80;
	    } else {
		pr("Unrecognized field %s\n", player->argp[n]);
	    }
	    if (n++ > 100) {
		pr("Too many fields\n");
		return RET_FAIL;
	    }
	}
	field[i] = 0;
    }

    if (player->god)
	pr("   ");
    time(&now);
    pr("DUMP SECTOR %ld\n", (long)now);
    if (player->god)
	pr("own ");
    pr("x y");
    n = 0;
    while (field[n]) {
	pr(" ");
	switch (field[n]) {
	case 1:
	    pr("des");
	    break;
	case 2:
	    pr("sdes");
	    break;
	case 3:
	    pr("eff");
	    break;
	case 4:
	    pr("mob");
	    break;
	case 5:
	    pr("*");
	    break;
	case 6:
	    pr("off");
	    break;
	case 7:
	    pr("min");
	    break;
	case 8:
	    pr("gold");
	    break;
	case 9:
	    pr("fert");
	    break;
	case 10:
	    pr("ocontent");
	    break;
	case 11:
	    pr("uran");
	    break;
	case 12:
	    pr("work");
	    break;
	case 13:
	    pr("avail");
	    break;
	case 14:
	    pr("terr");
	    break;
	case 15:
	    pr("civ");
	    break;
	case 16:
	    pr("mil");
	    break;
	case 17:
	    pr("uw");
	    break;
	case 18:
	    pr("food");
	    break;
	case 19:
	    pr("shell");
	    break;
	case 20:
	    pr("gun");
	    break;
	case 21:
	    pr("pet");
	    break;
	case 22:
	    pr("iron");
	    break;
	case 23:
	    pr("dust");
	    break;
	case 24:
	    pr("bar");
	    break;
	case 25:
	    pr("oil");
	    break;
	case 26:
	    pr("lcm");
	    break;
	case 27:
	    pr("hcm");
	    break;
	case 28:
	    pr("rad");
	    break;
	case 29:
	    pr("u_del");
	    break;
	case 30:
	    pr("f_del");
	    break;
	case 31:
	    pr("s_del");
	    break;
	case 32:
	    pr("g_del");
	    break;
	case 33:
	    pr("p_del");
	    break;
	case 34:
	    pr("i_del");
	    break;
	case 35:
	    pr("d_del");
	    break;
	case 36:
	    pr("b_del");
	    break;
	case 37:
	    pr("o_del");
	    break;
	case 38:
	    pr("l_del");
	    break;
	case 39:
	    pr("h_del");
	    break;
	case 40:
	    pr("r_del");
	    break;
	case 41:
	    pr("u_cut");
	    break;
	case 42:
	    pr("f_cut");
	    break;
	case 43:
	    pr("s_cut");
	    break;
	case 44:
	    pr("g_cut");
	    break;
	case 45:
	    pr("p_cut");
	    break;
	case 46:
	    pr("i_cut");
	    break;
	case 47:
	    pr("d_cut");
	    break;
	case 48:
	    pr("b_cut");
	    break;
	case 49:
	    pr("o_cut");
	    break;
	case 50:
	    pr("l_cut");
	    break;
	case 51:
	    pr("h_cut");
	    break;
	case 52:
	    pr("r_cut");
	    break;
	case 53:
	    pr("dist_x");
	    break;
	case 54:
	    pr("dist_y");
	    break;
	case 55:
	    pr("c_dist");
	    break;
	case 56:
	    pr("m_dist");
	    break;
	case 57:
	    pr("u_dist");
	    break;
	case 58:
	    pr("f_dist");
	    break;
	case 59:
	    pr("s_dist");
	    break;
	case 60:
	    pr("g_dist");
	    break;
	case 61:
	    pr("p_dist");
	    break;
	case 62:
	    pr("i_dist");
	    break;
	case 63:
	    pr("d_dist");
	    break;
	case 64:
	    pr("b_dist");
	    break;
	case 65:
	    pr("o_dist");
	    break;
	case 66:
	    pr("l_dist");
	    break;
	case 67:
	    pr("h_dist");
	    break;
	case 68:
	    pr("r_dist");
	    break;
	case 69:
	    pr("road");
	    break;
	case 70:
	    pr("rail");
	    break;
	case 71:
	    pr("defense");
	    break;
	case 72:
	    pr("fallout");
	    break;
	case 73:
	    pr("coast");
	    break;
	case 74:
	    pr("c_del");
	    break;
	case 75:
	    pr("m_del");
	    break;
	case 76:
	    pr("c_cut");
	    break;
	case 77:
	    pr("m_cut");
	    break;
	case 78:
	    pr("terr1");
	    break;
	case 79:
	    pr("terr2");
	    break;
	case 80:
	    pr("terr3");
	    break;
	}
	n++;
    }
    pr("\n");

    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	nsect++;

/* census */
	if (player->god)
	    pr("%d ", sect.sct_own);
	pr("%d %d", xrel(np, nstr.x), yrel(np, nstr.y));

	n = 0;
	while (field[n]) {
	    pr(" ");
	    switch (field[n++]) {
	    case 1:
		pr("%c", dchr[sect.sct_type].d_mnem);
		break;
	    case 2:
		if (sect.sct_newtype != sect.sct_type)
		    pr("%c", dchr[sect.sct_newtype].d_mnem);
		else
		    pr("_");
		break;
	    case 3:
		pr("%d", sect.sct_effic);
		break;
	    case 4:
		pr("%d", sect.sct_mobil);
		break;
	    case 5:
		pr("%c", sect.sct_own != sect.sct_oldown ? '*' : '.');
		break;
	    case 6:
		pr("%d", sect.sct_off);
		break;
	    case 7:
		pr("%d", sect.sct_min);
		break;
	    case 8:
		pr("%d", sect.sct_gmin);
		break;
	    case 9:
		pr("%d", sect.sct_fertil);
		break;
	    case 10:
		pr("%d", sect.sct_oil);
		break;
	    case 11:
		pr("%d", sect.sct_uran);
		break;
	    case 12:
		pr("%d", sect.sct_work);
		break;
	    case 13:
		pr("%d", sect.sct_avail);
		break;
	    case 14:
		pr("%d", sect.sct_terr);
		break;
	    case 15:
		pr("%d", sect.sct_item[I_CIVIL]);
		break;
	    case 16:
		pr("%d", sect.sct_item[I_MILIT]);
		break;
	    case 17:
		pr("%d", sect.sct_item[I_UW]);
		break;
	    case 18:
		pr("%d", sect.sct_item[I_FOOD]);
		break;
/* commodity */
	    case 19:
		pr("%d", sect.sct_item[I_SHELL]);
		break;
	    case 20:
		pr("%d", sect.sct_item[I_GUN]);
		break;
	    case 21:
		pr("%d", sect.sct_item[I_PETROL]);
		break;
	    case 22:
		pr("%d", sect.sct_item[I_IRON]);
		break;
	    case 23:
		pr("%d", sect.sct_item[I_DUST]);
		break;
	    case 24:
		pr("%d", sect.sct_item[I_BAR]);
		break;
	    case 25:
		pr("%d", sect.sct_item[I_OIL]);
		break;
	    case 26:
		pr("%d", sect.sct_item[I_LCM]);
		break;
	    case 27:
		pr("%d", sect.sct_item[I_HCM]);
		break;
	    case 28:
		pr("%d", sect.sct_item[I_RAD]);
		break;
/* cutoff */
	    case 29:
		pr("%c", dirstr[sect.sct_del[I_UW] & 0x7]);
		break;
	    case 30:
		pr("%c", dirstr[sect.sct_del[I_FOOD] & 0x7]);
		break;
	    case 31:
		pr("%c", dirstr[sect.sct_del[I_SHELL] & 0x7]);
		break;
	    case 32:
		pr("%c", dirstr[sect.sct_del[I_GUN] & 0x7]);
		break;
	    case 33:
		pr("%c", dirstr[sect.sct_del[I_PETROL] & 0x7]);
		break;
	    case 34:
		pr("%c", dirstr[sect.sct_del[I_IRON] & 0x7]);
		break;
	    case 35:
		pr("%c", dirstr[sect.sct_del[I_DUST] & 0x7]);
		break;
	    case 36:
		pr("%c", dirstr[sect.sct_del[I_BAR] & 0x7]);
		break;
	    case 37:
		pr("%c", dirstr[sect.sct_del[I_OIL] & 0x7]);
		break;
	    case 38:
		pr("%c", dirstr[sect.sct_del[I_LCM] & 0x7]);
		break;
	    case 39:
		pr("%c", dirstr[sect.sct_del[I_HCM] & 0x7]);
		break;
	    case 40:
		pr("%c", dirstr[sect.sct_del[I_RAD] & 0x7]);
		break;
	    case 41:
		pr("%d", sect.sct_del[I_UW] & ~0x7);
		break;
	    case 42:
		pr("%d", sect.sct_del[I_FOOD] & ~0x7);
		break;
	    case 43:
		pr("%d", sect.sct_del[I_SHELL] & ~0x7);
		break;
	    case 44:
		pr("%d", sect.sct_del[I_GUN] & ~0x7);
		break;
	    case 45:
		pr("%d", sect.sct_del[I_PETROL] & ~0x7);
		break;
	    case 46:
		pr("%d", sect.sct_del[I_IRON] & ~0x7);
		break;
	    case 47:
		pr("%d", sect.sct_del[I_DUST] & ~0x7);
		break;
	    case 48:
		pr("%d", sect.sct_del[I_BAR] & ~0x7);
		break;
	    case 49:
		pr("%d", sect.sct_del[I_OIL] & ~0x7);
		break;
	    case 50:
		pr("%d", sect.sct_del[I_LCM] & ~0x7);
		break;
	    case 51:
		pr("%d", sect.sct_del[I_HCM] & ~0x7);
		break;
	    case 52:
		pr("%d", sect.sct_del[I_RAD] & ~0x7);
		break;
/* level */
	    case 53:
		pr("%d", xrel(getnatp(player->cnum), sect.sct_dist_x));
		break;
	    case 54:
		pr("%d", yrel(getnatp(player->cnum), sect.sct_dist_y));
		break;
	    case 55:
		pr("%d", sect.sct_dist[I_CIVIL]);
		break;
	    case 56:
		pr("%d", sect.sct_dist[I_MILIT]);
		break;
	    case 57:
		pr("%d", sect.sct_dist[I_UW]);
		break;
	    case 58:
		pr("%d", sect.sct_dist[I_FOOD]);
		break;
	    case 59:
		pr("%d", sect.sct_dist[I_SHELL]);
		break;
	    case 60:
		pr("%d", sect.sct_dist[I_GUN]);
		break;
	    case 61:
		pr("%d", sect.sct_dist[I_PETROL]);
		break;
	    case 62:
		pr("%d", sect.sct_dist[I_IRON]);
		break;
	    case 63:
		pr("%d", sect.sct_dist[I_DUST]);
		break;
	    case 64:
		pr("%d", sect.sct_dist[I_BAR]);
		break;
	    case 65:
		pr("%d", sect.sct_dist[I_OIL]);
		break;
	    case 66:
		pr("%d", sect.sct_dist[I_LCM]);
		break;
	    case 67:
		pr("%d", sect.sct_dist[I_HCM]);
		break;
	    case 68:
		pr("%d", sect.sct_dist[I_RAD]);
		break;
	    case 69:
		pr("%d", sect.sct_road);
		break;
	    case 70:
		pr("%d", sect.sct_rail);
		break;
	    case 71:
		pr("%d", SCT_DEFENSE(&sect));
		break;
	    case 72:
		if (opt_FALLOUT)
		    pr("%d", sect.sct_fallout);
		else
		    pr("0");
		break;
	    case 73:
		pr("%d", sect.sct_coastal);
		break;
	    case 74:
		pr("%c", dirstr[sect.sct_del[I_CIVIL] & 0x7]);
		break;
	    case 75:
		pr("%c", dirstr[sect.sct_del[I_MILIT] & 0x7]);
		break;
	    case 76:
		pr("%d", sect.sct_del[I_CIVIL] & ~0x7);
		break;
	    case 77:
		pr("%d", sect.sct_del[I_MILIT] & ~0x7);
		break;
	    case 78:
		pr("%d", sect.sct_terr1);
		break;
	    case 79:
		pr("%d", sect.sct_terr2);
		break;
	    case 80:
		pr("%d", sect.sct_terr3);
		break;
	    }
	}
	pr("\n");
    }
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return RET_OK;
}
