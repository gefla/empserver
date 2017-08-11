.TH Server "Empire4 Changes"
.NA Empire4.0 "Changes in Empire 4.0 (1996-1998)"
.LV Expert
.s1
This document outlines the various changes to the game and how they
will affect you, the player.  These were coded as the Wolfpack project.
.NF
Changes to Empire 4.0.18 - Thu Sep 24 06:54:27 PDT 1998
 * Fixed bug in aircombat where planes in flight could intercept air
   defense planes and take no damage (i.e. they got to fight twice,
   once for free.)
 * Fixed bug in freeing memory after performing missions that could crash
   the server (it mainly happened after an interdiction mission using
   planes and escorts, where some escorts came from airports that didn't have
   bombers going up.  Freeing the leftover escorts was crashing the server.)
 * Fixed bug where you could load non-existant units if you were allied
   with country #0.
 * Fixed bug where you could pin-bomb a plane with itself.
 * Fixed bug where satellites over a bridge may get killed if the bridge
   is sunk.
 * Fixed bug where land units on a ship in the same sector as a bridge
   that is splashed may get sunk.
 * Fixed bug where planes on a ship in the same sector as a bridge
   that is splashed may get sunk.
 * Fixed doconfig with correct empire site text.
 * Fixed major problem with abms not firing, and sometimes crashing
   server.
 * Fixed bug in land units counting up loaded units wrong.
 * Fixed doconfig makefile to have doconfig.c as a dependency.
 * Fixed bug in resetting commodities (the comm_uid was not being
   properly set.)
 * Fixed bug in repaying loans not working correctly (the l_uid was not
   being properly set.)
 * Fixed bug in buying items from the trading block not allowing you
   to due to a perceived change in item status.
 * Fixed bug in mfir.c where a bogus input to a target could crash
   the server.
 * Fixed bug that after you read telegrams new telegrams may not send
   an inform message (the old telegram flags were not cleared.)
 * Fixed bug where fort support distance calculations are calculated
   twice instead of just once.  This bug caused a lower percentage
   of support fire than designed.
 * Fixed bomb.t to reflect land unit changes.
 * Put in some integrity checking for planes returing from bombing runs.
 * Added ability to edit land unit that a land unit is loaded on in
   edit command.
 * Consolidated bridgefall code into "knockdown" function (this code
   existed in at least 3 places, and was different in all of them.)
 * Subs returning fire are no longer reported in the news.
 * Visitor countries can now use the 'motd' command.
 * When trying to use a visitor country, if it is in use, you are not told
   by whom, just that it is in use.
 * Optimized (slightly) support fire from forts not getting supply and
   shells if not needed (out of range)
 * Updated Education.t
 * Modified (increased) chances of hitting mines slightly.
 * Removed unused variables from shp_check_mines.

Changes to Empire 4.0.17 - Fri Jul 31 06:12:21 PDT 1998
 * Added ability in edit to change coastal flag for sectors.
 * Added ability in edit to edit plague values for ships.
 * Added ability for "spy" to report all units/planes not owned by you
   that exist in the sector you are spying from.
 * Modified naval planes and anti-sub planes.
 * Changed so that missiles and bombs that miss their targets cause
   collateral damage in the target sector (they have to land somewhere!)
 * Changed llook so that non-spy units are required to have at least 1
   military personnel on board to see anything.
 * Fixed "llookout.t" to reflect change for military requirement.
 * Updated Plague.t
 * Updated upgrade.t
 * Added "lmine" flag for deities to see what sectors have mines in them
   (works for sea and land mines, used "lmine" to distinguish it from "min"
   which determines mineral (iron) content of a sector.)
 * No longer able to pin bomb land units on a ship.
 * Land units are required to have at least one military loaded to perform
   a mission.
 * Firing land units are required to have at least on military loaded to
   be able to fire (or return fire.)
 * Spies are not always seen when being pinbombed.  You have to look very
   carefully for them (as you usually would.)
 * Fixed typo in "Spies.t"
 * Added new info about spy ability to spy.t.
 * Updated tax information in Innards.t and Update-sequence.t
 * Fixed typo in fire.t information about units firing on ships.
 * Loading military onto land units now resets fortification.  (You gotta
   re-fortify the new guys.)
 * Fixed bug where planes that were mine capable could not drop mines if
   they were not cargo capable.
 * Fixed bug in potentially crashing in update code for nations tech/research.
 * Fixed bug in execute putting you into execute mode incorrectly.
 * Fixed bug in board not allowing land units to board from 0 mobility
   sectors.
 * Fixed bug where interdicted land units that were missed displayed
   a "SPLASH! Bombs miss your ships" message.
 * Fixed bug in minesweeping sectors where, even with mobility or
   having the sweep ability, ships would get hit by mines for doing
   nothing.
 * Fixed bug in count_land_planes always writing out land units unnecessarily.
 * Fixed bug in count_planes always writing out ships unnecessarily.
 * Fixed bug in lnd_count_units always writing out land units unnecessarily.
 * Fixed bug in count_units always writing out ships unnecessarily.
 * Fixed bug in llook that reported units on ships.
 * Fixed bug in llook that reported satellites launched over the unit.
 * Fixed bug in llook that always reported spies (it should be a 10-100%
   chance)
 * Fixed bug in anti possibly not saving lost items correctly.
 * Fixed bug in planes getting extended range when on missions and the
   op center is not where the plane is located.
 * Fixed bug in land unit defensive strength not being based on the eff of
   the unit when calculating odds of a battle.
 * Fixed bug in board not reporting consistant information.
 * Fixed bug in the way land unit casualties were being taken.
 * Fixed bug where land units on ships could return fire.
 * Fixed bug where land units on other land units could return fire.
 * Fixed bug where land units on other land units could fire.
 * Fixed bug in attacks/assaults/boardings spreading plague incorrectly.
 * Fixed bug in updating plague for ships.
 * Fixed bug in updating plague for land units.
 * Fixed bug in updating plague for attacking/defending land units.
 * Fixed bug where you couldn't pin-bomb land units that were < 20% eff.
 * Fixed bug which revealed the owner of torping subs when on a mission.

