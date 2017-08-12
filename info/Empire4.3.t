.TH Server "Empire4 Changes"
.NA Empire4.3 "Changes in Empire 4.3 (2006-2015)"
.LV Expert
.s1
This document outlines the various changes to the game and how they
will affect you, the player.  These were coded as the Wolfpack project.
.NF
Changes to Empire 4.3.33 - Wed May 20 18:17:43 UTC 2015
 * New client option -r for restricted mode.  Contributed by Marisa
   Giancarla.
 * The add syntax deprecated in 4.3.29 is now gone.
 * The explore, move, test, transport syntax deprecated in 4.3.27 is
   now gone.
 * The bmap flag syntax deprecated in 4.3.27 is now gone.
 * Stricter configuration validation:
   - Reject odd WORLD_X instead of making it even silently.
   - Reject odd WORLD_Y.
   - Reject missiles that lack capability VTOL instead of adding VTOL
     silently.
   - Reject ships that can carry planes, but have neither capability
     plane nor miss, instead of adding miss silently.
 * Outlaw ships that can drop depth charges, but not fire.  No such
   ships exist in the stock game.  4.3.31 permitted them, but keeping
   them working has turned out to be bothersome.
 * Changes to fire and torpedo:
   - Don't disclose that the torpedo's path to the target is blocked
     by land when the target is out of range.  Screwed up in 4.2.2.
   - When a submarine gets hit by return fire after firing its deck
     gun, the defender again learns the submarine's UID and type, just
     like for surface ships.  This is how it worked before Empire 2.3.
   - When a submarine gets hit by return fire after launching a
     torpedo, the defender again learns the submarine's UID.  Before
     Empire 2.3, he learned UID and type.
   - Print "Kaboom" even when the target is out of range.
   - Always clear missions when firing guns or dropping depth charges.
     Screwed up when missions were added in Chainsaw.
   - Report "Sector X,Y ready to fire" instead of "Sector X,Y firing",
     because the sector isn't firing, yet, then.  Also report ships
     and land units ready to fire.
   - Suppress bulletin when player shells or torpedoes his own ship.
 * When artillery on a bridge span shelled down the supporting bridge
   head, the bridge fell and the artillery drowned alright.  But then
   the bridge rose right back.  Broken in 4.3.14.
 * Fix board to charge mobility even when defender is less than 100%
   efficient.  Broken in Empire 2.
 * The convert command charged security land units mobility until
   4.3.16 broke that.  Drop this undocumented feature instead of
   fixing it.
 * Fix news to report the actual owner of ships sunk by return
   torpedoes instead of POGO.  Screwed up when return torpedoes were
   added in Chainsaw.
 * Don't disclose UID, type and owner of torpedoed submarines.  The
   latter leaked through the news.
 * Changes to bomb:
   - Fix damage to mobility when bombing planes.  Has always been
     broken.
   - Include position when reporting bombing of a land unit, like we
     do for ships.
   - Suppress bulletin when player bombs his own assets.
   - Report bombing of plane to owner once, not twice.
   - Don't permit bombing of dead planes.  Missed when we did the same
     for dead ships and land units in 4.3.16.
 * Changes to retreat and lretreat:
   - Be less loquacious when changing orders.
   - Deprecate pseudo-condition 'c'.  Use retreat path 'h' to cancel
     retreat orders.
   - Change query syntax from "retreat SHIPS" to "retreat SHIPS q".
     Consistent with mission, and makes prompting for arguments more
     usable.  Keep accepting the old syntax for now, but deprecate it.
   - Reject invalid retreat paths instead of silently ignoring invalid
     characters during retreat.  Unfortunately, the path needs to be
     valid even with deprecated pseudo-condition 'c', where it's not
     actually used.  This might break some existing usage.
   - Fail without charging BTUs when the player gives no conditions.
   - Fix infinite loop when third argument contains '?'.  Broken in
     4.3.16.
 * Rename retreat flag "torped" to "torpedoed" in xdump retreat-flags.
   This could conceivably break clients.  No actual breakage is known.
 * Rewrite automatic retreat to fix its many bugs and inconsistencies
   with navigate and march:
   - When a ship defeats a boarding party, but suffers enough damage
     to sink, retreat the victim's group (if any) instead of ship#0.
   - Permit retreat exactly when navigate and march would be
     permitted.  Before, land units happily retreated while on the
     trading block (forbidden with march since 4.0.9), crewless
     (likewise since 4.0.0), kidnapped in a foreign sector
     (inconsistent since land units were added in Chainsaw 3), loaded
     on a ship (likewise) or a land unit (inconsistent since that
     became possible in 4.0.0).  Ships retreated while on the trading
     block (forbidden with navigate since 4.0.9).
   - Land units can now retreat into foreign sectors exactly when they
     could march there: when the sector is allied, or when the land
     unit is a spy (with the usual risks).
   - Land units can now retreat into mountains.
   - Land unit retreat now resets fortification.
   - Group retreat now happens only along a single path.  Before, you
     could scatter a group in several directions, and even retreat in
     far away places.  The latter was a highly abusable design flaw.
   - Bulletins provide more useful information in fewer words.  In
     particular, they always report the end sector.
   - Fix retreat after a torpedo hit to include the torpedo damage in
     its mobility cost calculation.
   - When retreat runs into a sector that cannot be entered, don't
     consume the retreat direction.
   - Don't charge mobility for retreating in direction 'h'
   - You can no longer make your own ships or land units retreat.
   - Clear mission only when the ship or land unit actually retreats.
   - Land unit group retreat could corrupt the land unit file or crash
     the server.  Screwed up when Chainsaw added land unit retreat.
   - Fix ship retreat when helpless.  Surface ships claimed to retreat
     in a bulletin, but that was a lie.  Submarines didn't even
     pretend.
 * Don't tell the player he sunk a ship when it survives an attack
   with bomb, fire, launch or torpedo, but sinks during retreat.  bomb
   even reported where it had retreated to when it sank.
 * Land mine fixes, affecting march and, if option INTERDICT_ATT is
   enabled, attack and assault:
   - Fix march sub-command 'm' to not let non-engineers hit mines.
     Broken in Empire 2.  Fixed in 4.0.17 for ships only.
   - Engineers now risk hitting mines twice instead of once on sector
     entry, just like minesweepers.
   - No sweeping with zero mobility.  Screwed up when Chainsaw added
     land units.
 * Navigate and march cleanup and bug hunt:
   - Stop on non-fatal mine hits, too.  Before, we carried on, and
     ships that lost all crew were left behind only at the next
     prompt.  Note that we already stop on non-fatal interdiction
     damage since Empire 2.
   - Fix use-after-free when a flagship or leader stays behind without
     a prompt following immediately.
   - When something other than movement charged mobility, the charge
     was lost on the next movement, unless there was a prompt in
     between.  For instance, mobility lost to mine hits could come
     back.
   - Fix parser not to get confused by white-space.
   - Make ships use radar always, not just most of the time.
   - Land units now use radar automatically like ships.
   - Report flagship and leader changes immediately, and always.
     Before, they were reported only around a prompt.
   - Wipe mission and retreat orders less eagerly.  Before, they where
     sometimes wiped even for ships and land units the command
     rejected.
   - Give up fortification only when a land unit actually moves or
     sweeps.
   - Report where exactly ships and land units sweep mines.
   - Nicer error messages for sub-command 'm'.
   - Fix sub-command 'm' not to drop engineers without mobility from
     the march.
   - When denying ships entry to an unfriendly sector with canals,
     don't disclose whether it's above 2%.
   - Both navigate and march now require all their ships and land
     units to be in the same sector.  Support for scattered ships and
     land units complicates the code and blows up the test matrix.
     It's also rather obscure; I suspect accidental use has been more
     frequent than intentional use.
   - Check for sector abandonment before anyone marches instead of
     right before moving the last land unit.  Fixes a bug that could
     scatter the group when something interferes while waiting for the
     player to confirm abandonment.
   - When marching a mixed group of spies and non-spies into a
     non-allied sector, remove non-spies from the group, and move the
     spies.  Before, only the spies before the first non-spy moved
     into the non-allied sector, which could scatter the group.
     Screwed up when 4.0.0 added spy units.
   - Don't permit trains to march out of sectors without rail.
   - When navigating a mixed group of ships with and without canal
     capability into a canal, remove incapable ships from the group,
     and move the capable ships.  Before, only the ships before the
     first incapable ship moved into the canal, which could scatter
     the group.  Broken in 4.3.0.
   - Fix navigate buffer overrun for impossibly long paths taken.  No
     remotely sane game configuration provides a ship fast enough to
     trigger it.  Broken in 4.0.0.
 * Remove option SAIL (commands follow, mquota, sail, unsail; ship
   selectors mquota, path, follow) and autonav (commands order,
   qorder, sorder; ship selectors xstart, xend, ystart, yend,
   cargostart, cargoend, amtstart, amtend, autonav) due to multiple
   issues:
   - The orders are executed at the update.  Crafty players can use
     them to get around the update window.
   - Usability is poor, especially for autonav.
   - Few players use them.
   - Documentation is inaccurate.
   - Code has bugs, some of them critical.
   - It's almost 1300 lines of rather crufty code nobody wants to
     touch.
   - Code sharing complicates maintenance of the navigate command.
 * Configuration tables reader and empdump improvements:
   - Better error messages.
   - Export now omits redundant data by default.  Use empdump -c to
     include it.
   - Import can cope with omitted sectors and realms.  Omitted sectors
     become sea, and omitted realms become empty.
   - Column order is now unrestricted.  Before, some tables required
     an ID field to come first.
   - Support splitting any table.  Before, tables sect, news, lost,
     realm, game, infrastructure could not be split.
   - Support strings longer than 65535 characters.
 * Fix empdump -i to reject strings that are one character too long.
   The bug affected ship table columns path, name and rpath, land
   table column rpath, nat table columns cname, passwd, ip, hostname
   and userid.
 * Reject nat selectors relations, contacts and rejects, because they
   don't actually make sense.  They exist just for xdump.
 * Drop the code to resolve player IP address.  It's been disabled
   since 4.2.13.  Deprecate nat selector hostname.  Its value has
   always been "" unless the deity messed with it.
 * Fix bridge spans next to a bridge tower or head taking damage to
   fall when they should with EASY_BRIDGES off.  Has always been
   broken.
 * Fix bridge spans next to a collapsing bridge tower to fall when
   they should with EASY_BRIDGES on.  Broken in 4.3.12.
 * When you scrap a plane, you get crew back only proportionally to
   plane efficiency.
 * Changes to build:
   - Fail the attempt to build a bridge not next to land or bridge
     tower more nicely.
   - Deities can now build anywhere, without materials, work or money.
   - Building a plane no longer uses at least one military.  Military
     for crew are now rounded just like the other materials.  The
     special case dates back to 4.2.3.
   - Report missing materials more nicely.
   - You now need the required materials rounded up to be present.
     Actual use is still randomly rounded.  Before, crafty players
     could exploit the rounding to save materials, or build
     sufficiently cheap things without materials.  In the stock game,
     linf and many plane types could be built without materials.
 * Remove option TREATIES.  Usability is very poor, virtually nobody
   uses them, conditions are incomplete, the code is buggy, and a
   burden to maintain.
 * Test suite improvements:
   - Coverage extended to commands build, navigate, march, fire,
     torpedo, retreat, lretreat, falling bridges, automatic retreat,
     command info page completeness, and the empdump utility program.
   - Maintainability of tests much improved.
   - Test harness refactoring.
   Much work remains.
 * Adjust stack sizes.  Stack overflow was possibly for tiny worlds.
 * Fix start, stop to not operate on dead units when used by a deity.
   Screwed up when start, stop were extended to units in 4.3.6.
 * Nukes can be grouped into stockpiles, just like ships can be
   grouped into fleets.  The new command stockpile does for nukes what
   fleet does for ships.  Likewise the new nuke selector stockpile.
 * wingadd no longer costs BTUs, for consistency with fleetadd and
   army.
 * Changes to edit:
   - Keep missions centered on ship, plane or land unit centered when
     teleporting it.  Screwed up when Chainsaw added missions.
   - Preserve "does not follow" when copying ships.  Preserve "no
     distribution center" when copying sectors, and don't mess up
     coastal flag.  Screwed when Chainsaw added the means to copy a
     ship or sector.
   - You can now edit nukes.
   - Suppress bulletin on no-op ship, plane, land unit location
     change.  Screwed up in 4.3.32.
   - You can now edit ship, plane and land unit types.
   - Changing the owner of a ship, plane or land unit away from POGO
     now works.
 * Fix "make uninstall" to remove HTML info pages (which are not
   installed by default) and directory share/empire/.
 * Fix "make install" to remove stale formatted info pages.
 * Code refactoring and cleanup, portability improvements.
 * Info file fixes and improvements.

Changes to Empire 4.3.32 - Sun Aug 25 07:55:34 UTC 2013
 * Fix an LWP stack overflow observed on AIX 7.1.
 * Fix portability bug in standalone client Makefile
 * Fix portability bug in configure test for curses.
 * The break command now honors new designation of sanctuaries.
 * Changes to edit, give, setsector, setresource, swapsector:
   - Generalize edit to multiple objects: accept
     <SECTS|SHIPS|PLANES|LANDS|NATS> argument instead of just
     <SECT|SHIP|PLANE|LAND|NAT>.  Obsoletes setsector and setresource.
   - Make interactive "edit c" detect and handle concurrent updates,
     like other edits do.
   - Don't reprint edited object when done with it.
   - Always print a message describing the actual change.  Necessary
     to give the deity a chance to catch unexpected changes, e.g. a
     player moving away stuff right before a give command, leaving
     fewer items than the deity intends to take.  Existing messages
     improved, missing ones added.
   - Consistently send bulletins to the victim / beneficiary of deity
     meddling, except when changing stuff mortals can't see.
   - Report much more deity meddling in the news (subject to option
     GODNEWS).
   - More robust argument checking.  Before, careless deity edits
     could cause internal errors.  Much bounds checking was missing,
     inconsistent or wrong.
   - Teleport planes and land units to carrier on load.
   - Refuse to move planes and land units on carriers.
   - Fix stack smash in edit l key 'L' that can mess up sector file.
   - Fix setsector to again print actual landmine change for occupied
     sectors.  Broken in 4.3.31.
   - Suppress bulletins for acts of god to himself.
   - Suppress news for acts of god to gods.
   - Suppress news and bulletins on no-op acts of god.
   - Always send bulletins from POGO.  Name the meddling deity in the
     bulletin text instead.  Inconsistent since Chainsaw.
   - Make interactive edit accept empty key argument again.
   - Make interactive edit treat blank input like empty input.
   - Don't let edit put a land unit or plane on two carriers
   - Do not disclose when a deity destroys a ship, tell the ex-owner
     only that he lost it.
   - edit p and u now show the edited unit like edit s.
   - edit s now shows the edited ship even when its owner is invalid.
 * Clean up output of satellite, land, sorder and mission for world
   sizes between 200 and 1998.  cutoff and level still have issues
   there.
 * Plug harmless file descriptor leak on OpenBSD with IPv6.  Broken in
   4.3.31.
 * New doc/contributing explains how to contribute to Empire.
 * Code refactoring and cleanup.
 * Info page, manual page and documentation fixes and clarifications.

