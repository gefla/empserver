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
 *  path.c: Path initializations needed outside of the file initialization
 *          tables
 * 
 *  Known contributors to this file:
 *     
 */

#include "gamesdef.h"
#include "misc.h"

#if !defined(_WIN32)
#define FILEPATH(p) EMPPATH(data/p)
#else
#define FILEPATH(p) EMPPATH(data\\p)
#endif

s_char *gamedir = EMPPATH(.);
s_char *infodir = EMPPATH(info.nr);
s_char *datadir = EMPPATH(data);
s_char *teldir = FILEPATH(tel);

s_char *upfil = FILEPATH(up);
s_char *downfil = FILEPATH(down);
s_char *disablefil = FILEPATH(disable);
#if !defined(_WIN32)
s_char *telfil = FILEPATH(tel / tel);
#else
s_char *telfil = FILEPATH(tel \\ tel);
#endif
s_char *annfil = FILEPATH(ann);
s_char *commfil = FILEPATH(comm);
s_char *banfil = FILEPATH(ban);
s_char *authfil = FILEPATH(auth);
s_char *timestampfil = FILEPATH(timestamp);
s_char *lostfil = FILEPATH(lostitems);

#if !defined(_WIN32)
s_char *playerbin = EMPPATH(bin / emp_player);
s_char *updatebin = EMPPATH(bin / emp_update);
#else
s_char *playerbin = EMPPATH(bin \\ emp_player);
s_char *updatebin = EMPPATH(bin \\ emp_update);
#endif
s_char *loginport = EMP_PORT;
