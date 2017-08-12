.TH Server "Empire4 Changes"
.NA Empire4.2 "Changes in Empire 4.2 (1998-2005)"
.LV Expert
.s1
This document outlines the various changes to the game and how they
will affect you, the player.  These were coded as the Wolfpack project.
.NF
Changes to Empire 4.2.23 - Sun Oct 23 09:28:26 UTC 2005
 * Make version pretend option SHIPNAMES still exists, for the benefit
   of clients that still depend on it.
 * The change to conv in 4.2.22 could crash or compute incorrect
   maximum population.
 * Info updates missing in 4.2.22.
 * A few code cleanups.

Changes to Empire 4.2.22 - Tue Oct 11 20:23:51 UTC 2005
 * Log player login, logout and session kill.
 * Close loophole in assembly point rules that allowed players to find
   all unowned sectors.  The assembly point must now be owned by you
   or an ally, or you or an ally must have a ship there.
 * Don't let embarked land units fire flak.
 * Fix display of the last connection's host when the host name was
   not resolved.
 * Fix change command to accept full length representative (password),
   currently 19 characters.  It used to chop off after eight
   characters.
 * Non-recon missions no longer establish contact to owners of
   overflown sectors, because those missions do not identify the
   owner.  This matches info Hidden.
 * The following events now establish contact:
   - Catching a spy
   - Sighting a ship when toggle coastwatch is off
   - Spotting overflying planes
   - satellite spotting land units
   - lookout and llookout spotting ships, planes and land units
   - Any relations change
 * The country command's `temporary' state of disorder ended after
   ten years.
 * Telegram notifications could get lost in exotic circumstances,
   e.g. while a second client attempted to connect.  Can be abused for
   stealthy attacks.
 * Remove option SNEAK_ATTACK.
 * Fix lboard not to let you board dead land units.
 * Fix stack smash in bomb, drop, fly, paradrop, recon and sail.
   Remote hole!
 * bomb, drop, fly, paradrop, recon and sail now deal with EOF
   correctly.
 * Fix fly to let the player abort the command at the last prompt.
 * Client no longer reads execute scripts in binary mode under
   Windows.
 * New econfig key listen_address.
 * econfig key port can now be a service name.  Service empire no
   longer takes precedence over econfig.
 * Client falls back to compiled-in host only if environment doesn't
   supply one, not if it can't be resolved.  Same for port.
 * Oil derrick could extract slightly more oil than was in the ground.
 * test command checked start sector for space and some other things
   instead of end sector.
 * convert now silently limits conversions to avoid exceeding maximum
   population.
 * shoot no longer limits you to 999 victims.
 * 4.2.19 broke pin-bombing commodities on some platforms.
 * Don't block on output while update is pending.  This is required to
   ensure the update can abort commands.
 * Commands submitted while the update is waiting for commands to
   abort now fail.
 * The force command no longer waits for the specified time.  It
   starts a scheduling thread instead, just like shutdown.
 * Correct grammar and punctuation in output of version.
 * Option SHIPNAMES is no longer optional.
 * Fix output of qorder and sorder for ships with sail path.
 * Don't change capital to agribusiness on retake.  From Pat Loney.
 * bdes prompt for new designation was messed up for empty bmaps.
 * Client now supports pipes under Windows.
 * Care for subtle Windows socket incompatibilities.
 * Fix strength command not to lie about units reacting into
   mountains.
 * strength now yields the processor periodically, to keep the game
   responsive.
 * Schedule commands more fairly.  Previously, a sufficiently fast
   player could starve out the others.
 * launch assumed plane maximum mobility 127 for calculating when
   satellites will be ready to use.
 * Drop support for DCE threads.
 * Drop support for pthread implementations lacking pthread_kill().
   This includes ancient versions of OS X.
 * Code cleanups.
 * Minor info file fixes.
 * The above includes major contributions from Ron Koenderink.  Many
   thanks!