Changes to Empire 4.0.16 - Fri Jun 12 08:52:06 EDT 1998
 * Added patches sent in by Steve McClure, Sverker Wiberg and Curtis
   Larsen.  They are described with other changes below.
 * Fixed bugs in the following commands that allowed two cooperating
   countries to create infinite numbers of any commodity and/or cash
   at any time (race conditions in the server): build, board, deliver,
   designate, distribute, explore, fuel, improve, load, ltend, mobquota,
   move, name, order, reset, sail, sell, set, tend, territory, test,
   threshold, torpedo, transport, unload
 * Fixed bug in board command giving out too much information about a
   non-owned sector when it shouldn't be.
 * Fixed bug in board command when firing on a sector in defense before
   checking mobility.
 * Fixed bug in rangeedit allowing plane(s) to possibly be stored wrong.
 * Fixed bug in launch allowing plane to possibly be stored wrong after
   launch.
 * Fixed bug in lrangeedit allowing land unit(s) to possibly be stored wrong.
 * Fixed bug in morale allowing land unit(s) to possibly be stored wrong.
 * Fixed bug in arm/disarm allowing a plane to possibly be stored wrong.
 * Fixed bug in loan sometimes not writing database correctly.
 * Fixed bug in collect.
 * Fixed bug in dropping mines from land units.
 * Fixed crashing bug in sector_strength routine when oceans take
   collateral damage.
 * Fixed bug in transport possibly decrementing too much mobility when
   moving a nuke.
 * Fixed some little warning type messages building with gcc -Wall.
 * Fixed problem compiling lwp threads with glibc6 under Linux.
 * Changed flash so that players can always flash deities.
 * Changed players command to always show deities and visitor countries
   that are logged on, and only show allied countries for normal player
   countries.
 * Fixed bug in anti command not stopping sectors when they revolt.
 * Fixed bug in set_coastal function not counting bridge spans as
   water based sectors (after all, the land is still a coastal sector,
   even if next to a bridge span.)
 * "flash" and "players" is re-enabled for visitor accounts.
 * Fixed bug in bleeding of technology and research to other players.
 * Fixed bug in explore not spreading plague correctly.
 * Fixed bug in move not spreading plague correctly.
 * Fixed bug in deliver not spreading plague correctly.
 * Fixed bug in distribute not spreading plague.
 * Included "postresults" script in the scripts directory which can be
   used to auto-post daily power chart/announcements to rec.games.empire.
 * Updated Plague.t
 * Updated Innards.t
 * Removed OVCONFIG from build.conf, and patched doconfig to match.
 * doconfig is only run if needed
 * emp_client and emp_server are only linked if needed
 * Added list of disabled options to the version command.
 * Fixed bug in survey allowing you to see hidden variables.
 * Re-enabled escort missions due to above bug fix most likely the problem.
 * Changed one instance of "restrict" to "restricted" in bestpath.c.  For
   some reason, this was causing a problem on one of the Linux builds (??).
 * Added "show sector capabilities" functionality (this didn't exist
   before.)
 * Fixed bug in neweff not reporting stopped sectors.  It now (correctly)
   reports them as not changing eff.

