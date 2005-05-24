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
 *  attsub.c: Attack subroutines
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1996-2000
 */

#include <math.h>
#include "misc.h"
#include "player.h"
#include "file.h"
#include "plague.h"
#include "sect.h"
#include "ship.h"
#include "path.h"
#include "news.h"
#include "treaty.h"
#include "nat.h"
#include "xy.h"
#include "land.h"
#include "nsc.h"
#include "mission.h"
#include "combat.h"
#include "item.h"
#include "optlist.h"
#include "prototypes.h"

#define CASUALTY_LUMP	1	/* How big casualty chunks should be */

static void ask_olist(int combat_mode, struct combat *off,
		      struct combat *def, struct emp_qelem *olist,
		      s_char *land_answer, int *a_spyp, int *a_engineerp);
static void take_move_in_mob(int combat_mode, struct llist *llp,
			     struct combat *off, struct combat *def);
static void move_in_land(int combat_mode, struct combat *off,
			 struct emp_qelem *olist, struct combat *def);
static void ask_move_in(struct combat *off, struct emp_qelem *olist,
			struct combat *def);
static void ask_move_in_off(struct combat *off, struct combat *def);

static int board_abort(struct combat *off, struct combat *def);
static int land_board_abort(struct combat *off, struct combat *def);
static int ask_off(int combat_mode, struct combat *off,
		   struct combat *def);
static int get_dlist(struct combat *def, struct emp_qelem *list, int a_spy,
		     int *d_spyp);
static int get_ototal(int combat_mode, struct combat *off,
		      struct emp_qelem *olist, double osupport, int check);
static int get_dtotal(struct combat *def, struct emp_qelem *list,
		      double dsupport, int check);
static int take_casualty(int combat_mode, struct combat *off,
			 struct emp_qelem *olist);

static void send_reacting_units_home(struct emp_qelem *list);
static int take_def(int combat_mode, struct emp_qelem *list,
		    struct combat *off, struct combat *def);

static int get_land(int combat_mode, struct combat *def, int uid,
		    struct llist *llp, int victim_land);

s_char *att_mode[] = {
    /* must match combat types in combat.h */
    "defend", "attack", "assault", "paradrop", "board", "lboard"
};


/*
 * The principal object in this code is the "combat" object.  A combat object
 * is either a sector or ship.  There are
 * usually two instances of this, the "def" or defense combat object, and
 * the array of "off" or offense objects.  The number of offense objects is
 * determined by the value of off->last (e.g. more than one attacking sector).
 * the type of the object is determined by combat->type which can take the
 * values EF_SECTOR, EF_SHIP, EF_PLANE, or EF_BAD.  Another important parameter
 * which is often passed to these functions is combat_mode.  This can take
 * the value A_DEFENSE, A_ATTACK, A_ASSAULT, A_PARA, A_BOARD and A_LBOARD.
 * As these six modes of being in combat affect things like mobcost and combat
 * value, there are often switches made on combat_mode.  Note that in all cases
 * no mobility is taken from sectors, ships, or land units until the player
 * has committed to a fight.  Instead, the cost is temporarily placed in
 * combat->mobcost, or llp->mobil as the case may be, and then when the object
 * is "put" back onto disk, then the amounts in these variables are subtracted
 * from the object's mobility.  It needs to be done this way as the objects
 * are constantly being re-read from disk, and we don't want to take any mob
 * unless a fight actually occurrs.
 * -Ken Stevens
 */

/* initialize combat object */

int
att_combat_init(struct combat *com, int type)
{
    memset(com, 0, sizeof(*com));
    com->type = type;
    return type;
}

/* print a combat object with optional preposition */

static s_char *
pr_com(int inon, struct combat *com, natid who)
{
    if (com->type == EF_SECTOR) {
	return prbuf("%s%s",
		     inon ? inon == 1 ? "in " : "into " : "",
		     xyas(com->x, com->y, who));
    } else if (com->type == EF_SHIP) {
	if (opt_SHIPNAMES) {
	    return prbuf("%s%s %s(#%d)",
			 inon ? inon == 1 ? "on " : "onto " : "",
			 com->shp_mcp->m_name, com->shp_name,
			 com->shp_uid);
	} else {
	    return prbuf("%s%s #%d",
			 inon ? inon == 1 ? "on " : "onto " : "",
			 com->shp_mcp->m_name, com->shp_uid);
	}
    } else if (com->type == EF_LAND) {
	return prbuf("%s%s #%d",
		     inon ? inon == 1 ? "on " : "onto " : "",
		     com->lnd_lcp->l_name, com->lnd_uid);
    } else {
	return "your forces";
    }
}

static s_char *
prcom(int inon, struct combat *com)
{
    return pr_com(inon, com, player->cnum);
}

/* Doing a sneak attack */
static void
do_sneak(struct combat *def, int success)
{
    struct sctstr sect;
    struct natstr *natp = getnatp(player->cnum);
    int issneak = getrel(natp, def->own);

    if (def->type != EF_SECTOR)
	return;

    getsect(def->x, def->y, &sect);

    if (issneak == AT_WAR || !def->own || sect.sct_oldown == player->cnum)
	return;

    if (success)
	pr("Your sneak attack was successful\nBut ");
    else
	pr("Your sneak attack was unsuccessful\nAnd ");

    pr("it will cost you $5000\n");
    pr("War has been declared!!!!\n");
    wu(0, def->own, "Country %s (#%d) has Sneak Attacked!!\n",
       cname(player->cnum), player->cnum);
    wu(0, def->own, "Country %s (#%d) has Declared WAR on you!!\n",
       cname(player->cnum), player->cnum);
    player->dolcost += 5000;
    issneak = min(issneak, MOBILIZATION);
    nreport(player->cnum, N_DECL_WAR, def->own, 1);
    setrel(player->cnum, def->own, issneak);
}

/*
 * This is the combat object "type" based integrity check.  It basically
 * splits along three divisions: ship/sector, attacker/defender, 
 * first time/not first time.
 */

int
att_get_combat(struct combat *com, int isdef)
{
    struct sctstr sect;
    struct shpstr ship;
    struct lndstr land;
    int pstage;
    natid owner;
    int mil;
    int eff;
    int mob;
    coord x, y;

    switch (com->type) {
    case EF_SECTOR:
	if (!getsect(com->x, com->y, &sect)) {
	    pr("Bad sector: %s\n", xyas(com->x, com->y, player->cnum));
	    return att_combat_init(com, EF_BAD);
	}
	com->sct_type = sect.sct_type;
	com->sct_dcp = &dchr[sect.sct_type];
	mil = sect.sct_item[I_MILIT];
	pstage = sect.sct_pstage;
	owner = sect.sct_own;
	eff = sect.sct_effic;
	mob = sect.sct_mobil;
	x = com->x;
	y = com->y;
	break;
    case EF_LAND:
	if (!getland(com->lnd_uid, &land)) {
	    if (isdef)
		pr("Land unit #%d is not in the same sector!\n",
		   com->lnd_uid);
	    return att_combat_init(com, EF_BAD);
	}
	if (isdef && player->owner) {
	    pr("Boarding yourself?  Try using the 'load' command.\n");
	    return att_combat_init(com, EF_BAD);
	}
	com->lnd_lcp = &lchr[(int)land.lnd_type];
	mil = land.lnd_item[I_MILIT];
	pstage = land.lnd_pstage;
	owner = land.lnd_own;
	eff = land.lnd_effic;
	mob = land.lnd_mobil;
	x = land.lnd_x;
	y = land.lnd_y;
	break;
    case EF_SHIP:
	if (!getship(com->shp_uid, &ship) || !ship.shp_own) {
	    if (isdef)
		pr("Ship #%d is not in the same sector!\n", com->shp_uid);
	    else
		pr("Ship #%d is not your ship!\n", com->shp_uid);
	    return att_combat_init(com, EF_BAD);
	}
	if (opt_MARKET) {
	    if (isdef && player->owner &&
		ontradingblock(EF_SHIP, (int *)&ship)) {
		pr("%s is on the trading block.\n", prcom(0, com));
		return att_combat_init(com, EF_BAD);
	    }
	}
	if (isdef && player->owner) {
	    pr("Boarding yourself?  Try using the 'tend' command.\n");
	    return att_combat_init(com, EF_BAD);
	}
	com->shp_mcp = &mchr[(int)ship.shp_type];
	if (opt_SHIPNAMES)
	    strncpy(com->shp_name, ship.shp_name, MAXSHPNAMLEN);
	if (!isdef && !player->owner) {
	    if (com->set)
		pr("%s was just sunk!\n", prcom(0, com));
	    else
		pr("Ship #%d is not your ship!\n", com->shp_uid);
	    return att_combat_init(com, EF_BAD);
	}
	mil = ship.shp_item[I_MILIT];
	pstage = ship.shp_pstage;
	owner = ship.shp_own;
	eff = ship.shp_effic;
	mob = ship.shp_mobil;
	x = ship.shp_x;
	y = ship.shp_y;
	break;
    case EF_PLANE:
	return com->mil;
    case EF_BAD:
	return EF_BAD;
    default:
	return att_combat_init(com, EF_BAD);
    }

    if (!com->set) {		/* first time */
	if (isdef) {		/* defender */
	    com->troops = mil;
	} else {		/* attacker */
	    if (!mil)
		pr("No mil %s\n", prcom(1, com));
	    else if (mil == 1)
		pr("Only 1 mil %s\n", prcom(1, com));
	    /* don't abandon attacking sectors or ships */
	    com->troops = max(0, mil - 1);
	}
	com->plague = pstage == PLG_INFECT;
    } else {			/* not first time */
	if (isdef) {		/* defender */
	    if (com->x != x || com->y != y) {
		pr("%s has moved!\n", prcom(0, com));
		return att_combat_init(com, EF_BAD);
	    }
	    if (owner != com->own) {
		if (owner) {
		    pr("WARNING: The ownership of %s just changed from %s to %s!\n", prcom(0, com), cname(com->own), cname(owner));
		} else if (com->type == EF_SECTOR) {
		    pr("WARNING: %s just abandoned sector %s!\n",
		       cname(com->own), xyas(com->x, com->y,
					     player->cnum));
		}
	    }
	    if (com->mil != mil)
		pr("WARNING: The enemy mil %s just %s from %d to %d!\n",
		   prcom(1, com),
		   com->mil < mil ? "increased" : "decreased", com->mil,
		   mil);
	    com->troops = mil;
	} else {		/* attacker */
	    if (owner != player->cnum && getrel(getnatp(owner), player->cnum) != ALLIED) {	/* must be EF_SECTOR */
		if (com->mil)
		    pr("WARNING: Your %d mil in %s were destroyed because %s just took the sector!\n", com->mil, xyas(com->x, com->y, player->cnum), cname(owner));
		else
		    pr("You no longer own %s\n",
		       xyas(com->x, com->y, player->cnum));
		return att_combat_init(com, EF_BAD);
	    }
	    if (com->troops && com->troops + 1 > mil) {
		if (com->own == owner && player->cnum == owner)	/* not a takeover */
		    pr("WARNING: Your mil %s has been reduced from %d to %d!\n", prcom(1, com), com->troops, max(0, mil - 1));
		com->troops = max(0, mil - 1);
	    }
	}
    }
    com->set = 1;
    com->mil = mil;
    com->own = owner;
    com->x = x;
    com->y = y;
    com->eff = eff;
    com->mob = mob;
    return com->troops;
}