Changes to Empire 4.3.31 - Sun May 12 16:58:34 UTC 2013
 * Fix read beyond end of conditional argument on missing operand.
 * Don't put broken links into info pages formatted as HTML.
 * Permit ships that can drop depth charges, but not fire.  No such
   ships exist in the stock game.
 * Consistently require guns for dropping depth charges.  Before, only
   the fire command required guns (since 4.3.12 and before 4.0.6).
 * Change econfig key rollover_avail_max from 0 to 50.
 * Make sector production more predictable.  Commands production and
   budget are now a bit more accurate.
 * Fix bugs that messed up xdump lost and xdump trade on big endian
   hosts (broken in 4.3.8), and made the server fail during startup on
   most big endian hosts (broken in 4.3.12).
 * Build process robustness fixes.
 * Use "Level: Obsolete" to mark obsolete info pages, and drop "info
   Obsolete".
 * Server now uses IPv4 format for IPv4-mapped addresses.  Simplifies
   safe use of econfig key privip.
 * Server logs a few more network connection details.
 * Fixes and workarounds for econfig key listen_addr "" (which is the
   default) when the system has both IPv4 and IPv6 addresses
   configured:
   - Server accepted only IPv6 connections on Windows and BSDs.  Fix
     to accept IPv4 ones, too.
   - Except for OpenBSD, where the fix doesn't work.  Accept IPv4
     connections instead of IPv6 ones there, and document how to get
     IPv6 accepted instead.
   - Linux systems are frequently configured in a way that makes the
     server accept only IPv4 connections.  Document workaround: use
     listen_addr "::".
 * Change fairland island size probability distribution: roll two dice
   instead of one.  This makes extreme sizes much less likely.
 * Make fairland record the island number in the deity territory.
 * Fix extension of market bidding time when high bidder changes.
   Introduced broken in 4.0.7, with insufficient fixes in 4.0.9 and
   4.2.0.
 * Fix market not to mess up ridiculously high unit prices.  Has been
   broken since 4.0.0 introduced the market.
 * Fix melting of big piles of stuff by ridiculously heavy fallout.
   Has been broken since fallout exists.
 * Fix crash on edit s, p, u key 'U' with negative argument.
 * When a deity builds ships or land units in foreign sectors, they're
   given to the sector owner.  This lets POGO build them, and it's how
   build works for planes and nukes.
 * Fix PRNG (pseudo-random number generator) seeding to resist
   guessing: seed with a kernel random number.  If we can't get one,
   fall back to a hash of high-resolution time and process ID.  This
   should happen only on old systems or Windows.  Far worse than a
   kernel random number, but far better than just low-resolution time.
   Fairland's seeding used to be low-resolution time and process ID,
   but regressed in 4.2.20 to just time.
 * Switch PRNG from BSD random() to Mersenne Twister.  This is a is a
   proven, high-quality PRNG.
 * Fix tiny error in distribution of some die rolls.
 * Increase land unit retreat chance and nuclear damage's chance to
   destroy a nuke by one percentage point, for simplicity.
 * Merge news items a bit more aggressively.
 * Fix a year 2038 bug in news item merging.
 * Retire the nightly build.
 * New make target check.  Just a few tests for now, derived from the
   smoke test that used to be part of the nightly build.
 * Really fix accepting connections from "long" IPv6 address.  The fix
   in 4.3.23 could mess up player's host address in player output and
   logs, and cause extra "Last connection" messages.
 * Take ship cost into account when picking missile interdiction
   target.  Broken in 4.3.8.
 * Really fix give, setsector and setresource not to wipe out
   concurrent updates.  The fix in 4.3.27 was insufficient.
 * Fix setsector not to disclose number of landmines to occupier.
 * Make capital fail more nicely when the sector is unsuitable.  Its
   argument must now be a single sector.  Before, it silently picked
   the first suitable one when the argument selected several.  Has
   always been that way, and never documented.
 * Changes to option GODNEWS:
   - Deity command give could downgrade the deity's relations to the
     sector owner when GODNEWS was enabled.  No more.
   - Likewise, deity commands edit and setsector can no longer
     downgrade the victim's relations to the deity.
   - News enabled by GODNEWS no longer affect news headlines.
 * Fix bomb, drop, fly, paradrop, recon and sweep to again accept a
   destination sector equal to the assembly point.  Broken in 4.3.27.
 * Fix portability bug in configure test for Windows API.
 * More user friendly server startup: journal open errors are detected
   while the server still runs in the foreground, which makes it
   possible to complain to stderr and to exit unsuccessfully.
 * Market changes:
   - Forbid selling conquered populace.  Only relevant when the deity
     allows selling civilians, which is probably a bad idea.
   - Permit selling military by default.
   - Forbid selling ships and land units carrying unsalable cargo.
 * Scrapping ships and land units now spreads the plague
 * Don't let scrap give away civilians, kill them off instead.
 * Pilots and air cargo now spread the plague.
 * Don't let fly and drop give away civilians.
 * Fix fly to permit flying civs to a carrier in an occupied sector.
   Broken in 4.2.17.
 * Don't let planes fly to a carrier without sufficient space.
   Before, planes that didn't fit were teleported home, which could be
   abused.  Broken in 4.3.17.  Almost the same bug was previously
   fixed in 4.2.17.
 * Fix tend to foreign ships:
   - Refuse to tend civilians to foreign ships.
   - Don't leak friendly foreign ships' commodity cargo.
   - Continue with remaining target ships after skipping a foreign one
     when tending a negative amount of commodities.
   All broken when Chainsaw 2 added tending to allies.
 * march sub-command 'm' can now sweep own landmines.
 * Land units no longer sweep allied landmines, except when executing
   march sub-command 'm'.  They don't hit them since 4.3.27.  Sweeping
   was forgotten then.
 * Fix bmap commands not to parse empty flags argument as "revert".
   Broken in 4.3.27.
 * Code refactoring and cleanup.
 * Info page fixes and clarifications.

Changes to Empire 4.3.30 - Tue May 22 18:34:52 UTC 2012
 * Disable damage to base sector or ship when missile explodes on
   launch for now, because damage done to other missiles there can be
   wiped out in some cases.  Broken in Empire 2.
 * Fix navigate and march not to wipe out concurrent updates.
   Abusable, but probably not worthwhile.
 * Fix some buffer overruns in fairland.  Has always been broken.
 * Fix arm to require nuke and plane to be in the same sector.  A
   remote nuke got teleported to its plane when the plane moved.
   Broken in 4.3.3.
 * Change login command kill (used by client option -k) to kill less
   ruthlessly: send a flash message and try to flush output, exactly
   like a server shutdown does.
 * Fix server shutdown to wait for player threads to reach a safe
   state.  Before, player threads raced with shutdown, and failed to
   update the treasury, record play time, and write log entries when
   they lost.  Bug goes back to Empire 2.  Patched partially or
   unsuccessfully in 4.2.10, 4.2.12, 4.2.20, 4.3.6, 4.3.10 and 4.3.23.
   The race was hard to lose in practice, until an unrelated
   "simplification" in 4.3.23 could get player threads stuck sending
   output after shutdown aborted a command.
 * Idle timeout changes:
   - The grace period for clients to complete login and logout is now
     separate from the idle timeout.  Configurable with new econfig
     key login_grace_time, default is 120 seconds.
   - The idle timeout for visitors is now separate from the one for
     players.  Configurable with new econfig key max_idle_visitor,
     default is five minutes.
   - Fix the idle timeout when the connection gets stuck sending
     output.  Broken in 4.3.20.
   - Idle timeout during execute now terminates the connection, not
     just the execute command.
 * Fix stack smash in build.  Remote hole!  Broken when Empire 2 added
   the confirmation prompt.
 * Fix stack smash in scrap and scuttle.  Remote hole!  Broken when
   Chainsaw added the confirmation prompt.  Additionally, the
   confirmation prompt is misleading when the player supplies
   conditionals.  Redesign the flawed prompt.
 * Fix execute's echo of the command.  Broken in 4.3.28.
 * The mobility cost of retreating a defending land unit was
   calculated for an uninitialized sector.  This could charge
   incorrect mobility, prevent retreat, or, if the stars align just
   right, crash the server.  Broken in 4.3.6.
 * Commands cutoff, headlines, land, lstat, motd, neweff, payoff,
   qorder, sorder, and wall now behave like the other information
   query commands when the player's connection can't take more output:
   they pause to let other code run.
 * Revised command permissions:
   - arm and disarm no longer require money, for consistency with the
     other commands to move stuff around.
   - satellite no longer requires money, for consistency with lookout,
     radar, sonar and skywatch.
   - qorder, sorder, survey and test no longer require a capital, for
     consistency with the other commands that report on stuff the
     player owns.
   - start and stop no longer require a capital, for consistency with
     the other commands to control production.
   - fortify now requires a capital, for consistency with mission,
     morale and range.
   - Anybody may now use country, echo and financial.
   - Visitors can no longer use census, commodity, map, nmap,
     sinfrastructure (useless without sectors), read (visitors don't
     get any telegrams), and change (always failed).
   - Players can no longer use map and nmap before break.  This is
     consistent with all the other commands to examine the
     environment.  It also prevents people from trying multiple
     unbroken countries in a blitz to find the one with the nicest
     vicinity.
   - Players can now use resource before break, just like census.
 * read could swallow the first telegram header when showing the
   telegrams that arrived while waiting for the player to confirm
   deletion.  Broken in 4.3.29.
 * Fix 'm' in path argument of explore, move, transport:
   - Don't moan about deprecated argument syntax ('m' without a space
     before its argument) even when there's no argument.
   - Recognize the flags argument again.
   Broken in 4.3.27.
 * Market fixes:
   - Command market is no longer available to visitors, because it
     triggers execution of trades that have become ready.
   - Fix a race condition that could cause commodity trades to be
     executed multiple times.  Abusing this to duplicate commodities
     seems tricky, but possible.  Broken in Empire 3.
   - Don't expropriate sellers of units.  POGO got the money, the
     telegrams and made the news, the seller got nothing.  Broken in
     4.3.17.
 * Login command quit and server shutdown could lose player output.
   4.3.23 tried to fix the shutdown case, but the fix was incomplete.
 * Fix login command quit to really quit.  It sometimes just swallowed
   buffered input, at least on some systems.
 * 4.2.22's fairer command scheduling failed to cover the execute
   command.  Fix that.
 * When a client shuts down input transmission to log out, output
   could get lost.  Our client never does that.
 * Don't skip post command treasury update and status notification on
   EOF from player, after quit command, and when command is aborted by
   server shutdown.  An unluckily timed EOF or shutdown can deprive a
   player of money gained from scuttling tradeships or sacking a
   capital.  Can be abused to build bridges and infrastructure free of
   charge.  Can also be abused to designate sectors for free, but the
   stock game's sector types are all free anyway.  Has always been
   broken.
 * Fix client not to send an empty line on player interrupt (normally
   ^C) before the first command.  This was missed in 4.3.26.
 * Code and info page cleanup.
 * Documentation fixes.

Changes to Empire 4.3.29 - Fri Jan 20 19:20:20 UTC 2012
 * drop and fly from carrier could fail to load last civilian or
   military.
 * Telegram and announcement changes:
   - wire no longer loops to show announcements that arrived while
     waiting for the player to confirm deletion.
   - At most five seconds worth of messages are now squashed together
     into one.  Before, only the time between adjacent telegrams was
     limited, not the total time.
   - When telegrams arrived while read was waiting for the player to
     confirm deletion, and we then showed them, we nevertheless
     claimed "you have new telegrams" before the next command prompt.
     Only with toggle inform off.  Known bug since Empire 2.
   - Fix read not to clobber asynchronous notification of telegrams
     arriving while waiting for the player to confirm deletion.
   - Fix wire not to reset number of pending telegrams with toggle
     inform on.  Broken when Empire 2 introduced toggle inform.
   - The number of pending announcements was off when announcements
     were squashed together.
   - More careful error handling and logging.
 * Ensure all of an update's output goes into a single production
   report:
   - Avoid splitting it up when the update is slow.  Empire 2 already
     did that for the number of pending telegrams, but not for read.
   - Don't turn parts of it into BULLETINs.  Autonav and sail could do
     that since Empire 2.
 * files now creates POGO with user interface flags beep, coastwatch,
   sonar and techlists set, for consistency with add.
 * Change newcap back to not wiping the country (it wiped since
   4.3.12).  Additionally, leave levels and telegrams alone.
 * Changes to deity command add:
   - Require confirmation for unadvisable actions.
   - Refuse to touch a country while it's being played.
   - Don't crash on negative country number.
   - Create deities with money, just like files creates POGO.
   - Always reset the country completely, not just when adding a
     player or a visitor.
   - Rename argument "new" to "player".  Keep recognizing "new" for
     now, but deprecate it.
   - Drop argument "active".  If you really want to create a player
     country without a capital, add the player country normally, then
     activate it with edit instead of newcap.
   - Drop the obscure sector check and wipe option.  If you really
     need to wipe out a country, there's much more to wipe than just
     sectors.
   - Drop the undocumented land unit destruction feature.
 * Permit no-op country name change again.  Accidentally outlawed in
   4.3.20.
 * Plug a few minor memory and file descriptor leaks.
 * Fix use-after-free when plane gets shot down or aborted in
   dogfight.  Broken in 4.3.27.
 * Code cleanup.
 * Info page and manual page fixes.

