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
 *  multifire.c: Fire at other sectors/ships
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "treaty.h"
#include "nat.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "retreat.h"
#include "news.h"
#include "nsc.h"
#include "file.h"
#include "queue.h"
#include <ctype.h>
#include "options.h"
#include "optlist.h"
#include "damage.h"
#include "commands.h"

enum targ_type {
    targ_land, targ_ship, targ_sub, targ_unit, targ_bogus
};

struct flist {
    struct emp_qelem queue;	/* list of fired things */
    int type;			/* ship? otherwise sector */
    int uid;			/* ship uid */
    coord x, y;			/* sector coords */
    int defdam;			/* damage defenders did */
    int victim;			/* who I was shooting at */
};

union item_u {
    struct shpstr ship;
    struct sctstr sect;
    struct lndstr land;
};

static void add_to_fired_queue(struct emp_qelem *, struct emp_qelem *);
static int defend(struct emp_qelem *al,
		  struct emp_qelem *dl,
		  enum targ_type target,
		  enum targ_type attacker,
		  struct sctstr *vsect,
		  struct sctstr *fsect,
		  struct shpstr *vship,
		  struct shpstr *fship, int fx, int fy, int *nd);
static void do_defdam(struct emp_qelem *, double);
static int quiet_bigdef(int, struct emp_qelem *, natid, natid, coord,
			coord, int *);
static void use_ammo(struct emp_qelem *);

