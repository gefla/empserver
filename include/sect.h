/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sect.h: Definitions for things having to do with sectors.
 *
 *  Known contributors to this file:
 *     Dave Pare
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2010
 */


#ifndef SECT_H
#define SECT_H

#include <time.h>
#include "item.h"
#include "types.h"

struct sctstr {
    /* initial part must match struct empobj */
    signed ef_type: 8;
    unsigned sct_seqno: 12;
    unsigned sct_generation: 12;
    int sct_uid;		/* equals XYOFFSET(sct_x, sct_y) */
    time_t sct_timestamp;	/* Last time this sector was written to */
    natid sct_own;		/* owner's country num */
    coord sct_x;		/* x coord of sector */
    coord sct_y;		/* y coord of sector */
    signed char sct_type;	/* sector type */
    signed char sct_effic;	/* 0% to 100% */
    signed char sct_mobil;	/* mobility units */
    unsigned char sct_off;	/* Should this sector produce? */
    /* end of part matching struct empobj */
    unsigned char sct_loyal;	/* updates until civilans "converted" */
    unsigned char sct_terr;	/* territory 0 id # of sector */
    unsigned char sct_terr1;	/* territory 1 id # of sector */
    unsigned char sct_terr2;	/* territory 2 id # of sector */
    unsigned char sct_terr3;	/* territory 3 id # of sector */
    unsigned char sct_dterr;	/* deity's territory # */
    coord sct_dist_x;		/* Dist sector */
    coord sct_dist_y;
    short sct_avail;		/* available workforce for "other things" */
    short sct_flags;		/* temporary flags */
    short sct_elev;		/* elevation/depth */
    unsigned char sct_work;	/* pct of civ actually working */
    unsigned char sct_coastal;	/* is this a coastal sector? */
    signed char sct_newtype;	/* for changing designations */
    unsigned char sct_min;	/* ease of mining ore */
    unsigned char sct_gmin;	/* amount of gold ore */
    unsigned char sct_fertil;	/* fertility of soil */
    unsigned char sct_oil;	/* oil content */
    unsigned char sct_uran;	/* uranium ore content */
    natid sct_oldown;		/* old owner of sector (for liberation) */
    unsigned char sct_updated;	/* Has this sect been updated? */
    short sct_item[I_MAX+1];	/* amount of items stored here */
    short sct_del[I_MAX+1];	/* delivieries */
    short sct_dist[I_MAX+1];	/* distribution thresholds */
    short sct_mines;		/* number of mines */
    short sct_pstage;		/* plague stage */
    short sct_ptime;		/* how many etus remain in this stage */
    unsigned char sct_che;	/* number of guerrillas */
    natid sct_che_target;	/* nation targeted by che */
    unsigned short sct_fallout;
    short sct_access;		/* Last tick mob was updated (MOB_ACCESS) */
    unsigned char sct_road;	/* Road value of a sector */
    unsigned char sct_rail;	/* Rail value of a sector */
    unsigned char sct_defense;	/* Defensive value of a sector */
};

enum d_navigation {
    NAV_NONE,	/* ships can't navigate */
    NAVOK,	/* ships can always navigate */
    NAV_02,	/* requires 2% effic to navigate */
    NAV_CANAL,	/* requires 2% effic to navigate and M_CANAL capability */
    NAV_60	/* requires 60% effic to navigate */
};

struct dchrstr {
    unsigned char d_uid;
    char d_mnem;		/* map symbol */
    unsigned char d_terrain;	/* terrain sector type */
    int d_prd;			/* product type */
    int d_peffic;		/* process efficiency, in percent */
    float d_mob0, d_mob1;	/* movement cost at 0 and 100% eff */
    enum d_navigation d_nav;	/* navigation capability */
    enum i_packing d_pkg;	/* type of packaging in these sects */
    float d_ostr;		/* offensive strength */
    float d_dstr;		/* defensive strength */
    int d_value;		/* resale ("collect") value */
    int d_cost;			/* cost to designate the sect */
    int d_build;		/* cost multiplier for eff */
    int d_lcms;			/* lcm's needed per point of eff */
    int d_hcms;			/* hcm's needed per point of eff */
    int d_maint;		/* maintenance cost per ETU */
    int d_maxpop;		/* maximum population */
    char *d_name;		/* full name of sector type */
};