Changes to Empire 4.0.15 - Wed May 20 12:35:53 EDT 1998
 * Fixed the spelling of Markus' name in CHANGES4.0 files.
 * Added Markus' patches/fixes.  Some are detailed below.
 * Added 'mipsultrix.gxx' build target.
 * Fixed doconfig to write ipglob.c in the correct target area.
 * Fixed a bunch of type casting that needed to be done correctly.
 * Only print out last connect by for non-visitor accounts.
 * 'players' command is only useable by non-visitor accounts.
 * Fixed ask_off in attsub.c to not print out allied sector mil counts
   when attacking from neighboring sectors.
 * Fixed targetting of che when taking over sectors.
 * 'anti' command only fights che that are targetted at you.
 * Fixed update not updating timestamps of objects (ships, planes, land
   units, sectors.)
 * Fixed bug delivering conquered populace.
 * Fixed potential bug scuttling ship with land units on it.
 * Moved heavy bombers to tech 90.
 * Fixed bug in setsector telling the deity coordinates of sectors being
   granted/taken away.
 * Fixed bug in setting budget of enlistment sectors to 0.

Changes to Empire 4.0.14 - Wed Apr  8 08:47:54 EDT 1998
 * Fixed time_t problem in common/log.c
 * Fixed bug in headlines
 * Replaced vaxultrix build flags with proper vanilla ones.
 * Fixed bug in update/prepare.c (sometimes not getting charged for
   mil on units and ships)
 * Fixed bug in printing of "No ship(s)" twice in cargo command when no
   ships were selected.
 * Temporarily disabled escort missions until a fix is found. They are
   randomly crashing the server.
 * Fixed morale.t to reflect that retreat percentage is based off of
   morale_base and not 75.
 * Fixed bug in bridgefall where planes and units on ship in a sector
   that has a bridge collapse are being sunk.
 * Fixed bug in update/produce.c when a sector overflows it's capacity
   on production.
 * Fixed bug in produce command reporting incorrect costs (sometimes).
 * Land units on ships will now try to draw supply from the ship they
   are on.

Changes to Empire 4.0.13 - Mon Mar  2 11:04:28 EST 1998
 * Fixed bug in distribute when world sizes are other than 64x32
 * Fixed bug in getcommand (not really a bug, just made it work like
   it used to so that the players command is useful for deities
   again)
 * Fixed building of POSIX threads on Alpha running Digital Unix.
 * Fixed line_of_sight prototype in sona.c
 * Fixed fairland not to conuse stupid C++ compilers.

Changes to Empire 4.0.12 - Tue Feb 24 11:27:31 EST 1998
 * Fixed client build on linux (whoops)

Changes to Empire 4.0.11 - Tue Feb 10 10:53:10 EST 1998
 * AIX build seems to only work with gcc right now (but at least that works)
 * Vax Ultrix (vaxultrix) build should work now (hopefully) out of the box.
 * Took out autosupply of airports when bombing or dropping shells.
    (This was the only commodity this was done for, and it was creating
    problems since supply is still somewhat broken somewhere)
 * Included Curtis Larsen's, Markus Armbruster's and Sverker Wiberg's
    submitted patches, which collectively included cleaning up most
    of the server prototypes and bogus declarations.  Many thanks.
 * Increased incoming command buffer to 1024 from 512 bytes
 * Increased the # of parsed arguments from 64 to 100
 * Fixed bug where spies were not dying when damaged.
 * Fixed bug in HIDDEN mods in declare command not printing country # of
    uncontacted country correctly (or at all as a matter of fact)

Changes to Empire 4.0.10 - Mon Aug 18 12:34:58 EDT 1997
 * Fixed bug where fleets were being interdicted but the damage was being
    spread to ships not in the same sectors.
 * Fixed but in market when buying goods without enough cash.
 * Planes in orbit over airports are no longer fixed up during updates.
 * Planes in orbit are no longer damaged when the sectors they are over
    are damaged.
 * Planes on ships are no longer damaged when the sectors they are in are
    damaged, unless the ship is damaged.
 * Fixed problem with no newline after partisan activity telegram in
    anti.c
 * Fixed problem in chan.c printing out change costs incorrectly.
 * Fixed problem in dispatch.c screwing things up on redirection.
 * financial should now handle 6 and 7 digit loans.
 * Planes on ships that are in sectors that revolt are no longer taken
    over.
 * Fixed bug in powe.c where the power report was mis-calculating the
    efficiency of planes.
 * Fixed doconfig.c to use STDC instead of multi-level #ifdefs.
 * Fixed bug in parse.c that was screwing up double quotes in conditional
    arguments.
 * Military in a sector now only produce up to maxpop, just like civvies
    and uw's.  No more stuffing 6K mil into a mountain to max it out.  You
    can still hold more mil there over the updates, but the extras just
    won't produce anymore.
 * Fixed bug in shark that allowed you to shark up loans even if you couldn't
    cover the debt.
 * Fixed bug in day of week calculation for server up time in common/keyword.c
    that is used for gamedays.
 * Documented what happens to standing military in collect.t when you collect
    a sector.
 * Documented mountains only holding and using 1/10th of the normal sector
    population in Sector-types.t.
 * Fixed documentation on ship's firing ranges in fire.t to be less
    ambiguous.
 * Updated nukes in nuke.t
 * Added apropos command (thanks to Mike Wise)
 * Added case-insensitivity to the info command (thanks to Mike Wise).  If
    there are two files of the same name, and you don't get a complete match,
    then whichever file is found first in the directory is used.
 * Changed documentation in wantupd.h