int
multifire(void)
{
    s_char vbuf[20];
    s_char *ptr;
    double range2, range;
    int trange;
    coord fx;
    coord fy;
    coord x;
    coord y;
    int mil;
    int gun;
    int shell;
    int shots;
    double guneff;
    int dam;
    int totaldefdam = 0;
    int fshipno;
    int vshipno;
    double prb;
    natid vict;
    struct shpstr fship;
    struct lndstr fland;
    struct sctstr fsect;
    struct shpstr vship;
    struct sctstr vsect;
    enum targ_type target, attacker, orig_attacker;
    int rel;
    struct natstr *natp;
    struct nstr_item nbst;
    int type;
    s_char *p;
    int nfiring = 0;
    int ndefending = 0;
    union item_u item;
    struct emp_qelem fired, defended;
    double odds;
    s_char buf[1024];
#if defined(_WIN32)
    time_t now;
#endif

    emp_initque(&fired);
    emp_initque(&defended);
    type = (-1);
    while ((type != EF_SECTOR) && (type != EF_SHIP) && (type != EF_LAND)) {
	if (!(p = getstarg(player->argp[1],
			   "Firing from ship(s), sect(s), or land unit(s)? ",
			   buf)))
	    return RET_SYN;
	player->argp[1] = 0;
	type = ef_byname(p);
	if (type == EF_SECTOR) {
	    if (opt_NO_FORT_FIRE) {
		pr("Fort firing is disabled.\n");
		return RET_FAIL;
	    }
	    orig_attacker = attacker = targ_land;
	    shots = 1;
	} else if (type == EF_SHIP) {
	    orig_attacker = attacker = targ_ship;
	} else if (type == EF_LAND) {
	    orig_attacker = attacker = targ_unit;
	} else
	    pr("Please type 'sh', 'l', or 'se'!\n");
    }
    if ((ptr = getstarg(player->argp[2], "Firing from? ", buf)) == 0
	|| *ptr == '\0')
	return RET_SYN;

    if (!snxtitem(&nbst, type, ptr))
	return RET_SYN;

    if (player->aborted) {
	pr("Fire aborted.\n");
	return RET_OK;
    }
    while (nxtitem(&nbst, (s_char *)&item)) {
	attacker = orig_attacker;
	if (attacker == targ_unit) {
	    if (!getland(item.land.lnd_uid, &fland))
		continue;
	    if (!getsect(item.land.lnd_x, item.land.lnd_y, &fsect))
		continue;
	    if (item.land.lnd_own != player->cnum)
		continue;

	    if (fland.lnd_frg == 0) {
		pr("Unit %d cannot fire!\n", fland.lnd_uid);
		continue;
	    }
	    if (lnd_getmil(&fland) < 1) {
		pr("Unit %d cannot fire because it has no military!\n",
		   fland.lnd_uid);
		continue;
	    }
	    if (fland.lnd_ship >= 0) {
		pr("Unit %d cannot fire because it is on a ship!\n",
		   fland.lnd_uid);
		continue;
	    }
	    if (fland.lnd_land >= 0) {
		pr("Unit %d cannot fire because it is on a land unit!\n",
		   fland.lnd_uid);
		continue;
	    }
	    if (fland.lnd_effic < LAND_MINFIREEFF) {
		pr("Unit %d cannot fire because it is less than %d%% efficient\n", fland.lnd_uid, LAND_MINFIREEFF);
		continue;
	    }
	    resupply_commod(&fland, I_SHELL);	/* Get more shells */
	    putland(fland.lnd_uid, &fland);
	    if (getvar(V_SHELL, (s_char *)&fland, EF_LAND) == 0) {
		pr("%s -- not enough shells\n", prland(&fland));
		continue;
	    }
	} else if (attacker == targ_ship) {
	    if (!getship(item.ship.shp_uid, &fship))
		continue;
	    if (item.ship.shp_own != player->cnum)
		continue;
	    if (getvar(V_MILIT, (s_char *)&item.ship, EF_SHIP) < 1) {
		pr("Not enough mil on ship #%d\n", item.ship.shp_uid);
		continue;
	    }
	    gun = getvar(V_GUN, (s_char *)&item.ship, EF_SHIP);
	    gun = min(gun, item.ship.shp_glim);
	    if (item.ship.shp_frnge == 0) {
		pr("Ships %d cannot fire guns!\n", item.ship.shp_uid);
		continue;
	    }
	    if (gun == 0) {
		pr("Not enough guns on ship #%d\n", item.ship.shp_uid);
		continue;
	    }
	    if (getvar(V_SHELL, (s_char *)&item.ship, EF_SHIP) == 0) {
		pr("Not enough shells on ship #%d\n", item.ship.shp_uid);
		continue;
	    }
	    if (item.ship.shp_effic < 60) {
		pr("Ship #%d is crippled!\n", item.ship.shp_uid);
		continue;
	    }
	    fshipno = fship.shp_uid;
	} else if (attacker == targ_land) {
	    if (!getsect(item.sect.sct_x, item.sect.sct_y, &fsect))
		continue;
	    if (item.sect.sct_own != player->cnum)
		continue;
	    if (item.sect.sct_type != SCT_FORTR)
		continue;
	    if (item.sect.sct_effic < ((u_char)FORTEFF)) {
		pr("Fort not efficient enough to fire!\n");
		continue;
	    }
	    if (getvar(V_GUN, (s_char *)&item.sect, EF_SECTOR) == 0) {
		pr("Not enough guns in sector %s!\n",
		   xyas(item.sect.sct_x, item.sect.sct_y, player->cnum));
		continue;
	    }
	    if (getvar(V_SHELL, (s_char *)&item.sect, EF_SECTOR) == 0) {
		pr("Not enough shells in sector %s!\n",
		   xyas(item.sect.sct_x, item.sect.sct_y, player->cnum));
		continue;
	    }
	    if (getvar(V_MILIT, (s_char *)&item.sect, EF_SECTOR) < 5) {
		pr("Not enough military in sector %s!\n",
		   xyas(item.sect.sct_x, item.sect.sct_y, player->cnum));
		continue;
	    }
	    pr("\nSector %s firing\n",
	       xyas(item.sect.sct_x, item.sect.sct_y, player->cnum));
	}
	if ((ptr = getstarg(player->argp[3], "Firing at? ", buf)) == 0
	    || *ptr == '\0')
	    continue;
	if (player->aborted) {
	    pr("Fire aborted.\n");
	    continue;
	}
	ptr[19] = 0;
	(void)strcpy(vbuf, ptr);
	if (issector(vbuf))
	    target = targ_land;
	else
	    target = targ_ship;
	if (target == targ_ship) {
	    vshipno = atoi(vbuf);
	    if (vshipno < 0 || !getship(vshipno, &vship) ||
		(!vship.shp_own)) {
		pr("No such ship exists!\n");
		continue;
	    }
	    target = (mchr[(int)vship.shp_type].m_flags & M_SUB) ?
		targ_sub : targ_ship;
	    vict = vship.shp_own;
	    x = vship.shp_x;
	    y = vship.shp_y;
	    if (!getsect(x, y, &vsect)) {
		pr("No such sector exists!\n");
		continue;
	    }
	} else {
	    if (!sarg_xy(vbuf, &x, &y) || !getsect(x, y, &vsect)) {
		pr("No such sector exists!\n");
		continue;
	    }
	    /* We check the sector type, but we only use it for damage, not
	       reporting.  That way, you don't get extra information you wouldn't
	       normally get.  Besides, what if they want to slam water? :)  */
	    if (vsect.sct_type == SCT_SANCT || vsect.sct_type == SCT_WATER)
		target = targ_bogus;
	    else
		target = targ_land;
	    vict = vsect.sct_own;
	    x = vsect.sct_x;
	    y = vsect.sct_y;
	}
	if (attacker == targ_ship) {
	    shots = -1;		/* convert to max later */
	    if (fship.shp_own != player->cnum) {
		pr("Not your ship!\n");
		continue;
	    }
	    if (target == targ_sub || target == targ_ship) {
		if (fship.shp_uid == vship.shp_uid) {
		    pr("You can't fire upon yourself!\n");
		    continue;
		}
	    }
	    fx = fship.shp_x;
	    fy = fship.shp_y;
/*
  attacker = (mchr[fship.shp_type].m_flags & M_SUB) ?
  targ_sub : targ_ship;
  if (attacker == targ_sub){
  pr("Subs may not fire normally.. use torpedo.\n");
  continue;
  }
*/
	    attacker = targ_ship;
	    if ((mil = getvar(V_MILIT, (s_char *)&fship, EF_SHIP)) < 1) {
		pr("Not enough military for firing crew.\n");
		continue;
	    }
	    gun = getvar(V_GUN, (s_char *)&fship, EF_SHIP);
	    gun = min(gun, fship.shp_glim);
	    if (fship.shp_frnge == 0 || gun == 0) {
		pr("Insufficient arms.\n");
		continue;
	    }
	    shell = getvar(V_SHELL, (s_char *)&fship, EF_SHIP);
	    if (shell < 2)
		shell += supply_commod(fship.shp_own, fship.shp_x,
				       fship.shp_y, I_SHELL, 2 - shell);
	    if (shell <= 0) {
		pr("Klick!     ...\n");
		continue;
	    }
	    if (fship.shp_effic < 60) {
		pr("Ship #%d is crippled (%d%%)\n", fshipno,
		   fship.shp_effic);
		continue;
	    }
	    range = techfact(fship.shp_tech,
			     (double)fship.shp_frnge / 2.0);
	    range2 = (double)roundrange(range);
	    pr("range is %.2f (%.2f)\n", range2, range);
	    if (target == targ_sub) {
		if ((mchr[(int)fship.shp_type].m_flags & M_DCH) == 0) {
		    pr("A %s can't drop depth charges!\n",
		       mchr[(int)fship.shp_type].m_name);
		    continue;
		}
		if (shell < 2) {
		    pr("Not enough shells for depth charge!\n");
		    continue;
		}
	    }
	    gun = min(gun, shell * 2);
	    gun = min(gun, mil / 2);
	    gun = max(gun, 1);
	    if (shots > gun || shots < 0)
		shots = gun;
	    else if (shots == 0) {
		pr("No shots fired.\n");
		continue;
	    }
	    guneff = seagun(fship.shp_effic, shots);
	    dam = (int)guneff;
	    shell -= ldround(((double)shots) / 2.0, 1);
	    putvar(V_SHELL, shell, (s_char *)&fship, EF_SHIP);
	    putship(fship.shp_uid, &fship);
	    if (opt_NOMOBCOST == 0)
		fship.shp_mobil = max(fship.shp_mobil - 15, -100);
	} else if (attacker == targ_unit) {
	    if (fland.lnd_own != player->cnum) {
		pr("Not your unit!\n");
		continue;
	    }

	    if (target == targ_land) {
		if (fland.lnd_x == vsect.sct_x
		    && fland.lnd_y == vsect.sct_y) {
		    pr("You can't fire upon yourself!\n");
		    continue;
		}
	    }

	    fx = fland.lnd_x;
	    fy = fland.lnd_y;

	    if (fland.lnd_frg == 0) {
		pr("Unit %d cannot fire!\n", fland.lnd_uid);
		continue;
	    }
	    if (getvar(V_SHELL, (s_char *)&fland, EF_LAND) == 0) {
		pr("%s -- not enough shells\n", prland(&fland));
		continue;
	    }

	    shell = getvar(V_SHELL, (s_char *)&fland, EF_LAND);

	    range = techfact((int)fland.lnd_tech,
			     (double)fland.lnd_frg / 2.0);
	    range2 = (double)roundrange(range);
	    pr("range is %.2f (%.2f)\n", range2, range);
	    if (target == targ_sub) {
		pr("A %s can't drop depth charges!\n",
		   lchr[(int)fland.lnd_type].l_name);
		continue;
	    }

	    gun = getvar(V_GUN, (s_char *)&fland, EF_LAND);
	    if (gun <= 0) {
		pr("%s -- not enough guns\n", prland(&fland));
		continue;
	    }

	    dam = (int)landunitgun(fland.lnd_effic, fland.lnd_dam, gun,
				   fland.lnd_ammo, shell);
	    if (target == targ_ship) {
		if (chance(((double)fland.lnd_acc) / 100.0))
		    dam = ldround(((double)dam / 2.0), 1);
	    }
	    use_supply(&fland);
	    resupply_commod(&fland, I_SHELL);	/* Get more shells */
	    putland(fland.lnd_uid, &fland);
	} else {
	    fx = fsect.sct_x;
	    fy = fsect.sct_y;
	    if (fsect.sct_own != player->cnum ||
		fsect.sct_type != SCT_FORTR) {
		pr("No fortress at %s\n", xyas(fsect.sct_x,
					       fsect.sct_y, player->cnum));
		continue;
	    }
	    if (target == targ_land) {
		if (fsect.sct_x == vsect.sct_x
		    && fsect.sct_y == vsect.sct_y) {
		    pr("You can't fire upon yourself!\n");
		    continue;
		}
	    }
	    attacker = targ_land;
	    if ((gun = getvar(V_GUN, (s_char *)&fsect, EF_SECTOR)) == 0) {
		pr("Insufficient arms.\n");
		continue;
	    }
	    shell = getvar(V_SHELL, (s_char *)&fsect, EF_SECTOR);
	    if (shell <= 0)
		shell += supply_commod(fsect.sct_own, fsect.sct_x,
				       fsect.sct_y, I_SHELL, 1);
	    if (shell <= 0) {
		pr("Klick!     ...\n");
		continue;
	    }
	    if (getvar(V_MILIT, (s_char *)&fsect, EF_SECTOR) < 5) {
		pr("Not enough military for firing crew.\n");
		continue;
	    }
	    if (target == targ_sub) {
		pr("Target ship not sighted!\n");
		continue;
	    }
	    if (gun > 7)
		gun = 7;
	    range = tfactfire(player->cnum, 7.0);
	    if (fsect.sct_effic > 59)
		range++;
	    range2 = (double)roundrange(range);
	    pr("range is %.2f (%.2f)\n", range2, range);
	    guneff = landgun((int)fsect.sct_effic, gun);
	    dam = (int)guneff;
	    shell--;
	    putvar(V_SHELL, shell, (s_char *)&fsect, EF_SECTOR);
	    putsect(&fsect);
	}
	trange = mapdist(x, y, fx, fy);
	if (trange > range2) {
	    pr("Target out of range.\n");
/*
			switch (target) {
			case targ_land:
			case targ_bogus:
				pr("Target out of range.  Thud.\n");
				break ;
			default:
				pr("Target ship out of range.  Splash.\n");
				break ;
			}	
 */
	    switch (attacker) {
	    case targ_land:
		putsect(&fsect);
		break;
	    case targ_unit:
		fland.lnd_mission = 0;
		putland(fland.lnd_uid, &fland);
		break;
	    default:
		fship.shp_mission = 0;
		putship(fship.shp_uid, &fship);
	    }
	    continue;
	}
/*
		if (target == targ_bogus) {
			if (vsect.sct_type == SCT_SANCT) {
				pr("%s is a %s!!\n", vbuf,
				   dchr[SCT_SANCT].d_name);
				continue;
			} else if (vsect.sct_type == SCT_WATER) {
				pr("You must specify a ship in sector %s!\n",
				   vbuf);
				continue;
			}
		}
*/
	switch (target) {
	case targ_ship:
	    if (!trechk(player->cnum, vict, SEAFIR))
		continue;
	    break;
	case targ_sub:
	    if (!trechk(player->cnum, vict, SUBFIR))
		continue;
	    break;
	case targ_unit:
	case targ_land:
	    if (!trechk(player->cnum, vict, LANFIR))
		continue;
	    break;
	default:
	    break;
	}

	if (opt_SLOW_WAR) {
	    if (target == targ_land) {
		natp = getnatp(player->cnum);
		rel = getrel(natp, vict);
		if ((rel != AT_WAR) && (player->cnum != vict) &&
		    (vict) && (vsect.sct_oldown != player->cnum)) {
		    pr("You're not at war with them!\n");
		    continue;
		}
	    }
	}
	nfiring++;
	switch (target) {
	case targ_sub:
	    pr_beep();
	    pr("Kawhomp!!!\n");
	    if (vship.shp_rflags & RET_DCHRGED)
		retreat_ship(&vship, 'd');
	    break;
	default:
	    pr_beep();
	    pr("Kaboom!!!\n");
	    prb = (double)(range2 ? (trange / range2) : 1.0);
	    prb *= prb;
#if !defined(_WIN32)
	    srandom(random());
#else
	    (void)time(&now);
	    (void)srand(now);
#endif
	    if (chance(prb)) {
		pr("Wind deflects shell%s.\n", splur(shots));
/*			dam = (int)((double)dam / 2.0);*/
		dam =
		    (int)((double)dam *
			  (double)((double)(90 - (random() % 11)) /
				   100.0));
		if (dam < 0)
		    dam = 0;
	    }
	    break;
	}
	switch (target) {
	case targ_bogus:
	case targ_land:
	    nreport(player->cnum, N_SCT_SHELL, vict, 1);
	    if (vict && vict != player->cnum)
		wu(0, vict,
		   "Country #%d shelled sector %s for %d damage.\n",
		   player->cnum, xyas(x, y, vict), dam);
	    pr("Shell%s hit sector %s for %d damage.\n",
	       splur(shots), xyas(x, y, player->cnum), dam);
	    /* Ok, it wasn't a bogus target, so do damage. */
	    if (target != targ_bogus)
		sectdamage(&vsect, dam, 0);
	    break;
	case targ_ship:
	    nreport(player->cnum, N_SHP_SHELL, vict, 1);
	default:
	    if ((target != targ_sub) ||
		((vship.shp_rflags & RET_DCHRGED) == 0))
		check_retreat_and_do_shipdamage(&vship, dam);
	    else
		shipdamage(&vship, dam);
	    if (vict) {
		wu(0, vict,
		   "Country #%d shelled %s in %s for %d damage.\n",
		   player->cnum, prship(&vship),
		   xyas(vship.shp_x, vship.shp_y, vict), dam);
	    }
	    pr("Shell%s hit %s in %s for %d damage.\n",
	       splur(shots), prsub(&vship),
	       xyas(vship.shp_x, vship.shp_y, player->cnum), dam);

	    if (vship.shp_effic < SHIP_MINEFF)
		pr("%s sunk!\n", prsub(&vship));

	    break;
	}
	/*  Ok, now, check if we had a bogus target.  If so,
	   just continue on, since there is no defender. */
	if (target == targ_bogus)
	    continue;
	if (attacker == targ_unit) {
	    attacker = targ_land;
	    getsect(fland.lnd_x, fland.lnd_y, &fsect);
	}
	totaldefdam =
	    defend(&fired, &defended, target, attacker, &vsect, &fsect,
		   &vship, &fship, fx, fy, &ndefending);
	switch (target) {
	case targ_land:
	    putsect(&vsect);
	    break;
	default:
	    putship(vship.shp_uid, &vship);
	    break;
	}
	if ((totaldefdam == 0) && (target == targ_ship))
	    if (vship.shp_rflags & RET_INJURED)
		retreat_ship(&vship, 'h');
	switch (attacker) {
	case targ_land:
	    putsect(&fsect);
	    break;
	default:
	    if ((target == targ_ship) || (target == targ_sub)) {
		if (fship.shp_effic > SHIP_MINEFF) {
		    shp_missdef(&fship, vict);
		};
	    };
	    putship(fship.shp_uid, &fship);
	    break;
	}
    }

    use_ammo(&defended);
    if (nfiring)
	odds = ((double)ndefending) / ((double)nfiring);
    else
	odds = 1.0;
    do_defdam(&fired, odds);
    return RET_OK;
}