Changes to Empire 4.3.28 - Sat Jul 16 11:30:53 UTC 2011
 * Don't let POGO (#0) navigate dead ships, and march dead land units.
   The ghosts even got sighted and interdicted, and could hit mines.
   Has always been that way.
 * llook can't see land units and planes loaded on land units anymore.
 * Fix board to ignore land units loaded on land units when testing
   whether a sector can board.  Broken in 4.0.17.
 * Fix transport to reject planes loaded on land units.  The plane
   remained on its carrier.  When the land unit moved, the plane was
   teleported right back to it.  Broken since Chainsaw 3 added land
   units.
 * Fix lradar not to let land units loaded on land units use radar.
   Broken since 4.0.0 added trains.
 * Fix strength to ignore land units loaded on land units.  Broken in
   4.2.0.
 * Fix navigate and march to find paths longer than a few sectors
   again (7 for 64 bit hosts, 3 for 32 bit hosts).  Broken in 4.3.27.
 * march sub-command 'i' now shows number of military and land units
   loaded.
 * New server option -F to force start even when state looks bad.
   Risks crashes and further corruption, but gives deities a chance to
   fix up a bad game state with edit commands and such.
 * empdump -x no longer refuses to export game state that looks bad.
   Gives deities another tool to fix up a bad game state.
 * Land units loaded on land units fight che again, as they used to
   before 4.3.26.
 * When a land unit dies fighting che, land units loaded on it get
   unloaded, and planes loaded on it die.  Before, the update left
   them stuck on their dead carrier.  Impact like the next item.
   Abusable.  Broken since Chainsaw 3 added land units.
 * The update no longer destroys ships, planes and land units for lack
   of maintenance.  Before, it left any embarked planes, land units
   and nukes on their dead carrier.  In this state, units behaved as
   if their carrier was still alive, with additional protection from
   the fact that a dead carrier can't be damaged or boarded.  If
   another unit with the same number got built, it picked up the stuck
   cargo.  The cargo got teleported to its new carrier when the
   carrier moved.  Abusable, but it involves going broke, so it's
   rarely practical.  Slightly more practical before 4.3.6 removed
   budget priorities.  Broken for ships and land units when Empire 2
   added their maintenance cost, and for planes when 4.3.3 replaced
   nuclear stockpiles by nuke units.
 * Fix bogus internal error triggered by navigate and march
   sub-commands 'r', 'l' and 's'.  Broken in 4.3.27.
 * Fix client not to reject redirections and execute containing
   non-ASCII characters with a bogus scary warning when using UTF-8.
 * Fix execute not to mangle non-ASCII characters in the argument when
   prompting for it while login option utf-8 is on.
 * Fix handling of non-ASCII and control characters in batch files.
 * Conversion from UTF-8 to ASCII ate the character following a
   replaced non-ASCII character.  Buffer overrun possible if the
   terminating zero gets eaten.  Could happen in players, read, flash,
   wall, and execute.
 * Fix handling of empty commands:
   - Time used was not updated.
   - Mortal player wasn't logged off for game hours, game down, and
     time limit.
   - Notifications were delayed: going broke, becoming solvent, new
     telegrams (toggle inform off only), new announcements, capital
     lost.
 * Minor tweaks to nightly build.
 * Server's and empdump's sanity checking of configuration and game
   state is now more rigorous.
 * Deity xdump no longer dumps unused countries' realms.
 * Remove option LANDSPIES.  Deities can customize the land table to
   disable spy units.
 * Remove option TRADESHIPS.  Deities can customize the ship table to
   enable trade ships.
 * Configuration table changes (builtin and custom):
   - Rows must be in ascending UID order.
   - Omitting rows in tables item, sect-chr and infrastructure is no
     longer permitted.
   - Custom tables now replace the builtin table completely.  Before,
     omitted rows defaulted to the builtin version, except at the end
     of a table.  Commenting out unwanted stuff just works now.
   - Permit custom table product having fewer than 14 entries.
   - Reject custom tables where customization has no effect (updates,
     table, meta, all symbol tables) or where it's unsafe (news-chr).
   - Input is checked more rigorously.
 * empdump -i fixes:
   - Don't touch plane file when import fails.
   - Refuse import of incorrectly sized table instead of silently
     creating one the server will reject.
   - Replace old state completely.  Before, omitted rows in the dump
     defaulted to the old state, except at the end of a table.
   - Input is checked more rigorously.
 * Fix xdump updates not to dump bogus extra updates.
 * Fix use-after-free when a plane got shot down or aborted by flak.
   Broken in 4.3.27.
 * Friendlier diagnostics in the build command.
 * Fix build to set nuke's tech exactly like for ships, planes and
   land units.  It's not currently used for anything.
 * Research required for nukes was slightly off sometimes due to
   incorrect rounding.
 * Bridge building required 0.005 tech less than advertised, fix.
 * "show nuke" now obeys toggle techlists.
 * Fix "show land s" to show columns xpl and lnd again.  Broken in
   4.3.15.
 * Code refactoring and cleanup.
 * Documentation on custom tables and xdump updated.
 * Info file fixes.

Changes to Empire 4.3.27 - Sun Apr 17 11:36:29 UTC 2011
 * License upgrade to GPL version 3 or later.
 * Fix buy not to wipe out concurrent updates.  Can be abused to
   duplicate commodities.
 * Don't let fighters, SAMs and ABMs intercept while on trading block.
 * Don't let missiles interdict ships or land units while on trading
   block.
 * Fix client to log long input lines untruncated.
 * Fix client crash for long input lines.
 * info subject pages now mark unusually long pages with a !.
 * The edit command keys deprecated in 4.3.15, 4.3.17 and 4.3.20 are
   now gone.
 * Fix give, setsector and setres not to wipe out concurrent updates.
 * Fix explore, move, test, transport not to ignore spaces in path
   arguments.  Broken in 4.3.7.  Deprecate use of 'm' without space
   before its argument.
 * Improvements to map drawing commands:
   - Don't fail silently when asked to draw a map around an invalid
     unit, explain the problem.
   - Deprecate bmap flag "r".  Use (a prefix of) "revert" instead.
   - bmap flags following "t" are no longer ignored.  However, info
     pages previously documented an argument "true" in addition to
     flag "t".  To keep that working, keep ignoring flags following
     "t" when they're bad.  But deprecate that usage.
   - Reverting the bmap no longer draws a map.
   - Bad conditionals could make the commands misinterpret their first
     argument.
   - The commands now fail when they reject their second argument.
 * The xdump syntax deprecated in 4.3.12 is now gone.
 * Fix satellite to fail on bad conditionals instead of ignoring them.
 * Journal improvements:
   - Optional logging of output.
   - Long lines are no longer truncated.
   - Login phase input is now logged, too.
   - Commands are now logged.  Allows making sense of input without
     context.
 * Option BRIDGETOWERS is now disabled by default.
 * Bridge towers now take 300 hcms to build, down from 400.
 * Fix stop orders to expire even when the country is broke.  Broken
   in 4.3.8.
 * Fix march and navigate not to advise on "legal directions" when
   path finding fails.
 * New path finder:
   - Much faster, especially for distribution, i.e. where it matters
     most.  Speedups in excess of 30x have been observed for updates
     taken from real games.
   - Old land path finder leaked memory occasionally.
   - Old land path finder overran buffers for very long paths.  A
     malicious player can arrange sufficiently long paths, but it
     takes a lot of land.  Distribution, the distribute command, land
     unit reaction, and automatic supply now handle arbitrary long
     paths.  Commands explore, move, test, transport, march, navigate
     and path now handle paths up to 1023 characters.  Command
     bestpath shows paths longer than 1023 characters with the tail
     replaced by "...".
   - Old sea and air path finder treated destinations as unreachable
     when the best path was longer than 99 characters.  Command sorder
     now handles arbitrary long paths.  Commands bomb, drop, fly,
     paradrop, recon, sweep and sail now handle paths up to 1023
     characters.  Autonav considers up to 1023 characters at the
     update.  Planes can fly missions up to 1023 sectors from their
     base.
 * Automatic supply charged mobility for the path back to the source
   instead forward to the sink.
 * Make bestpath work for deities in foreign land.
 * More robust savecore example script.
 * Fix buffer overruns in the lookout, spy, map and nmap commands when
   WORLD_X * WORLD_Y is not a multiple of 16.
 * The path command's maps weren't always fitted to the path
   correctly.  Broken in 4.3.17.
 * Land units no longer hit allied mines.
 * Change sharebmap with yourself to do nothing instead of telling you
   you're not friendly to yourself.
 * You can now flash yourself.  Enjoy!
 * A race condition could make the flash command claim an ally was not
   logged on when he was.  Hard to trigger outside the lab.
 * The flash command no longer prints "Flash sent" for deities.  It's
   annoying.  It never did that for mortals.
 * Plug memory leaks in mission code on a number of conditions:
   - Submarines moving where own planes would interdict hostile
     submarines
   - Ground combat where countries hostile to one of the parties would
     support if relations were right (allied to one party, at war with
     the other)
   - Ships or land units moving within hostile missile mission op
     areas
   - Any player action within marine missile mission op areas that
     could trigger the missile owners' missions.
 * Attack fixes:
   - When you ordered a sector to attack with military, but lost the
     sector before the attack was executed, and the new owner was
     allied to you, the server got confused: it let the military
     attack even though they were already dead, but not occupy.
   - When you ordered a land unit to attack from an allied sector, and
     the sector got lost before the attack was executed, the server
     reported things as if you had owned the sector yourself.  The
     land unit still attacked when the new owner was also allied to
     you.  No more.
   - When a land unit attacked out of an allied sector, its mobility
     cost was computed as if the ally owned the land unit.  Attacking
     sectors old-owned by that ally was too cheap, and taking back
     one's own was too expensive.
 * Fix trade ships to pay off in own harbor.  Broken in 4.3.17.
 * Remove option SLOW_WAR.  It hasn't been enabled in a public game
   for years, and the code implementing it was buggy.
 * Fix a bug in sail that limited the sail path to 27 instead of the
   intended 28 characters.
 * Fix race condition in transmission of flash and telegram
   information messages that could double output and possibly crash
   the server.
 * Planes get to sweep and sonar only after flak and interception.
 * Don't limit the radar command's range to fit its output into a
   world map without clipping.  The limiting goes back to Chainsaw 2.
 * Make flak vs. pinpoint bombing consistent with ordinary flak:
   - 4.3.6 reduced the plane abort chance on damage, but missed the
     additional flak vs. pinpoint bombing.  Fix that.
   - Fix to charge mobility appropriate for damage.
 * Prettier flak damage reporting.
 * Fix bugs that could cause bmap updates from recon and satellite to
   be lost.
 * Fix lookout to properly report sector ownership to deities.
 * Suppress bogus messages deities with non-zero country number got
   when attempting to navigate foreign ships or march foreign land
   units.  Such deity use hasn't worked since Empire 2.
 * Deities can no longer fly foreign planes.  It didn't fully work
   since Chainsaw 3, and can trigger internal errors since 4.3.11.
 * Don't split lines over several bulletins.  The read command
   normally merges these bulletins, but if they are more than five
   seconds apart (clock jumped somehow), we get a bulletin header in
   the middle of a line.  Broken since Empire 2.
 * Don't beep when plane, land unit or nuke die on a collapsing
   bridge.  Not nice, because it could beep many times, and could put
   beeps in bulletins.  Beeping was added in 4.0.18.
 * Minor improvements to nuclear damage reporting.
 * When a deity dropped a foreign nuke, parts of the output went to
   the owner of the nuke instead of the deity.
 * When a player triggered a foreign missile launch (mission or
   interception), and the missile exploded on launch, part of the
   report went to the player instead of the missile owner.  This
   disclosed the missile owner's origin.  Broken in Empire 2.
 * When autonav reported to a ship owner that it can't load or unload
   foreign civilians, it used the sector owner's coordinate system.
   This disclosed the sector owner's origin.  Abusable.
 * When nuclear damage bounced off a sanctuary, the bulletin to the
   sanctuary owner used the attacker's coordinate system.  This
   disclosed the attacker's origin.
 * When a deity used sweep, lmine, sail or sabotage with a foreign
   plane, land unit or ship, its location was reported in the foreign
   coordinate system.
 * When a deity asked for a foreign nation report, the capital
   location was reported in the foreign coordinate system.
 * Code refactoring and cleanup.
 * Info file improvements.

Changes to Empire 4.3.26 - Mon May 24 16:37:40 UTC 2010
 * Land units loaded on ships or land units can no longer fight che.
 * When foreign land units were present, a che revolt could damage
   them instead of the defending land units.  This could let defenders
   survive the revolt.  They were captured or blown up if che took
   over the sector.
 * Fix che revolt to damage only land units that actually fight
 * Fix loading x-light missiles on ships without capability plane.
   These ships could only use their x-light slots for x-light planes,
   not their plane slots.  Broken in 4.3.17.
 * Update bmap when nuclear detonation makes wasteland.
 * Fix fly and drop to report discarded cargo items correctly.
 * New client option -s to specify server host and port.
 * Hitting landmines could crash the server.  When not, the damage
   reduction for capability engineer was applied unpredictably.
   Broken in 4.3.24.
 * Fix retreat and lretreat condition 'c'.  Broken in 4.3.16.
 * coastwatch and skywatch could see too far.  Up to one sector for
   practical radar ranges.
 * skywatch now reports satellite UIDs.  The UID is required for
   launching a-sats since 4.3.23.
 * Fix client not to send an empty line of input before aborting the
   command on player interrupt (normally ^C).
 * Fix sabotage not to kill the spy every time.  Broken in 4.3.17.
 * Code cleanup.
 * Info page fixes, manual page improvements.

