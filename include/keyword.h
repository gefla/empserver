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
 *  keyword.h: Definitions for parsed keyword package
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _KEYWORD_H_
#define _KEYWORD_H_

#include <stdio.h>
#include "misc.h"

#define CF_VALUE	1
#define CF_TIME		2
#define CF_TIMERANGE	3
#define CF_WEEKDAY	4

extern s_char * kw_find(s_char * );
extern s_char * kw_parse(int  , s_char *  , int * );
extern int kw_read(FILE * );
extern s_char * get_time(s_char *  , int * );
extern s_char * weekday(s_char *  , int * );

#endif /* _KEYWORD_H_ */