static int
defend(struct emp_qelem *al, struct emp_qelem *dl, enum targ_type target,
       enum targ_type attacker, struct sctstr *vsect, struct sctstr *fsect,
       struct shpstr *vship, struct shpstr *fship, int fx, int fy, int *nd)
{

    int dam;
    int vict, nfiring = 0;
    struct flist *fp;
    int aown;

    if (attacker == targ_land)
	aown = fsect->sct_own;
    else
	aown = fship->shp_own;

    if (target == targ_land)
	vict = vsect->sct_own;
    else
	vict = vship->shp_own;

    if (0 !=
	(dam = quiet_bigdef(attacker, dl, vict, aown, fx, fy, &nfiring))) {
	if (nfiring > *nd)
	    *nd = nfiring;
	fp = (struct flist *)malloc(sizeof(struct flist));
	memset(fp, 0, sizeof(struct flist));
	fp->defdam = dam;
	fp->victim = vict;
	switch (attacker) {
	case targ_land:
	    fp->x = fsect->sct_x;
	    fp->y = fsect->sct_y;
	    fp->type = targ_land;
	    break;
	default:
	    fp->type = targ_ship;
	    fp->uid = fship->shp_uid;
	    break;
	}
	emp_insque(&fp->queue, al);
    }

    return (dam);
}

