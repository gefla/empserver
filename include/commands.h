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
 *  commands.h: Definitions used for command stuff
 * 
 *  Known contributors to this file:
 *  
 */

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "misc.h"
#include "plane.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "nsc.h"
#include "nat.h"
#include "queue.h"
#include "subs.h"
#include "gen.h"
#include "common.h"
#include "lost.h"
#include "map.h"
#include "update.h"

#include "prototypes.h"		/* must come at end, after defines and typedefs */

int acce(void);
int add(void);
int anti(void);
int apro(void);
int arm(void);
int army(void);
int assa(void);
int atta(void);
int bdes(void);
int best(void);
int boar(void);
int bomb(void);
int brea(void);
int budg(void);
int buil(void);
int buy(void);
int capi(void);
int carg(void);
int cede(void);
int cens(void);
int chan(void);
int coas(void);
int coll(void);
int comm(void);
int cons(void);
int conv(void);
int coun(void);
int cuto(void);
int decl(void);
int deli(void);
int demo(void);
int desi(void);
int disa(void);
int dist(void);
int drop(void);
int dump(void);
int echo(void);
int edit(void);
int enab(void);
int enli(void);
int execute(void);
int explain(void);
int explore(void);
int fina(void);
int flash(void);
int flee(void);
int fly(void);
int foll(void);
int force(void);
int fort(void);
int fuel(void);
int give(void);
int grin(void);
int hard(void);
int head(void);
int hidd(void);
int improve(void);
int info(void);
int land(void);
int landmine(void);
int laun(void);
int lboa(void);
int lcarg(void);
int ldump(void);
int ledg(void);
int leve(void);
int lload(void);
int llook(void);
int load(void);
int look(void);
int lost(void);
int lrange(void);
int lretr(void);
int lsta(void);
int ltend(void);
int map(void);
int march(void);
int mark(void);
int mine(void);
int mission(void);
int mobq(void);
int mobupdate(void);
int morale(void);
int move(void);
int multifire(void);
int name(void);
int nati(void);
int navi(void);
int ndump(void);
int new(void);
int newe(void);
int news(void);
int nuke(void);
int offe(void);
int offs(void);
int orde(void);
int orig(void);
int para(void);
int path(void);
int payo(void);
int pboa(void);
int pdump(void);
int plan(void);
int play(void);
int powe(void);
int prod(void);
int pstat(void);
int qorde(void);
int quit(void);
int rada(void);
int range(void);
int rea(void);
int real(void);
int reco(void);
int reje(void);
int rela(void);
int repa(void);
int repo(void);
int rese(void);
int reso(void);
int retr(void);
int rout(void);
int sabo(void);
int sail(void);
int sate(void);
int scra(void);
int sct(void);
int scut(void);
int sdump(void);
int sell(void);
int set(void);
int setres(void);
int setsector(void);
int shar(void);
int shark(void);
int shi(void);
int shoo(void);
int show(void);
int show_motd(void);
int shut(void);
int sinfra(void);
int skyw(void);
int sona(void);
int sorde(void);
int spy(void);
int sstat(void);
int start(void);
int starve(void);
int stop(void);
int stre(void);
int supp(void);
int surv(void);
int swaps(void);
int tele(void);
int tend(void);
int terr(void);
int thre(void);
int togg(void);
int torp(void);
int trad(void);
int tran(void);
int trea(void);
int turn(void);
int upda(void);
int upgr(void);
int vers(void);
int wai(void);
int wall(void);
int wing(void);
int wipe(void);
int work(void);
int xdump(void);
int zdon(void);

#endif /* _COMMANDS_H_ */