/*
 * In the course of the fight, the combat object may have lost mil, eff, or
 * mobility.  This is the place where the data in the object gets flushed to
 * disk to make it "real".
 */

static void
put_combat(struct combat *com)
{
    struct sctstr sect;
    struct shpstr ship;
    struct lndstr land;
    int deff;

    switch (com->type) {
    case EF_SECTOR:
	getsect(com->x, com->y, &sect);
	sect.sct_type = com->sct_type;
	deff = sect.sct_effic - com->eff;
	if (deff > 0) {
	    sect.sct_road -= (sect.sct_road * deff / 100.0);
	    sect.sct_rail -= (sect.sct_rail * deff / 100.0);
	    sect.sct_defense -= (sect.sct_defense * deff / 100.0);
	    if (sect.sct_road <= 0)
		sect.sct_road = 0;
	    if (sect.sct_rail <= 0)
		sect.sct_rail = 0;
	    if (sect.sct_defense <= 0)
		sect.sct_defense = 0;
	}
	sect.sct_effic = com->eff;
	if (!opt_DEFENSE_INFRA)
	    sect.sct_defense = sect.sct_effic;
	if (com->mobcost) {
	    if (opt_MOB_ACCESS) {
		if ((com->mob - com->mobcost) < -127)
		    sect.sct_mobil = -127;
		else
		    sect.sct_mobil = (short)(com->mob - com->mobcost);
	    } else {
		if ((com->mob - com->mobcost) < 0)
		    sect.sct_mobil = 0;
		else
		    sect.sct_mobil = (short)(com->mob - com->mobcost);
	    }
	}
	makelost(EF_SECTOR, sect.sct_own, 0, sect.sct_x, sect.sct_y);
	makenotlost(EF_SECTOR, com->own, 0, sect.sct_x, sect.sct_y);
	sect.sct_own = com->own;
	if (com->plague) {
	    if (sect.sct_pstage == PLG_HEALTHY)
		sect.sct_pstage = PLG_EXPOSED;
	}
	sect.sct_item[I_MILIT] = com->mil;
	putsect(&sect);
	com->own = sect.sct_own;	/* avoid WARNING if sector reverts */
	break;
    case EF_LAND:
	getland(com->lnd_uid, &land);
	land.lnd_effic = com->eff;
	if (com->mobcost) {
	    if (com->mob - com->mobcost < -127)
		land.lnd_mobil = -127;
	    else
		land.lnd_mobil = (s_char)(com->mob - com->mobcost);
	}
	makelost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x,
		 land.lnd_y);
	land.lnd_own = com->own;
	makenotlost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x,
		    land.lnd_y);
	if (com->plague) {
	    if (land.lnd_pstage == PLG_HEALTHY)
		land.lnd_pstage = PLG_EXPOSED;
	}
	if (!(com->lnd_lcp->l_flags & L_SPY))
	    land.lnd_item[I_MILIT] = com->mil;
	lnd_count_units(&land);
	if (com->own == player->cnum) {
	    land.lnd_mission = 0;
	    land.lnd_rflags = 0;
	    memset(land.lnd_rpath, 0, sizeof(land.lnd_rpath));
	}
	putland(com->lnd_uid, &land);
	break;
    case EF_SHIP:
	getship(com->shp_uid, &ship);
	ship.shp_effic = com->eff;
	if (com->mobcost) {
	    if (com->mob - com->mobcost < -127)
		ship.shp_mobil = -127;
	    else
		ship.shp_mobil = (s_char)(com->mob - com->mobcost);
	}
	makelost(EF_SHIP, ship.shp_own, ship.shp_uid, ship.shp_x,
		 ship.shp_y);
	ship.shp_own = com->own;
	makenotlost(EF_SHIP, ship.shp_own, ship.shp_uid, ship.shp_x,
		    ship.shp_y);
	if (com->plague) {
	    if (ship.shp_pstage == PLG_HEALTHY)
		ship.shp_pstage = PLG_EXPOSED;
	}
	ship.shp_item[I_MILIT] = com->mil;
	count_units(&ship);
	if (com->own == player->cnum) {
	    ship.shp_mission = 0;
	    ship.shp_rflags = 0;
	    memset(ship.shp_rpath, 0, sizeof(ship.shp_rpath));
	}
	putship(com->shp_uid, &ship);
    }
    com->mobcost = 0;
    att_get_combat(com, com->own != player->cnum);
}

/* If pre-attack, abort fight.  If post-attack, don't move anything in */

static int
abort_attack(void)
{
    return player->aborted = 1;
}

/*
 * This is the combat_mode based integrity check.  It splits among two main
 * divisions: first time/not first time, and attack/assault/para/board.
 */

int
att_abort(int combat_mode, struct combat *off, struct combat *def)
{
    struct sctstr sect;
    int issneak;
    s_char y_or_n[512];
    struct natstr *natp;

    if (player->aborted)
	return 1;
    if (att_get_combat(def, 1) < 0)
	return abort_attack();

    if (off && combat_mode != A_ATTACK) {
	if (att_get_combat(off, 0) < 0)
	    return abort_attack();
	if (off->type == EF_SHIP &&
	    (!getsect(off->x, off->y, &sect) ||
	     sect.sct_type != SCT_WATER)) {
	    pr("%s can not %s from that far inland!\n",
	       prcom(0, off), att_mode[combat_mode]);
	    return abort_attack();
	}
    }
    switch (combat_mode) {
    case A_ATTACK:
	if (!neigh(def->x, def->y, player->cnum) &&
	    !adj_units(def->x, def->y, player->cnum)) {
	    pr("You are not adjacent to %s\n",
	       xyas(def->x, def->y, player->cnum));
	    return abort_attack();
	}
	if (def->own == player->cnum) {
	    pr("You can't attack your own sector.\n");
	    return abort_attack();
	}
	break;
    case A_ASSAULT:
	if (off && mapdist(off->x, off->y, def->x, def->y) > 1) {
	    pr("You'll have to get there first...\n");
	    return abort_attack();
	}
	if (off && def->sct_type == SCT_MOUNT) {
	    pr("You can't assault a %s sector!\n", def->sct_dcp->d_name);
	    return abort_attack();
	}
	break;
    case A_PARA:
	if (def->own == player->cnum) {
	    pr("You can't air-assault your own sector.\n");
	    return abort_attack();
	}
	if (off && (def->sct_type == SCT_MOUNT ||
		    def->sct_type == SCT_WATER ||
		    def->sct_type == SCT_CAPIT ||
		    def->sct_type == SCT_FORTR ||
		    def->sct_type == SCT_WASTE)) {
	    pr("You can't air-assault a %s sector!\n",
	       def->sct_dcp->d_name);
	    return abort_attack();
	}
	break;
    case A_BOARD:
	return board_abort(off, def);
    case A_LBOARD:
	return land_board_abort(off, def);
    }

    if (off && def->sct_dcp->d_mcst <= 0) {
	pr("You can't %s a %s sector!\n", att_mode[combat_mode],
	   def->sct_dcp->d_name);
	return abort_attack();
    }
    if (!off || off->relations_checked)
	return 0;
    off->relations_checked = 1;

    if (opt_HIDDEN) {
	setcont(player->cnum, def->own, FOUND_SPY);
	setcont(def->own, player->cnum, FOUND_SPY);
    }
    if (opt_SLOW_WAR && def->own != player->cnum) {
	natp = getnatp(player->cnum);
	issneak = getrel(natp, def->own);

	if (issneak == ALLIED) {
	    sprintf(y_or_n, "Sector is owned by %s, your ally, %s [yn]? ",
		    cname(def->own), att_mode[combat_mode]);
	    if (!confirm(y_or_n))
		return abort_attack();

	}
	if (opt_SNEAK_ATTACK) {
	    getsect(def->x, def->y, &sect);
	    if ((issneak != AT_WAR) && (def->own)
		&& (def->own != player->cnum)
		&& (sect.sct_oldown != player->cnum)
		&& (issneak != SITZKRIEG) && (issneak != MOBILIZATION)) {
		pr("You're not at war with them!\n");
		if (!confirm("Do you really want to sneak attack (it will cost you $5000) [yn]? ")) {
		    pr("Sneak attack cancelled!\n");
		    return abort_attack();
		}
	    }
	    if ((issneak != AT_WAR) && (def->own)
		&& (def->own != player->cnum)
		&& (sect.sct_oldown != player->cnum)
		&& ((issneak == MOBILIZATION) || (issneak == SITZKRIEG))) {
		pr("You're not at war with them!\n");
		return abort_attack();
	    }
	} else {
	    if ((issneak != AT_WAR) && (def->own) &&
		(sect.sct_oldown != player->cnum)) {
		pr("You're not at war with them!\n");
		return abort_attack();
	    }
	}
    }
    return 0;
}

/*
 * Lots of special things need to be checked for boarding, so I put it in
 * it's own function.
 */

static int
board_abort(struct combat *off, struct combat *def)
{
    struct shpstr aship, dship;	/* for tech levels */
    struct sctstr sect;

    if (att_get_combat(def, 1) < 0)
	return abort_attack();

    if (!off)
	return 0;

    if (att_get_combat(off, 0) < 0)
	return abort_attack();

    if (off->x != def->x || off->y != def->y) {
	pr("Ship #%d is not in the same sector!\n", def->shp_uid);
	return abort_attack();
    }
    if (off->type == EF_SHIP) {
	if (off->mob <= 0) {
	    pr("%s has no mobility!\n", prcom(0, off));
	    return abort_attack();
	}
	getship(off->shp_uid, &aship);
	getship(def->shp_uid, &dship);
	if (techfact(aship.shp_tech, 1.0) * aship.shp_speed * off->eff
	    <= techfact(dship.shp_tech, 1.0) * dship.shp_speed * def->eff) {
	    pr("Victim ship moves faster than you do!\n");
	    if (def->own)
		wu(0, def->own,
		   "%s (#%d) %s failed to catch %s\n",
		   cname(aship.shp_own), aship.shp_own,
		   pr_com(0, off, def->own), pr_com(0, def, def->own));
	    return abort_attack();
	}
    } else if (off->type != EF_SECTOR) {
	pr("Please tell the deity that you got the 'banana boat' error\n");
	return abort_attack();
    }
    if (def->shp_mcp->m_flags & M_SUB) {
	getsect(def->x, def->y, &sect);
	if (sect.sct_type == SCT_WATER) {
	    pr("You can't board a submarine!\n");
	    return abort_attack();
	}
    }
    return 0;
}