static void
do_defdam(struct emp_qelem *list, double odds)
{

    int dam, vict, first = 1;
    struct flist *fp;
    struct shpstr ship;
    struct sctstr sect;
    struct emp_qelem *qp, *next;

    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	fp = (struct flist *)qp;
	if (fp->type == targ_ship) {
	    if (!getship(fp->uid, &ship) || !ship.shp_own)
		continue;
	}
	if (first) {
	    pr_beep();
	    pr("\nDefenders fire back!\n");
	    first = 0;
	}
	dam = (odds * (double)fp->defdam);

	if (fp->type == targ_ship) {
	    vict = fp->victim;
	    pr("Return fire hit %s in %s for %d damage.\n",
	       prship(&ship),
	       xyas(ship.shp_x, ship.shp_y, player->cnum), dam);
	    if (vict)
		wu(0, vict,
		   "Return fire hit %s in %s for %d damage.\n",
		   prsub(&ship), xyas(ship.shp_x, ship.shp_y, vict), dam);
	    shipdamage(&ship, dam);
	    putship(ship.shp_uid, &ship);
	} else {
	    getsect(fp->x, fp->y, &sect);
	    vict = fp->victim;
	    pr("Return fire hit sector %s for %d damage.\n",
	       xyas(fp->x, fp->y, player->cnum), dam);
	    sectdamage(&sect, dam, 0);
	    putsect(&sect);
	    if (vict)
		wu(0, vict, "Return fire hit sector %s for %d damage.\n",
		   xyas(fp->x, fp->y, vict), dam);
	}
	emp_remque(&fp->queue);
	free((s_char *)fp);
    }
}

