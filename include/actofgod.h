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
 *  actofgod.h: Deity meddling
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2013
 */

#ifndef ACTOFGOD_H
#define ACTOFGOD_H

#include "item.h"
#include "nat.h"
#include "types.h"

extern void report_god_takes(char *, char *, natid);
extern void report_god_gives(char *, char *, natid);
extern void divine_load(struct empobj *, int, int);
extern void divine_unload(struct empobj *, int, int);
extern void divine_sct_change(struct sctstr *, char *, int, int, char *, ...)
    ATTRIBUTE((format (printf, 5, 6)));
#define divine_sct_change_quiet(sp, name, change, ...) \
    divine_sct_change((sp), (name), -(change), 0, __VA_ARGS__)
extern void divine_nat_change(struct natstr *, char *, int, int, char *, ...)
    ATTRIBUTE((format (printf, 5, 6)));
#define divine_nat_change_quiet(np, name, change, ...) \
    divine_nat_change((np), (name), -(change), 0, __VA_ARGS__)
extern void divine_unit_change(struct empobj *, char *, int, int, char *, ...)
    ATTRIBUTE((format (printf, 5, 6)));
#define divine_unit_change_quiet(unit, name, change, ...) \
    divine_unit_change((unit), (name), -(change), 0, __VA_ARGS__)
extern void divine_flag_change(struct empobj *, char *, int, int,
			       struct symbol *);
extern void report_divine_gift(natid, struct ichrstr *, int, char *);

#endif