/*
 * Lots of special things need to be checked for boarding, so I put it in
 * it's own function.
 * STM - I copied it for land unit boarding. :)
 */

static int
land_board_abort(struct combat *off, struct combat *def)
{
    if (att_get_combat(def, 1) < 0)
	return abort_attack();

    if (!off)
	return 0;

    if (att_get_combat(off, 0) < 0)
	return abort_attack();

    if (off->x != def->x || off->y != def->y) {
	pr("Land unit #%d is not in the same sector!\n", def->lnd_uid);
	return abort_attack();
    }

    return 0;
}

/* If we are boarding, then the defending ship gets a chance to fire back */
int
att_approach(struct combat *off, struct combat *def)
{
    int dam;
    struct sctstr sect;
    struct shpstr ship;

    pr("Approaching %s...\n", prcom(0, def));
    if (def->own)
	wu(0, def->own,
	   "%s is being approached by %s...\n",
	   pr_com(0, def, def->own), pr_com(0, off, def->own));
    if (!(dam = shipdef(player->cnum, def->own, def->x, def->y)))
	return 0;

    pr("They're firing at us sir!\n");
    if (def->own) {
	wu(0, def->own,
	   "Your fleet at %s does %d damage to %s\n",
	   xyas(def->x, def->y, def->own), dam, pr_com(0, off, def->own));
    }
    if (off->type == EF_SECTOR) {
	getsect(off->x, off->y, &sect);
	sectdamage(&sect, dam, 0);
	putsect(&sect);
	pr("Enemy fleet at %s does %d damage to %s\n",
	   xyas(def->x, def->y, player->cnum), dam, prcom(0, off));
    } else if (off->type == EF_SHIP) {
	getship(off->shp_uid, &ship);
	shipdamage(&ship, dam);
	putship(off->shp_uid, &ship);
	if (def->own && ship.shp_effic < SHIP_MINEFF) {
	    wu(0, def->own, "%s sunk!\n", pr_com(0, off, def->own));
	    nreport(player->cnum, N_SHP_LOSE, def->own, 1);
	}
    }
    if (att_get_combat(off, 0) < 0)
	return abort_attack();
    return 0;
}

/* The attack is valid.  Tell the attacker about what they're going to hit */

int
att_show(struct combat *def)
{
    /* Note that we tell the player about the treaty BEFORE we tell them
       about the item.  If we didn't, then they gain free information */
    if (def->type == EF_SECTOR) {
	if (!trechk(player->cnum, def->own, LANATT))
	    return abort_attack();
	pr("%s is a %d%% %s %s with approximately %d military.\n",
	   xyas(def->x, def->y, player->cnum),
	   roundintby((int)def->eff, 10),
	   cname(def->own), def->sct_dcp->d_name,
	   roundintby(def->troops, 10));
	if (map_set(player->cnum, def->x, def->y, def->sct_dcp->d_mnem, 0))
	    writemap(player->cnum);
    } else if (def->type == EF_SHIP || def->type == EF_LAND) {
	if (def->type == EF_SHIP) {
	    if (!trechk(player->cnum, def->own, SEAATT))
		return abort_attack();
	} else {
	    if (!trechk(player->cnum, def->own, LNDATT))
		return abort_attack();
	}
	pr("%s is about %d%% efficient and has approximately %d mil on board.\n", prcom(0, def), roundintby((int)def->eff, 10), roundintby(def->troops, 10));
    }
    /* Ok, everything is fine */
    return 0;
}

/* Attack and assault ask the user which kind of support they want */

int
att_ask_support(int offset, int *fortp, int *shipp, int *landp,
		int *planep)
{
    s_char buf[1024];
    s_char *p;
    *fortp = *shipp = *landp = *planep = 1;

    if (player->argp[offset] != (s_char *)0) {
	if ((player->argp[offset + 1] == (s_char *)0) ||
	    (player->argp[offset + 2] == (s_char *)0) ||
	    (player->argp[offset + 3] == (s_char *)0)) {
	    pr("If any support arguments are used, all must be!\n");
	    return RET_SYN;
	}

	*fortp = *shipp = 0;
	*landp = *planep = 0;

	if (!(p = getstarg(player->argp[offset], "Use fort support? ",
			   buf)))
	    return RET_SYN;

	if ((*p == 'y') || (*p == 'Y'))
	    *fortp = 1;

	if (!(p = getstarg(player->argp[offset + 1], "Use ship support? ",
			   buf)))
	    return RET_SYN;

	if ((*p == 'y') || (*p == 'Y'))
	    *shipp = 1;

	if (!(p = getstarg(player->argp[offset + 2], "Use land support? ",
			   buf)))
	    return RET_SYN;

	if ((*p == 'y') || (*p == 'Y'))
	    *landp = 1;

	if (!(p = getstarg(player->argp[offset + 3], "Use plane support? ",
			   buf)))
	    return RET_SYN;

	if ((*p == 'y') || (*p == 'Y'))
	    *planep = 1;
    }
    return RET_OK;
}

/*
 * Attack, assault, and board ask the attacker what they'd like to attack
 * with.  This includes mil and land units from each "off" object.  Note that
 * after each sub-prompt, we check to make sure that the attack is still
 * valid, and if it's not, then we abort the attack.
 */

int
att_ask_offense(int combat_mode, struct combat *off, struct combat *def,
		struct emp_qelem *olist, int *a_spyp, int *a_engineerp)
{
    int n;
    s_char land_answer[1024];

    emp_initque(olist);
    if (att_abort(combat_mode, off, def))
	return 0;
    memset(land_answer, 0, sizeof(land_answer));
    for (n = 0; n <= off->last; ++n) {
	off[n].troops = ask_off(combat_mode, off + n, def);
	if (att_abort(combat_mode, off, def))
	    return 0;
	ask_olist(combat_mode, off + n, def, olist, land_answer,
		  a_spyp, a_engineerp);
	if (att_abort(combat_mode, off, def))
	    return 0;
    }
    return 0;
}

/* How many mil is off allowed to attack with when it attacks def? */

static int
get_mob_support(int combat_mode, struct combat *off, struct combat *def)
{
    int mob_support;

    switch (combat_mode) {
    case A_ATTACK:
	mob_support = off->mob / sector_mcost(getsectp(def->x, def->y),
					      MOB_ROAD);
	if (mob_support < 0)
	    mob_support = 0;
/*		mob_support = off->mob / sector_mcost(def->sct_type, def->eff);*/
	if (mob_support < off->troops)
	    pr("Sector %s has %d mobility which can only support %d mil,\n", xyas(off->x, off->y, player->cnum), off->mob, mob_support);
	else
	    mob_support = off->troops;
	return mob_support;
    case A_ASSAULT:
	if (def->own != player->cnum && def->mil) {
	    if (off->shp_mcp->m_flags & M_SEMILAND)
		return off->troops / 4;
	    else if (!(off->shp_mcp->m_flags & M_LAND))
		return off->troops / 10;
	}
	break;
    case A_BOARD:
	if (off->type == EF_SECTOR && off->mob <= 0)
	    return 0;
	mob_support = def->shp_mcp->m_item[I_MILIT];
	if (mob_support < off->troops)
	    pr("The size of the ship you are trying to board limits your party to %d mil,\n", mob_support);
	else
	    mob_support = off->troops;
	return mob_support;
    case A_LBOARD:
	if (off->mob <= 0)
	    return 0;
	if (def->lnd_lcp->l_flags & L_SPY)
	    return 1;
	mob_support = def->lnd_lcp->l_item[I_MILIT];
	if (mob_support < off->troops)
	    pr("The size of the unit you are trying to board limits your party to %d mil,\n", mob_support);
	else
	    mob_support = off->troops;
	return mob_support;
    }
    return off->troops;
}

/*
 * If the attacker decides to go through with the attack, then the
 * sectors/ships they are attacking with may be charged some mobility.
 * This is where that amount of mobility is calculated.  It is actually
 * subtracted "for real" from the object's mobility in put_combat().
 */

static void
calc_mobcost(int combat_mode, struct combat *off, struct combat *def,
	     int attacking_mil)
{
    struct shpstr ship;

    if (!attacking_mil)
	return;
    switch (combat_mode) {
    case A_ATTACK:
	off->mobcost +=
	    max(1,
		(int)(attacking_mil *
		      sector_mcost(getsectp(def->x, def->y), MOB_ROAD)));
	break;
    case A_LBOARD:
	off->mobcost += max(1, attacking_mil / 5);
	break;
    case A_BOARD:
	switch (off->type) {
	case EF_SECTOR:
	    off->mobcost += max(1, attacking_mil / 5);
	    break;
	case EF_SHIP:
	    /* the 2 in the formula below is a fudge factor */
	    getship(def->shp_uid, &ship);
	    off->mobcost += (def->eff / 100) * (ship.shp_speed / 2);
	}
    }
}

/* How many mil to we want to attack from off against def? */

static int
ask_off(int combat_mode, struct combat *off, struct combat *def)
{
    int attacking_mil;
    int mob_support;
    s_char prompt[512];

    if (att_get_combat(off, 0) <= 0)
	return 0;
    if ((off->type == EF_SECTOR) && (off->own != player->cnum))
	return 0;
    if ((mob_support = get_mob_support(combat_mode, off, def)) <= 0)
	return 0;
    if (off->type == EF_SECTOR) {
	if (off->own != player->cnum)
	    return 0;
	sprintf(prompt, "Number of mil from %s at %s (max %d) : ",
		off->sct_dcp->d_name,
		xyas(off->x, off->y, player->cnum), mob_support);
    } else {
	sprintf(prompt, "Number of mil from %s (max %d) : ",
		prcom(0, off), mob_support);
    }
    if ((attacking_mil = onearg(0, prompt)) < 0)
	abort_attack();
    if (att_abort(combat_mode, off, def))
	return 0;
    if (att_get_combat(off, 0) <= 0)
	return 0;
    if ((attacking_mil =
	 min(attacking_mil, min(mob_support, off->troops))) <= 0)
	return 0;

    calc_mobcost(combat_mode, off, def, attacking_mil);
    return attacking_mil;
}