Changes to Empire 4.0.9 - Sat Apr 19 23:01:51 EDT 1997
 * Fixed dump info pages that were getting formatted funny.
 * Fixed improve info page.
 * Fixed bug in allied planes/units not moving when the carriers move.
 * Fixed bug in satellite output for <100% satellites.
 * Fixed bug in load/unload not putting a newline after some unloadings
    in allied sectors.
 * Fixed bug in harden not printing correct values.
 * Fixed bug in creating/moving/etc. nuclear stockpiles.
 * Fixed bug where subs were trying to torp commodities moving on land when
    on interdiction.  (This was funny ;-) )
 * Fixed bug in "move" where you could keep a sector even after someone else
    took it from you.
 * Budget now correctly reports the # of units being built.
 * Mil on units & ships are now reported as normal military costs, not
    ship or unit maintainence costs in budget.
 * Fixed bug in update code where taxes could potentially be initialized
    incorrectly (affected budgets too.)
 * Fixed bug in nat.h header so that it uses SCT_MAXDEF instead of a fixed
    number (that was incorrect.)
 * Fixed bug with trains - they needed the xlight flag to carry planes.
 * Revamped and improved flak.
 * Added "Flak" info page.
 * Added "Fallout" info page.
 * Fixed repay/offer/consider to all need a capital to be used.
 * Fixed bug with air defense missions not running when not AT WAR.  They now
    fly when HOSTILE.
 * plane/ship/unit short names are all now 4 characters, padded if needed.
 * pdump/sdump/ldump/ndump now just print the short name for the type.
 * sdump now has ship name at very end in quotes.
    These were done at the request of some client developers for ease of use.
 * Fixed extra space in the dump output.
 * Fixed fallout - not quite so nasty anymore.
 * Fixed fallout - things on ships/units are damaged now.
 * Fixed and balanced planes/ships/units/nukes in conjunction with each other.
 * Fixed bug in doconfig.c calculating wrong s_p_etu sometimes.
 * Fixed bug where harbors weren't being used to resupply.
 * Fixed bug where selling units loaded with planes and units wouldn't take
    the loaded planes or units - they are now dropped.
 * Fixed bug in arm/disarm where you could arm/disarm planes on the trading
    block.
 * Deities can now remove things from the trading block/market.
 * Fixed bug in "work" not charging engineers enough mob.
 * Fixed bug in "work" not adding teardown and buildup avail costs together.
 * Subs no longer need mobility to return fire when fired upon.
 * Fixed "reset".
 * Fixed bug in "trade" allowing 2 players to pay for the same item, but only
    the last player gets it.
 * Fixed bug where you couldn't launch missiles from allied ships or sectors.
 * Fixed bombing so that pin-bombing can cause collateral damage too.
 * Fixed sector damage to damage planes there too.
 * Fixed flag in lload so that it doesn't always print if not needed.
 * Fixed "sell.t" info page.
 * Fixed bug in "buy" where not entering the price correctly could cause
    a crash.
 * Fixed "sell" so that at least 1 mobility is required to sell goods.  Keeps
    the midnight "raid and sell" abuse down. :)
 * Fixed "show plane cap" to move the last column over 1 more where it should
    be.
 * Fixed timestamps to be updated for units/planes on ships/units that move,
    since those units/planes move too.

Changes to Empire 4.0.8 - Wed Feb 26 23:00:51 EST 1997
 * Fixed bug in nstr_exec that was wiping out the previous conditional.
    This was major because it affected timestamp values which are more
    than 65535.
 * Fixed bug in sdump.c for typo in reporting the trade ships origin.

