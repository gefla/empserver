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
 *  event.h: Describes how to send an event to a user thread
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _EVENT_H_
#define _EVENT_H_

/* country #1's battleship #2 hit your battleship #3 for 30% damage */
/* actor's from_type #from_id hit your to_type #to_id for data% damage */
struct event {
	short verb;
	natid actor;
	natid victim;
	short from_type;
	short data;
	int from_id;
	short to_type;
	int to_id;
	time_t time;
};

#endif /* _EVENT_H_ */