Changes to Empire 4.3.25 - Sun Mar 21 08:27:37 UTC 2010
 * Hitting sea mines could crash the server.  When not, the damage
   reduction for capability sweep was applied unpredictably.  Broken
   in 4.3.24.
 * When an interdiction did damage other than collateral damage, the
   planes that intercepted it from interdicted carriers used no
   petrol.  4.3.24 fixed only the "no damage" case.
 * Ship anti-missile defense failed to charge shells when the ship was
   interdicted.  The fix in 4.3.24 covered only launch.
 * Fix a bug in navigate and march that let players lay mines free of
   charge.  Also fix them not to prompt for the number of mines.
 * Missile exploding on launch could not damage its base if it was
   armed with a nuke.  Broken in 4.3.23.
 * When a torpedo attack triggered a return torpedo, the news reported
   it to be fired by the attacker instead of the defender.
 * Interdiction attacked submarines with surface-only weapons.  Broken
   in 4.3.24.
 * Fix a bug in lradar, path, radar, route, satellite, sect and survey
   that could leak maps to other players.  Broken in 4.2.0.  Fixed in
   4.2.8 for bmap, lbmap, lmap, map, nmap, pbmap, pmap, sbmap and smap
   only.
 * files could clobber the game when reading confirmation fails.
 * fairland no longer rejects small worlds without trying.

Changes to Empire 4.3.24 - Tue Jan 26 21:29:14 UTC 2010
 * The client sometimes hung under Windows when the server closed the
   connection.  Broken in 4.3.23.
 * Fix navigate and march not to crash the server when a path argument
   consists of a valid path plus whitespace.  Broken in 4.3.7.
 * When a client sent a blank line in the login phase, the server
   crashed on some systems.
 * Fix crash bug in bomb, drop, fly, paradrop, recon and sail.  Broken
   in 4.3.16.
 * Fix bogus internal error on escort, recon, and satellite launch.
   Mostly harmless, as error recovery worked fine.  Broken in 4.3.23.
 * Stop ship and land unit movement on interdiction even when it does
   no damage.
 * When an interdiction did no or only collateral damage, the planes
   that intercepted it from interdicted carriers used no petrol, and
   interdicted land units were immune to collateral damage.
 * The limit for the number of sectors, ships, planes, land units,
   nukes, and so forth is now large enough not to matter.  It used to
   be 32768 on common machines.  The sector limit was introduced in
   4.3.12.  World x- and y-size are still limited to 65536 on common
   machines.
 * News use much less space, and thus I/O.  Space was wasted in
   4.3.12.  Side effects of the change:
   - Expiry of old news no longer updates news timestamps.  Updating
     timestamps was wrong, because it defeated incremental xdump news.
   - The empdump utility can no longer update timestamps of imported
     news.  Unfortunate, as it breaks incremental xdump news.
 * New news selector duration, the time span covered by this news item
   in seconds.
 * ABMs failed to charge supplies when their sector was the
   intercepted missile's target.  The stock game's ABMs use no
   supplies.
 * Ship anti-missile defense failed to charge shells when the ship was
   the missile's target.
 * Code refactoring and cleanup.

Changes to Empire 4.3.23 - Sun Dec 13 16:34:49 UTC 2009
 * Fix missile interception not to intercept tactical and marine
   missiles attacking missiles or satellites.  No such missiles exist
   in the stock game.  Interception of tactical ABMs could crash the
   server.  Broken in Empire 2.
 * Missiles missing their target do collateral damage again.  Was
   disabled because of bugs in 4.0.18 for manual launch and in 4.3.16
   for automatic launch.
 * News reported victim as actor for sub-launched anti-sat and ABM.
 * Defense value of missiles vs. ABMs and satellites vs. anti-sats
   failed to improve with tech.
 * Fix a bug that let missiles interdict ships outside their op area.
   This could happen when a group navigating together was partly
   inside the op area.
 * Only bomb strategic and launch at sector can use nukes.  Before,
   they could also be used by missions, bomb pinpoint, and launch at
   ships, but there were several bugs and inconsistencies, and the
   code was messy.  The arm command now rejects marine missiles in
   addition to satellites, ABMs and SAMs, and clears the mission.  The
   mission command now rejects planes armed with nukes.
 * Missiles exploding on launch pad no longer set off their nukes.
 * The launch command now more accurately reports why a missile can't
   be equipped.  It no longer draws supplies automatically.
 * Manually launched anti-sat now always kills when it hits, for
   consistency with automatically launched ones.
 * Don't permit nukes on satellites, ABMs and SAMs.  Nukes on
   satellites could be armed and disarmed even in orbit.  Nukes on
   ABMs and SAMs were lost without effect when their missile
   intercepted.  The stock game is not affected, because its
   satellites, ABMs and SAMs all have zero load.
 * Remove obsolete plane capabilities stealth and half-stealth.  Not
   used by the stock game.
 * Penalize fighter combat value for any load, not just bombs.  The
   stock game's fighters can't carry anything but bombs.
 * Make bomb work for non-tactical cargo bomber.  No such planes exist
   in the stock game.
 * Revised cargo plane rules: a cargo flight can be either an airlift
   or an airdrop now.  Airlifts carry more cargo than airdrops.  A
   cargo drop or paradrop with a non-VTOL plane is an airdrop.
   Anything else is an airlift.  This makes paradrop loads consistent
   with drop loads.  Paradrop with VTOL transports now carries twice
   the punch, and drop with non-VTOL transports hauls less than fly.
   In particular, the stock game's tr can't drop guns anymore.
 * Enforce plane selection rules more tightly:
   - bomb command can select only planes with capability bomber or
     tactical.  Before, other planes with non-zero load flew along,
     but their bombs were silently lost.
   - sweep command can select only planes with capability sweep.
     Before, other planes performed ordinary reconnaissance instead.
   - drop command can select only planes with capability cargo.
     Before, other planes flew along but dropped nothing.
 * Fix paradrop to fail without destroying the paratroopers when the
   player owns the target sector.
 * Launching an anti-sat now takes the target plane as argument.
   Before, it took a sector argument, and targeted the lowest-numbered
   satellite there.  Rather inconvenient when your own satellite masks
   one of the enemy's.
 * Remove option PINPOINTMISSILE.  Deities can customize the plane
   table to disable marine missiles.
 * Ridiculously impotent nukes could do unpredictable interdiction
   damage.  No such nukes exist in the stock game.
 * The production command could mispredict resource-depleting level
   production.  No such products exist in the stock game.  In fact,
   they'd be highly unusual.
 * The update could crash or corrupt the game when a (misconfigured)
   product depleted resource "none".
 * Revamp the Windows port based on ideas stolen from Gnulib.  Share
   the code between server and client.
 * Don't log out player when update aborts a command under Windows.
   Broken in 4.3.20, and not fully fixed in 4.3.21.
 * Fix accepting connections from hosts with "long" IPv6 address.  The
   internal buffer had insufficient space.
 * Delay shutdown up to 3s to let player output buffers drain.
 * Fix a race between main thread and player threads, which could
   theoretically make the server crash on start.
 * Clean up synchronization between commands, update and shutdown, and
   when player threads sleep on I/O.
 * Clean up the cruft that has accumulated in and behind the empio
   interface, and, to a lesser degree, the empthread interface.
 * Fix time difference underflows in pthread and Windows code.  They
   could potentially cause hangs, although none have been observed.
 * Make budget's "Sector building" line look better.
 * Make sector maintenance cost configurable.  New sect-chr selector
   maint.  Capitals now pay maintenance regardless of efficiency.
 * Overhaul show sect b.
 * Fix mine production resource limit for sector peffic != 100.  This
   affects mountains in the stock game, but only with an impractically
   large number of ETUs per update.
 * Code refactoring and cleanup.
 * Info file fixes and improvements.

Changes to Empire 4.3.22 - Sat Apr 25 11:56:29 UTC 2009
 * Fix a Windows client bug that could lead to hangs, at least with
   some versions of the C run-time.  Broken in 4.3.11.
 * Allow building without curses terminfo.  Highlighting in the client
   doesn't work then.
 * Really fix bomb not to wipe out plane updates made while it asked
   for pinpoint targets.  The fix in 4.3.21 didn't cover escorts.
 * Fix tend land not to wipe out concurrent updates.
 * Automatic supply fixes:
   - Ships with just two shells could fire a torpedo to return fire or
     interdict.
   - Ships with just one shell could use their anti-missile defense.
   - Land units were considered in supply even when they had not quite
     enough supplies.  Such land units could defend without penalty,
     attack and react.
   Broken in 4.3.20.
 * Fix update to take dead units off carriers.
 * Standalone client distribution built empire instead of empire.exe
   for Windows.  Broken in 4.3.0.
 * Fix client to abort script execution on SIGINT.
 * Fix Windows client not to hang on EOF on standard input when that
   is a tty.
 * Fix a client bug that could make it hang when EOF on standard input
   follows an execute command closely.
 * Sea mines under bridge spans were disabled a long time ago, in
   Empire 2.  The drop command refuses to mine there.  Change mine
   command to do so as well.
 * Sea mines under bridge spans were mistaken for landmines by ground
   combat, land units retreating from bombs, and non-land unit ground
   movement.
 * When a bridge tower collapsed, its landmines magically became
   sea mines.
 * Don't log out player when he interrupts a command.  Broken in
   4.3.19.
 * Code cleanup and portability improvements.
 * Info file and documentation fixes.

Changes to Empire 4.3.21 - Mon Mar  9 21:34:52 UTC 2009
 * Fix swapsector not to wipe out concurrent changes to the swapped
   sectors.
 * Fix a bug in crash dumping that could mess up the journal and, at
   least in theory, mess up commands info, read, turn, and wire;
   announcement expiry, and reading of econfig and schedule.
 * Fix bomb not to wipe out plane updates made while it asked for
   pinpoint targets.
 * When bombing ships with a force containing both planes with and
   without capability ASW, the bomb command could fail to report
   presence of submarines, and could refuse to bomb ships when there
   were only submarines.
 * Leaving land units behind after a victorious attack could in some
   cases wipe out changes made to them since the victory.
 * The timestamp selectors added to commodity, load and trade in
   4.3.12 could lead to spurious command failures.  None have been
   observed in the wild.
 * Don't log out player when update aborts a command.  Broken in
   4.3.20.
 * Code refactoring and cleanup.

Changes to Empire 4.3.20 - Fri Feb 20 18:14:20 UTC 2009
 * Option LOANS is now disabled by default.
 * Option RAILWAYS is now enabled by default.
 * Many fixes to automatic supply:
   - Planes flying interception or support missions, abms intercepting
     ballistic missiles, and the launch command for missiles and
     anti-sats could all supply more shells than necessary, lose
     shells, or conjure them up.
   - Ships shooting down marine missiles could supply more shells than
     necessary, or lose shells.
   - Supply ships could conjure up shells when firing guns using
     multiple shells, or launching torpedoes, or dropping depth
     charges, or shooting down marine missiles.  No supply ships
     capable of doing that exist in the stock game.
   - Supply engineers could conjure up shells when laying mines.  They
     don't exist in the stock game.
   - load, lload and supply could supply land units with enough food
     from the sector containing them to starve the sector.
   - Sectors were charged too much mobility for moving supplies
     sometimes.
   - Land units serving as supply source no longer draw supplies
     recursively, because the bugs there are too hard to fix to be
     worth it.
   - Defending and reacting units only checked whether they could draw
     supplies, but didn't actually draw them.
   - load, lload, supply, assault and board no longer supply land
     units from ships carrying them.  This feature, added in 4.0.14,
     was inconsistent with other ways to supply.
   - Don't use automatic supply to avoid starvation at the update.  It
     adds complexity to the update.  How much good it does to players
     is highly doubtful; certainly nobody can rely on it.  It isn't
     covered by the starvation command.  Ships or land units could
     steal enough food from their sector to make it starve.
   - lmine could only fetch shells for engineers that use ammo.  The
     stock game's engineers all do.
   The changes listed for load and lload are actually lies, because
   supply has been disabled there since 4.3.17.
 * Don't let an engineer lay mines while it is on a ship or land unit,
   or in a foreign sector.
 * Sectors and ships no longer need shells to fire flak.
 * Use IPv4 and v6 only when suitable interfaces are configured.
 * Fix turn off not to fail when the message is empty.  Broken in
   4.2.20.
 * Don't nag the deity about game hours restriction in force all the
   time.
 * Tell deity when the game is down, just like players.  Also make the
   information available in xdump, as game selector down.
 * Corrupt mailboxes could crash the server.
 * Fix reject accept.  Broken in 4.3.4.
 * If a player was at a prompt when the deity turned the game down, he
   was allowed to enter one more command.
 * Air defense is no longer separate from interception.  Putting a
   plane on air defense now merely changes where it intercepts.
   Separate air defense added too much pointless complexity.  Its
   implementation violated design assumptions of the intercept code.
   Because of that, the same plane could intercept both an air defense
   mission and the planes that triggered it, and the damage it
   received in the first interception was wiped out.
 * Intercept the same all along the flight path.  In each sector, any
   country owning the sector, a surface ship or a land unit there gets
   to intercept.  Before, only the sector owner got to intercept,
   except for the assembly point and the target sector.  In the target
   sector, any country owning surface ships or land units got to
   intercept in addition to the sector owner.  Thus, a sector owner
   with surface ships or land units there got to intercept twice.  The
   sector owner did not get to intercept at the assembly point,
   even when it was the target sector.
 * Spotting rule changes:
   - Ships and land units now spot overflying planes along all of the
     flight path instead of just the target sector, and no longer
     report allied planes.
   - Planes now spot ships and land units only when flying recon or
     sweep, and along all of their flight path instead of just the
     target sector.  It still takes a spy plane to identify ships and
     land units.
   - Planes now spot ships and land units in a sector even when all
     planes abort there.
   - Planes now spot all foreign ships and land units, not just
     hostile ones.
 * Fix SAM interception for intercepts other than the first.
 * Fix air defense and flak over sectors allied to the planes.  Air
   defense was broken in Empire 2.  Flak was broken in 4.2.8.
 * Establishing contact by spotting planes from sectors didn't work.
 * Fix reconnaissance patrols to use sonar when any capable plane is
   present.  Before, all planes had to be capable.
 * Fix recon and sweep not to spy after all spy planes are gone.
 * Fix a bug that made multiple clients running on the same Windows
   machine interfere with each other.  Broken in 4.3.11.
 * Fix enforcing game hours for players already logged in.  Broken in
   4.3.19.
 * Fix distribution not to abandon a distribution center by exporting
   the last civilian, or the last military if there are no civilians.
 * Many fixes to load, unload, lload and lunload:
   - Now usable for deities.
   - Plug loopholes that let you steal foreign commodities by making
     load/lload unload and unload/lunload load through use of negative
     amount arguments.
   - Fix not to let you give away civilians.
   - You can't use lload in foreign sectors or lunload with foreign
     land units anymore.  Loading stuff in foreign sectors was
     prohibited already for ships.
   - lload and lunload now work on foreign land units only when
     they're explicitly named by UID.  This matches behavior of load
     and unload.
   - Fix to test relations of foreign object's owner to player instead
     of the other way round.
 * Make sure land units with maximum mobility can attack mountains.
 * Fix land unit attack mobility cost.  Broken in 4.3.6.
 * Conditions comparing string-valued selectors like wing to unquoted
   values are now interpreted more smartly.  Before, unquoted strings
   were prone to be interpreted as selector names even when that made
   no sense.  You still have to disambiguate truly ambiguous cases
   like w=w for planes, e.g. as wing=w or w='w'.
 * Fix reporting of shot spies in news.  Broken in 4.3.16.
 * Remove econfig key mission_mob_cost.  Questionable feature, and
   hasn't been used in a long time.
 * Reserve and escort missions now have an op-area, like all the other
   missions.
 * Land units now react only when on a reserve mission.  The lrange
   command is gone.  The edit command still accepts and ignores the
   land unit key 'P' for compatibility, but it is deprecated.
 * Fix a bug that could mess up mission op-area when the range of
   ship, plane or land unit on the mission decreases, e.g. through the
   range command.
 * Reserve missions no longer give a bonus to reaction range.
 * Info file fixes.
 * Code refactoring and cleanup.

