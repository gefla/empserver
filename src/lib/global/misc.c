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
 *  misc.c: Misc type description lists
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"

s_char    *effadv_list[]   = {
	"minimally",	"partially",	"moderately",	"completely", 0,
};

/* must follow nation relation defines in nat.h */
s_char	*relates[] = {
        "At War", "Sitzkrieg", "Mobilizing", "Hostile", "Neutral", "Friendly", "Allied"
};

/* must follow reject flags defined in nat.h */
s_char	*rejects[] = {
	"  YES  YES  YES  YES",
	"  NO   YES  YES  YES",
	"  YES  NO   YES  YES",
	"  NO   NO   YES  YES",
	"  YES  YES  NO   YES",
	"  NO   YES  NO   YES",
	"  YES  NO   NO   YES",
	"  NO   NO   NO   YES",
	"  YES  YES  YES  NO ",
	"  NO   YES  YES  NO ",
	"  YES  NO   YES  NO ",
	"  NO   NO   YES  NO ",
	"  YES  YES  NO   NO ",
	"  NO   YES  NO   NO ",
	"  YES  NO   NO   NO ",
	"  NO   NO   NO   NO "
};

/* must follow TEL_ defines in tel.h */
s_char	*telnames[] = {
	"Telegram", "Announcement", "BULLETIN", "Production Report"
};

s_char  *att_mode[] = {
	"defend", "attack", "assault", "paradrop", "board", "lboard"
};
