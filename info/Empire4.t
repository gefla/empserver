.TH Server "Empire4 Changes"
.NA Empire4 "Changes from the Empire3 server to the Empire4 Server"
.LV Expert
.s1
There have been several changes/fixes made to the Empire3 code in the
new Empire4 Server.  This outlines the various changes and how they
will affect you, the player.  These were coded as the Wolfpack project,
and bug-reports should be sent to <wolfpack@wolfpackempire.com>.
.NF
Changes to Empire 4.3.6 - Wed Jun 14 18:18:34 UTC 2006
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
   Broken when Empire3 changed land unit mobility use not to depend on
   efficiency, except for supply units.
 * New journal file to log complete player input plus selected
   important events.  Controlled by new econfig key keep_journal, off
   by default.
 * Fix unsafe shutdown on SIGINT and SIGTERM.
 * Fix land unit path finding: paths weren't always cheapest (broken
   in 4.0.2), and trains could be sent down paths without rail (never
   worked).  Also fixes unit reaction path cost.
 * Improved custom table reader can now deal with sector, item,
   product and infrastructure tables more comfortably.
 * Remove option BIG_CITY.  Deities can load a suitable sect table to
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
 * Minor info file and manual page fixes.

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
 * Fix trade lot change detection in set command.  Probably abusable.
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
   e.g. while a second client attempted to connect.  Abusable.
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
 * 4.2.19 broke pin bombing commodities on some platforms.
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
 * Drop support for for DCE threads.
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
 * tele and turn now abort on EOF.
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
   so far is UTF-8.
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
 * Fix a bug in fire that allowed players to find all submarine uids.
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
 * Fix plague, which was was broken in 4.2.14: it got stuck in stage I.
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
 * emp_server and and files print a usage message on command line
   syntax errors.
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
   collide with types, like land units' rad and spy, now work.  Selectors with
   non-integral values, like land units' att, now work.
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
 * Land units now fortify automatically at the update using excess
   mobility.  With code from Marc Olzheim.
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
 * Fixed Sector-types.t to show a '\' instead of a '/' for wasteland,
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
.SA "Clients, Server, Infrastructure, Sectors, LandUnits, Planes, Ships"