static int
quiet_bigdef(int attacker, struct emp_qelem *list, natid own, natid aown,
	     coord ax, coord ay, int *nfiring)
{
    int nshot;
    double range, erange, hitchance;
    struct shpstr ship;
    struct lndstr land;
    struct nstr_item ni;
    int vec[I_MAX + 1];
    int dam, dam2, rel, rel2;
    double tech;
    struct sctstr firing;
    struct nstr_sect ns;
    struct flist *fp;
    int gun;

    if (own == 0)
	return 0;
    dam = 0;
    snxtitem_dist(&ni, EF_SHIP, ax, ay, 8);
    while (nxtitem(&ni, (caddr_t)&ship)) {
	if (ship.shp_own == 0)
	    continue;

	if ((mchr[(int)ship.shp_type].m_flags & M_SUB) &&
	    (attacker == targ_land))
	    continue;

	rel = getrel(getnatp(ship.shp_own), own);
	rel2 = getrel(getnatp(ship.shp_own), aown);
	if ((ship.shp_own != own) && ((rel != ALLIED) || (rel2 != AT_WAR)))
	    continue;
	/* Don't shoot yourself */
	if (ship.shp_own == aown)
	    continue;
	if (ship.shp_effic < 60)
	    continue;
	if (getvec(VT_ITEM, vec, (caddr_t)&ship, EF_SHIP) < 0)
	    continue;

	if (vec[I_MILIT] < 1)
	    continue;

	if (mchr[(int)ship.shp_type].m_flags & M_SUB) {
	    if (vec[I_SHELL] < 3)
		vec[I_SHELL] += supply_commod(ship.shp_own,
					      ship.shp_x, ship.shp_y,
					      I_SHELL, 3 - vec[I_SHELL]);
	    if (vec[I_SHELL] < 3)
		continue;
	    if (vec[I_GUN] < 1)
		continue;
/*
  if (ship.shp_mobil <= 0)
  continue;
*/
	    erange = ship.shp_effic *
		techfact(ship.shp_tech, ((double)ship.shp_frnge))
		/ 100.0;
	    erange = (double)roundrange(erange);
	    range = mapdist(ship.shp_x, ship.shp_y, ax, ay);
	    if (range > erange)
		continue;
	    if (!line_of_sight((s_char **)0, ship.shp_x, ship.shp_y, ax, ay))
		continue;

	    (*nfiring)++;
	    fp = (struct flist *)malloc(sizeof(struct flist));
	    memset(fp, 0, sizeof(struct flist));
	    fp->type = targ_ship;
	    fp->uid = ship.shp_uid;
	    add_to_fired_queue(&fp->queue, list);
/*
  nreport(ship.shp_own, N_FIRE_BACK, player->cnum, 1);
*/
	    hitchance = DTORP_HITCHANCE(range, ship.shp_visib);
	    if (!chance(hitchance))
		continue;

	    dam += TORP_DAMAGE();
	} else {
	    range = techfact(ship.shp_tech,
			     ship.shp_frnge * ship.shp_effic / 200.0);
	    range = (double)roundrange(range);
	    if (range < ni.curdist)
		continue;
	    /* must have gun, shell, and milit to fire */
	    if (vec[I_SHELL] < 1)
		vec[I_SHELL] += supply_commod(ship.shp_own,
					      ship.shp_x, ship.shp_y,
					      I_SHELL, 1);
	    /* only need 1 shell, so don't check that */
	    if (vec[I_SHELL] < 1)
		continue;
	    nshot = min(vec[I_GUN], vec[I_MILIT]);
	    nshot = min(nshot, ship.shp_glim);
	    if (nshot == 0)
		continue;
	    (*nfiring)++;
	    fp = (struct flist *)malloc(sizeof(struct flist));
	    memset(fp, 0, sizeof(struct flist));
	    fp->type = targ_ship;
	    fp->uid = ship.shp_uid;
	    add_to_fired_queue(&fp->queue, list);
	    nreport(ship.shp_own, N_FIRE_BACK, player->cnum, 1);
	    dam += seagun(ship.shp_effic, nshot);
	}
    }
    snxtitem_dist(&ni, EF_LAND, ax, ay, 8);
    while (nxtitem(&ni, (caddr_t)&land)) {
	if (land.lnd_own == 0)
	    continue;
	if (land.lnd_effic < LAND_MINFIREEFF)
	    continue;
	/* Can't fire if on a ship */
	if (land.lnd_ship >= 0)
	    continue;
	if (land.lnd_land >= 0)
	    continue;
	/* Gotta have military */
	if (lnd_getmil(&land) < 1)
	    continue;
	/* Don't shoot yourself */
	if (land.lnd_own == aown)
	    continue;

	rel = getrel(getnatp(land.lnd_own), own);
	rel2 = getrel(getnatp(land.lnd_own), aown);

	if ((land.lnd_own != own) && ((rel != ALLIED) || (rel2 != AT_WAR)))
	    continue;


	range = techfact((int)land.lnd_tech, (double)land.lnd_frg / 2.0);
	range = (double)roundrange(range);
	if (range < ni.curdist)
	    continue;

	resupply_all(&land);
	if (!has_supply(&land))
	    continue;

	if (getvec(VT_ITEM, vec, (caddr_t)&land, EF_LAND) < 0)
	    continue;

	if (vec[I_MILIT] == 0 || vec[I_SHELL] == 0 || vec[I_GUN] == 0)
	    continue;

	dam2 = (int)landunitgun(land.lnd_effic, land.lnd_dam, vec[I_GUN],
				land.lnd_ammo, vec[I_SHELL]);

	(*nfiring)++;
	fp = (struct flist *)malloc(sizeof(struct flist));
	memset(fp, 0, sizeof(struct flist));
	fp->type = targ_unit;
	fp->uid = land.lnd_uid;
	add_to_fired_queue(&fp->queue, list);
	use_supply(&land);
	putland(land.lnd_uid, &land);
	nreport(land.lnd_own, N_FIRE_BACK, player->cnum, 1);
	dam += dam2;
    }

    /*
     * Determine if any nearby gun-equipped sectors are within
     * range and able to fire at an attacker.  Firing sectors
     * need to have guns, shells, and military.  Sector being
     * attacked is x,y -- attacker is at ax,ay.
     */

    if (!opt_NO_FORT_FIRE) {
	snxtsct_dist(&ns, ax, ay, 8);
	while (nxtsct(&ns, &firing)) {

	    if (firing.sct_type != SCT_FORTR)
		continue;
	    if (firing.sct_own == 0)
		continue;
	    rel = getrel(getnatp(firing.sct_own), own);
	    rel2 = getrel(getnatp(firing.sct_own), aown);

	    if ((firing.sct_own != own) &&
		((rel != ALLIED) || (rel2 != AT_WAR)))
		continue;
	    /* Don't shoot yourself */
	    if (firing.sct_own == aown)
		continue;
	    tech = tfactfire(firing.sct_own, 1.0);
	    range = tech * 7.0;
	    if (firing.sct_effic > 59)	/* fort bonus */
		range++;
	    range = (double)roundrange(range);
	    if (range < ns.curdist)
		continue;
	    if (getvec(VT_ITEM, vec, (caddr_t)&firing, EF_SECTOR) < 0)
		continue;
	    if (vec[I_SHELL] < 1)
		vec[I_SHELL] += supply_commod(firing.sct_own,
					      firing.sct_x, firing.sct_y,
					      I_SHELL, 1);
	    if (vec[I_GUN] == 0 || vec[I_MILIT] < 5 || vec[I_SHELL] == 0)
		continue;
	    (*nfiring)++;
	    fp = (struct flist *)malloc(sizeof(struct flist));
	    memset(fp, 0, sizeof(struct flist));
	    fp->x = firing.sct_x;
	    fp->y = firing.sct_y;
	    fp->type = targ_land;
	    add_to_fired_queue(&fp->queue, list);
	    nreport(firing.sct_own, N_FIRE_BACK, player->cnum, 1);
	    gun = vec[I_GUN];
	    if (gun > 7)
		gun = 7;
	    dam += landgun((int)firing.sct_effic, gun);
	}
    }

    return ((*nfiring) == 0 ? 0 : (dam / (*nfiring)));
}