Changes to Empire 4.2.21 - Sat Jul 16 17:51:01 UTC 2005
 * Don't let threads block on output to another player thread's
   socket.  Such blocking violates basic design assumptions and can
   cause all kinds of grief.  In particular, it causes player session
   hangs under Windows with 4.2.20.  Broken since the introduction of
   flash and asynchronous telegram notification many years ago.
 * Threads sleeping until a specified time could return early under
   Windows.  This could cause double-updates.
 * Fix fallout damage (was broken in 4.2.19).
 * Fix show sector s (was broken in 4.2.19).
 * Fix deliver not to deliver friendly civilians into occupied
   sectors.
 * Fix mil duplication in board (was broken in 4.2.14).
 * Fix stack smash in lrange, shoot and execute.  Remote holes, and
   the last one doesn't even require authentication.
 * Fix crash bug in emp_config parsing.
 * Fix buffer overruns in turn and shutdown.
 * Fix second argument of shutdown.
 * announce, pray, telegram and turn now abort on EOF.
 * Fix display of MOTD and game down message of maximum size.
 * Correctly support telnet line termination on input.  Previously,
   carriage returns could slip into ship names and such.
 * Fix flash not to lie about recipients that are just logging in.
 * Remove obscure login command list.
 * Convert client from obsolete termcap to terminfo, and clean up
   terminal handling.
 * New fairland option -h to print usage information.  Improved
   command line syntax diagnostics.  Exit unsuccessfully on unknown
   options.
 * fleet and army now copy retreat orders of the fleet's or army's
   first member with group retreat orders in the same sector.
   Previously, they used the retreat orders of the first fleet or army
   member, which is less than useful, and failed to take ownership
   into account, which is an abusable bug.
 * New xdump nation.
 * Fix xdump not to disclose other countries' losses.
 * Fix exit status of files program when it can't create files.
 * Support UTF-8 encoded Unicode in human-human communications.  This
   affects MOTD, game down message, telexes, announcements and flash
   messages.  See new doc/unicode for technical details.
 * New login command `options' for client/server option negotiation.
   See new doc/clients-howto for technical details.  The only option
   so far is utf-8.
 * emp_client -u now requests UTF-8.  This requires a terminal that
   understands UTF-8.
 * Fix news for land unit defensive support.
 * Properly mark sea mines hit by retreating ships.  Unmark mines
   cleared by bridges.
 * Fix diagnostics for unknown selector names in conditionals (was
   broken in 4.2.20).
 * Fix selector name clashes: rename ship selector mobquota to mquota,
   land selector for reaction range limit from rad to rmax.
 * Fix Windows services for relative file names on command line or in
   econfig (was broken in 4.2.20).
 * econfig keys ship_grow_scale, plane_grow_scale and land_grow_scale
   are now float.  This gives deities finer control over maximum
   efficiency gain.
 * Fix production command for sectors with 60% efficiency.
 * Improve syntax of read for deities.
 * Code cleanups.
 * Some info topic names differed only in case, which loses on
   case-insensitive file systems, and is a bad user interface even
   when it works.  Change those names.  Rename deity command hidden to
   peek because its info topic clashed with that of option HIDDEN.
 * Thanks to Doug Wescott sheep are finally covered properly in info.
 * Documentation fixes.
 * Much of the above was contributed by Ron Koenderink.  Many thanks!

Changes to Empire 4.2.20 - Tue Mar 22 21:07:18 UTC 2005
 * When units in different sectors marched, only units in the sector
   with the highest-numbered unit were interdicted.  A marching
   non-spy made all marching spies visible to interdiction regardless
   of location.
 * Fix crash when interdiction sinks second ship in a sector.  Broken
   in 4.0.10.
 * Fix an ancient bug in simulation of level production that clobbered
   stack and could lead to low predictions, typically zero, which
   screwed up budget.  The bug had no effect until 4.2.19.
 * When a sector revolts, and there wasn't enough space to liberate
   all uw, the number of remaining uw got the wrong sign.  Broken in
   4.2.14.
 * Log and abort when LWP stack check detects stack smash.  Server
   option -s no longer implies -p.
 * The native Windows build now supports -p and -s.
 * The native Windows build now properly deals with ^C when running in
   the foreground.  It no longer prompts for a quit command on the
   terminal.
 * The meaning of server option -D in presence of -e is unclear.
   Other programs only support -e.  Remove option -D.  Use -e
   DIR/econfig instead of -D DIR.
 * New server option -v shows version information.
 * Fix the server's exit status with -h.  Broken in 4.2.19.
 * The linux-pthreads build failed to shut down on signals.
 * Fix threading bugs in the native Windows build that made shutdown
   unsafe and screwed up update aborting commands.
 * New econfig key pre_update_hook to allow automatic, safe backup
   right before the update.  See scripts/backup for an example.
 * The server now prints log messages to stderr as well when running
   in the foreground.
 * Fix potential buffer overflows in logerror() and lwpStatus().
 * Server now requires POSIX signals.  It no longer catches SIGUSR1.
 * Remove support for ancient versions of HP-UX.
 * Fix seeding of PRNG broken in 4.2.19.
 * Don't reseed the PRNG in commands, it hurts randomness and could be
   abused by crafty players.
 * The server now makes the data directory its working directory.
   This also gets rid of a bug that changed the file name of the
   sector, power, nation and lostitems file unless econfig could not
   be read.
 * The server now writes a pid file.  Thanks to Daniel O'Connor.
 * More user friendly server startup: more errors are detected while
   the server still runs in the foreground, which makes it possible to
   complain to stderr and to exit unsuccessfully.
 * Early log messages went to the log file in the default data
   directory, even though econfig specifies another data directory.
 * Specifying an econfig file on the command line that can't be read
   is now fatal.
 * Errors in econfig are now fatal.
 * The server now refuses to start when it can't open data files.
 * News are now sorted and expire sanely.
 * Conditionals can now compare strings as well.
 * Corrupted down and motd files could cause buffer overflows.
 * Fix recon to let player abort at the flight path prompt.
 * power no longer rounds worldwide numbers for deities.
 * power command failed to recognize deities with non-zero country
   number.
 * Change output of production command so that columns are aligned
   even for large values.  Replace column wkfc by more useful column
   avail.
 * neweff and production miscalculated work when mil exceeded
   population limit.
 * neweff miscalculated work when the population limit exceeded 999.
 * Fix capping of avail when a big city is torn down.
 * Fix a bug in fire that allowed players to find all submarine UIDs.
 * Maximum sector population is no longer hardcoded, and now covered
   by xdump.  Option RES_POP now affects mountains and plains as
   well.  From Ray Hyatt.
 * Fix fina and news not to ignore errors in conditions.
 * The native Windows build now requires Windows Sockets 2.
 * The native Windows build now supports multiple instances of the
   service.  The ability to supply additional startup parameters to
   the service has been removed.
 * Implement fairland command line parsing under Windows.
 * Improved fairland and emp_server usage message.
 * Code cleanups.
 * Minor info file and documentation fixes.
 * Much of the above was contributed by Ron Koenderink.  Many thanks!

Changes to Empire 4.2.19 - Thu Dec 23 20:24:24 UTC 2004
 * Fix order not to unload into full sectors.
 * Fix scrap not to dump stuff into full sectors.  Excess stuff is now
   destroyed.
 * Fix distribute not to conjure up a civilian when importing
   civilians from a dist center that doesn't have any.
 * Fix flak to resupply no more shells than actually needed.
 * Fix rounding error in shoot that could result in more people shot
   than actually there.
 * Fix info and apro error handling.  In particular, do not leak
   potentially sensitive information on the server to players.
 * info no longer shows a random matching page when the topic argument
   is ambiguous.  Instead, it lists all matching topics.
 * Embarked land units and overflying planes no longer spot each
   other.
 * Fix show command to show only sectors that players can designate
   (was broken in 4.2.18).
 * Fix a rounding error in production command column `max'.
 * Fix production command for overpopulated mountains and plains.
 * Fix tradeship ally cut to match documentation.
 * Fix plague, which was broken in 4.2.14: it got stuck in stage I.
 * Fix a bug in aerial sonar where ships could mask submarines with
   the same number modulo 256.
 * Don't allow bombing of subs that were not detected.
 * MOTD and the game down message are now entirely separate: use `turn
   motd' for the MOTD, and `turn on' and `turn off' for the game down
   message.  `turn on' no longer lets you edit the MOTD.
 * Remove option NUKEFAILDETONATE.
 * Option ORBIT is no longer optional.
 * New option GUINEA_PIGS to control experimental stuff not ready for
   prime time.  If your deity enables this in a serious game, run!
 * Many xdump fixes.  xdump is now enabled by option GUINEA_PIGS.
 * New ship and land selectors rflags, rpath.
 * New ship selectors for order: xstart. xend, ystart, yend,
   cargostart, cargend, amtstart, amtend
 * New ship selectors for sail: mobquota, path, follow.
 * New news selector uid.
 * Implement command line parsing under Windows where it was missing.
 * emp_server and files print a usage message on command line syntax
   errors.
 * Fix how emp_server exits after being panicked by a signal.  It can
   now dump core when this happens.
 * Fix bug in client that could crash it when a line of input starts
   with '?'.
 * Client now gets user name from environment variable LOGNAME if set,
   else from system.
 * The native Windows build can now run as a service.
 * Redirecting client input now works under Windows.
 * Code cleanups.
 * Don't confuse work with loyalty in info Citizens.
 * Minor info file and manual page fixes.
 * Much of the above was contributed by Ron Koenderink and Marc
   Olzheim.  Many thanks!

Changes to Empire 4.2.18 - Tue Sep  7 14:08:59 UTC 2004
 * Fix info to accept abbreviated arguments under Windows.  From Ron
   Koenderink.
 * Buying planes or land units loaded on something didn't work when
   that something moved.
 * Fix a bug that could produce update cascades with update policy
   UDP_TIMES.
 * Fix delivery and distribution to grant packing bonus only to
   efficient sectors, just like move.  Previously, delivery ignored
   efficiency, and distribution behaved as if it were an efficient
   sector without special bonuses.
 * New sector selectors loyal, access; ship selectors radius, access,
   xbuilt, ybuilt, builder, name; plane selectors radius, access,
   theta; land unit selectors radius, react, nland, access; lost
   selector id.
 * Fix sector selectors che, che_target; plane selectors att, def;
   nuke selector types; treaty selector exp; news selector time;
   commodity selectors type, xbuy, ybuy.
 * Option PLANENAMES is no longer optional.
 * Ensure config parameter variables and their description in
   configkeys[] are consistent.  Inconsistencies there caused bugs in
   the past.  From Marc Olzheim.
 * Complain if econfig file can't be read.
 * Better messages for errors in econfig file.
 * Remove unused econfig keys hard_tech and last_demand_update.
 * New econfig key news_keep_days to configure news expiry.
 * New econfig key anno_keep_days to configure announcement expiry.
 * Fix announcement expiry.  It leaked file descriptors and memory,
   and could corrupt the anno file.  From Ron Koenderink.
 * New command xdump: experimental extended dump.  Disabled for now.
 * Bankers no longer round down debts, and they collect interest for
   the time debtors are idling at sub-prompts.
 * Loan interest calculation was broken at the due date and after a
   last payment made at the due date.  With luck, debtors could abuse
   this to repay loans cheap (they couldn't gain cash, though), sharks
   to purchase loans cheap, and creditors to collect excessively.
 * Fix major bug in transport that allowed two cooperating countries
   to duplicate items.
 * Don't report interdiction damage twice when transporting planes.
 * Fix message for attempting to transport a plane in a sector not
   owned by the player.
 * Fix confusing diagnostics when refusing to improve defense because
   option DEFENSE_INFRA is off.
 * Simplify confusing code in budget.
 * Fix spacing in output of budget and show.
 * Change one-way plane sorties to match ordinary sorties: offer
   carriers regardless of the sector they're in, require carriers to
   be efficient.
 * Allow `realm #'.
 * Don't print owner of incoming missile twice when intercepting it.
 * Simplify constituent storage in product characteristics.
 * Replace remaining `variables' leftovers; mainly in unit
   characteristics.
 * Remove leading spaces in deity unit dumps.
 * Replace inappropriate uses of compile time constant ETUS by 60.
   Deities don't change ETUS, they change etu_per_update by editing
   econfig.  This nonsense dates back at least to chainsaw 3.31.
 * Change ship production (fishing and drilling for oil) to match
   sector production.  Ship efficiency and tech are now relevant, and
   all people on board work.  From Ron Koenderink.
 * Code cleanups.
 * Minor info file fixes.

Changes to Empire 4.2.17 - Mon Aug 16 16:21:53 UTC 2004
 * Fix seeding of PRNG under Windows.  The bug made updates occur at
   a predictable time in the update window.  From Ron Koenderink.
 * Close major loophole in drop that allowed players to determine
   whether an arbitrary sector is sea, allied land, or other land.
 * fly could fly civilians into occupied sectors.  For consistency
   with other means to move around stuff, this is no longer possible.
 * Don't allow flying to a carrier that doesn't have space for the
   planes.  Previously, planes that did this were teleported home,
   which could be abused.
 * Close loophole in bomb that allowed players to find all
   sanctuaries.
 * Fix move and explore not to crash when moving civilians to a sector
   without civilians, and interdiction destroys them all.  Found and
   debugged by Jeff Cotter.
 * Fix printing of messages entered with turn command.  '%' characters
   could cause havoc, even crashes.
 * Use sector's actual maximum population to compute chance of plague
   outbreak.  Previously, code used 9999 for big cities and 999 for
   other sectors, and info Plague said 999.  This makes all crowded
   sectors equally vulnerable, even mountains, plains, and inefficient
   big cities.
 * Oil derricks no longer drill and deplete more oil than they can
   hold.  From Ron Koenderink.
 * Version 4.2.15's coordinate and list parsing fixes reject trailing
   junk.  That's too strict.  Silently ignore trailing junk as long as
   it starts with whitespace.
 * Version 4.2.15's selector rewrite broke some type literals,
   e.g. `?des=n' was interpreted as `?des=newdes'.  Change to prefer
   type literals to selectors.
 * Nightly build patches game to get repeatable PRNG results.  From
   Marc Olzheim.