/*
 * Which units would you like to attack with or move in with [ynYNq?]
 */

static s_char
att_prompt(s_char *prompt, s_char army)
{
    s_char buf[1024];
    s_char *p = buf;

    if (army == ' ')
	army = '~';
    *buf = -2;
    while (!p || (*p != 'y' && *p != 'n' && *p != 'Y' && *p != 'N')) {
	if (p && *p == 'q') {
	    abort_attack();
	    return 'N';
	}
	if (!p || !*p)
	    return 'n';
	if (p && *p != -2)
	    pr("y - yes this unit\nn - no this unit\nY - yes to all units in army '%c'\nN - no to all units in army '%c'\nq - quit\n? - this help message\n\n", army, army);
	p = getstring(prompt, buf);
	if (player->aborted) {
	    buf[0] = 'N';
	    p = buf;
	}
    }
    return *p;
}

/* Ask the attacker which units they want to attack/assault/board with */

static void
ask_olist(int combat_mode, struct combat *off, struct combat *def,
	  struct emp_qelem *olist, s_char *land_answer, int *a_spyp,
	  int *a_engineerp)
{
    struct nstr_item ni;
    struct lndstr land;
    double mobcost;
    struct llist *llp;
    struct lchrstr *lcp;
    int att_val;
    int count = 0;
    int maxland = 0;
    int first_time = 1;
    s_char prompt[512];

    if (def->type == EF_LAND)
	return;
    if (def->type == EF_SHIP)
	maxland = def->shp_mcp->m_nland;

    snxtitem_xy(&ni, EF_LAND, off->x, off->y);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (land.lnd_own != player->cnum)
	    continue;
	if (land.lnd_effic < LAND_MINEFF)
	    continue;
	if (land_answer[(int)land.lnd_army] == 'N')
	    continue;
	if (!lnd_can_attack(&land))
	    continue;
	lcp = &lchr[(int)land.lnd_type];

	if (def->type == EF_SHIP && !maxland) {
	    pr("Land units are not able to board this kind of ship\n");
	    return;
	}
	if (land.lnd_mobil <= 0) {
	    pr("%s is out of mobility, and cannot %s\n",
	       prland(&land), att_mode[combat_mode]);
	    continue;
	}

	if (opt_MARKET) {
	    if (ontradingblock(EF_LAND, (int *)&land)) {
		pr("%s is on the trading block, and cannot %s\n",
		   prland(&land), att_mode[combat_mode]);
		continue;
	    }
	}

	if (off->type == EF_SECTOR && land.lnd_ship >= 0) {
	    pr("%s is on ship #%d, and cannot %s\n",
	       prland(&land), land.lnd_ship, att_mode[combat_mode]);
	    continue;
	} else if (off->type == EF_SHIP) {
	    if (land.lnd_ship != off->shp_uid)
		continue;
	} else if (land.lnd_land >= 0) {
	    pr("%s is on unit #%d, and cannot %s\n",
	       prland(&land), land.lnd_land, att_mode[combat_mode]);
	    continue;
	}
	switch (combat_mode) {
	case A_ATTACK:
	    mobcost =
		lnd_mobcost(&land, getsectp(def->x, def->y), MOB_NONE);
	    if (land.lnd_mobil < mobcost) {
		pr("%s does not have enough mobility (%d needed)\n",
		   prland(&land), (int)ceil(mobcost));
		continue;
	    }
	    break;
	case A_ASSAULT:
	case A_BOARD:
	    mobcost = 0;
	    if (!(lcp->l_flags & L_ASSAULT))
		continue;
	}
	att_val = attack_val(combat_mode, &land);
	if (!att_val) {
	    pr("%s has no offensive strength\n", prland(&land));
	    continue;
	}
	resupply_all(&land);
	putland(land.lnd_uid, &land);
	if (!has_supply(&land)) {
	    pr("%s is out of supply, and cannot %s\n",
	       prland(&land), att_mode[combat_mode]);
	    continue;
	}
	if (def->type == EF_SHIP && first_time) {
	    first_time = 0;
	    pr("You may board with a maximum of %d land units\n", maxland);
	}
	pr("%s has a base %s value of %d\n",
	   prland(&land), att_mode[combat_mode], att_val);
	if (land_answer[(int)land.lnd_army] != 'Y') {
	    sprintf(prompt,
		    "%s with %s %s (%c %d%%) [ynYNq?] ",
		    att_mode[combat_mode],
		    prland(&land),
		    prcom(1, off),
		    land.lnd_army == ' ' ? '~' : land.lnd_army,
		    land.lnd_effic);
	    land_answer[(int)land.lnd_army] =
		att_prompt(prompt, land.lnd_army);
	    if (att_abort(combat_mode, off, def))
		return;
	    if (land_answer[(int)land.lnd_army] != 'y' &&
		land_answer[(int)land.lnd_army] != 'Y')
		continue;
	}
	if (!(llp = (struct llist *)malloc(sizeof(struct llist)))) {
	    logerror("Malloc failed in attack!\n");
	    abort_attack();
	    return;
	}
	memset(llp, 0, sizeof(struct llist));
	emp_insque(&llp->queue, olist);
	llp->mobil = mobcost;
	if (!get_land(combat_mode, def, land.lnd_uid, llp, 0))
	    continue;
	if (lnd_spyval(&land) > *a_spyp)
	    *a_spyp = lnd_spyval(&land);
	if (llp->lcp->l_flags & L_ENGINEER)
	    ++(*a_engineerp);
	if (def->type == EF_SHIP && ++count >= maxland)
	    break;
    }
}

/* What's the offense or defense multiplier? */

double
att_combat_eff(struct combat *com)
{
    double eff = 1.0;
    double str;
    struct shpstr ship;

    if (com->type == EF_SECTOR) {
	eff = com->eff / 100.0;
	if (com->own == player->cnum) {
	    str = com->sct_dcp->d_ostr;
	    eff = 1.0 + ((str - 1.0) * eff);
	} else
	    eff = sector_strength(getsectp(com->x, com->y));
/*			str = com->sct_dcp->d_dstr;*/
    } else if (com->type == EF_SHIP && com->own != player->cnum) {
	getship(com->shp_uid, &ship);
	eff = (1.0 + ship.shp_armor / 100.0);
    }
    return eff;
}

/*
 * Estimate the defense strength and give the attacker a chance to abort
 * if the odds are less than 50%
 */

int
att_estimate_defense(int combat_mode, struct combat *off,
		     struct emp_qelem *olist, struct combat *def,
		     int a_spy)
{
    int ototal;
    int estimate;
    int odds;

    /*
     * Get the attacker units & mil again in case they changed while the
     * attacker was answering sub-prompts.
     */

    ototal = get_ototal(combat_mode, off, olist, 1.0, 1);
    if (att_empty_attack(combat_mode, ototal, def))
	return abort_attack();
    if (combat_mode == A_PARA)
	return ototal;
    pr("\n             Initial attack strength: %8d\n", ototal);

    estimate = att_combat_eff(def) * roundintby(def->troops, 10);
    estimate += att_combat_eff(def) * get_dlist(def, 0, a_spy, 0);

    /*
     * Calculate the initial (pre-support) attack odds.  If they're less
     * than 50%, ask for a confirmation.
     */

    odds = (int)(att_calcodds(ototal, estimate) * 100);

/*
	if (odds < 50) {
		pr("          Estimated defense strength: %8d\n", estimate);
		pr("                      Estimated odds: %8d%%\n\n",odds);
		sprintf(prompt, "Are you sure you want to %s [yn]? ",
			att_mode[combat_mode]);
		if (!confirm(prompt))
			return abort_attack();
		ototal = get_ototal(combat_mode, off, olist,1.0,1);
		if (att_empty_attack(combat_mode, ototal, def))
			return abort_attack();
	}
 */
    return ototal;
}

/* Get the defensive units and reacting units */
int
att_get_defense(struct emp_qelem *olist, struct combat *def,
		struct emp_qelem *dlist, int a_spy, int ototal)
{
    int d_spy = 0;
    struct emp_qelem *qp;
    struct llist *llp;
    int dtotal;
    int old_dtotal;

    emp_initque(dlist);
    get_dlist(def, dlist, 0, &d_spy);
    dtotal = get_dtotal(def, dlist, 1.0, 0);

    /*
     * Call in reacting units
     */

    if (def->type == EF_SECTOR && def->sct_type != SCT_MOUNT)
	att_reacting_units(def, dlist, a_spy, &d_spy, ototal);

    for (qp = olist->q_forw; qp != olist; qp = qp->q_forw) {
	llp = (struct llist *)qp;
	intelligence_report(def->own, &llp->land, d_spy,
			    "Scouts report attacking unit:");
    }

    old_dtotal = dtotal;
    dtotal = get_dtotal(def, dlist, 1.0, 0);
    if (dtotal != old_dtotal)
	pr("Defense strength with reacting units: %8d\n", dtotal);

    return dtotal;
}

/* Get the defensive land units in the sector or on the ship */

static int
get_dlist(struct combat *def, struct emp_qelem *list, int a_spy,
	  int *d_spyp)
{
    struct nstr_item ni;
    struct llist *llp;
    struct lndstr land;
    int estimate = 0;

/* In here is where you need to take out spies and trains from the defending
   lists.  Spies try to hide, trains get trapped and can be boarded. */

    snxtitem_xy(&ni, EF_LAND, def->x, def->y);
    while (nxtitem(&ni, (s_char *)&land)) {
	if (!land.lnd_own)
	    continue;
	if (land.lnd_own != def->own)
	    continue;
	if (def->type == EF_SECTOR && land.lnd_ship >= 0)
	    continue;
	if (def->type == EF_SECTOR && land.lnd_land >= 0)
	    continue;
	if (def->type == EF_SHIP && land.lnd_ship != def->shp_uid)
	    continue;
	if (def->type == EF_LAND && land.lnd_land != def->lnd_uid)
	    continue;
	if (!list) {		/* Just estimating the enemy strength */
	    estimate += intelligence_report(player->cnum,
					    &land, a_spy,
					    "Scouts report defending unit:");
	    continue;
	}
	if (!(llp = (struct llist *)malloc(sizeof(struct llist)))) {
	    logerror("Malloc failed in attack!\n");
	    abort_attack();
	    return 0;
	}
	memset(llp, 0, sizeof(struct llist));
	emp_insque(&llp->queue, list);
	llp->supplied = has_supply(&land);
	if (!get_land(A_DEFEND, def, land.lnd_uid, llp, 1))
	    continue;
	if (lnd_spyval(&land) > *d_spyp)
	    *d_spyp = lnd_spyval(&land);
    }
    return estimate;
}