static void
use_ammo(struct emp_qelem *list)
{
    struct emp_qelem *qp, *next;
    struct flist *fp;
    struct shpstr ship;
    struct lndstr land;
    struct sctstr sect;
    int shell, type;
    s_char *ptr;
    double mobcost;
    struct mchrstr *mcp;

    /* use 1 shell from everyone */
    for (qp = list->q_forw; qp != list; qp = next) {
	next = qp->q_forw;
	fp = (struct flist *)qp;
	if (fp->type == targ_ship) {
	    getship(fp->uid, &ship);
	    ptr = (s_char *)&ship;
	    type = EF_SHIP;
	    if (mchr[(int)ship.shp_type].m_flags & M_SUB) {
		shell = getvar(V_SHELL, ptr, type);
		shell--;
		if (shell < 0)
		    shell = 0;
		putvar(V_SHELL, shell, ptr, type);
		putship(ship.shp_uid, &ship);
		mcp = &mchr[(int)ship.shp_type];
		mobcost = ship.shp_effic * 0.01 * ship.shp_speed;
		mobcost = (480.0 / (mobcost +
				    techfact(ship.shp_tech, mobcost)));
		/* mob cost = 1/2 a sect's mob */
		mobcost /= 2.0;
		ship.shp_mobil -= mobcost;
	    }
	} else if (fp->type == targ_land) {
	    getsect(fp->x, fp->y, &sect);
	    ptr = (s_char *)&sect;
	    type = EF_SECTOR;
	} else {
	    getland(fp->uid, &land);
	    ptr = (s_char *)&land;
	    type = EF_LAND;
	}
	shell = getvar(V_SHELL, ptr, type);
	shell--;
	if (shell < 0)
	    shell = 0;
	putvar(V_SHELL, shell, ptr, type);
	if (fp->type == targ_ship)
	    putship(ship.shp_uid, &ship);
	else if (fp->type == targ_land)
	    putsect(&sect);
	else
	    putland(land.lnd_uid, &land);

	emp_remque(&fp->queue);
	free((s_char *)fp);
    }

}

static void
add_to_fired_queue(struct emp_qelem *elem, struct emp_qelem *list)
{
    struct emp_qelem *qp;
    struct flist *fp, *ep;
    int bad = 0;

    ep = (struct flist *)elem;

    /* Don't put them on the list if they're already there */
    for (qp = list->q_forw; qp != list; qp = qp->q_forw) {
	fp = (struct flist *)qp;
	if ((fp->type == targ_ship) && (fp->uid == ep->uid))
	    bad = 1;
	if ((fp->type != targ_ship) && (fp->x == ep->x) &&
	    (fp->y == ep->y))
	    bad = 1;
    }

    if (!bad)
	emp_insque(elem, list);
}