Changes to Empire 4.2.16 - Wed Jun 16 18:56:16 UTC 2004
 * Previous version broke sector production when part of the work is
   used for sector construction.  Fix courtesy Jeff Cotter.
 * Add rudimentary tests to nightly build.  From Marc Olzheim.

Changes to Empire 4.2.15 - Wed May 26 17:55:58 UTC 2004
 * test command no longer asks whether to abandon the sector.
 * explore command could generate a bogus `lost' record when it
   regains an abandoned start sector.
 * edit command now properly updates stats when editing tech level of
   ships, planes or land units.  A few of these stats can be edited
   separately, which doesn't make much sense, and is now deprecated.
 * edit command no longer accepts efficiency values that would destroy
   the edited unit.  Deities can still delete units by setting the
   owner to zero.
 * upgrade command used to charge an amount of avail that didn't match
   info upgrade.  Neither the implemented nor the documented amount
   made much sense.  Change it to 15% of the full build cost.  Rewrite
   info upgrade.
 * Commands upgrade and build could be off by one when reporting
   required avail.
 * upgrade command reported rounded cost but charged unrounded
   cost.  Round cost of upgrade down to match behavior of build.
 * Upgrading planes now preserves range when it is less than the
   maximum range.  It used to set it to the new maximum range.
 * show bridge and show tower no longer claim bridge building requires
   `workers' (whatever that is), since it doesn't.
 * Replace option ROLLOVER_AVAIL by econfig parameter
   rollover_avail_max, which is the maximum amount of avail that rolls
   over.  Previously, avail was limited after rollover, and even when
   ROLLOVER_AVAIL was off.  Fix that.
 * Ships weren't always charged mobility for firing with option
   NOMOBCOST off.
 * Fix shell use of return torpedoes.
 * designate no longer complains twice about deity only designations.
 * Ensure designate zaps efficiency only when sector type really
   changes.
 * Fix strength to always report mines to deities.
 * Remove option SHIP_DECAY.
 * Fix bug in sonar that could crash or yield unpredictable results
   near the true origin.
 * Fix buffer overflow in flash and wall.
 * Fix wall not to strip out first word of message.
 * Change market and order to use item types instead of mnemo
   characters internally.
 * Market now reports sales with full item names instead of single
   letter codes.
 * buy now requires first argument to be an item name, as documented.
 * production command handled production backlog incorrectly, failed
   to limit production for production efficiency greater than one,
   e.g. agribusiness, and incorrectly limited raw material
   consumption.  Also fix a rounding error.  With help from Ron
   Koenderink.
 * Sector work was accidentally truncated to even.
 * Fix coordinate parsing for large numbers when WORLD_X or WORLD_Y
   don't divide SHRT_MAX+1.
 * Properly diagnose over-long unit lists.
 * Rewrite selector code.  Old code let players smash the stack by
   supplying too many conditions.  New code properly rejects ambiguous
   abbreviations and prefers exact matches.  Selectors with names that
   collide with types, like land units' rad and spy, now work.
   Selectors with non-integral values, like land units' att, now work.
 * survey used to display hundreds for everything but variables.  Now
   it displays hundreds only for values that can be large.
 * Rewrite info survey.  Document ability to survey properties other
   than items and resources.
 * Rewrite info build, since it was inaccurate and poorly structured.
 * Fix info fire on required military.  From Marc Olzheim.
 * New fairland argument -R to set the PRNG seed.  Print the seed.
   This allows you to recreate the same world again.
 * New facilities to deal with internal errors.  These crash when the
   server is started with -d, which is appropriate for debugging, and
   potentially disastrous for running a real game.
 * Automated nightly builds.
 * Code cleanups.