/* Calculate the total offensive strength */

static int
get_ototal(int combat_mode, struct combat *off, struct emp_qelem *olist,
	   double osupport, int check)
{
    double ototal = 0.0;
    struct emp_qelem *qp, *next;
    struct llist *llp;
    int n, w;

    /*
     * first, total the attacking mil
     */

    for (n = 0; n <= off->last; ++n) {
	if (off[n].type == EF_BAD || (check &&
				      att_get_combat(&off[n], 0) <= 0))
	    continue;
	ototal += off[n].troops * att_combat_eff(off + n);
    }

    /*
     * next, add in the attack_values of all
     * the attacking units
     */

    for (qp = olist->q_forw; qp != olist; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	if (check && !get_land(combat_mode, 0, llp->land.lnd_uid, llp, 0))
	    continue;
	if (combat_mode == A_ATTACK) {
	    w = -1;
	    for (n = 0; n <= off->last; ++n) {
		if (off[n].type == EF_BAD)
		    continue;
		if ((off[n].x == llp->land.lnd_x) &&
		    (off[n].y == llp->land.lnd_y))
		    w = n;
	    }
	    if (w < 0) {
		lnd_delete(llp, "is in a sector not owned by you");
		continue;
	    }
	    ototal += attack_val(combat_mode, &llp->land) *
		att_combat_eff(off + w);
	} else {
	    ototal += attack_val(combat_mode, &llp->land);
	}
    }
    ototal *= osupport;

    return ldround(ototal, 1);
}

/* Calculate the total defensive strength */

static int
get_dtotal(struct combat *def, struct emp_qelem *list, double dsupport,
	   int check)
{
    double dtotal = 0.0, eff = 1.0, d_unit;
    struct emp_qelem *qp, *next;
    struct llist *llp;

    if (check && att_get_combat(def, 1) < 0)
	return 0;
    eff = att_combat_eff(def);
    dtotal = def->troops * eff;

    /*
     * next, add in the defense_values of all
     * the defending non-retreating units
     */

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	d_unit = 0.0;
	if (check && !get_land(A_DEFEND, def, llp->land.lnd_uid, llp, 1))
	    continue;
	d_unit = defense_val(&llp->land);
	if (!llp->supplied)
	    d_unit = ((double)defense_val(&llp->land) / 2.0);
	dtotal += d_unit * eff;
    }

    dtotal *= dsupport;

    return ldround(dtotal, 1);
}

/*
 * This is the land unit integrity check.  Note that we don't print
 * warnings about victim land units because the attacker may not have seen them
 */

static int
get_land(int combat_mode, struct combat *def, int uid, struct llist *llp,
	 int victim_land)
{
    struct lndstr *lp = &llp->land;
    s_char buf[512];

    getland(uid, lp);

    if (!llp->lcp) {		/* first time */
	llp->x = llp->land.lnd_x;
	llp->y = llp->land.lnd_y;
	llp->lcp = &lchr[(int)llp->land.lnd_type];
    } else {			/* not first time */
	if (lp->lnd_effic < LAND_MINEFF) {
	    sprintf(buf, "was destroyed and is no longer a part of the %s",
		    att_mode[combat_mode]);
	    lnd_delete(llp, buf);
	    return 0;
	}
	if (victim_land) {
	    if (lp->lnd_x != def->x || lp->lnd_y != def->y) {
		lnd_delete(llp,
			   "left to go fight another battle and is no longer a part of the defense");
		return 0;
	    }
	} else {
	    if (lp->lnd_own != player->cnum) {
		sprintf(buf,
			"was destroyed and is no longer a part of the %s",
			att_mode[combat_mode]);
		lnd_delete(llp, buf);
		return 0;
	    }
	    if (lp->lnd_x != llp->x || lp->lnd_y != llp->y) {
		sprintf(buf,
			"left to fight another battle and is no longer a part of the %s",
			att_mode[combat_mode]);
		lnd_delete(llp, buf);
		return 0;
	    }
	    if (lp->lnd_effic < llp->eff) {
		sprintf(buf, "damaged from %d%% to %d%%",
			llp->eff, lp->lnd_effic);
		lnd_print(llp, buf);
	    }
	}
    }
    llp->eff = llp->land.lnd_effic;

    return 1;
}

/*
 * Put the land unit on the disk.  If there was some mobility cost, then
 * subtract it from the units mobility.  Note that this works the same way
 * as sectors & ships in that no mobility is actually taken until the attacker
 * has committed to attacking.
 */

static void
kill_land(struct emp_qelem *list)
{
    struct emp_qelem *qp, *next;
    struct llist *llp;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	if (llp->land.lnd_ship >= 0) {
	    llp->land.lnd_effic = 0;
	    lnd_delete(llp, "cannot return to the ship, and dies!");
	}
    }
}

static void
att_infect_units(struct emp_qelem *list, int plague)
{
    struct emp_qelem *qp, *next;
    struct llist *llp;

    if (!plague)
	return;
    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	if (llp->land.lnd_pstage == PLG_HEALTHY)
	    llp->land.lnd_pstage = PLG_EXPOSED;
    }
}

static void
put_land(struct emp_qelem *list)
{
    struct emp_qelem *qp, *next;
    struct llist *llp;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	llp->land.lnd_mission = 0;
	llp->land.lnd_harden = 0;
	llp->land.lnd_mobil -= (int)llp->mobil;
	llp->mobil = 0.0;
	putland(llp->land.lnd_uid, &llp->land);
	if (llp->land.lnd_own != player->cnum) {
	    emp_remque((struct emp_qelem *)llp);
	    free((s_char *)llp);
	} else
	    get_land(A_ATTACK, 0, llp->land.lnd_uid, llp, 0);
    }
}

/*
 * Keep sending in reinforcements until it looks like we're going to win.
 * Note that the "strength" command also calls this routine.
 */

int
att_reacting_units(struct combat *def, struct emp_qelem *list, int a_spy,
		   int *d_spyp, int ototal)
{
    struct nstr_item ni;
    struct lndstr land;
    struct sctstr sect, dsect;
    struct llist *llp;
    int dtotal;
    int new_land = 0;
    double mobcost;
    double move_cost;
    int supply_state;
    int dist;
    int radius;
    int origx, origy;
    double eff = att_combat_eff(def);
    s_char *p;
    s_char buf[1024];

    /*
     *
     * All units that are within their reaction radius and not damaged
     * below their morale value now get to react to the threatened sect.
     * Once we've sent enough to counter the threat, stop sending them.
     *
     * Not anymore.  All units get to react. :)
     */

    if (list)
	dtotal = get_dtotal(def, list, 1.0, 1);
    else
	dtotal = 0;
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, (s_char *)&land) &&
	   (dtotal + new_land * eff < (int)(1.2 * (float)ototal))) {
	if (!land.lnd_own)
	    continue;
	if (!land.lnd_rad_max)
	    continue;
	if ((land.lnd_x == def->x) && (land.lnd_y == def->y))
	    continue;
	if (land.lnd_own != def->own)
	    continue;
	if (land.lnd_ship >= 0)
	    continue;
	if (!defense_val(&land))
	    continue;
/*
		if (land.lnd_effic <= land.lnd_retreat)
			continue;
 */
	if (!lnd_can_attack(&land))
	    continue;

	/* Only supplied units can react */
	if (!(supply_state = has_supply(&land)))
	    continue;

	dist = mapdist(land.lnd_x, land.lnd_y, def->x, def->y);

	getsect(land.lnd_x, land.lnd_y, &sect);
	/* Units on efficient headquarters can react 1 farther */
	if ((sect.sct_type == SCT_HEADQ) && (sect.sct_effic >= 60))
	    radius = land.lnd_rad_max + 1;
	else
	    radius = land.lnd_rad_max;

	if (land.lnd_mission == MI_RESERVE)
	    radius += 2;

	if (dist > radius)
	    continue;

	getsect(def->x, def->y, &dsect);
	if (!(p = BestLandPath(buf, &sect, &dsect, &move_cost, MOB_ROAD)))
	    continue;

	mobcost = land.lnd_effic * 0.01 * lchr[(int)land.lnd_type].l_spd;
	if (mobcost < 0.01)
	    mobcost = 0.01;
	mobcost = 480.0 / (mobcost + techfact(land.lnd_tech, mobcost));
	mobcost *= (move_cost * 5.0);

	if (land.lnd_mobil < mobcost)
	    continue;

	new_land += defense_val(&land);

	if (!list)		/* we are in the "strength" command */
	    continue;

	/* move to defending sector */
	land.lnd_mobil -= ldround(mobcost, 1);
	origx = land.lnd_x;
	origy = land.lnd_y;
	land.lnd_x = def->x;
	land.lnd_y = def->y;
	putland(land.lnd_uid, &land);
	wu(0, land.lnd_own, "%s reacts to %s.\n",
	   prland(&land), xyas(land.lnd_x, land.lnd_y, land.lnd_own));

	llp = (struct llist *)
	    malloc(sizeof(struct llist));

	memset(llp, 0, sizeof(struct llist));
	llp->supplied = supply_state;
	llp->x = origx;
	llp->y = origy;
	llp->lcp = &lchr[(int)land.lnd_type];
	llp->land = land;
	emp_insque(&llp->queue, list);
	if (lnd_spyval(&land) > *d_spyp)
	    *d_spyp = lnd_spyval(&land);

	intelligence_report(player->cnum, &land, a_spy,
			    "Scouts sight reacting enemy unit:");
    }
    return new_land;
}

/* Pop off shells and fly bombing missions to get your attack multiplier up */

static double
get_osupport(s_char *outs, struct combat *def, int fort_sup, int ship_sup,
	     int land_sup, int plane_sup)
{
    double osupport = 1.0;
    int dam;
    double af, as, au, ap;

    af = as = au = ap = 0.0;
    if (fort_sup) {
	dam = dd(def->own, player->cnum, def->x, def->y, 0, 0);
	af = ((double)dam / 100.0);
	osupport += af;
    }
    if (ship_sup) {
	dam = sd(def->own, player->cnum, def->x, def->y, 0, 0, 0);

	as = ((double)dam / 100.0);
	osupport += as;
    }

    if (land_sup) {
	dam = lnd_support(def->own, player->cnum, def->x, def->y, 0);
	au = ((double)dam / 100.0);
	osupport += au;
    }

    if (plane_sup) {
	dam = off_support(def->x, def->y, def->own, player->cnum);
	ap = (((double)dam) / 100.0);
	osupport += ap;
    }
    sprintf(outs, "attacker\t%1.2f\t%1.2f\t%1.2f\t%1.2f\n", af, as, au,
	    ap);
    return osupport;
}