Changes to Empire 4.0.7 - Mon Feb 24 22:48:54 EST 1997
 * Fixed bug in aircombat.c when calculating the air combat odds.  Negative
    numbers were screwing things up good...
 * Fort sector's coordinates are no longer printed when auto-firing at
    ships.
 * Subs can now surface and fire deck guns (again)  They can also be
    hit by return fire when doing so.
 * Fixed bug in setting of plane attack and defensive values so that
    negative numbers don't keep going further negative.
 * Fixed incorrect military control calculation in the sell command.
 * Trading posts are no longer required to sell goods from.  You can sell
    from harbors and warehouses now too.
 * Spy now prints out the owner of land units you see when spying.  Before
    this was assumed to be sector owner, which is no longer true.
 * Loading land units now prints out what was loaded onto each unit,
    just as loading ships do.
 * Added fallout, coast, c_del, m_del, c_cut and m_cut to dump output.
 * Added "GO_RENEW" option.  This option means that gold and oil resources
    are renewable.
 * Added "lost_items_timeout" config variable, and set the default to
    48 hours.  This determines how long lost items stay in the lost items
    database.
 * Fixed land unit names to be more consistent.
 * Fixed mission.t info page.
 * Fixed bug in that if mission_mob_cost was set to 0, even negative
    mob units should be able to be put on missions.
 * Removed restriction on things needing to be at least 60% to be put
    on a mission.  Note that while you can put them on missions at < 60%
    now, when trying to do the mission, it still checks the eff.  This
    is to help in automatic setup (build, put on mission, forget) instead
    of having to come back repeatedly.
 * Spies caught in Neutral or Friendly countries cause the Neutral or
    Friendly country to go Hostile towards the owner of the spy.
 * Tweaked ammunition numbers for artillery units.
 * No more automatic declarations of War should be made.  You will go
    hostile, but since going to war doesn't increase your countries
    defenses, and is purely political, it is left to the player to go
    that final step.
 * Fixed bug in trade that wasn't incrementing the time if last second bidding.
 * Fixed bug in "lmine" that was crashing the server if the land unit was
    out of mobility.
 * Fixed buy so that if you specify a product, you can only bid on that
    type of product.
 * Added "lost.t" info page.
 * Added "lost items" database (EF_LOST)
 * Fixed bug where you could move in allied forces after an attack if they
    bordered the victim area.
 * Fixed nstr_comp to deal with values > 16 bits coming in from
    client for comparisons.
 * Changed last minute market/trade timers to increment 5 minutes.  2 minutes
    was too quick.
 * Fixed decode in lib/common/nstr_subs.c to deal with NSC_TIME better.
 * Commented out the logging of the market checking in server/marketup.c,
    lib/commands/buy.c and lib/commands/trad.c  It was generating lots of
    pretty much useless data that made parsing the server.log file more
    more difficult than it needed to be.
 * Fixed crashing bug in ldump.
 * sdump, ldump, ndump, pdump and dump now print out the current
    timestamp on the "DUMP XXX" line.
 * Fixed denial of service bug in lib/player/accept.c in
    player_find_other function.
 * Fixed bug in shark reporting incorrect buyer of the loan.
 * Fixed bug in sdump.c (case 0 should have been case 10)
 * ldump and sdump now always print the fuel column if asked for,
    even if opt_FUEL is turned off.  In the case that it is turned
    off, the fuel is listed as 0.
 * Added "timestamp" field, which is updated every time an item is
    changed.
 * Fixed up the Clients.t info page.
 * sdump now prints trade ship building origin.
 * sdump now always prints name even if SHIPNAMES isn't defined.  If
    it is not defined, then the name is empty.
 * Added "timestamp" to info/Concepts/Selector.t
 * Added some more relevant info to info/Server/Empire4.t
 * Fixed Infrastructure.t info page to reflect the fact that infrastructure
    is no longer torn down when a sector is re-designated.
 * Fixed sell.t to more accurately reflect the time-delay market.
 * Fixed read.t and wire.t to reference accept and reject.
 * Fixed reject.t not referencing accept command.
 * Fixed collect.t to more accurately reflect what goes on when collecting
    sectors.
 * Fixed financial.t to reflect defaulted loans.
 * Fixed dump.t, sdump.t, ldump.t, ndump.t, pdump.t to reflect new
    timestamp info.
 * Fixed "census.t" (had incorrect reference to "info syntax" instead
    of "info Syntax".

Changes to Empire 4.0.6 - Thu Jan 16 11:33 EST 1997
 * Increased damage from depthcharges because on increased damage from
    torpedos.  Sub frange decreased to be more balanced with destroyer frange.
 * Intelligence reports (spy) on units will now report the estimated number
    of mil on the unit.
 * Fixed equation for ship visibility so it will drop as tech increases.
 * Added new commands sdump, ldump, pdump, and ndump to dump data on ships,
    land units, planes, and nukes.
 * If fields are provided, dump will only supply those fields requested.
 * Decreased speed of subs.
 * When options NO_LCMS or NO_HCMS are set, those commodities are no longer
    required for infrastructure improvements.
 * Units that lose an assault or a boarding attempt from a ship will no
    longer swim back to the ship they came from.
 * Units that take extra casualties will no longer lose all their mil at
    once.
 * Infrastucture is retained when redesignating a sector.
 * Dieing spies will no longer crash the server.
 * Units, planes, and ships must have mobility to perform missions.
 * Units on ship being scrapped are transferred to harbor.
 * Food is no longer autoloaded onto units when they are built.
 * Fixed show plane stat to show correct range.
 * Infantries now take damage at same rate as casualties.
 * Fixed bug with hap_fact.  Having more happiness now helps with fighting
    che.
 * Fixed anti to write back target country so che will continue to fight
    when they survive.
 * Fixed llookout to show correct estimate on number of mil on unit.
 * Added buildable architecture "hp".  This type will build the server on
    a HP/UX machine using the standard compiler instead of gcc.

Changes to Empire 4.0.5 - Thu Dec 12 10:28:48 EST 1996
 * Fixed bug in update/distribute.c where pathcost was not being called with
    the MOB_ROAD argument (and thus distribution costs could be GREATLY
    affected.) - Thanks Ice!

Changes to Empire 4.0.4 - Mon Dec  9 11:00:00 EST 1996
 * Fixed Solaris port using gcc.
 * Fixed doconfig.c to create directories with right modes.
 * Fixed install macros in makefiles to move binaries to the right
    places.

Changes to Empire 4.0.3 - Wed Dec  4 22:46:53 EST 1996
 * Added HP/UX port.
 * Fixed doconfig.c (put exit(0) at the end of main.)
 * Fixed improve.c (moved the prompt[] string outside the fcn.)

Changes to Empire 4.0.2 - Mon Oct 14 12:26:40 EDT 1996
 * Put in LND_MINMOBCOST in land.h
 * Put in change in lnd_mobcost in lndsub.c
 * Put in new nuke costs in nuke.c
 * Put in fix so that total work can only be done by the max pop. No more extra
    civvies tossed in will do it anymore in human.c.
 * Put in fix for src/util/Makefile - beefed it up.
 * Put in fix to show where your ship is when it gets shelled in mfir.c.
 * Changed infrastructure of roads from .040 to .020 (122 instead of 150 in
    common/move.c)
 * Fixed Update-sequence.t.
 * Put in fix for mobcost bonus for 0% highways.
 * Fixed stop.t
 * Fixed commodity.t
 * Spruced up torpedo damage somewhat. :) :)
 * Fixed "assault bmap bug" in attsub.c
 * Fixed Produce.t (bars cost)
 * Changed mountains to get an automatic "2" for defensive bonus in attsub.c
 * Fixed consider.t
 * Fixed repay.t
 * Fixed offer.t
 * Fixed "offer" and "consider" not being legal commands in player/empmod.c
 * Fixed Damage.t
 * Fixed sstat.t
 * Fixed lstat.c, pstat.c and sstat.c
 * Fixed cutoff command in cuto.c
 * Fixed attack value in attack_val in lndsub.c
 * Fixed lload in commands/load.c
 * Fixed defense_val in lndsub.c - Made it a minimum of 1, so that units will
    always fight until dead or retreating.  0 makes them get stuck.
 * Fixed update/deliver.c - no delivery of non-oldowned civvies (or anything
    else for that matter.)
 * Fixed llook in commands/look.c
 * Fixed che bug in subs/nstr.c
 * Fixed bug in strv.c by overcompensating by 1 food per sector (minute amounts
    of people would starve.)
 * Fixed update/human.c - feed_people rounding problems (I hope.)
 * Fixed morale problem in update/land.c
 * Fixed count_bodies in attsub.c
 * Fixed lnd_mobcost.
 * Fixed subs/land,plane,ship.c to handle sunken units and planes correctly.
 * Fixed interest rate in commands/offe.c
 * Fixed bug in buy.c.
 * Put in DEFENSE_INFRA soption o you can turn on/off the use of the defensive
    infrastructure.  When off, the defensive infrastructure is the same
    as the sector efficiency, and you can't improve it.  This is OFF by
    default.
 * Fixed Empire4.t
 * Added Wolfpack.t
 * Land units are now built with a default reaction radius of 0.
 * Changed name of lt artilleries to "lat" from "lart" so you can now build
    "lar"s again.
 * Lowered the speed and firing range of pt boats.
 * Lowered the initial att strength of cavs from 1.5 to 1.3 (they were TOO
    powerful.)
 * Changed the max mob gain defaults of units/planes to 1.0 and ships to 1.5
    (This is * ETU_RATE, so it is equal to ETU_RATE for units/planes and
     1.5 * ETU_RATE for ships.)
 * Conquered civvies only pay 1/4 taxes.
 * Railways don't get torn down when you rebuild sectors anymore (but roads
    and defenses do (if enabled.)