Changes to Empire 4.2.14 - Mon Mar 22 09:04:12 UTC 2004
 * Options NEWPOWER, NEW_STARVE and NEW_WORK are no longer optional.
 * Gripe about unknown options in econfig.
 * Remove option GRAB_THINGS.
 * Utility program files takes new option -f.
 * Remove utility programs land, ore and perfect, which don't compile
   since 4.2.0 (Oct '98).
 * fairland no longer warns when it added resources, but when it
   didn't.
 * Fallout code could claim destruction of more items than actually
   existed.
 * Fix recursive resupply (resupply from supply land unit triggering a
   resupply of the land unit) when just testing for how much can be
   resupplied.  The bug could have led to insufficient resupply.
 * Fix recursive resupply not to consume all fuel in supply units.  No
   supply units consuming fuel exist in the stock game.
 * Limit delivery and distribution thresholds to ITEM_MAX (9999).
   Distribution threshold were previously limited to 10000, and
   delivery thresholds to 65528.
 * Limit items in sectors to ITEM_MAX (9999).  Commands give and buy
   used to limit to 9990.  Market transactions and commands explore,
   move, drop and fly to 32767.  Limiting in autonav code (order
   command) didn't work.  Item delivery used to stop at 9990,
   distribution at 9999.  Food growing and sector production used to
   stop at 9999.  Grind, assault lunchboxes, demobilization, mine
   sweeping and che liberating uw could theoretically attempt to
   increase items beyond 65535, which would have been ignored
   (destroying the increase).
 * Properly saturate fallout at FALLOUT_MAX (9999).  Previously,
   fallout increases behaved inconsistently.  Increases beyond 65535
   could get ignored, or fallout could saturate at 9999, or it could
   overflow at 65536 (that would take quite some nuking, though).
 * Properly saturate mines at MINES_MAX (65535).  Previously, a mining
   operation that would have gone beyond 65535 was ignored.
 * Fix `are we loading or unloading' tests for load, unload, lload and
   lunload.
 * Fix editing of land unit attack and defense values.  Implement
   editing of ship defense values.  Contributed by Ron Koenderink.
 * edit now edits che number and target separately.
 * Fix edit to properly extend unit files.  Contributed by Ron
   Koenderink.
 * Major internal reorganisation: Sectors need space for items,
   deliveries and distribution thresholds.  To save space, the
   ancients invented `variables': a collection of key-value pairs,
   missing means zero value, space for `enough' keys.  This
   complicates the code, as assigning to a `variable' can fail for
   lack of space.  Over time, `enough' increased, and for quite some
   time now `variables' have been *wasting* space.  Replace them,
   except in unit characteristics, where they are read-only, and will
   be replaced later.  This also removes misfeatures like plague
   outbreaks silently deleting deliveries.
 * Remove disabled commands dissolve and mult.  Remove configuration
   parameters minmult and maxmult.
 * trade command indexed an array out of bounds when attempting to
   display the plague as commodity.  Fix.
 * New make goal interix.  Contributed by Ron Koenderink.
 * The type selector now prefers an exact match to a partial match.
   ?type=sb now selects sb, not sbc, even when sbc happens to be found
   first.
 * Value of selector sell was always zero, remove it.
 * Clean up selector code.
 * satellite now displays all sectors owned by other players, and none
   else.  It used to display all but sea, mountains, sanctuaries,
   wasteland and wilderness.
 * map commands now display wilderness and plains owned by other
   players as '?'.  Radar commands now display all wilderness and
   plains as '?'.
 * Fix power timestamp for Windows.  Contributed by Ron Koenderink.
 * Make client abort the current command on ^C under Windows, as
   everywhere else.  Contributed by Ron Koenderink.
 * Command line parser interpreted trailing whitespace as an another
   (empty) argument.  Ignore it instead.
 * Properly declare stuff in headers, include proper system headers.
   Remove all nested external declarations.  Source code is now fully
   prototyped.  Enable appropriate gcc warnings.
 * Fix several type errors.  Some may have been show-stoppers on big
   endian machines where sizeof(long) != sizeof(int).
 * Remove pre-ISO C compatibility cruft (hey, it's been only fifteen
   years).
 * Various portability fixes.  Some bugs may have been show-stoppers
   when sizeof(long) != sizeof(int).  With help from Marc Olzheim and
   Ron Koenderink.
 * Move some code to more logical places.  Remove some unused code,
   some of which was hard to port.
 * The files program could print error messages to a log file in some
   circumstances.  Make it print to stderr.
 * deliver can now change direction without changing the threshold.
 * deliver now provides appropriate context when prompting for
   thresholds.
 * Login command play no longer ignores up to two arguments if there
   isn't a third one.
 * Fix payoff (was broken in 4.2.13).
 * Capability fish didn't work together with capability oil.
 * Fix and clarify plague outbreak chance in info Plague.
 * Fix RES_POP maximum population in info Research.
 * Fix info telegram and info announce on ~ escapes.
 * Update info Selector to match code.
 * Document all undocumented unit capabilities and stats.
 * Document submarines' ability to carry landspies.
 * Fix info shoot on security units.
 * Various small info file fixes.
 * Many fixes in the troff macro package for info files.
 * Many fixes for translating info files to HTML.
 * Many info file markup fixes.

Changes to Empire 4.2.13 - Fri Feb 13 22:12:36 UTC 2004
 * Source code reindented and cleaned up somewhat.
 * Various small info file fixes.
 * Fixed change requiring 254 BTUs even when not charging any
   (BLITZ).
 * Fixed tactical non-marine missile crash.
 * Fixed spy unit detection chance for eff<100.
 * Fixed output of sorder.
 * `edit' can now work non-interactively.  Contributed by Ron
   Koenderink.
 * Fixed and extended edit for sectors.
 * Fixed use of errno.
 * Minor security fix: doconfig now writes example auth entry as
   comment.
 * Fixed reading of country name and password in client for Windows.
 * Replace use of some obsolete non-portable library functions by
   portable equivalents.
 * designate now warns about redesignated capital only when it
   succeeds in redesignating the capital.
 * attack could be off by one when reporting required unit mobility.
 * Fixed command matching.  Unique prefixes were not always
   recognized, and junk suffixes were ignored.
 * Removed bestdistpath.  bestpath yields the same results.
 * Renamed lstats to lstat for consistency and to match info page.
 * Fixed non-portable fd_set * arguments of select().
 * New `map' flag `h' to highlight own sectors.  Contributed by Marc
   Olzheim.
 * Fixed `sabo' reporting damage to player in deity coordinates.
 * New option ROLLOVER_AVAIL.  Contributed by Drake Diedrich.
 * Fix Windows server shutdown on reading "quit" from stdin.
 * Land units now fortify automatically when mobility plus growth
   exceeds the maximum.  With code from Marc Olzheim.
 * Wandering che are less predictable.
 * Land units no longer regenerate mobility faster while it is
   negative.
 * Fixed emp_client -k.
 * No longer allow pirates to ferret out where a ship was built.
 * Fixed map commands to reject bmap flags `t' and `r' instead of
   turning into bmap.
 * Fixed lmine shell resupply.
 * News no longer call all sub-launched missiles nuclear.
 * Incoming missiles are now reported with coordinates.
 * Removed some code that served no purpose except crashing on
   Windows.
 * Fixed Mac OSX and AIX build problems.
 * Don't resolve player IP addresses for now, because it can crash
   with certain versions of GNU libc.
 * Fixed recording of lost nukes.

Changes to Empire 4.2.12 - Mon Aug 18 16:54:21 MDT 2003
 * Corrected contact information for Wolfpack.
 * Sector isn't abandoned until move or explore is complete.
 * Fixed multiple cases where return value of getstarg() was
   used without being checked which crashed the server.
 * Used stdarg.h instead of varargs.h.
 * Call only_subs and save result before mission frees attacker
   list.  Call with free list could cause server crash.
 * Fixed spy sat map for units on opposite side of world wrap.
 * Don't sleep in main thread when called from signal handler.
 * Fixed error checking in pthreads.
 * Properly detach pthreads.
 * Pass unlocked mutex to pthread_mutex_unlock.
 * Cope with interrupted sleep in pthread empth_sleep to prevent
   double update.
 * Added server configurable update window.
 * Initialized variables passed to setsockopt in accept.c.
 * Fixed bug where sharing bmap with uncontacted country crashes
   server.

Changes to Empire 4.2.11 - Sun Sep  1 09:54:59 MDT 2002
 * Added compile option for the Mac OSX architecture.
 * Fixed bug in cargo where unit array was indexed by shp_maxno
   instead of lnd_maxno.
 * Fixed bug in reject where aborting the command crashes the
   server.
 * Can't buy land units into enemy headquarters.
 * Added sabotage command and commando unit.
 * Spies moving by themselves will not trigger interdiction.
 * Fixed error in map distance calculation that caused errant
   interdiction.
 * Ships going under -127 mobility will be set to -127 mob and
   not roll over to positive mobility.
 * Corrected plane cost in info Maintenance.
 * Changed error message for loans rejected for being too big.
 * Land units will take casualty damage proportional to their
   ability to carry mil.
 * Fixed bug where "route i *" crashes server.
 * Cannot upgrade planes in orbit.
 * Fixed bug in declare with deity aborting command at last
   prompt crashes the server.
 * Coastwatch notify works with NO_FORT_FIRE option.
 * Thread that kills idle connection will charge player for
   minutes the player was logged on.
 * Removed separate sendeof at end of execute from emp_client
   that logged players off on certain platforms.
 * Updated player idle counter during read prompt so players
   wouldn't be kicked off during long flashes or writing telegrams.
 * LOSE_CONTACT will work as advertised.
 * Allied units marching through 0 mobility conquered sectors
   are charged at least LND_MINMOBCOST.

Changes to Empire 4.2.10 - Mon Aug 13 14:34:39 MDT 2001
 * Assault can reinforce own sector when SLOW_WAR is enabled.
 * emp_client no longer converts lines into tags but still verifies executes,
   pipes, and file redirect match players input.
 * Added pboard command to capture enemy planes in your territory.
 * Declaring war when at Sitzkrieg won't return relations to Mobilizing.
 * Declaring war won't charge money if you're already Mobilizing.
 * Added multiple territory fields.
 * "show nuke build" displays the proper avail.
 * retreating land units will only retreat to sectors owned by the player
   that owns the unit.
 * added hard cap of "250 + easy" to limit_level.
 * education p.e. calculation divides edu constant by etu per update.
 * Added patch for Linux for the PowerPC.

Changes to Empire 4.2.9 - Sun Jan  7 15:49:13 PST 2001
 * Fixed data corruption when bogus target gets fired upon.
 * Increased size of ancillary stacks to prevent stack overflow.
 * Changed all references to empire.cx.
 * Ships, planes, and land units lost to lack of maintenance will now be
   lost.
 * No longer allowed to sack deity's capital.
 * Accuracy calculation in land unit support was backwards.
 * Corrected formula for likelihood of plague in info Plague.
 * Fixed files to correctly size map and bmap files from econfig file.
 * No longer allowed to add country 0.  It corrupts deity country.

Changes to Empire 4.2.8 - Mon Oct  9 17:35:35 MDT 2000
 * Fixed range in radar.t and Sector-types.t.  Maximum range at infinite
   tech is 16.
 * Changed so that player can't drop civilians into occupied sectors.
 * Fixed bug that crashes update if etus/update is less than 8.
 * Fixed sometimes not reporting overflights
 * Fixing flak for units/ships to be in all sectors as they should be
 * Updated order of firing flak in Flak.t.
 * Fixed 80% efficient planes on ships must be maintained.
 * Fixed initial MOB_ACCESS check thread sleep time was set by an uninitialized
   variable.
 * Removed "TEMPORARILY DISABLED" message from mission command for escort
   missions.  Escort missions have been re-enabled since 4.0.15.
 * Using sonar from the navigate command will print the sonar map.
 * Eliminated annoying error messages when using navigate with conditionals.
 * Fixed Technology.t to correctly add 1 before taking the logarithm.
 * Fixed bug that allowed players to steal opponents maps.
 * Put time limit for bidding on market and trade in econfig file.
 * Moved hours file information into econfig file.
 * Added comments on setting update policy in econfig file.
 * Added logging of cases where write extends data file by more than one id.
 * Option MARKET is turned off by default.

Changes to Empire 4.2.7 - Fri Mar 10 18:41:04 PST 2000
 * Added new NF_SACKED flag to indicate when a country has had it's
   capital sacked.  This flag is only cleared when the country actively
   redesignates it's capital using the "capital" command, and changed
   capital.t to reflect this (no more re-sacking after an update if the
   country doesn't reset and repeating until they are out of cash.)
 * Added land unit to edit command for planes.
 * Added nuketype to edit command for planes.
 * Changed so that if the leader of a group of units is a train, the
   railways are used for bestpath instead of roadways.
 * Changed mission command to also print reaction radius of reserve
   missions when set for land units.
 * Changed Flak.t to reflect guns having to be loaded onto a land unit
   for flak to fire.
 * Changed so that visitor countries cannot change their name or password.
 * Changed so that you cannot upgrade deity planes, ships or units.
 * Changed so that you know where missions are flown from.
 * Changed so that the airport owner you fly a mission from is informed.
 * Changed so that preperations for takeoff are reported to the owner of
   the sector, ship or unit a plane is taking off from if it is not owned
   by the owner of the plane.
 * Changed so that when a sector reverts during a guerrilla revolt, the
   mobility is not reset.  When MOB_ACCESS is not enabled, this used to mean
   you would always get at least an updates worth of mobility.  With
   MOB_ACCESS, sectors that reverted during the update were useless, since you
   got no mobility for a long time.  This fixes that problem, and makes che
   more useful as well.
 * Changed so that air defense missions don't always send up every plane in the
   area, but instead always send up at most two times the number of incoming
   planes (for each interdicting country.) This helps to stop 1 plane from
   stripping the mobility from 25 air defense planes all at once, but doesn't
   limit it to n+1 as there is for regular interception.
 * Clarified deity reversion of sectors in Sector-ownership.t
 * Clarified production efficiency and added pointer to "show sector cap"
   in Product.t
 * Clarified nuketype selector for planes in Selector.t
 * Clarified that fortifying units does not affect mission status in info
   pages for mission and fortify.
 * Clarified fuel syntax upon error in fuel command in empmod.c.
 * Cleaned up comments causing minor complaints in some builds.
 * Fixed bug in loans where you could collect on proposed loans.
 * Fixed production command to be more accurate (though it *still* has some
   rounding errors, it's better.)
 * Fixed potential memory leak in air defense missions.
 * Fixed bug where you couldn't pinbomb some commodities if other commodities
   didn't already exist in the sector.
 * Fixed bug where you couldn't build 0 crew planes without military in the
   sector.  (This is different from needing at least 1 military to build all
   planes that need a crew which introduced another related bug earlier.)
 * Fixed bug where scrapping land unit 0 could cause erroneous transfer
   messages to be displayed for deity owned planes.
 * Fixed bug where land units on ships in a sector taken over were blown
   up or captured.
 * Fixed bug where planes on ships in a sector taken over were blown up
   or captured.
 * Fixed bug in sdump printing origx and origy in deity coordinates.
 * Fixed bug in satellites showing wrong sectors (sometimes) when using the
   optional arguments.
 * Fixed bug in launch showing wrong satellite target (showed asat instead
   of the target satellite.)
 * Fixed bug where you could gain information not normally available using
   the "fire" command to determine sector type information.
 * Fixed bug in market/trade creating extra money when loans are taken out.
 * Fixed problem with land units not being reported sunk after being sunk
   on a ship that was nuked or wastelanded in a sector.
 * Fixed problem with Spies.t using "llook" instead of "llookout".
 * Fixed problem in lwp/arch.c including jmp_buf.h instead of setjmp.h
   on some Linux boxes.
 * Fixed problem with plague infection being way too high in cities when
   the BIG_CITY option is enabled.
 * Fixed bug in edit command getting confused with arguments.
 * Fixed bug in ask_move_in_off asking you to move in a negative number of
   troops (hey, it could happen. :) )
 * Fixed bug in add command not keeping flags or relations initially correct.
 * Fixed bug in torpedo command sometimes telling victim about torpedo
   sightings even when way out of range.
 * Fixed bug in spy command not reporting planes in adjacent sectors, as well
   as not formatting them properly when reporting them.
 * Fixed bug in satellite with noisy transmission causing a potential
   crash of the server with non-100% efficient satellites.
 * Fixed potential crashing of the server during satellite display when not
   calculating distances to ships and units correctly.
 * Fixed bug where planes in non-allied sectors, ships and units could be
   used on missions.
 * Fixed bug where planes on the trading block could be used on missions.
 * Fixed bug where planes and units that get traded don't have mobility set
   correctly when using MOB_ACCESS option.
 * Fixed bug which made security units virtually useless.
 * Fixed access times and mobility not being set right when building planes,
   ships, units, bridges and bridge towers when MOB_ACCESS is enabled.
 * Fixed minor formatting problem with launch of satellites.
 * Fixed minor formatting problem with planes with greater than 999 tech.
 * Fixed minor formatting problem with ships with greater than 999 tech.
 * Fixed problem with "llook" showing up in TOP info file.
 * Fixed problem with lwp Makefile choking on NT builds with clean target.
 * General cleanup of potentially ambiguous statements.

Changes to Empire 4.2.6 - Fri Jun  4 05:55:20 PDT 1999
 * Added "TECH_POP" as an option where technology costs more to make
   as your population grows past 50K civilians.  It is disabled by
   default.
 * Changed "produce" command to accurately print what the true p.e. is.
 * Changed "update" command to display if mobility updating is enabled for
   MOB_ACCESS option.
 * Fixed bug where toggling off the coastwatch flag also turned off
   forts firing on hostile ships coming into range.
 * Fixed bug where assaulting your own land would violate any treaties
   you have where assaults are a violation.
 * Fixed bug where all planes (even those without need for a crew, such
   as missiles) needed at least 1 military to build.
 * Fixed bug where when a ship sinks during the update due to lack of maint,
   land units and planes on it were left stranded.
 * Fixed bug where when a land unit dies during the update due to lack of
   maint, land units and planes on it were left stranded on it.
 * Fixed bug where nukes could be lost due to MOB_ACCESS updating mobility
   while arming.
 * Fixed bug in "show sector capabilities" not showing products correctly.
 * Fixed bug in "show tower build" printing "bridges" instead of "bridge
   towers".
 * Fixed bug in sectors that don't revolt not showing up as lost items.
 * Fixed bug where maps with an X of exactly 200 is not drawing third line.
 * Fixed bug where MOB_ACCESS was not updating the mobility just before
   the update.
 * Fixed bug in the way treaties are examined and sometimes produce
   wrong results.
 * Fixed edit to allow creating negative mobility for sectors.
 * Fixed setsector to allow creating negative mobility for sectors.
 * Fixed bug where when writing out the value of a sector that had
   negative mobility and was damaged in combat, mobility was being
   set back to 0.
 * Fixed Taxes.t info page to reflect that captured civvies only pay
   1/4 taxes.
 * Fixed Technology.t info page to reflect TECH_POP option.
 * Fixed navigate.t info page to reflect that only ships in the fleet
   in the same sector that are fired upon have damage divided up.
 * Made techlists toggle on by default (so things are shown in order of
   technological advances.)

Changes to Empire 4.2.5 - Mon Mar  1 06:42:24 PST 1999
 * Added optimization to increasing mobility to check if an object is
   already at max mob, just return since it can't be increased.
 * Added "-ltermcap" for client libs for hp build (it was already in
   hpux build.)
 * Added clearing of telegram flags after the update so that the next
   telegram is flagged as new and not part of the update.
 * Fixed Update-sequence.t to reflect MOB_ACCESS.
 * Fixed bug where fortification amount was not being limited to
   maximum mobility for land units (land_mob_max).
 * Fixed bug where land unit fortification strength was being calculated
   by using 127 instead of land_mob_max.
 * Fixed bug where scrapping land units was creating military.
 * Fixed description of sect_mob_neg_factor in econfig file.
 * Increased speed of PT boats.

Changes to Empire 4.2.4 - Tue Feb  2 05:47:44 PST 1999
 * Added check to make military values match up correctly for land
   units.
 * Fixed bug in doconfig not putting ipglob.c in the right place.
 * Fixed bug where attacking deity sectors will violate a treaty.
 * Fixed newspaper.t information file.
 * Fixed potential bug in fixing up timestamp information when restarting a
   game with MOB_ACCESS turned on.
 * Fixed bug in explore command not setting mobility to correct
   value when MOB_ACCESS was enabled.
 * Fixed bug in enlist setting mobility incorrectly sometimes.
 * Added doc/backup file for deities which recommends how backups and
   restores of the data directory should be done.

Changes to Empire 4.2.3 - Wed Jan 13 06:02:35 PST 1999
 * Added linux-pthreads target and build for using pthreads under Linux.
 * Added NO_FORT_FIRE option which disables the ability of forts to
   fire when enabled.
 * Added more error checking and recovery for corrupt data files.
 * Changed alphapos target to alpha-pthreads for better clarity.
 * Changed fairland to allow 0 sector distance to other islands and
   continents (James Risner)
 * Changed "frg" and "dam" land unit stats to "rng" and "fir" so they
   match the way ships are described (since this is how they really work.)
 * Changed Unit-types.t to now describe "rng" as firing range of a unit,
   and "fir" as the number of guns that a land unit fires.
 * Cleaned up misc. build warnings.
 * Changed artillery damage to be 5 + d6 per gun firing from just d6
   per gun firing and updated Damage.t to reflect this change.
 * Changed artillery firing ranges to be like ships ranges - divided
   by two, and modified them to make more sense.
 * Changed artillery units to be "slightly" :) more powerful.
 * Changed "cavalry" unit to tech 30 and lowered mil content to 20.
 * Changed "artillery" unit to tech 35.
 * Changed so that guns are no longer required to build units, and
   that guns must be loaded onto artillery units for them to fire.
   Client developers note: the show command has not changed yet to
   remove the 'guns' column (since guns are no longer required) but
   will in a future revision (possibly 4.2.4) so "be prepared" for
   "show land build" to change. :)
 * Fixed landunitgun to handle all the damage calculations like it should.
 * Fixed bug sinking planes when the ship they are on sinks.
 * Fixed bug destroying planes when the land unit they are on is destroyed.
 * Fixed bug where land units could fire support without enough military.
 * Fixed bug in abandoning sectors by marching out a land unit where it
   would not let you sometimes (uninitialized variable problem.)
 * Fixed gets problem in files.c (James Risner)
 * Fixed bug in determinig operations range of a ship.
 * Fixed bug in building planes where you could manufacture military.
 * Fixed warnings in threading package(s).
 * Fixed fire.t to reflect new firing changes.
 * Fixed lstat.t to reflect new firing changes.
 * Fixed sstat.t to reflect the way things really work.
 * Fixed Ship-types.t to reflect the way things really work.
 * Fixed the way shutdowns work to hopefully better protect data files.
 * Fixed bug in attacking when sector mobility is less than 0 and it would
   prompt for attacking with a negative amount of military.
 * Fixed typo in fire.t stating wrong parameters for firing from a sector.
 * Fixed bug in client when it prompts for country name it was putting
   an extraneous end of line on the end that needed to be stripped off.
 * Fixed problems running on Linux running on an Alpha machine.  Thanks
   to Rocky Mountain Internet and Jeremy A. Cunningham for giving us time
   on a machine to work out the bugs.  (Note that the only build that
   works on Linux/Alpha is the linux-pthreads)