/* Pop off shells and fly bombing missions to get your defense multiplier up */

static double
get_dsupport(s_char *outs, struct emp_qelem *list, struct combat *def,
	     int ototal, int dtotal)
{
    double dsupport = 1.0;
    int dam;
    double df, ds, du, dp;
    int good = 0;

    df = ds = du = dp = 0.0;
    if (dtotal < 0.1 * ototal) {
	good = -1;
    } else if (dtotal >= 1.2 * ototal) {
	good = 1;
    } else {
	dam = dd(player->cnum, def->own, def->x, def->y, 0, 1);
	df = ((double)dam / 100.0);
	dsupport += df;

	dtotal = get_dtotal(def, list, dsupport, 0);
	if (dtotal < 1.2 * ototal) {
	    dam = sd(player->cnum, def->own, def->x, def->y, 0, 1, 0);
	    ds = ((double)dam / 100.0);
	    dsupport += ds;
	    dtotal = get_dtotal(def, list, dsupport, 0);
	}
	if (dtotal < 1.2 * ototal) {
	    dam = lnd_support(player->cnum, def->own, def->x, def->y, 1);
	    du = ((double)dam / 100.0);
	    dsupport += du;
	    dtotal = get_dtotal(def, list, dsupport, 1);
	}
	if (dtotal < 1.2 * ototal) {
	    dam = def_support(def->x, def->y, player->cnum, def->own);
	    dp = (((double)dam) / 100.0);
	    dsupport += dp;
	}
    }
    if (good)
	*outs = '\0';
    else
	sprintf(outs, "defender\t%1.2f\t%1.2f\t%1.2f\t%1.2f\n\n", df, ds,
		du, dp);
    if (def->own) {
	if (good < 0)
	    wu(0, def->own,
	       "\nOdds are bad for us...support cancelled.\n\n");
	else if (good > 0)
	    wu(0, def->own,
	       "\nOdds are good for us...support cancelled.\n\n");
    }
    return dsupport;
}

/*
 * Land mines add to the defense multiplier.  If the attacker has engineers
 * then this multiplier is cut in half.
 */

static double
get_mine_dsupport(struct combat *def, int a_engineer)
{
    int mines;
    struct sctstr sect;

    getsect(def->x, def->y, &sect);

    if (sect.sct_oldown != player->cnum) {
	mines = min(sect.sct_mines, 20);
	if (a_engineer)
	    mines = ldround(((double)mines / 2.0), 1);
	if (mines > 0) {
	    if (def->own)
		wu(0, def->own, "Defending mines add %1.2f\n",
		   mines * 0.02);
	    pr("Defending mines add %1.2f\n", mines * 0.02);
	    return mines * 0.02;
	}
    }
    return 0.0;
}

/* Get the offensive and defensive support */
int
att_get_support(int combat_mode, int ofort, int oship, int oland,
		int oplane, struct emp_qelem *olist, struct combat *off,
		struct emp_qelem *dlist, struct combat *def,
		double *osupportp, double *dsupportp, int a_engineer)
{
    int ototal, dtotal;
    s_char osupports[512];
    s_char dsupports[512];

    if (combat_mode == A_PARA)
	*osupports = '\0';
    else
	*osupportp = get_osupport(osupports, def,
				  ofort, oship, oland, oplane);

    /*
     * I need to put a 1 at the end of the next four total_stren calls
     * becauase units & mil may have been damaged by collateral damage or
     * neclear warheads from the offensive & defensive support.
     */

    ototal = get_ototal(combat_mode, off, olist, *osupportp, 1);
    if (att_empty_attack(combat_mode, ototal, def))
	return abort_attack();
    dtotal = get_dtotal(def, dlist, *dsupportp, 1);

    /*
     * Calculate defensive support.  If odds are too good or too bad
     * then don't call in support.
     */

    *dsupportp = get_dsupport(dsupports, dlist, def, ototal, dtotal);
    ototal = get_ototal(combat_mode, off, olist, *osupportp, 1);
    if (att_empty_attack(combat_mode, ototal, def))
	return abort_attack();

    if ((*osupports || *dsupports) &&
	(*osupportp != 1.0 || *dsupportp != 1.0)) {
	pr("\n\t\tsupport values\n");
	pr("\t\tforts\tships\tunits\tplanes\n");
	if (*osupportp != 1.0)
	    pr("%s", osupports);
	if (*dsupportp != 1.0)
	    pr("%s", dsupports);
	if (def->own) {
	    wu(0, def->own, "\n\t\tsupport values\n");
	    wu(0, def->own, "\t\tforts\tships\tunits\tplanes\n");
	    if (*osupportp != 1.0)
		wu(0, def->own, "%s", osupports);
	    if (*dsupportp != 1.0)
		wu(0, def->own, "%s", dsupports);
	}
    }

    dtotal = get_dtotal(def, dlist, *dsupportp, 1);
    if (dtotal && def->type == EF_SECTOR)
	*dsupportp += get_mine_dsupport(def, a_engineer);
    return 0;
}

/* How many two-legged bipeds are in this combat force? */

static int
count_bodies(struct combat *off, struct emp_qelem *list)
{
    int n;
    int bodies = 0;
    struct emp_qelem *qp;
    struct llist *llp;

    for (n = 0; n <= off->last; ++n)
	bodies += off[n].troops;
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	llp = (struct llist *)qp;
	bodies += total_mil(&llp->land);
    }
    return bodies;
}

/* This is where the fighting actually occurs. */

int
att_fight(int combat_mode, struct combat *off, struct emp_qelem *olist,
	  double osupport, struct combat *def, struct emp_qelem *dlist,
	  double dsupport)
{
    int success = 0;
    int a_cas = 0;		/* Casualty counts */
    int d_cas = 0;
    int ototal;			/* total attacking strength */
    int dtotal;			/* total defending strength */
    int a_bodies;		/* total attacking mil (incl. mil in units) */
    int d_bodies;		/* total defending mil (incl. mil in units) */
    int d_mil;
    int a_troops[6];
    int n;
    int news_item;
    int recalctime;
    double odds;
    s_char *action;

    ototal = get_ototal(combat_mode, off, olist, osupport,
			combat_mode != A_PARA);
    dtotal = get_dtotal(def, dlist, dsupport, 0);
    if (!dtotal)
	success = 1;

    a_bodies = count_bodies(off, olist);
    d_bodies = count_bodies(def, dlist);
    d_mil = def->troops;
    for (n = 0; n <= off->last; ++n)
	if (off[n].type == EF_BAD)
	    a_troops[n] = 0;
	else
	    a_troops[n] = off[n].troops;

    /* This switch is required to get the spacing right */
    switch (combat_mode) {
    case A_ATTACK:
	pr("               Final attack strength: %8d\n", ototal);
	break;
    case A_ASSAULT:
	pr("              Final assault strength: %8d\n", ototal);
	break;
    case A_PARA:
	if (def->sct_type == SCT_MOUNT ||
	    def->sct_type == SCT_WATER ||
	    def->sct_type == SCT_CAPIT ||
	    def->sct_type == SCT_FORTR || def->sct_type == SCT_WASTE) {
	    pr("You can't air-assault a %s sector!\n",
	       def->sct_dcp->d_name);
	    a_cas = a_bodies;
	    off[0].troops = 0;
	    ototal = get_ototal(A_PARA, off, olist, osupport, 0);
	}
	pr("          Final air-assault strength: %8d\n", ototal);
	break;
    case A_BOARD:
    case A_LBOARD:
	pr("                Final board strength: %8d\n", ototal);
    }


    pr("              Final defense strength: %8d\n", dtotal);
    odds = att_calcodds(ototal, dtotal);
    pr("                          Final odds: %8d%%\n", (int)(odds * 100));

    /* spread the plague */
    if (combat_mode != A_PARA) {
	if (!def->plague)
	    for (n = 0; n <= off->last; ++n)
		if (off[n].type != EF_BAD)
		    def->plague |= off[n].plague;
	for (n = 0; n <= off->last; ++n)
	    if (off[n].type != EF_BAD)
		off[n].plague |= def->plague;
    }
    att_infect_units(olist, off->plague);
    att_infect_units(dlist, def->plague);

    /* Fighting is slightly random.  There is always that last little 
     * effort you see people put in.  Or the stray bullet that takes out
     * an officer and the rest go into chaos.  Things like that.
     * Thus, we have added a very slight random factor that will sometimes
     * allow the little guy to win. We modify the odds a little
     * (either +- 5%) to account for this randomness.  We also only
     * recalculate the odds every 8-50 casualties, not every cacsualty,
     * since a single dead guy normally wouldn't cause a commander to
     * rethink his strategies, but 50 dead guys might. */
    odds = odds + (double)((double)((random() % 11) - 5) / 100.0);
    if (odds < 0.0)
	odds = 0.1;
    if (odds > 1.0)
	odds = 1.0;
    recalctime = 8 + (random() % 43);
    while (!success && ototal) {
	if (chance(odds)) {
	    pr("!");
	    d_cas += take_casualty(A_DEFEND, def, dlist);
	    dtotal = get_dtotal(def, dlist, dsupport, 0);
	    if (!dtotal)
		++success;
	} else {
	    pr("@");
	    a_cas += take_casualty(combat_mode, off, olist);
	    ototal = get_ototal(combat_mode, off, olist, osupport, 0);
	}
	if (((a_cas + d_cas) % 70) == 69)
	    pr("\n");
	if (recalctime-- <= 0) {
	    recalctime = 8 + (random() % 43);
	    odds = att_calcodds(ototal, dtotal);
	    odds = odds + (double)((double)((random() % 11) - 5) / 100.0);
	    if (odds < 0.0)
		odds = 0.1;
	    if (odds > 1.0)
		odds = 1.0;
	}
    }
    pr("\n");
    /* update defense mobility & mil */
    if (success)
	def->mil = 0;
    else {
	if (def->type == EF_SECTOR && d_mil && d_cas) {
	    int tmob;

	    /* Make sure we use a positive mobility here */
	    tmob = ((def->mob < 0) ? -(def->mob) : (def->mob));
	    def->mobcost =
		min(20, min(1, tmob - damage(tmob, 100 * d_cas / d_mil)));
	}
	def->mil = def->troops;
    }

    /* update attack mobility & mil */
    for (n = 0; n <= off->last; ++n)
	if (off[n].type != EF_BAD && off[n].troops < a_troops[n]) {
	    if (off[n].type == EF_SECTOR && off[n].mil)
		off[n].mobcost +=
		    min(20,
			min(1,
			    off[n].mob - damage(off[n].mob,
						100 * (a_troops[n] -
						       off[n].troops) /
						off[n].mil)));
	    off[n].mil -= a_troops[n] - off[n].troops;
	}

    /* update land unit mobility */
    if (d_bodies && d_cas)
	lnd_takemob(dlist, (double)d_cas / d_bodies);
    if (a_bodies && a_cas)
	lnd_takemob(olist, (double)a_cas / a_bodies);

    /* damage attacked sector */
    def->eff = effdamage(def->eff, (d_cas + a_cas) / 10);

    pr("- Casualties -\n     Yours: %d\n", a_cas);
    pr("    Theirs: %d\n", d_cas);
    pr("Papershuffling ... %.1f B.T.U\n", (d_cas + a_cas) * 0.15);
    player->btused += (int)((d_cas + a_cas) * 0.015 + 0.5);

    if (success) {
	switch (combat_mode) {
	case A_ATTACK:
	    news_item = def->own ? N_WON_SECT : N_TOOK_UNOCC;
	    pr("We have captured %s, sir!\n", prcom(0, def));
	    action = "taking";
	    break;
	case A_ASSAULT:
	    news_item = def->own ? N_AWON_SECT : N_START_COL;
	    pr("We have secured a beachhead at %s, sir!\n", prcom(0, def));
	    action = "assaulting and taking";
	    break;
	case A_PARA:
	    news_item = def->own ? N_PWON_SECT : N_PARA_UNOCC;
	    pr("We have captured %s, sir!\n", prcom(0, def));
	    action = "air-assaulting and taking";
	    break;
	case A_BOARD:
	    news_item = N_BOARD_SHIP;
	    pr("We have boarded %s, sir!\n", prcom(0, def));
	    action = "boarding";
	    break;
	case A_LBOARD:
	    news_item = N_BOARD_LAND;
	    pr("We have boarded %s, sir!\n", prcom(0, def));
	    action = "boarding";
	    break;
	}
    } else {
	switch (combat_mode) {
	case A_ATTACK:
	    news_item = N_SCT_LOSE;
	    pr("You have been defeated!\n");
	    action = "attacking";
	    break;
	case A_ASSAULT:
	    news_item = N_ALOSE_SCT;
	    pr("You have been defeated!\n");
	    kill_land(olist);
	    action = "trying to assault";
	    break;
	case A_PARA:
	    news_item = N_PLOSE_SCT;
	    pr("All of your troops were destroyed\n");
	    action = "trying to air-assault";
	    break;
	case A_BOARD:
	    news_item = N_SHP_LOSE;
	    pr("You have been repelled\n");
	    kill_land(olist);
	    action = "trying to board";
	    break;
	case A_LBOARD:
	    news_item = N_LND_LOSE;
	    pr("You have been repelled\n");
	    kill_land(olist);
	    action = "trying to board";
	    break;
	}
    }
    nreport(player->cnum, news_item, def->own, 1);
    if (def->own) {
	wu(0, def->own,
	   "%s (#%d) lost %d troops %s %s\nWe lost %d troops defending\n",
	   cname(player->cnum), player->cnum, a_cas,
	   action, pr_com(0, def, def->own), d_cas);
    }

    if (opt_SNEAK_ATTACK) {
	do_sneak(def, success);
    }

    send_reacting_units_home(dlist);

    /* putland the defending land */
    lnd_put(dlist, 0);

    /* putland the attacking land */
    put_land(olist);

    /* put the victim sector/ship/land */
    if (!success || !take_def(combat_mode, olist, off, def))
	put_combat(def);

    /* put the attacking sectors/ship */
    for (n = 0; n <= off->last; ++n)
	if (off[n].type != EF_BAD)
	    put_combat(&off[n]);

    if (!success)
	return 0;

    switch (combat_mode) {
    case A_ATTACK:
	ask_move_in(off, olist, def);

	/* put sectors again to get abandon warnings */
	for (n = 0; n <= off->last; ++n)
	    if (off[n].type != EF_BAD)
		put_combat(&off[n]);
	break;
    default:
	att_move_in_off(combat_mode, off, olist, def);
    }
    if (def->mil > 0)
	pr("%d of your troops now occupy %s\n", def->mil, prcom(0, def));
    return 1;
}