Changes to Empire 4.0.1 - Wed Aug 28 11:35:40 EDT 1996
 * Added "extern double tradetax" to check_trade in trad.c
 * Improved description of data directory in build.conf
 * Fixed bug in attacking land units that retreat so they now get
    charged mobility for attacking.
 * Fixed dump to report road, rail and defense of a sector at the end.
 * Fixed doconfig bug in not checking the right directory to see if the
    "data" directory existed or not before trying to create it.
 * "change" now works for sanctuary countries.
 * Fixed the description of "sinfrastructure".
 * Added more info to "Infrastructure".
 * Units/planes are no longer reduced to 75% when bought from the trading
    block.
 * Supply units were slowed down to the speed of infantry units.
 * Trade-ship payoffs have been lowered to 2.5, 3.5 and 5.0
 * Bars interest is up to $250 per 1K again.
 * Civ taxes were raised back up to a 10:1 mil/civ tax ratio (was 20:1)
 * Fixed various info pages.
 * Fixed bug in people not getting truncated when broke (this was thought
    to be fixed, but wasn't. Now it is, dammit!)
 * Fixed bug with units marching across oceans (scuba gear not included. ;-) )
 * Market/trade taxes and trade ship payoff figures were added to version.
 * "cede" has been removed as a default command.
 * Makefile rule was fixed for depend build.
 * Fixed bug in mapdist not taking world edges into account nicely enough.
 * Added a "scuttle" order for autoscuttling trade ships.  Makes using them
    easier.  Cleaned up scuttle code while in there.
 * Fixed bug with scuttling a ship with units on it not scuttling the units
    too.

