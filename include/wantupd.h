/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  wantupd.h: Header used to check to see if an update is wanted
 *             and/or allowed.
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1991
 */

/*
 * In the "hours" file,
 *
 * update_policy: N
 *      0 - normal policy, update each time.
 *	1 - update at times specified by keyword "update_times".
 *      2 - blitz updates
 *      3 - no regular updates.  (Only demand ones.)
 *      default: 0.
 *
 * update_times:  1:00 3:00 8:00
 *      Sets the times when updates occur under policy #1.
 *      NOTE that the schedual times should coincide.
 *
 * update_timeslop: N
 *      The number of minutes that the update check is allowed to slip
 *      to match one of the update times shown.
 *      default 5.
 *
 * update_wantmin: N
 *      0 - disabled, no requests.
 *      >0 - minimum number of requests before an update.
 *            Setting it to more than num of coun will max at num of coun.
 *	defaults to MAXNOC.
 *
 * update_abswantmin: N
 *      N - absolute minimum number of requests for an update.
 *          Prevents quick breaks and updates.
 *	defaults to 1.
 *
 * update_demandtimes:  1:00-3:00  5:00-6:00
 *      Time intervals when update requests are allowed.
 *      NOTE that time ranges CANNOT cross midnight.
 *      They must go like:  20:00-24:00 0:00-3:00
 *
 * update_demandpolicy: N
 *      0 - demand updates occur only emp_tm checks.
 *      1 - demand updates occur right after setting command.
 *      2 - demand updates disabled.
 *      defaults to 0.
 *
 * update_demandmaxperday: N
 *      Not implemented yet.
 */

#ifndef _WANTUPD_H_
#define _WANTUPD_H_

#define  UDP_NORMAL	0
#define  UDP_TIMES	1
#define	 UDP_NOREG	3
#define  UDP_BLITZ	2
#define  UDP_MAX	3
#define  UDP_DEFAULT	UDP_NORMAL

/* The maximum number of "update_times" allowed */
#define  UDTIMES_MAX	20

#define  UDDEM_TMCHECK	0
#define  UDDEM_COMSET	1
#define  UDDEM_DISABLE	2
#define  UDDEM_MAX	2
#define  UDDEM_DEFAULT	UDDEM_TMCHECK

#define BLITZTIME 20

#endif /* _WANTUPD_H_ */