/* What percentage of the combat forces going head-to-head are we? */

double
att_calcodds(int ototal, int dtotal)
{
    double odds;

    /* calculate odds */
    if (ototal <= 0)
	odds = 0.0;
    else if (dtotal <= 0)
	odds = 1.0;
    else
	odds = ((double)ototal) / (dtotal + ototal);

    return odds;
}

/* Here's where the dead soldiers get dragged off the battlefield */

static int
take_casualty(int combat_mode, struct combat *off, struct emp_qelem *olist)
{
    int to_take = CASUALTY_LUMP;
    int biggest_troops = 0, index = -1;
    int n, tot_troops = 0, biggest_mil, cas;
    struct emp_qelem *qp, *biggest;
    struct llist *llp;

    for (n = 0; n <= off->last; ++n) {
	if (off[n].type != EF_BAD) {
	    tot_troops += off[n].troops;
	    if (off[n].troops > biggest_troops) {
		biggest_troops = off[n].troops;
		index = n;
	    }
	}
    }

    if (tot_troops)
	to_take -= tot_troops;

    if (to_take >= 0) {
	for (n = 0; n <= off->last; ++n)
	    if (off[n].type != EF_BAD)
		off[n].troops = 0;
    } else {
	/*
	 * They can all come off mil.  We rotate the casualties,
	 * starting with the sector containing the most mil.
	 */
	to_take = CASUALTY_LUMP;
	if (index < 0) {
	    pr("ERROR: Tell the deity that you got the 'green librarian' error\n");
	    index = 0;
	}
	while (to_take > 0) {
	    for (n = index; n <= off->last && to_take; ++n) {
		if (off[n].type != EF_BAD && off[n].troops > 0) {
		    --to_take;
		    --off[n].troops;
		}
	    }
	    for (n = 0; n < index && to_take; ++n) {
		if (off[n].type != EF_BAD && off[n].troops > 0) {
		    --to_take;
		    --off[n].troops;
		}
	    }
	}
	return CASUALTY_LUMP;
    }

    if (QEMPTY(olist))
	return (CASUALTY_LUMP - to_take);

    /*
     *  Need to take some casualties from attacking units
     *  Procedure: find the biggest unit remaining (in
     *  terms of mil) and give it the casualties.
     */
    biggest = (struct emp_qelem *)0;
    biggest_mil = -1;
    for (qp = olist->q_forw; qp != olist; qp = qp->q_forw) {
	llp = (struct llist *)qp;

	if (total_mil(&llp->land) > biggest_mil) {
	    biggest_mil = total_mil(&llp->land);
	    biggest = qp;
	}
    }
    if (biggest == (struct emp_qelem *)0)
	return (CASUALTY_LUMP - to_take);

    llp = (struct llist *)biggest;
    cas = lnd_take_casualty(combat_mode, llp, to_take);
    return (CASUALTY_LUMP - (to_take - cas));
}

/* Send reacting defense units back to where they came from (at no mob cost) */

static void
send_reacting_units_home(struct emp_qelem *list)
{
    struct emp_qelem *qp, *next;
    struct llist *llp;
    s_char buf[1024];

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	if ((llp->land.lnd_x != llp->x) || (llp->land.lnd_y != llp->y)) {
	    sprintf(buf, "returns to %s",
		    xyas(llp->x, llp->y, llp->land.lnd_own));
	    llp->land.lnd_x = llp->x;
	    llp->land.lnd_y = llp->y;
	    lnd_delete(llp, buf);
	}
    }
}

/* Check for 0 offense strength.  This call will always preceed an abort */

int
att_empty_attack(int combat_mode, int ototal, struct combat *def)
{
    if (ototal <= 0) {
	if (def->own && player->cnum != def->own) {
	    wu(0, def->own,
	       "%s (#%d) considered %sing you @%s\n",
	       cname(player->cnum), player->cnum,
	       att_mode[combat_mode], xyas(def->x, def->y, def->own));
	}
	pr("No troops for %s...\n", att_mode[combat_mode]);
	return 1;
    }
    return 0;
}

/*
 * Take the defending sector or ship from the defender and give it to the
 * attacker.
 */

static int
take_def(int combat_mode, struct emp_qelem *list, struct combat *off,
	 struct combat *def)
{
    int n;
    int occuppied = 0;
    struct llist *llp, *delete_me = 0;
    s_char buf[1024];
    struct sctstr sect;
    struct shpstr ship;
    struct lndstr land;

    for (n = 0; n <= off->last && !occuppied; ++n) {
	if (off[n].type != EF_BAD &&
	    off[n].troops > 0 &&
	    (off[n].type != EF_SECTOR || off[n].mob)) {
	    ++occuppied;
	    if (def->type == EF_LAND) {
		if (def->lnd_lcp->l_flags & L_SPY) {
		    continue;
		}
	    }
	    --(off[n].troops);
	    --(off[n].mil);
	    ++def->mil;
	    pr("1 mil from %s moves %s\n",
	       prcom(0, off + n), prcom(2, def));
	}
    }
    if (!occuppied) {
	if (QEMPTY(list)) {
	    pr("%s left unoccupied\n", prcom(0, def));
	    if (def->own)
		wu(0, def->own,
		   "No enemy troops moved %s so you still own it!\n",
		   pr_com(2, def, def->own));
	    return 0;
	} else {
	    llp = (struct llist *)list->q_forw;
	    llp->land.lnd_x = def->x;
	    llp->land.lnd_y = def->y;
	    take_move_in_mob(combat_mode, llp, off, def);
	    if (def->type == EF_SHIP) {
		llp->land.lnd_ship = def->shp_uid;
		sprintf(buf, "boards %s", prcom(0, def));
		delete_me = llp;
	    } else {
		llp->land.lnd_ship = -1;
		sprintf(buf, "moves in to occupy %s",
			xyas(def->x, def->y, player->cnum));
		lnd_delete(llp, buf);
	    }
	}
    }
    put_combat(def);
    if (def->type == EF_SECTOR) {
	getsect(def->x, def->y, &sect);
	takeover(&sect, player->cnum);
	if (sect.sct_type == SCT_CAPIT || sect.sct_type == SCT_MOUNT)
	    caploss(&sect, def->own,
		    "* We have captured %s's capital, sir! *\n");
	putsect(&sect);
    } else if (def->type == EF_SHIP) {
	getship(def->shp_uid, &ship);
	takeover_ship(&ship, player->cnum, 1);
	putship(ship.shp_uid, &ship);
    } else if (def->type == EF_LAND) {
	getland(def->lnd_uid, &land);
	takeover_land(&land, player->cnum, 1);
	putland(land.lnd_uid, &land);
    }
    if (delete_me)
	lnd_delete(delete_me, buf);
    att_get_combat(def, 0);
    return 1;
}