Changes to Empire 4.0.0 - Wed Aug 21 1996
 * Initial Wolfpack release - Long live the Wolfpack!!!!
 * Cleaned up the build environment.  Now all that is needed is to edit the
    build.conf file, answer the questions there and type "make <arch>" to
    build for a specific architecture.  Thus, no more reliance on gnumake
    or special shell scripts.
 * We are now shipping the pre-built info pages with the server for those who
    Don't want to build them.  You can also still just type "make" and have
    the info pages build, but it doesn't completely work (i.e. rebuild new
    Subjects) if you don't have perl5 installed.
 * Put in the "help" command that does the same thing as "info".
 * Removed C_SYNC.  This is done for 2 reasons.  1) None of us like it or
    wish to support it.  2) We envision a better scheme for doing similar
    things will come along.
 * Put in MOB_ACCESS - This allows real-time updating of mobility.
 * Put in MARKET - This is the time-based market (yes, still teleports,
    but it's not as bad as it was.)
 * Lots of the documenation has been updated, but there is more to do.
 * Added NO_LCMS, NO_HCMS and NO_OIL options.  When any of these options are
    enabled, you don't need any of that type of material to build things.
    If NO_HCMS is enabled, you don't need HCMS to build bridges, you need
    lcms.  If both NO_HCMS and NO_LCMS are enabled, then you don't need
    any materials to build a bridge.
 * There is no mobility cost for assigning missions.  But, there is also
    no longer any benefit for being on a mission either.
 * Damage is the same for all commodities (people too.)  This means you
    can deity shell/bomb sectors again.
 * Units are now dependant on tech.  What this means is that their statistics
    now increase with tech.  In addition, there is only 1 type of each basic
    unit now (i.e. just "cavalry" instead of "cavalry 1", "cavalry 2", etc.)
    This reflects the idea that as you learn more, you learn how to not only
    build better units, but you learn to build units better. :)  This also
    opens the door for a "lupgrade" command along the same lines as the
    "upgrade" command for ships.
 * Units now have a minimum mob cost for attacking a sector.  Marching
    mob costs have not changed.
 * Starting units have been removed.  This means that at the beginning, people
    may live a little longer since their neighbor can't come visiting quite
    as quick.
 * Shells are no longer required to build units.
 * Mil are no longer required to build units.
 * Mil are no longer an intrinsic part of a unit.  They are now a loadable
    commodity.  The way a unit's defense/att bonuses work now are
        attack = (att * mil * eff)
        defense = (def * mil * eff)
    In addition, when a unit takes damage, both the eff and the mil go
    down.  You can quickly toss in new hacks, but you need to wait to repair
    the unit at the update.  Thus, you can now look at unit's efficiency
    as their training.
 * You need at least 1 mil on a unit to march it (spies are the exception.)
 * Units always react if in range and they have the mobility, no matter
    what their efficency.
 * Units may now march anywhere - in your own sectors, deity owned sectors
    or allied sectors, with 1 exception, spies.  Also, your units can get
    trapped if your ally declares non-alliance with you while your units
    are still in his country.)
 * Units always march at their speed, efficiency doesn't matter.
 * LANDSPIES was added - This creates land unit based spies.  Spies may march
    anywhere, with a chance of getting caught (except in allied territory.)
    See "info Spies" for more info on them.
 * Planes may now be based out of allied airports.  Landing planes on
    ships/sectors you do not own no longer changes thier owner.  You can only
    land on owned/allied sectors/ships, and you may only
    fly/bomb/para/recon/drop from owned/allied sectors.  So, if a country
    you are allied with goes hostile at you, your planes are now stuck there.
    Note that if you are using an allied airport, their commodities get
    sucked up, not yours (obviously.)
 * Planes are now tech-based.  Their statistics increase as their tech
    increases.  Deleted extraneous planes.
 * There is a new toggle, "techlists", which allows you to see what you
    can build/stats/capabilities sorted by tech instead of groupings.
 * Ships are now tech-based.  Their statistics increase as their tech
    increases.  Deleted extraneous ships.
 * Che can now be lessened by making happiness.  If you have more happiness
    than your conquered populace, they don't fight as hard or recruit as
    much.  If you have less than them, they fight harder and recruit more.
    It's not much though - it ranges from 2.0 in your favor to only 0.8
    against you (whereas it was always 1.0 before.)  So, it definitely
    favors the attackers to make lots of happy now.
 * Bridgeheads can only be built on coasts.
 * Players command was fixed so you only see allies, and don't get
    approx #'s of players anymore.
 * POSIX threads support has been added.
 * The attack bmap bug has been fixed.
 * BTUs regenerate 3 times faster now.
 * GRAB_THINGS is off by default.
 * Big nuclear bombs are back, and FALLOUT has been fixed and enabled
    as a default (you couldn't make wasteland before with FALLOUT, now
    you can.)
 * You can now deliver military and civilians.
 * Added SHIP_DECAY option - off by default (turns off ships decaying out
    at sea if not enough mil.)
 * The reverting owner bug has been fixed in territory, thresh, dist
    and deliver.  There are probably more, and when found, will be fixed.
 * The "sectors don't starve when stopped or broke" bug has been fixed.
 * The "people never get truncated" bug has been fixed.
 * Added infrastructure to sectors.  What this means is that a sector
    can now have it's mobility improved by building both roads and
    railways.  It also means that a sectors defense is now based on the
    defensive infrastructure you build into the sector (no more intrinsic
    better defenses based on the sector, you determine which sectors are
    defended heavily, and which aren't.)
 * The defensive value of a sector no longer relies on the efficiency of
    the sector. Instead, it relies on the defensive efficiency of the sector.
    Thus, the "production" and "defensive" aspects of sectors have been
    separated.
 * "show sect stats" now shows the maximum defensive value for sectors.
 * Changed sector structure to take floats for off/def values so we can use
    a base of 1 instead of 2.
 * Added "improve" and "sinfrastructure" to support the infrastructure
    concepts.
 * Added L_TRAIN units which can only travel along railways.  Very fast on
    efficient railways, slow on non-efficient ones.
 * Added the ability for units to carry other units.
 * Fixed census (shortened up "fallout" and "coast" to "fall" and "coa".)
 * Forts only cost $500 to build and 100 hcms now (since they don't get any
    better defenses automagically.)
 * Modified "show sect build" to show the costs for building up the
    infrastructure of a sector.
 * Changed "spy" and the satellite recon stuff (which shows sector stuff for
    spyplanes too) to show the new infrastructure stuff, rounded of course.
 * Added Drake's info->html scripts, with some modifications, so you can
    now type "make html" and have your info pages built as html files too.
    This adds a new directory, "info.html", to the build tree.
 * Modified upgrade so that planes and land units may now be upgraded.
 * Added pstat command to list the statistics of your planes.
 * Added lstat command to list the statistics of your land units.
 * Added sstat command to list the statistics of your ships.
 * Added nmap command to show a map of your sectors after their new
    designations have taken place.

.FI
.s1
.SA "Server"