Changes to Empire 4.2.2 - Sun Dec 27 12:46:34 PST 1998
 * Added some code optimizations into the update code when building paths.
 * Added some new cacheing for building paths to help speed up updates.
 * Added so that you can use "name" and "password" with the change command
   as well as "country" and "representative".
 * Added flag to power command so that if you are a deity and enter
   a negative number of countries you want to see, you see the power chart
   for that many countries without power numbers on the next line.  Only
   useful for deities that want to see the chart un-broken up by that line.
 * Added new functionality to fortify command.  You can now use a negative
   fortification value to cause the unit to be fortified and leave at
   least that much mobility on the unit. I.E. "fort * -67" will fortify
   all units and make sure the mobility of each unit doesn't go below
   67.  If the mobility is already below that level (or equal) the unit
   is left unchanged.
 * Added that the realm command prints "Realm #n is x:x,y:y" after
   you set a new realm.
 * Added TREATIES option and enabled it by default.
 * Added "no depth-charging subs" and "no new land units" treaty
   clauses.
 * Added Trannie Carter's basic client fix to use fgets instead of gets.
 * Changed market and command to only print the lowest priced lot of
    each given commodity by default.  If you specify "all" it shows
    all lots on the market, and if you specify a specific item, it shows
    all lots of that item type.
 * Changed start command to only write out sector if it changed.
 * Changed stop command to only write out sector if it changed.
 * Changed how plane names were changed on server startup if the
   PLANENAMES option was enabled.
 * Changed move command to use standard askyn function for abandoning
   sector prompt.
 * Changed plane overlight sightings to take stealth into account,
   and if the planes managed to evade all flak and interceptors, they
   are not marked as "spotted" over enemy sectors.
 * Changed all units with the supply flag to have their marching speeds
   based on efficiency since that is their purpose.  What this means is
   that supply units and trains are more effective at 100% than at 10%
   (just as fighting units are more effective at 100% than at 10%, but
    their effectiveness is determined by how well they fight, not how well
    they "run away, run away!" :) )
 * Changed so that flash toggle is on by default for POGO at setup
   time.
 * Changed so that when a sector is taken, all land units owned by the
   current owner are treated as planes are during takeovers (i.e. there
   is a pretty good chance they are blown up, and if not, they change owner
   to the attacker and are beat up pretty good.)
 * Fixed treaties to work again.
 * Fixed bug in taking over land units not using correct pointer (can
   cause a crash or data corruption.)
 * Fixed collect command to wipe deliver and distribution information
   correctly.
 * Fixed potential bug in bmap not working when destination bmap has
   blank spaces in it.
 * Fixed bug in shark command not getting right nation structure.
 * Fixed bug in server where empty commands (for example, all spaces
   or tabs) were being reported as bogus commands.  They are now just
   ignored.
 * Fixed bug in scrap not dropping land units off of scrapped units carrying
   them.
 * Fixed bug in news where boarding of land units was reported backwards.
 * Fixed bug in calculating new people when growing them during an update.
 * Fixed missing -ltermcap in HP/UX build.
 * Fixed setsector to limit mobility to 127 not 255 (255 was being caught
   later and being reset to 0 anyway)
 * Fixed Sector-types.t to show a '\e' instead of a '/' for wasteland,
   since that is what it really is.
 * Fixed retreat.t documentation to reflect the "retreat upon failed
   boarding" flag.
 * Fixed bug where satellites were not orbiting the world during the
   update.
 * Fixed bug in torpedos being too smart.  They knew how to jump over
   land! :)  Now the "line_of_sight" routine is used to determine if a
   torpedo has a straight path to the destination.  If it doesn't, the
   torpedo no longer jumps over land, but instead slams into it.  This was
   an interesting bug because you could torp ships on the other side of
   a very skinny island as long as they were in range, even if there was
   no sea route possible.
 * Fixed bug when pinbombing and you run out of an object to pinbomb (land
   units for example) but you still have to pick something to bomb.  For
   ships it worked ok (just aborted that it couldn't find any more ships)
   and this was fixed for land units and planes.
 * Fixed server crashing bug when flying a plane and not carrying
   any cargo.
 * Fixed bug in move losing commodities when it runs out of room in the
   destination sector.  Goods are now attempted to be returned to the
   start sector, and apporpriate steps taken if the start sector is no
   longer available.
 * Fixed march to prompt you before you abandon a sector you own by
   marching out the last land unit.
 * Fixed check functions to only check the relevant portions of the structure
   and not the timestamp info that doesn't affect how the object functions.
 * Fixed bug in spy command that always told you if a spy unit was in a
   non-owned sector.
 * Fixed bug in displaying of land unit missions not showing correct land
   unit range.
 * Fixed bug in wire command where new announcements that you read at
   the last second don't get wiped out properly.
 * Fixed bug where announcement file could be corrupted by very long lines.
 * Fixed bug where bmap was not set when player was told what kind of
   sector they were attacking (this was an old abusable bug that was
   removed long ago to fix the abuse, and it's been fixed in a non-abusable
   way finally.)
 * Fixed bug where change command would warn you about monetary and
   BTU costs, but not prompt you to break out if you didn't want to really
   change your country name.
 * Fixed bug in sub-launched nukes that are intercepted being reported in
   the news incorrectly.
 * Fixed bug in load where you could abandon a sector and not know it by
   loading your last civilians or military onto a ship.
 * Fixed potential memory leak in autonav code.
 * Fixed potential bug where you could possibly determine if a sector is
   owned or not using one way plane missions.
 * Fixed Damage.t info page to properly show damages for planes and
   land unit shelling.
 * Fixed deliver.t to include syntax for command.
 * Fixed country.t to include syntax for command.
 * Fixed bug where ships on orders were not adding radar information to
   the bmap during an update.
 * Fixed bug where ships on sail were not adding radar information to
   the bmap during an update.
 * Fixed bestownedpath code to use the bmap properly.  Note this is a very
   important bug fix.  When navigating a ship, players are no longer given
   free information that they would not normally know.  For example, if you
   try sailing your ship off into uncharted areas of your bmap, the bestpath
   code will only use as much information as you have on-hand (i.e. your
   bmap) to plan out your path for you.  If you have no information on an
   area, it just keeps forging on, until bumping into something.  Of course,
   after the initial exploration through an area, the bmap will be set and
   used for all future sailings through that area.  Harbors and bridges
   are still checked for construction worthiness if you know where they
   are (i.e. they are on your bmap.) (Overall, what happened before
   was that the bestpath code would route your ship around islands that
   you didn't even know were there, and you could use various commands
   to see how that ship was going to sail during the update and thus you
   gained information that you wouldn't normally know.)
 * Merged bestpath and bestownedpath and wrote new wrappers, "BestShipPath"
   and "BestAirPath" to use it properly.  It is also good to note that
   bestownedpath is used to determine paths for ships and for planes,
   and that best_path (which uses the A* algorithm) is used for all land
   based paths, and that the two are never interchanged.
 * Removed "jet recon" plane (it slipped in during the PLANENAMES conversion,
   and RECON was never used before.)
 * Removed extraneous "resnoise" and "resbenefit" functions and combined
   the two for setresource and setsector commands.

Changes to Empire 4.2.1 - Tue Nov  3 12:56:20 PST 1998
 * Fixed problem with global/plane.c not defining last structure element
   properly, and thus causing crashes when accessing certain plane
   routines.  This happened during the move to make the PLANENAMES option
   run-time configurable.

Changes to Empire 4.2.0 - Thu Oct 29 06:27:15 PST 1998
 * Bumped rev to 4.2.0 since this is a major release (the server is now
   run-time configurable for just about everything and is released under
   the GNU GPL.)
 * Put in official licensing information.
 * Re-arranged and commented the econfig (Empire Configuration) file.  The
   auto-generated "data/econfig" file is now pumped out chock full of
   comments (which may or may not be useful.)  Since the server is
   pretty much fully run-time configurable (MAXNOC is not, but it gets
   a default of 99 anyway) deities will probably be spending more time
   in the config file.
 * Added server support for building under NT, including mods from
   Doug Hay and Steve McClure to get the server building and running
   under NT. (This was built using MSVC++ 5.0 on NT 4.0 and command
   line "nmake nt")
 * Added Doug Hay's ntthread.c implementation for NT threading.
 * Added max pop column to "show sector stats" output.
 * Added lboard command for boarding land units from sectors.  Only raw
   mil can board land units, and only mil and land units aboard the unit
   being boarded fight back.
 * Added GODNEWS option.  When enabled, the deity giving people stuff
   is shown in the news.
 * Added bridge tower sector ('@')
 * Added BRIDGETOWERS option.  When enabled you can build bridge towers
   from a bridge span.  You can then build bridge spans from the tower.
   If the tower is destroyed ( <20% eff) bridges connected fall unless
   supported on the other side.  You can only build bridge towers in open
   water not adjacent to land and other towers.  Expensive.
 * Added plains sector ('~') - Max pop is 1/40th regular sectors, and
   it is deity creatable only (can't redesignate unless you wasteland it
   with a nuke :) )
 * Changed "info all" to no longer use printdir, instead it just uses the "all"
   info page which contains this information.
 * Changed so you can now load up to 2 spy units onto non-land unit carrying
   submarines if the LANDSPIES option is enabled.  Useful for sneaking up
   to shore and spying on your neighbors.
 * Changed so spies unloaded from ships are not given as gifts, they are just
   unloaded quietly.
 * Changed build command to handle building towers ("build t")
 * Changed show command to show tower stats ("show t b")
 * Changed reject so you can now reject things from any country except
   deity countries (this now includes sanctuary countries and visitor
   countries you don't want to deal with)
 * Changed ndump to dump nuclear stockpile ID as well.
 * Changed flash so that if someone allied to you is either not logged on or
   not accepting flashes, you are notified.  This is info you could gain
   otherwise since you can see other countries that are allied to you via
   players.
 * Fixed bug in news command when HIDDEN mods are enabled.
 * Fixed distribute command to only write out to the database if we
   actually changed the sector (i.e. if we really moved the dist
   center, we write.  Otherwise, it makes no point.)
 * Fixed threshold command to only write out to the database if
   we actually changed the sector (i.e. if we really changed the
   threshold, we write.  Otherwise, it makes no point.)
 * Fixed - population growth and truncation in "Update-sequence.t" info
   file is now clearer
 * Fixed the way fortify takes mobility away from a land unit if
   engineers are present.
 * Fixed bug in buying commodities at the last minute not resetting the
   time correctly in all situations.
 * Fixed bug in building nukes where it would always ask if you tried
   building more than one at a time.
 * Fixed bug where if you put (either by building or by transporting)
   more than 127 of one type of nuke in a sector, they all got lost. A
   negative wrapover sort of thing.
 * Fixed ndump to print # of stockpiles dumped.
 * Fixed ndump.t to reflect changes
 * Fixed bug in build command that didn't account for EOL characters.
 * Fixed bug where you could use planes to drop conquered populace
 * Fixed bug in distribute command in how it checked for current distribution
   sector.
 * Fixed pr_flash and pr_inform to no longer send messages if the player
   is still in the process of logging in (i.e. not in the PLAYING state)
 * Fixed report command output for deities.
 * Fixed bug in nuclear damage either taking out submarines when it
   shouldn't, or not taking them out when it should.
 * Fixed bug in loading units that are carrying units onto other units
   (note that it doesn't happen since only HEAVY units can carry other
   units and that check works, but that might change some day, and we
   don't want units carrying units to be carried by other units, etc. :) )
 * Fixed info pages to reflect new sector types.
 * Fixed info pages to reflect new spy unit capabilities.
 * Fixed show commands to only show trade ships if the TRADESHIPS option
   is enabled.
 * Fixed build command to only allow building of trade ships if the
   TRADESHIPS options is enabled.
 * Fixed up some definitions located in many places used for checking
   sectors for navigation rights.
 * Fixed power.t to correctly describe NEW_POWER formula.
 * Made HIDDEN option run time configurable.
 * Made LOSE_CONTACT option run time configurable.
 * Made ORBIT option run time configurable.
 * Made SAIL option run time configurable.
 * Made MOB_ACCESS option run time configurable.
 * Made FALLOUT option run time configurable.
 * Made SLOW_WAR option run time configurable.
 * Made SNEAK_ATTACK option run time configurable.
 * Made WORLD_X and WORLD_Y run time configurable.
 * Made MARKET option run time configurable.
 * Made LOANS option run time configurable.
 * Made BIG_CITY option run time configurable.
 * Made TRADESHIPS option run time configurable.
 * Made SHIPNAMES option run time configurable.
 * Made DEMANDUPDATE option run time configurable.
 * Made UPDATESCHED option run time configurable.
 * Made LANDSPIES option run time configurable.
 * Made NONUKES option run time configurable.
 * Made PLANENAMES option run time configurable.
 * Removed SMALL_SERVER stuff (unused baggage)
 * Removed trading post sector ('v')

.FI
.s1
.SA "Server"