/* Sector types, must match sect.config */
#define SCT_WATER	0	/* basics */
#define SCT_MOUNT	1
#define SCT_SANCT	2
#define SCT_WASTE	3
#define SCT_RURAL	4
#define SCT_CAPIT	5
#define SCT_URAN	6
#define SCT_PARK	7
#define SCT_ARMSF	8	/* industries */
#define SCT_AMMOF	9
#define SCT_MINE	10
#define SCT_GMINE	11
#define SCT_HARBR	12
#define SCT_WAREH	13
#define SCT_AIRPT	14
#define SCT_AGRI	15
#define SCT_OIL		16
#define SCT_LIGHT	17
#define SCT_HEAVY	18
#define SCT_FORTR	19	/* military/scientific */
#define SCT_TECH	20
#define SCT_RSRCH	21
#define SCT_NUKE	22
#define SCT_LIBR	23
#define SCT_HIWAY	24	/* communications */
#define SCT_RADAR	25
#define SCT_HEADQ	26	/* headquarters */
#define SCT_BHEAD	27	/* Bridge head */
#define SCT_BSPAN	28	/* Bridge span */
#define SCT_BANK	29	/* financial */
#define SCT_REFINE	30	/* refinery */
#define SCT_ENLIST	31	/* enlistment center */
#define SCT_PLAINS	32	/* plains sector */
#define SCT_BTOWER	33	/* Bridge tower */

#define SCT_TYPE_MAX	38

#define getsect(x, y, p) ef_read(EF_SECTOR, sctoff((x), (y)), (p))
#define putsect(p) ef_write(EF_SECTOR, (p)->sct_uid, (p))
#define getsectp(x, y) (struct sctstr *)ef_ptr(EF_SECTOR, sctoff((x), (y)))
#define getsectid(id) (struct sctstr *)ef_ptr(EF_SECTOR, (id))

/* things relating to sectors */
extern int sctoff(coord x, coord y);

extern struct dchrstr dchr[SCT_TYPE_MAX + 2];
#define IS_BIG_CITY(type) (dchr[(type)].d_pkg == UPKG)

/* Minimal efficiency of sectors that can be knocked down (bridges) */
#define SCT_MINEFF 20

/* Work required for building */
#define SCT_BLD_WORK(lcm, hcm) ((lcm) + 2 * (hcm))

/* Return SP's defense efficiency */
#define SCT_DEFENSE(sp) \
    (intrchr[INT_DEF].in_enable ? (sp)->sct_defense : (sp)->sct_effic)

#define FORTEFF 5		/* forts must be 5% efficient to fire. */

/* Can trains enter sector SP? */
#define SCT_HAS_RAIL(sp)					\
    (opt_RAILWAYS ? sct_rail_track((sp)) != 0			\
     : intrchr[INT_RAIL].in_enable && (sp)->sct_rail != 0)

#define SCT_MINES_ARE_SEAMINES(sp) \
    ((sp)->sct_type == SCT_WATER || (sp)->sct_type == SCT_BSPAN)
#define SCT_SEAMINES(sp) \
    (SCT_MINES_ARE_SEAMINES((sp)) ? (sp)->sct_mines : 0)
#define SCT_LANDMINES(sp) \
    (SCT_MINES_ARE_SEAMINES((sp)) ? 0 : (sp)->sct_mines)

/* Sector infrastructure types, must match infra.config */
#define INT_ROAD	0
#define INT_RAIL	1
#define INT_DEF		2

/* Sector flags */
#define MOVE_IN_PROGRESS	bit(0)	/* move in progress */

/* maximum item amount, must fit into sct_item[], sct_del[], sct_dist[] */
#define ITEM_MAX 9999
/* maximum number of mines, must fit into struct sctstr member sct_mines */
#define MINES_MAX 32767
/* maximum number of che, must fit into struct sctstr member sct_che */
#define CHE_MAX 255
/* maximum fallout, must fit into struct sctstr member sct_fallout */
#define FALLOUT_MAX 9999
/* maximum territory, must fit into struct sctstr members sct_terr etc */
#define TERR_MAX 99

/* Each cost is per point of efficency */
struct sctintrins {
    char *in_name;
    unsigned char in_lcms;	/* construction materials */
    unsigned char in_hcms;
    unsigned char in_dcost;	/* dollars */
    unsigned char in_mcost;	/* mobility */
    unsigned char in_enable;	/* enabled iff non-zero */
};

extern struct sctintrins intrchr[INT_DEF + 2];

extern int fort_fire(struct sctstr *);
extern int sct_rail_track(struct sctstr *);

#endif