Changes to Empire 4.3.19 - Sun Dec  7 23:15:34 UTC 2008
 * LWP doesn't work with Darwin due to OS bugs, avoid it for now.
 * Don't let ships double-retreat first on 'i' (injured) and then on
   'h' (helpless) when shelled.
 * Fix a bug that suppressed the "sunk!" message.  Broken in 4.3.17.
 * Log out player when maximum time per day is exceeded.  Before, he
   could remain logged in with visitor privileges only, but once
   logged out could not log back in.
 * If a player was at a prompt when either game hours restriction
   started or maximum time per day was exceeded, he was allowed to
   enter one more command.
 * Deitying a capital failed to charge money and report news.  Broken
   in 4.3.17.
 * Overhaul accounting of play time, fixing minor inaccuracies.
 * Fix parsing of 24:00 in econfig key game_hours and
   update_demandtimes.
 * New option RAILWAYS.
 * Che ages just like military reserves (1% per 24 ETUs).
 * Permit ships to assault the sector they're in.
 * Sectors eating less than one unit of food don't make one unit of
   food for free anymore.  They still can't starve.
 * New concept sector terrain.  Deities can use it to make sectors
   other than wilderness redesignatable.  See sect.config for more
   info.
 * Represent play time in seconds rather than minutes.  This affects
   xdump: nat selector minused (counting minutes) is replaced by
   timeused (counting seconds).
 * Improved nightly build, with additional tests.

Changes to Empire 4.3.18 - Sat Oct 18 18:39:17 UTC 2008
 * Fix building of planes, land units and nukes with UIDs that have
   never been used before.  This could crash the server on some
   systems in certain states.  Broken in 4.3.17.
 * Improved nightly build, with restructured and extended tests.