/*
 * Ask the attacker which mil & land units they'd like to move into the
 * conquered sector.
 */

static void
ask_move_in(struct combat *off, struct emp_qelem *olist,
	    struct combat *def)
{
    int n;
    struct emp_qelem *qp, *next;
    struct llist *llp;
    s_char buf[512];
    s_char prompt[512];
    s_char land_answer[1024];
    s_char *answerp;

    for (n = 0; n <= off->last; ++n)
	if (off[n].type != EF_BAD && off[n].troops > 0)
	    if (off[n].mob) {
		ask_move_in_off(&off[n], def);
		if (player->aborted)
		    break;
	    }

    if (QEMPTY(olist))
	return;
    memset(land_answer, 0, sizeof(land_answer));
    for (qp = olist->q_forw; qp != olist; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	answerp = &land_answer[(int)llp->land.lnd_army];
	if (player->aborted || att_get_combat(def, 0) < 0)
	    *answerp = 'N';
	if (*answerp == 'Y')
	    continue;
	if (*answerp != 'N') {
	    if (!get_land(A_ATTACK, def, llp->land.lnd_uid, llp, 0))
		continue;
	    sprintf(prompt, "Move in with %s (%c %d%%) [ynYNq?] ",
		    prland(&llp->land),
		    llp->land.lnd_army == ' ' ? '~' : llp->land.lnd_army,
		    llp->land.lnd_effic);
	    *answerp = att_prompt(prompt, llp->land.lnd_army);
	    if (player->aborted || att_get_combat(def, 0) < 0)
		*answerp = 'N';
	    if (!get_land(A_ATTACK, def, llp->land.lnd_uid, llp, 0))
		continue;
	}
	if (*answerp == 'y' || *answerp == 'Y')
	    continue;
	sprintf(buf, "stays in %s",
		xyas(llp->land.lnd_x, llp->land.lnd_y, player->cnum));
	lnd_delete(llp, buf);
    }
    if (QEMPTY(olist))
	return;
    if (att_get_combat(def, 0) < 0) {
	for (qp = olist->q_forw; qp != olist; qp = next) {
	    next = qp->q_forw;
	    llp = (struct llist *)qp;
	    if (!get_land(A_ATTACK, def, llp->land.lnd_uid, llp, 0))
		continue;
	    sprintf(buf, "stays in %s",
		    xyas(llp->land.lnd_x, llp->land.lnd_y, player->cnum));
	    lnd_delete(llp, buf);
	}
	return;
    }
    if (opt_INTERDICT_ATT)
	lnd_interdict(olist, def->x, def->y, player->cnum);
    move_in_land(A_ATTACK, off, olist, def);
}

/* Move offensive land units to the conquered sector or ship */

static void
move_in_land(int combat_mode, struct combat *off, struct emp_qelem *olist,
	     struct combat *def)
{
    struct emp_qelem *qp, *next;
    struct llist *llp;
    s_char buf[512];

    if (QEMPTY(olist))
	return;
    for (qp = olist->q_forw; qp != olist; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	if (!get_land(combat_mode, def, llp->land.lnd_uid, llp, 0))
	    continue;
	take_move_in_mob(combat_mode, llp, off, def);
	llp->land.lnd_x = def->x;
	llp->land.lnd_y = def->y;
	if (def->type == EF_SHIP)
	    llp->land.lnd_ship = def->shp_uid;
	else
	    llp->land.lnd_ship = -1;
    }
    if (QEMPTY(olist))
	return;
    if (def->type == EF_SECTOR) {
	if (opt_INTERDICT_ATT) {
	    lnd_sweep(olist, 0, 0, def->own);
	    lnd_check_mines(olist);
	}
	sprintf(buf, "now occupies %s", prcom(0, def));
    } else {
	sprintf(buf, "boards %s", prcom(0, def));
    }
    if (QEMPTY(olist))
	return;
    for (qp = olist->q_forw; qp != olist; qp = next) {
	next = qp->q_forw;
	llp = (struct llist *)qp;
	lnd_print(llp, buf);
    }
    if (QEMPTY(olist))
	return;
    lnd_put(olist, 0);
}

/*
 * Move assaulting, paradropping, or boarding mil & units into def
 * If the mil are coming from a ship, then pack a lunch.
 */

void
att_move_in_off(int combat_mode, struct combat *off,
		struct emp_qelem *olist, struct combat *def)
{
    struct sctstr sect;
    struct shpstr ship;
    int troops;
    int n, lunchbox;

    move_in_land(combat_mode, off, olist, def);

    getsect(def->x, def->y, &sect);

    for (n = 0; n <= off->last; ++n) {
	if (off[n].type == EF_BAD || !off[n].troops)
	    continue;
	troops = off[n].troops;
	off[n].troops = 0;
	off[n].mil -= troops;
	def->mil += troops;
	put_combat(off + n);
	if (combat_mode == A_ASSAULT) {
	    if (off[n].type != EF_SHIP || def->type != EF_SECTOR) {
		logerror("att_move_in_off: strange assault");
		continue;
	    }
	    getship(off[n].shp_uid, &ship);
	    lunchbox = (int)((troops + 1) * ship.shp_item[I_FOOD]
			     / (ship.shp_item[I_MILIT] + troops
				+ ship.shp_item[I_CIVIL] + 0.5));
	    if (lunchbox > ITEM_MAX - sect.sct_item[I_FOOD])
		lunchbox = ITEM_MAX - sect.sct_item[I_FOOD];

	    ship.shp_item[I_FOOD] -= lunchbox;
	    sect.sct_item[I_FOOD] += lunchbox;
	    putship(ship.shp_uid, &ship);
	}
    }

    putsect(&sect);
    put_combat(def);
}


/* Ask how many mil to move in from each sector */

static void
ask_move_in_off(struct combat *off, struct combat *def)
{
    int mob_support;
    int num_mil, dam = 0, left;
    double d, weight;
    s_char prompt[512];
    s_char buf[1024];
    s_char *p;

    if (att_get_combat(off, 0) <= 0)
	return;
    if (att_get_combat(def, 0) < 0)
	return;
    if (off->own != player->cnum)
	return;
    d = sector_mcost(getsectp(def->x, def->y), MOB_ROAD);
    if ((mob_support = min(off->troops, (int)(off->mob / d))) <= 0)
	return;
    sprintf(prompt, "How many mil to move in from %s (%d max)? ",
	    xyas(off->x, off->y, player->cnum), mob_support);
    if (!(p = getstring(prompt, buf)) || !*p || (num_mil = atoi(p)) <= 0) {
	num_mil = 0;
	return;
    }
/* Make sure we don't move in more than we can support mobility-wise */
    if (num_mil > mob_support)
	num_mil = mob_support;
    if (att_get_combat(off, 0) <= 0)
	return;
    if (att_get_combat(def, 0) < 0)
	return;
    if ((num_mil = min(off->troops, num_mil)) <= 0) {
	pr("No mil moved in from %s\n",
	   xyas(off->x, off->y, player->cnum));
	return;
    }
    mob_support = max(1, (int)(num_mil * d));
    off->mob -= min(off->mob, mob_support);
    off->mil -= num_mil;
    off->troops -= num_mil;
    put_combat(off);
    left = num_mil;
    weight = num_mil * ichr[I_MILIT].i_lbs;
    if (opt_INTERDICT_ATT && chance(weight / 200.0)) {
	if (chance(weight / 100.0))
	    dam +=
		ground_interdict(def->x, def->y, player->cnum, "military");
	dam += check_lmines(def->x, def->y, weight);
    }

    if (dam) {
	left = commdamage(num_mil, dam, I_MILIT);
	if (left < num_mil) {
	    if (left) {
		pr("%d of the mil you were moving were destroyed!\nOnly %d mil made it to %s\n", num_mil - left, left, xyas(def->x, def->y, player->cnum));
	    } else {
		pr("All of the mil you were moving were destroyed!\n");
	    }
	}
	/* maybe got nuked */
	if (att_get_combat(def, 0) < 0)
	    return;
    }
    def->mil += left;
    put_combat(def);
}


/* Charge land units for moving into a sector or onto a ship */

static void
take_move_in_mob(int combat_mode, struct llist *llp, struct combat *off,
		 struct combat *def)
{
    int mobcost;
    int new;

    switch (combat_mode) {
    case A_ATTACK:
	mobcost =
	    lnd_mobcost(&llp->land, getsectp(def->x, def->y), MOB_NONE);
	new = llp->land.lnd_mobil - mobcost;
	if (new < -127)
	    new = -127;
	llp->land.lnd_mobil = new;
	break;
    case A_ASSAULT:
	if (off->shp_mcp->m_flags & M_LAND) {
	    if (llp->lcp->l_flags & L_MARINE)
		llp->land.lnd_mobil -=
		    ((float)etu_per_update * land_mob_scale * 0.5);
	    else
		llp->land.lnd_mobil -= ((float)etu_per_update
					* land_mob_scale);
	} else {
	    if (llp->lcp->l_flags & L_MARINE)
		llp->land.lnd_mobil = 0;
	    else
		llp->land.lnd_mobil = (((float)etu_per_update
					* land_mob_scale) * (-1));
	}
	break;
    case A_BOARD:
	/* I arbitrarily chose the numbers 10 and 40 below -KHS */
	if (llp->lcp->l_flags & L_MARINE)
	    llp->land.lnd_mobil -= 10;
	else
	    llp->land.lnd_mobil -= 40;
	break;
    }
    llp->land.lnd_harden = 0;
}

static void
free_list(struct emp_qelem *list)
{
    struct emp_qelem *qp, *next;

    if (!list || QEMPTY(list))
	return;

    qp = list->q_forw;
    while (qp != list) {
	next = qp->q_forw;
	emp_remque(qp);
	free(qp);
	qp = next;
    }
}

int
att_free_lists(struct emp_qelem *olist, struct emp_qelem *dlist)
{
    free_list(olist);
    free_list(dlist);
    return RET_OK;
}

/*
 * sector_strength - Everyone starts at 1.  You can get up to a max
 *                   of d_dstr, depending on how much you build up the
 *                   defenses of the sector. 
 */

double
sector_strength(struct sctstr *sp)
{
    double d;

    d = 1.0;

    if (sp->sct_type == SCT_MOUNT)
	d = 2.0;

    d = d + ((double)(dchr[sp->sct_type].d_dstr - d) *
	     ((double)sp->sct_defense / 100.0));

    if (d > dchr[sp->sct_type].d_dstr)
	d = dchr[sp->sct_type].d_dstr;
    if (d < 0.1)
	d = 0.1;
    return d;
}