Changes to Empire 4.3.17 - Sat Sep 20 16:07:44 UTC 2008
 * Fix standalone client build for Windows.  Broken in 4.3.11.
 * Disable automatic supply of land units on load for now, it's
   broken, and can be abused to duplicate supplies.
 * Make trade show exactly what's on sale, remove option SHOWPLANE.
 * Stop ships, planes, land units and nukes on violent takeover, just
   like sectors.
 * Wipe orders when ships, planes, land units and nukes are taken over
   violently or given away peacefully.  Fleet, wing, army and mission
   were already cleared in many, but not all cases.  Other orders were
   never cleared: mission op-areas (visible in xdump), ship autonav
   orders, ship sail path (including ship to follow and mobility
   quota), land unit retreat orders, plane range limit, and land unit
   retreat percentage.
 * Fix takeover of stuff by attack, assault, board, lboard, paradrop
   and pboard:
   - Corrupt land unit file could lead to infinite recursion.
   - Take over nuke armed on plane along with the plane.  Broken in
     4.3.3.
   - Taking over land units with negative mobility increased mobility
     to zero.  Ditto for planes embarked on ships or land units.
 * Fix a bug that made set and xdump not recognize unused trade lots
   as such.
 * Fix giving away stuff by trading it:
   - When an armed nuke is sold separately from its plane, take it off
     the plane.  You couldn't do that before 4.3.3.
   - Give away nuke armed on sold plane along with the plane.  Broken
     in 4.3.3.
   - Don't zero mobility of planes and land units on sold ships.
 * Fix giving away stuff by unloading from its carrier: Give away any
   nukes armed on planes (broken in 4.3.3) and land units loaded on
   land units.  The latter can't happen in the stock game.
 * Fix giving away stuff by scrapping or scuttling its carrier:
   - Fix up output.
   - Disarm the nuke from a plane instead of destroying it.
   - Give away any loaded land units and planes, too.
 * Tradeship fixes:
   - Fix scuttle to ask for confirmation when scuttling a tradeship in
     an unsuitable sector even when the tradeship is pirated.  Broken
     in 4.2.13.
   - Fix scuttle to require 2% harbor efficiency for a trade ship to
     pay.  Also require at least friendly relations.
   - Make autonav refuse scuttle orders in unsuitable sectors.
 * Clean up rules on where you can scrap stuff:
   - Ships in own or friendly, efficient harbors (relations used not
     to matter, contrary to info)
   - Planes in own or allied, efficient airfields (friendly used to
     work as well, contrary to info)
   - Land units in any own or allied sector (relations used not to
     matter, and you couldn't scrap them on ships)
   Also tell player why something can't be scrapped instead of
   ignoring it silently.
 * Rewrite the broken code to move land units, planes and nukes along
   with their carrier.  The old code was a hack that didn't update
   timestamps (so incremental dumps didn't pick up the movement), and
   it didn't cover all uses.  The update, in particular, was prone to
   see cargo in old locations, which could screw up repairs.
 * Expire lost items at the update instead of continuously.
 * Replace econfig key lost_items_timeout by lost_keep_hours.
 * Minor Windows threading fixes.
 * Journal now uses human-readable names to identify threads.
 * Ship and land unit load counters are no longer stored in game
   state, because they duplicate information already there.  Same for
   the type of nuke loaded on a plane.  Remove field nuketype from
   xdump plane.  Clients can simply find the plane's nuke instead.
   The edit command still accepts and ignores the plane key 'n' for
   compatibility, but it is deprecated.
 * Ship and land unit load counts as displayed by ship, sdump, land,
   ldump, ship selectors nplane, nxlight, nchoppers, nland, and land
   selectors nxlight, nland are now reliable.  They used to get out of
   sync at times.
 * Ship load counts now reflect what is loaded, not how it is stored.
   For instance, if a light carrier, capable of carrying 4 x-lights
   and 20 light planes carries nothing but 5 sams, we now have
   nxlight=5, nplane=0 instead of nxlight=4, nplane=1.
 * New cargo lists data structure to let code find loaded planes, land
   units and nukes easily and efficiently.
 * Revamped upkeep of lost file.  It should be reliable now.
 * Disable the ill-advised error recovery on sector access that swept
   bugs under the carpet rather than recovering from them.
 * Occupied sectors no longer revert to the old owner when all
   military and land units go away.  This feature was added in
   Chainsaw 3.  It wasn't integrated properly with the che code
   (things went wrong when che killed all defenders, but couldn't
   convert the sector due to its loyalty), and combat code (if an
   attack killed off all local defenders and got defeated by reacting
   units, the sector changed ownership anyway, silently).
 * Include destination coordinates in interception and plane mission
   messages.
 * Client could misdiagnose redirections and executes as unauthorized,
   or die outright.  Broken in 4.3.11.
 * Fix xdump realm to dump player instead of absolute coordinates.
   Disclosed the true origin.  Broken in 4.3.0.
 * Fix spy to reliably avoid spying same sector more than once.
 * The map drawing code could smash the heap when the world was
   ridiculously small.
 * When a satellite's launch trajectory was off, its coordinates could
   get screwed up, which made it invisible in skywatch.
 * Fix several bugs that made the path command screw up when the path
   spans large areas.
 * Fix the default size of the map shown by move, test and transport
   sub-command M.
 * Fix bugs that could make arguments in area or dist syntax miss
   ships, planes, land units and nukes at the border of the selected
   area, if that area spans the whole world.  In sufficiently small
   worlds, it could also make radar miss satellites and ships, sonar
   miss ships, satellite miss ships and land units, nuclear
   detonations miss ships, planes, land units and nukes, automatic
   supply miss ship and land unit supply sources, ships and land units
   fail to return fire, ships fail to fire support.
 * Maps sometimes showed x-coordinates as three lines instead of two
   unnecessarily.
 * Improved nightly build, tests in particular.
 * Code refactoring and cleanup.

Changes to Empire 4.3.16 - Sat Aug  9 15:56:52 UTC 2008
 * Production command incorrectly limited the number of graduates,
   happy strollers, technological breakthroughs and medical
   discoveries produced by a sector to 999.  Broken in 4.2.15.
 * Fix a bug in improve that could let you spend more money than you
   have.
 * Fix work command not to spend more mobility than requested, and to
   round mobility use randomly instead of down.
 * Commands demobilize, convert and work no longer let you spend more
   money than you have.  Commands demobilize and convert no longer ask
   for confirmation before spending more than half your money.  Saving
   that feature would have been more work than it's worth.
 * Remove undocumented key 'd' at bomb ship, plane and land unit
   target prompts.
 * Fix land unit flak when pin-bombed to match flak when overflown:
   proportional to efficiency, and randomly rounded.  Was missed in
   4.3.6.
 * Don't permit bombing of embarked planes and land units.
 * Fix capital maintenance to charge at 60% efficiency.
 * launch command now requires only 40% plane efficiency, for
   consistency with other ways to launch missiles.
 * Disable collateral damage when automatically launched missiles miss
   their target.  Collateral damage was done even when the launch
   failed or the missile got intercepted.  It's been disabled for
   manually launched missiles since 4.0.18, for the same reason.
 * Fix a bug that corrupted the lostitems file.  Broken in 4.3.12.
 * Show command changes:
   - Move product information from "show sect c" into new "show
     product", and reformat it to be easier to understand.
   - New column navigability in "show sect s".
   - "show sect s" now shows the packing bonus type instead of
     selected packing bonuses.  "show item" shows the actual bonuses
     for each type.
   - "show sect b" now shows the usual values in addition to unusual
     ones.
 * Fix interdiction to obey op-area for missiles.
 * Fix a bug that caused a stale sources.mk to be distributed in the
   4.3.14 an 4.3.15 tarballs.  The latter does not build out of the
   box because of that.
 * Don't strip tab characters from quoted command arguments.  Broken
   in 4.2.21.
 * shutdown commands now always take effect immediately.  Before, they
   could be delayed by up to a minute.
 * shutdown could start multiple shutdown threads when you initiated a
   shutdown shortly after cancelling one.  This screwed up the timing
   of the shutdown.
 * Deity syntax of zdone changed, and is now documented in info.
 * The threshold command could be tricked into accepting negative
   thresholds.
 * Fix designate to always write updated bmap to disk.
 * Fix bomb to fail on empty <pin|strat> argument instead of flying
   without bombs.
 * Fix a bug in distribute that screwed up the current sector's
   distribution center when the command got aborted.
 * Commands arm, bomb, deliver, drop, fire, fly, paradrop, recon,
   lmine, order, power, range, sail, shutdown, sweep and tend did not
   honor command abortion at some prompts.
 * Fix a bug in handling of EOF from client that could bite in the
   following ways:
   - EOF at target prompts for bombing commodities, ships, planes and
     land units sent the server into an infinite loop that ate all
     memory.
   - EOF at sail's path prompt crashed the server.
   - EOF at some prompts in deliver, designate, fly, morale, zdone,
     attack and assault didn't abort the command.
   Broken in 4.3.11.
 * Selector terr is now the same as terr0 for mortal players, and
   dterr for deities.  This matches how the territory command works.
   Unfortunately, this required us to rename xdump sect field terr to
   terr0.  Sorry about that.
 * Fix bomb not to let you bomb dead ships and land units.
 * Make the retreat command a bit more helpful on retreat conditions,
   and make it fail on bad conditions.
 * Fix the mapper scripts.  Broken in 4.2.0.
 * Fix flying commands not to let planes do double duty as escorts.
   Broken in 4.3.12.
 * When a minesweeper hit a mine after sweeping, the mine wasn't used
   up.  If it sank the minesweeper, the code accessed freed memory,
   which could clobber the ship file or worse.  Broken in 4.0.17.
 * Don't let non-light units board ships that can't carry them.
 * Capitals can now employ up to 1000 civilians instead of 999.
 * Spy command changes:
   - Spy now requires sector military.  Before, presence of land units
     was sufficient, but spies shot were only deduced from sector
     military.  If you lost more spies than you had sector military,
     the rest came back from the dead.
   - Spy no longer ignores non-sea sectors without civilians, military
     and land units.
   - Remove columns lnd, pln from spy report.  The values didn't match
     the reported land units and planes, and could leak the presence
     of spy units.
   - Establish contact when spy succeeds even though the spy was
     caught.
 * Fix news reporting and damage when land units interdict ships in
   sectors other than sea.
 * Fix land unit return fire damage to ships to take accuracy into
   account, like the fire command and interdiction does.
 * Remove the inconsistent shelling damage reduction for range.  Fire
   command got damage reduced by 10-20% with a chance depending on
   range ("Wind deflects shells").  Ships interdicting got it reduced
   by half.  Other ways to shell did not get damage reduced for range.
 * Fix ground combat to report defending land units.  Broken in 4.3.4.
 * Info file and documentation fixes and improvements.

Changes to Empire 4.3.15 - Sun May 18 08:59:30 UTC 2008
 * The edit command keys deprecated in 4.3.10 are now gone.
 * fairland now obeys game file locks.
 * Do not leak world creation time in files utility, because that
   facilitates attacks against fairland's PRNG.  Broken in 4.3.12.
 * Fix starvation not to starve one more than it should.  The last man
   on a boat or land unit now can't starve anymore.
 * Fix a crash bug in satellite.  Broken in 4.2.7 and not fixed
   correctly in 4.2.12.
 * Fix a coordinate normalization bug that could theoretically lead to
   buffer overruns and other unpleasantness.  None have been
   reproduced, though.  Broken in 4.3.12.
 * Remove option FUEL.  The abstract idea of tying ships and land
   units to a logistical tether is sound, the concrete implementation
   as option FUEL is flawed.  It adds too much busy-work to the game
   to be enjoyable.  It hasn't been enabled in a public game for
   years.  The code implementing it is ugly, repetitive, and a burden
   to maintain.  The edit command still accepts and ignores the fuel
   keys for compatibility, but they are deprecated.
 * Fix pin-bomb not to report subs when there are none.
 * You now have to take a capital to capture the victim's money, loans
   or market lots.  Merely obliterating the capital doesn't cut it.
   It still makes the victim lose money, though.
 * Fix a bug that could theoretically allow sacking of non-existant
   capitals of visitors, deities and such.
 * You now gain BTUs and MOB_ACCESS mobility before the first update
   again.  You didn't since 4.3.10 introduced the ETU clock.
 * Fix edit to detect when the edited object changes while it's being
   edited.
 * Fix xdump nat for relations: it got HIDDEN backwards.  Broken in
   4.3.12.
 * Fix origin command not to prompt twice for its argument.  Broken in
   4.3.0.
 * Info file improvements.

Changes to Empire 4.3.14 - Mon May  5 04:57:03 UTC 2008
 * Another round of fixes to the fire command:
   - Don't disclose where the target retreated to.
   - Charge the target shells for returning fire.
   - Always use guns when the target is out of range, even if it's a
     submarine.  Before, the difference in shell use disclosed whether
     the target is a submarine.  Loophole opened in 4.3.12.
   - Make depth charges again work with just one shell.  Failing the
     command in that case lets players find out whether the target is
     a submarine.  Loophole opened in 4.3.12.
   - Detect when the firing sector, ship or land unit changes across
     the target prompt.  Can be abused to duplicate commodities, and
     more.
 * Fix parsing of anchor-relative time in update schedule.  Broken in
   4.3.13.
 * Fix logging of player input in the journal to include command
   interruption and EOF.
 * Fix oversights in 4.3.12's change to keep track of flying planes:
   planes could be erroneously reported damaged by their own ground
   strikes.  Happened with missions and the launch command.
 * Fix load not load hardened missiles.
 * Fix harden not to harden missiles loaded on land units.
 * New server option -E to choose what to do on recoverable internal
   error.  You can now make the server dump core and keep going (not
   implemented for Windows).  New econfig key post_crash_dump_hook to
   run a program after crash-dumping.  See scripts/savecore for an
   example.
 * Journal entries are now flushed to disk right away even when the
   server is not running in debug mode.
 * Don't let land units load trains.
 * Allow deity to display power report for all types of countries,
   with power c.
 * Fix designate's check for disallowed sector types.  Broken in
   4.3.12.
 * Documentation fixes.

Changes to Empire 4.3.13 - Tue Apr 15 19:03:11 UTC 2008
 * Work around strptime() lossage on some systems.  Fixes the parsing
   of the schedule file there.
 * New selector maxnoc for xdump version.
 * Fix initialization of realms in newcap.  Broken in 4.3.12.
 * Fix add for status arguments active, god, delete.  Broken in
   4.3.12.
 * Info file, manual page and documentation fixes.

Changes to Empire 4.3.12 - Wed Apr  2 05:35:06 UTC 2008
 * Fix rounding error in update that could result in more babies than
   food permits, and negative food.
 * Fix a bug that could lead to unfair thread scheduling under
   Windows.
 * Fix ship and land unit missile interdiction and ballistic missile
   interception to require a proper base.  Before, missiles could
   launch from anywhere for that.
 * Launch bug fixes:
   - Don't allow launching from unowned sectors.
   - Don't destroy missiles stuck on foreign ships.
   - Use up the supplies required for launching an asat only when it
     actually launches.
   - Don't ignore satellites' fuel use.
 * Deity command newcap now requires its second argument (sanctuary
   coordinates).  Before, it tried to pick a suitable location when
   none was given.  The code implementing this feature had problems,
   and it's not worth fixing, because it is obscure, and rarely (if
   ever) used: no conscientious deity would use it for a real game,
   and for blitzes fairland does a better job.
 * Keep track of flying planes properly.  This fixes a number of bugs:
   - While the bomb command awaited pin-bombing target input from the
     player, other players could interact with the pin-bombers and
     escorts as if they were sitting in their bases: spy them, damage,
     capture, load, unload, or upgrade them, even get intercepted by
     them.  But any changes to those planes were wiped out when they
     landed.  Abusable.
   - The bomb command could bomb its own escorts, directly (pin-bomb
     planes) or through collateral damage, strategic sector damage,
     collapsing bridges or nuke damage.  The damage to the escorts was
     wiped out when they landed.
   - If you asked for a plane to fly both in the primary mission and
     escort, you got charged fuel for two sorties instead of one.
 * Plug memory leaks in plane interception.
 * Fix trade not to let the buyer teleport satellites (not in orbit)
   and asats to an arbitrary sector.  Abusable, because abms
   intercepted from anywhere, and satellites could be launched from
   unowned sectors.
 * The PRNG seed is now logged in the journal.
 * Fix nightly build for Windows.
 * Fix crash bug that bit when custom tables contained columns names
   that existed, but weren't supposed to be in the custom table.
 * New timestamp selector for commodity, country, game, loan, nation,
   news, realm, trade, treaty to support incremental xdump.
 * Deity commands newcap and add wipe the nation more thoroughly.  add
   no longer touches relations and flags for status active and god.
 * New utility program empdump to export and import game state as
   plain text.  Check its manual for details, including limitations.
 * Plane, ship and land unit stats are no longer stored as game state,
   but recomputed from tech and base stats as needed.
 * xdump ver is no longer a special case.  Syntax "xdump ver" is now
   deprecated.  Use "xdump version *".
 * Fire and torpedo cleanup and bug hunt:
   - Plug memory leak in fire command.
   - Fix ammunition use when returning fire: resupply could lose
     shells when returning fire to multiple targets, and land units
     were charged per target instead of just once.
   - Interdicting land units now resupply shells automatically, for
     consistency with other ways to fire.
   - torpedo and fire no longer resupply shells automatically.  The
     latter used to resupply land units only.
   - Land units no longer have to be in supply to fire actively, for
     consistency with other ways to fire.
   - Submarines with zero mobility can no longer interdict, for
     consistency with other ways to torpedo.
   - Fix bugs that let submarines without capability torp use
     torpedoes to return fire and interdict.
   - Fix torpedo command not to require a line of sight for return
     fire and depth charges, and to use torpedo range instead of
     firing range for return torpedoes.
   - Active fire and interdiction didn't work for ships with zero
     firing range, even though return fire and support did.  No such
     ships exist in the stock game.
   - Let land units with zero firing range and non-zero firing damage
     fire, for consistency with ships.  No such units exist in the
     stock game.
   - Ships required different numbers of military to operate their
     guns for the various kinds of fire.  Unify to require 2*N-1
     military to fire N guns.
   - Ship ammunition use differed for the various kinds of fire.
     Unify to use one shell per two guns.
   - Forts could fire support even when there were not enough mil.
   - Fix automatic shell resupply in several places to supply exactly
     the shells actually fired, no more, no less.
 * Change depth charges back to how they are documented and worked
   before 4.0.6, mostly: require no guns, one military, do damage like
   shell fire from three guns (before 4.0.6: two guns), use two
   shells.  In 4.0.6's model, they worked exactly like shell fire
   (require guns and gun crew, non-zero firing range, damage and
   ammunition use scales with available guns), except for missions,
   which was a bug.  Note that depth charge damage for all ships is
   now like fire from three guns for two shells.  No change for dd; af
   did two gun damage for one shell before, and nas did four gun
   damage for two shells.
 * The edit command keys deprecated in 4.3.3 are now gone.
 * Fix build not to screw up automatic dependencies on certain compile
   errors.
 * Fix a bug that could prevent repeated news from properly
   aggregating into one news item.
 * Properly initialize all bits in game state files, even those that
   aren't used.
 * Fix explore's test for stopping on a splashed bridge.  The bug made
   explore print to a bogus message.
 * Fix spelling of symbol airburst in table plane-flags.  This could
   affect clients.
 * Change designate not to check total cost before executing the
   redesignation.  The code implementing that was buggy and too ugly
   to live.  Designate doesn't cost anything in the stock game.
 * Always charge land units at least as much mobility for assaulting
   from non-landing ships as from landing ships.  Before, marines lost
   all mobility when assaulting from a non-landing ship, which could
   be less than what the same assault costs from a landing ship (half
   an update's worth).
 * Fix LWP's stack initialization for -s.  It caused crashes on some
   systems.
 * Fix confused and buggy bridge splashing code:
   - Flying planes could be reported drowned.
   - Pin-bombing a bridge head failed to collapse bridge spans for
     lack of support, unless the pin-bombing caused collateral damage.
   - Corrupt sector file could lead to infinite recursion.
 * Relative names now work for econfig keys data and info.
 * Fix utility programs to abort on internal errors.  They used to
   print a message and attempt to recover.
 * Fix off-by-one in fairland that ate the last expansion island.
 * Switch from CVS to git.
 * Code refactoring and cleanup.
 * Portability fixes.
 * Info file, manual page and documentation fixes and updates.

Changes to Empire 4.3.11 - Tue Jan  1 18:57:38 UTC 2008
 * New sector selector elev.  It's set by fairland, but has no effect
   on the game.  It can be useful for deities to customize a world
   created by fairland.
 * The client now copes with IDs greater than 15.  The Empire
   protocol currently uses 14 IDs.
 * Rewrite the client's code for reading server output during login.
   The old code could write one byte beyond the end of the buffer
   (theoretically a remote hole), got confused by long lines, and
   lines that didn't arrive in one piece.
 * Long country name, password or user name crashed the client.
 * Fix xdump not to deny access to game state tables when maximum
   minutes per day are exceeded.
 * The server considers a country bankrupt when its treasury is
   negative.  Except when it doesn't.  Fix that:
   - report misreported countries with $0 as broke.
   - If a command spent a few cents more than you had, commands requiring
     money became unavailable even when rounding brought you back to
     $0.  But logging out and back in then bankrupted you.
   - If your treasury rose to exactly $0, commands requiring money
     remained unavailable.
   - If you had $0, sectors didn't build and sectors and ships didn't
     produce at the update.
 * Fix a bug in trade that made your money evaporate when a trade fell
   through because you couldn't fully pay.  This bug was there since
   the introduction of the market in 4.0.0.  It got aggravated by the
   removal of forced loans in 4.3.0.
 * Fix printing of the new group in army, fleet and wing.  Broken in
   4.3.5.
 * Fix flash not to screw up the message when the first argument
   contains white-space.
 * Don't let planes intercept when they are stuck in foreign bases, on
   inefficient ships or land units, or on land units loaded on ships
   or land units.
 * Fix a bug that made interception require petrol to be present even
   for planes that don't use fuel (SAMs).  Broken in 4.3.3.  Air
   defense missions weren't affected.
 * fairland and setsector now limit fertility to 100 instead of 120,
   like the other resources.
 * Generalize the autonav hack for fishing boats so it covers oil
   derricks as well.
 * Fix bug in path command that made it swallow every other step.
   Broken in 4.2.22.
 * The execute command no longer supports redirections and execute in
   batch files.  Flaws in the Empire protocol make it next to
   impossible for clients to implement that correctly.  See
   doc/clients-howto for an explanation.
 * Rewrite much of the client's code for sending commands and
   receiving server output:
   - No longer blocks on sending commands, which could deadlock the
     session.
   - Fix race condition that could cause server output to be discarded
     on EOF on standard input.
   - Don't discard server output when reading standard input fails.
   - Fix error handling for execute and other, more obscure cases.
   - More rigorous error checking for redirections and execute.
     Clearer error messages, too.
   - Fix execute redirected to file or pipe.  Believed to be broken in
     4.2.0.
   - The client now creates files for redirections with the same
     permissions whether the redirection is in a pipe or not.  Before,
     redirections without a pipe created files with no permissions for
     group and others.  This change may make your redirected output
     visible to other users on your system.  Use the usual controls,
     like umask, to restrict permissions to your liking.
   - Reject unexpected redirections (violating the protocol).  Before,
     they silently replaced the current one, and in the case of pipes
     leaked file descriptors and memory.
   - Redirections in batch files are not implemented.  They didn't
     quite work before, and could leak memory and file descriptors.  A
     flaw in the Empire protocol makes them next to impossible to
     implement correctly.
   - The execute command in batch files is not implemented.  A flaw in
     the Empire protocol makes this practically useless anyway.
   - Old code leaked memory in various places.
   - Unfortunate incompatibility with older servers: if you type the
     EOF character (normally ^D) at an argument prompt, the session
     hangs.  Use the interrupt character (normally ^C) to get out.
 * The client now supports options -h and -v.
 * The client no longer zaps its command line in a lame attempt at
   protecting users who foolishly specify passwords on the command
   line.  Zapping annoys system administrators and creates a false
   sense of security.
 * Journal entries are now flushed to disk right away when the server
   is running in debug mode.
 * Fix bugs in load that could make loading operations fail noisily
   instead of silently on some systems.
 * Server now reliably shuts down session when it receives an
   end-of-file condition from the client.  Before, it merely failed
   the current command when it got it at an argument prompt.
 * Fix bug in execute that could crash the server on some systems when
   it prompts for the argument.
 * Fix bug that let you check whether a non-allied carrier is
   efficient enough for takeoff when one of your planes got stuck
   there somehow.
 * Fix bug that let you attempt to escort a one-way mission to a
   carrier with non-light, non-interceptor escorts.  Such planes don't
   fit on carriers, and the command then failed with a bogus error
   message.  Before 4.2.17, the planes flew, and the escorts were
   teleported home.
 * Change the rules for fitting planes on ships slightly, so that the
   order of loading no longer matters. Choppers and x-lights can now
   use plane slots even when not light.  X-light choppers, which don't
   exist in the stock game, can no longer use x-light slots.
 * The server now keeps its log file open, just like the journal.
 * New server option -R to set the PRNG seed.  This is for
   reproducible tests.  Change the Windows-only service uninstall
   options from -r/-R to -u/-U.
 * Supply a real PRNG for Windows instead of the leaky water-pistol
   Windows provides.
 * Threads sleeping until a specified time could return early under
   Windows.  This could cause missed updates.  Broken in 4.3.10.
 * Fix missing newline in show updates.
 * Power report didn't factor tech into power correctly.  Broken in
   4.3.3.
 * Code refactoring and cleanup.
 * Info file and documentation fixes and updates.

Changes to Empire 4.3.10 - Thu Sep  6 17:59:53 UTC 2007
 * Bridge spans now cost $1000 instead of $2000, and bridge towers
   $3000 instead of $7500.
 * The Windows port now supports IPv6, but it still needs to be
   enabled manually at compile-time.
 * The custom table reader detects more errors.
 * Do not leak world creation time in files and fairland, because
   that facilitates attacks against the PRNG.  This misfeature crept
   into 4.3.0.
 * Remove the ancient, crufty non-UCONTEXT system-dependent LWP code.
   Using it required manual hackery since 4.3.0.
 * Plug file descriptor leak in add command.
 * Don't kill player connections violently when their thread appears
   to be hung.  Threads being aborted by update or shutdown could be
   misidentified as hung, and the violence could lead to resource
   leaks, locked out updates, and corrupted game state.
 * Fix LWP to reliably wake up threads sleeping for a time.  Before,
   players with a sufficiently fast connection could starve out system
   threads, including the update.
 * Fix a class of bugs that made commands behave differently while the
   update is attempting to take control:
   - No MOB_ACCESS mobility was gained.
   - Telegrams were miscounted.
   - Bulletins got misfiled as production reports.
   - The navigate command ignored sail paths.
   - The declare command was more quiet, and declarations of war
     failed silently when SLOW_WAR was enabled.
   - Many messages got misdirected to bulletins.
   Some of these bugs go back all the way to Empire 2, ca 1995.
 * New deity command reload, effect similar to SIGHUP.
 * Not voting for a demand update no longer lets you veto further
   demand updates.  This feature was flawed (it encourages players to
   vote late so that they can tactically vote no and thus build up
   veto rights), virtually unused, and buggy.  The edit command still
   accepts and ignores the country key 'U' for compatibility, but it
   is deprecated.
 * Fix zdone not to claim to have triggered an update when they're
   disabled.
 * New update scheduler and ETU clock:
   - New schedule file.  See doc/schedule for how to use it.  Replaces
     econfig keys update_policy, adj_update, update_times, hourslop,
     blitz_time.  A change of schedule does not require a server
     restart.
   - The force command can no longer force updates in the future.
     Edit the schedule file for that.
   - New command show updates to show the update schedule.  Obsoletes
     the update command.
   - New xdump game and xdump updates.
   - New utility program empsched to help test update schedules.
   - Replace option DEMANDUPDATE and econfig key update_demandpolicy
     by econfig key update_demand.
   - econfig key update_demandtimes no longer applies to scheduled
     demand updates.
   - You now receive new BTUs at the update in addition to login.
   - You now always gain the same BTUs and MOB_ACCESS mobility per
     turn, even when the update schedule changes, updates get missed,
     or unscheduled demand updates run.
   - Remove the mobupdate command, because porting it to the new ETU
     clock is not worth it.
 * Redesign of synchronization between commands, update and shutdown,
   and how updates are triggered:
   - Simplify update to a single thread waiting for the trigger.
     Before, three update threads had to perform a carefully
     choreographed dance to make updates happen.  The dance relied on
     thread priorities for correctness, which are only implemented by
     LWP.  With pthreads and under Windows, demand updates were prone
     to two update threads starting up concurrently.  Even with LWP,
     forced and demand updates could lead to double updates.
   - Use a lock for synchronization: commands take it shared, update
     and shutdown take it exclusive.  This makes update and shutdown
     block until all aborted commands terminated and gave up their
     lock.  Before, they proceeded blindly after waiting two or one
     seconds respectively, which was cheesy and unsafe.
   - New commands no longer fail while the update is pending, they are
     delayed until after the update.
   - New commands can no longer start during shutdown.  Before, they
     could, but risked getting killed violently, possibly corrupting
     game state.
   - Fix command execution so that commands blocked in writing
     redirections are abortable.  The bug allowed players to delay
     update and shutdown indefinitely.
   - Fix shutdown not to let player threads block on output.  This
     let players delay shutdown indefinitely.
   - Fix reading of player input not to block again after update or
     shutdown aborted it.  The bug allowed players to delay update and
     shutdown indefinitely.
 * Thread priorities are only implemented by LWP and no longer used.
   With LWP, they let players with a sufficiently fast connection
   starve out the threads that clean the lost file and kill idle
   connections.  Drop them.
 * Show nuke rounded required research incorrectly.
 * Log unprintable characters as octal escapes rather than question
   marks in the journal file.
 * The files program no longer changes permissions of existing files.
 * The files program now creates all files with permissions ug=rw and
   all directories ug=rwx, modified by umask.
 * Fix a bug that could make arm, disarm and nuke detonation work with
   a used-up nuke instead of the armed one.  This could be abused to
   detonate nukes multiple times.  Broken in 4.3.3.
 * Fix nuke detonation output for zero blast radius.
 * Plug memory leaks in thread code.
 * Fix a bug that made LWP I/O wakeup unreliable.
 * New march sub-command v for view.
 * Journal was initialized before threads were, which was wrong.
 * Fix server not to create a journal on SIGHUP when it is disabled.
 * shutdown 0 now requests immediate shutdown instead of cancellation
   of a pending shutdown.  shutdown -1 now cancels.
 * flash and wall no longer split long message lines.
 * Fix a bug in collect that screwed up a fully collected loan instead
   of deleting it.  The screwed up loan could not be sharked or
   collected, but it could be repaid.
 * Remove the wait command, it's been broken and restricted to deity
   since Empire 2, ca. 1995.
 * Budget failed to take sector production into account for predicting
   unit repairs.  Before 4.3.6, this bug bit only when you fooled
   around with budget priorities.
 * When unit repairs are limited by materials, the efficiency gain is
   now rounded down.
 * Ship repairs outside harbors and plane repairs by carriers used to
   consume commodities when and as far as available.  Now, they
   consume the same fraction of the real cost of each commodity,
   i.e. commodity use is limited by the most scarce commodity.
   Neither old nor new behavior make much sense, but the new code is
   simpler.
 * Code refactoring and cleanup, in particular to make the Windows
   port less ugly.
 * Portability fixes.
 * Info file and manual page updates.  All manual pages are now
   installed in section 6.

Changes to Empire 4.3.9 - Sat Jan  6 12:42:19 UTC 2007
 * Fix declare to prevent the deity from changing relations for a
   player to self.  Allow the deity to set the relations of a player
   towards the deity.
 * Fix arm not to put the same nuke on multiple planes (broken in
   4.3.3).
 * New option AUTO_POWER.
 * Sectors now repair allied land units.
 * Sectors now repair only own and allied planes.
 * Fix repair of planes on foreign carriers: repair allied planes,
   ignore the others.  Before 4.3.3, carriers destroyed rather than
   repaired foreign planes.  Since 4.3.3, foreign planes were treated
   as recoverable error.
 * Fix automatic fortification of land units to use all excess
   mobility even when mobility delta exceeds mobility maximum.
 * Disable automatic fortification of land units when MOB_ACCESS is
   on, because it is prohibitively slow then (broken in 4.2.13).
 * Budget now shows level production (tech, education, research and
   happiness) rounded instead of randomly rounded.
 * Fix update not to randomly flush small level production to zero.
 * Fix overly restrictive game file locking under Windows (broken in
   4.3.8).
 * Fix navigate to charge mobility for sweeping mines without moving
   (broken in 4.3.6).
 * Code cleanup.

Changes to Empire 4.3.8 - Fri Oct 20 18:30:28 UTC 2006
 * Ships no longer produce stuff when the government is broke.
 * Fix land unit casualties in revolts.
 * Fix security units not to raid friendly che.
 * Fix update sequence to ensure stopped sectors don't work on the
   ships, planes and land units in the sector.  Before 4.3.6, this bug
   bit only when you fooled around with budget priorities.
 * Fix canal loading (was broken in 4.3.0).
 * Fix tend not to tend land units that could not be loaded.
 * Make xdump command available before break, but deny access to game
   state.
 * Lock game files to prevent concurrent access by multiple programs.
   fairland does not yet obey the lock.
 * Fix production to consider sector item limit.
 * Fix swapsector to update coastal flag correctly (broken in 4.3.0).
 * Enhanced mapping sub-commands in move, test, explore and transport.
 * Code cleanup.
 * Info file fixes.

Changes to Empire 4.3.7 - Fri Jul 14 23:05:19 UTC 2006
 * Sanctuaries are again fully populated with civilians (4.3.6 changed
   the limit, but not the population).  Food is now spread evenly
   among them.
 * Fix path finder to find shortest naval and aerial paths again
   (broken in 4.3.4).
 * transport's map sub-command now does the right thing for a unit
   number argument.
 * Fix bug in map sub-command of move, explore and transport that
   passed junk to map command.  Could theoretically crash.
 * Food need reported by starvation was one short for sectors without
   food.
 * Change land unit ammunition use back to what it was before 4.2.3.
   This lets deities customize ammunition use independent of damage.
   It also substantially reduces ammunition consumption at high tech.
 * navigation and march enhancements:
   - Sub-command 'f' now takes an optional ship / land unit number.
   - Mapping sub-commands now take an optional ship / land unit
     number.
   - New sub-command 'd' to drop mines.
 * Let minesweepers retreat at their full speed (missed in 4.3.6).
 * SUPER_BARS cannot be harmed, so don't interdict them.
 * Don't let trains and non-trains march together for now, because the
   march code isn't prepared for that.
 * New mapping flag 'n' to show nukes.
 * Mapping flag '*' now shows nukes as well.
 * New command nbmap to bmap around a nuke.  Overload command nmap to
   map around a nuke when given a nuke argument.
 * Fix an inconsistency that gave tactical bombers a small bonus
   vs. flak when pin-bombing.
 * Fix production to show the designation that actually produces, not
   the designation as it is now (broken in 4.3.6).
 * Fix accidental whitespace change in output of sdump, pdump and
   ldump that crept into 4.3.5.
 * Support user-defined sector types and products in custom tables,
   not just customization of existing ones.
 * Fix off-by-one bug in budget that used to be masked by budget
   priorities until their demise in 4.3.6.  The bug could crash the
   game on certain systems.
 * Code cleanup.
 * Info file fixes.

Changes to Empire 4.3.6 - Tue Jun 20 18:46:18 UTC 2006
 * The shutdown command now terminates the server process correctly
   even with LinuxThreads pthreads under Linux 2.4 and earlier.
 * Cope with old versions of groff (broken in 4.3.0).
 * You can now arm allied planes with nukes.
 * Taking a nuke off a plane gives it to the sector owner again.
   Feature was accidentally dropped in 4.3.3.  Recipient must be
   allied.
 * Naval path went bananas for impassable end sectors (broken in
   4.3.1).
 * New econfig key disabled_commands.
 * New territory selector dterr, restricted to deities.  The territory
   command defaults to it for deities.
 * Let land units attack as long as they have positive mobility,
   except for high-mobility terrain (mountains), where the rules
   remain as they were: land units need to have all the mobility
   charged for the attack, not counting combat and moving in to
   occupy.  Rationale: Making sure your land units reach attack
   positions with enough mobility left is a pain in the neck.
   Requiring only positive mobility is friendlier, but allowed rushing
   of mountains, so we make an exception there.
 * Can typeset info pages as PostScript.  Result isn't exactly pretty.
 * Support disabling of infrastructure in infra custom table.  Remove
   redundant option DEFENSE_INFRA.  show command now shows only enabled
   infrastructure.
 * Fix crash on empty argument in bmap, lbmap, lmap, map, nmap, pbmap,
   pmap sbmap, smap (broken in 4.3.1).
 * New, simpler sector mobility cost formula:
   - The basic cost is still a linear function of sector efficiency,
     with customizable cost at 0% and at 100%.  The latter used to be
     derived from the former.  The show command shows both parameters
     now.
   - Mountains are no longer a special case.
   - Tweak costs: mountains now range from 2.4 to 1.2 instead of 2.174
     to 1.304, highway, bridge head, bridge span and bridge tower
     range from 0.4 to 0.001 instead of 0.2 to 0.001, and wilderness
     is 0.4 flat instead of 0.6 to 0.4.  All sector types mortal
     players can designate now cost the same at 0%.  That's a feature.
   - Road infrastructure now reduces cost up to 90%, linear in road
     efficiency (used to be close to that, but non-linear).  Same for
     rail infrastructure, but 99%.  Adapted from Ice World V.
   - Double land unit minimum cost to 0.02, to slow them down on
     highways.
   - Change land unit cost penalty in newly taken land to an extra 0.2
     instead of a minimum of 0.2, to reduce penetration depth.
 * Reduce bridge head defense and collect value to match highway's.
 * sinfrastructure displayed bogus rail mobility costs when there was
   no rail.
 * Retreating engineers took full landmine damage instead of half.
 * Fix socket creation code not to print a null pointer.
 * Fix login option code not to search beyond end of table (broken in
   4.2.21).
 * Spy planes and satellites can no longer spot landspies.
 * Spy planes can now spot surface ships at sea.  They can no longer
   spot land units loaded on ships or land units.
 * Fix train retreat mobility cost, and don't permit retreat off rail.
 * Change maximum population to 100 for plains (was 49), mountains
   (was 99), bridge spans and bridge towers (was 999), and to 1000 for
   the other habitable sector types (was 999).
 * Let minesweepers navigate at the speed their stats indicate by
   letting them sweep on the move at no extra mobility charge.
 * Reduce chance for planes to abort from 100-eff (if eff<80) to
   80-eff.  This should make it easier to punch through air defense,
   flak in particular, and increase the risk of getting shot down.
 * Ship and land unit flak intensity is now proportional to efficiency.
 * Ship and land unit flak intensity is no longer scaled by the tech
   of the owner of the last thing firing.  It's still scaled by each
   thing's tech.
 * All ships and land units firing flak now spot the plane and are
   spotted by the plane.  Previously, spotting stopped as soon as flak
   maxed out.
 * Inefficient land units were overcharged mobility when reacting.
   Broken when Empire 3 changed land unit mobility use not to depend
   on efficiency, except for supply units.
 * New journal file to log complete player input plus selected
   important events.  Controlled by new econfig key keep_journal, off
   by default.
 * Fix unsafe shutdown on SIGINT and SIGTERM.
 * Fix land unit path finding: paths weren't always cheapest (broken
   in 4.0.2), and trains could be sent down paths without rail (never
   worked).  Also fixes unit reaction path cost.
 * The custom table reader can now deal with sector, item, product and
   infra tables more comfortably.
 * Remove option BIG_CITY.  Deities can customize the sect table to
   get the same effect.
 * Many more selectors can now be compared with names, not just
   numbers.  This includes nations, e.g. census ?old=Mirkwood.  Does
   not fully work for queer names like &&&.
 * Let land units on missions support, just like ships.
 * Don't let units loaded on a train react.
 * Change update sequence to repair ships, planes and land units after
   sectors produce.  This makes repairs use new avail instead of old.
 * Let players stop and start units.
 * Remove budget priorities.
 * Make plane report easier to understand.
 * Change land report to show carriers in the same format as plane and
   nuke report do.
 * Code cleanup.
 * Info file and manual page fixes.

Changes to Empire 4.3.5 - Sun May 28 07:49:54 UTC 2006
 * In conditionals, ~ compared to a string selector now stands for an
   empty string.
 * Change encoding of `not in any group' from " " to "".  This makes
   fleet=~ work.  It also affects xdump.
 * Fix plane capture code to clear wing.
 * People now eat and starve exactly the same in sectors, ships and
   land units.  There used to be small differences, and civilians and
   uw loaded on land units didn't eat at all.  The starvation command
   has been updated accordingly.
 * Fix bug that makes it impossible to assault or board with land
   units (broken in 4.3.4).
 * Code cleanup.

Changes to Empire 4.3.4 - Thu May 25 05:46:10 UTC 2006
 * Don't use non-portable groff features (broken in 4.3.0).
 * Work around potential crash bugs in command argument access.
 * Plug major information leak through nation selectors (broken in
   4.2.21).  The fix unfortunately exchanges the meaning of xdump
   nation and xdump country.  Sorry about that.
 * Minimize redundancy between (mortal's) xdump nation and xdump
   country.
 * New nation selector rejects.
 * xdump nation now shows foreign relations and no longer discloses
   hidden relations.
 * Fix crash bug in edit.
 * Fix number of planes building in budget command (broken in 4.3.3).
 * Selectors group, fleet, wing and army are now strings, and obvious
   syntax like fleet#a works.  xdump also shows the strings now.
   Units not assigned to a group currently have group string " ", but
   that may still change.
 * Don't fold conditionals to lower case, because it breaks valid
   conditionals like fleet#A.
 * Mobility cost of a plane sortie is no longer limited to current
   mobility + 32.
 * Various formulas no longer truncate or round intermediate results.
 * Interdiction mission charged twice the normal mobility cost for
   torpedoes.
 * Fix sail not to interpret negative x-coordinates as unsail
   argument.
 * Improved configure script.
 * Code cleanup.

Changes to Empire 4.3.3 - Tue May 16 21:35:47 UTC 2006
 * power c can now show powerless countries.  power c * works.
 * Fortresses now require 5% efficiency to return fire and interdict.
 * Range of ship return fire and support fire no longer depends on
   efficiency.
 * Fix heap smash in login command client.  Remote hole!
 * Properly destroy nukes on collapsing bridges.  Before, you could
   rescue them by building a new bridge or arming them on naval
   planes.
 * Nukes are now numbered like other units:
   - New selector type replaces selectors number and types.
   - New selector plane points to plane armed with this nuke.
   - New selectors effic, mobil, tech, group, opx, opy, mission,
     radius.  Provided because all units have them; values are
     currently not interesting.
   - nuke command output redesigned.
   - ndump output changed subtly: nukes are no longer sorted by
     location, nukes in the same location no longer share id, and
     nukes on planes are shown.
   - transport nuke now works like transport plane.
 * arm and disarm now accept the common plane syntax, not just plane
   numbers.
 * Fix arm to let you change an armed plane's air-burst/ground-burst
   (broken in 4.2.6).
 * territory no longer re-prompts when supplied with a bad argument,
   it just fails.
 * edit lets you edit stuff that is actually computed from other
   state, like sector's coastal flag.  This makes no sense and is now
   deprecated.  Stuff deprecated since 4.2.15 is now gone.
 * Code cleanup.

Changes to Empire 4.3.2 - Thu Apr 27 17:59:58 UTC 2006
 * Fix security units' raid at the update (broken in 4.0.0, fixed only
   for 100% units in 4.2.7).  Back out 4.2.7's 2.5x deadliness
   increase.  Make sure empty units don't kill anyone.
 * Fix bug in player_init(), which was most likely fatal on 64 bit
   hosts.  Broken in 4.3.0.
 * Fix xdump not to disclose other countries' realms (new in 4.3.0).
 * The order in which sectors were visited for `*' arguments disclosed
   the real origin, fix.  Mapping commands used to work around this
   bug, until 4.3.1 exposed it there as well.
 * Code didn't compile on some systems.
 * Fix edit plane not to truncate large values for keys 'a' and 'd'.

Changes to Empire 4.3.1 - Sat Apr 15 11:40:47 UTC 2006
 * Fix check of game state on server startup (new in 4.3.0).
 * sail now accepts a destination sector.
 * Tend could send bogus bulletins (broken in 4.3.0).
 * Plug memory leaks in mission command.
 * Close loophole in navigate that allowed players to find all sectors
   that are unfriendly or inefficient harbors or bridges.
 * Server didn't start when default econfig was missing and user
   didn't supply one with -e (broken in 4.3.0).
 * Retreat condition 'h' (helpless) never worked, fix.
 * Fix retreating mine sweepers to use mobility for sweeping.
 * Fix retreating land units to hit mines with the same probability
   and to take the same damage as in normal movement.  Before,
   hit probability was higher, and damage was lower.
 * Ships could retreat into sectors owned by whoever made them
   retreat, but not in their owner's sectors.
 * Fix 'm' at the movement prompt of transport and test.
 * Fix march and test not to apply conditionals to sectors mapped,
   which led to spurious error messages.
 * Fix break not to respect conditionals.  The bug let players keep
   sanctuaries.
 * Fix retreat code not to respect conditionals.  Attackers could
   abuse this to suppress fleet and army retreat orders.
 * Fix retreat and edit to zero-terminate retreat paths.
   Theoretically abusable for gaining information.
 * Fix format string crash bugs in load, spy and bomb.
 * Fix the telegram editor: empty telegrams were broken in 4.2.21, and
   ~p before the first line of text never worked.
 * Fix ancient bridge building race condition that could break the
   test whether the player can afford the bridge.
 * Fix trade lot change detection in set command.  Can probably be
   abused to defraud sellers.
 * The pthread code now compiles under Solaris.
 * navigate and march now permit any whitespace between route and
   optional argument for radar, lookout, sonar, not just spaces.
 * Fix canal navigation (was broken in 4.3.0).
 * Info file fixes.

Changes to Empire 4.3.0 - Sun Mar 19 08:13:10 UTC 2006
 * version no longer pretends option SHIPNAMES still exists.
 * Remove files auth and ban.  They could be used to restrict client
   address and user.  Restricting user is useless, because players can
   send whatever they want.  Banning IPs is better done with a packet
   filter.  That leaves restricting deity IPs.  Provide new econfig
   key privip for that.  Apply it to session kill as well as login.
 * xdump escapes GUINEA_PIGS status:
   - xdump is now enabled unconditionally.
   - xdump sect now dumps selector coastal, because it can't be
     reliably computed from other xdump information.
   - New xdump realm and country.  Realm information is now stored in
     a separate file, with appropriate selectors.
   - Reviewed for completeness and information leaks.  See info xdump
     for remaining issues.  New treaty selector uid, nat selectors
     relations, contacts.  Removed useless selectors: news and lost
     uid, nat dayno.
   - Command syntax changed.
   - xdump opt is no more, option information is now in xdump ver.
   - xdump can now dump meta-data.
   - Header and footer changed to be more compact.  You now need
     meta-data to identify fields.
   - Bug fixes.
   - Documentation in doc/xdump.
 * New econfig key builtindir.
 * New econfig key custom_tables lets you customize configuration
   tables without recompiling.  Customizing tables other than unit
   tables is not recommended for now.  See doc/econfig for
   instructions.
 * Check configuration and game state are sane on server startup.
 * Remove option NONUKES and NEUTRON.  Deities can load a suitable
   nuke table to get the same effect.
 * Remove option DRNUKE.  Use econfig key drnuke_const to control
   whether nukes need research.
 * Disqualify land units from performing missions while loaded on
   ships or trains.
 * Fix dump for strings without terminating zero.
 * Fix login command option not to send an extra prompt.
 * zdone now displays information about demand update vetoing.
 * The Windows port now requires Winsock version 2.
 * Remove econfig key powe_cost.
 * version report now shows all the configurable settings of interest
   to players.
 * carg and lcarg now display people as well.
 * ldump now dumps civ and uw as well.
 * Fix dangling pointer dereference in dogfighting code.
 * Tell sector owner when partisans take over.  But don't disclose che
   casualties then.
 * emp_server, files, fairland now support options -h and -v.
 * Simplify baroque version numbering to a single version number.
   This affects output of version.
 * New show item and show news.
 * New build process:
   - Follow the usual steps: configure; make; make install.  The
     latter is optional.  The only file deities still have to edit is
     econfig.
   - Use GNU Autoconf for system configuration.
   - Use GNU Make for portability and features.
   - Support separate build trees.
   - Remake out-of-date dependencies automatically.
   - Remake out-of-date info files automatically.
   - Flatten info directory.
   - Formatted info files are no longer distributed in source tarball.
   - Use LWP only with UCONTEXT.  Autoconfiguring the other LWP
     low-level options doesn't seem to be worth the effort, as
     UCONTEXT should be pretty portable.
 * Fix use of freed memory in sail.
 * sail could let large ships enter big cities.
 * Fix reject and accept not to require contact.
 * Fix sharebmap and telegram to prompt for a missing first argument.
 * Fix report to align column research correctly for deities.
 * When the optional newspaper argument doesn't work because HIDDEN is
   enabled, complain instead of ignoring it silently.
 * When BLITZ is enabled, fairland no longer permutes country numbers
   in newcap_script so that start islands are ordered from top left to
   bottom right.
 * Minimize output format differences between options:
   - census shows column fall regardless of FALLOUT.
   - show nuke now shows column res regardless of DRNUKE.
   - report no longer suppress column status when HIDDEN is enabled.
     HIDDEN limits status information like for the country command.
   - relations column width differed depending on option SLOW_WAR.
     Always use the wider format.
 * Fix a bug in edit that suppressed the player notification telex on
   military reserve change.
 * Fix command dispatch not to let deities use commands that are
   declared not to be available to them.  The only command affected is
   break.
 * Fix the list command for visitor.
 * econfig key m_m_p_d no longer applies to deities.
 * Fix the origin command for country #0.  Allow moving origin to
   another country's origin.  Allow deities to reset origin to
   absolute 0,0.  This turns origin into a superset of the offset
   command.  Remove offset command.
 * Fix a bug that caused unpredictable flak damage when it should have
   been maximum damage (was broken in 4.2.19).
 * Don't store initial capital location, remove the associated nat
   selectors xstart, ystart.
 * Fix production command to consider resource depletion.
 * Fix justification of coordinate output in cutoff and mission.
 * Option UPDATESCHED is no longer optional.  Disabling it didn't
   work anyway.
 * Support IPv6.
 * Turn options into ordinary econfig keys.
 * New econfig key start_cash.
 * BTU regeneration now depends on the capital's work percentage.
 * nation now supports an optional argument that lets deities ask for
   somebody else's nation report.
 * Fix nation to display SANCTUARY instead of VISITOR for countries in
   sanctuary.
 * Fix capital to let you use a regained sacked capital without
   activating another one in between.
 * Don't move capital when it is sacked.
 * New ship capability canal, replacing obscure formula based on build
   materials.
 * You now need a capital to request a demand update.
 * power now ignores landspies.
 * Fix LWP stack checking.
 * Clean up LWP, drop dead machine-specific code, fix portability bugs
   in UCONTEXT code.
 * census now shows the old owner instead of `*'.  dump is unchanged
   to avoid breaking clients.
 * peek no longer prints the old owner.
 * When a bombed land unit retreated, collateral damage was applied to
   the wrong sector.
 * paradrop, drop and fly can no longer abandon the base sector.
 * Tending foreign ships now sends a bulletin to the owner.
 * Fix bug that permitted login for inactive countries, including ""
 * Fix tend for spies.
 * Change launch to fire all missiles selected at the same target.
   Previously, it prompted for a new target after each missile.
 * add, consider, build, launch and fire no longer re-prompt when
   supplied with a bad argument, they just fail.  Consequently, the
   build command no longer lets you list types you can build.  It
   points to the show command instead.
 * Fix add not to truncate country number.
 * Fix selector coastal to be accurate at all times.
 * Fix obscure bug in fairland: the sector added last to an island
   wasn't considered coastal.  Affected resource computation.
 * Fix sector wiping in add command.
 * Remove options NO_LCMS, NO_HCMS, NO_OIL.  They were broken in many
   ways.  Custom configuration tables should become a serviceable
   replacement eventually.
 * Client no longer freezes on long lines of server output.
 * Fix uninitialized signal mask bugs.
 * Fix Windows service code's treatment of -e.
 * Sub interdiction disclosed sub owner through news' actor selector.
 * country reports on all countries, but carefully limits information
   disclosed to mortals.  Output format changed.
 * Market no longer forces sellers to grant loans.
 * Fix potential crash bug in set command.  Friendlier prompt and
   diagnostics.
 * Substantial code cleanup and portability fixes.
 * Extend empfile abstraction to cover game configuration.
 * Overhaul of game state storage.
 * The above includes major contributions from Ron Koenderink.  Many
   thanks!

.FI
.s1
.SA "Server"
