.TH Server "Empire4 Changes"
.NA Empire4.4 "Changes in Empire 4.4 (2015-present)"
.LV Expert
.s1
This document outlines the various changes to the game and how they
will affect you, the player.  These were coded as the Wolfpack project.
.NF
Changes to Empire 4.4.1 - Sat Feb 13 2021
 * Fix land unit casualties in guerrilla fighting.  Broken in 4.4.0.
 * Changes to edit:
   - You can now edit bars on ships and land units, and missile
     fortification.
   - Fix to reject fortification of embarked land units.
   - Fix to zap land unit and plane fortification on load.
   - Fix to take satellite off its carrier when moving it to orbit.
 * Changes to load, unload, lload, lunload, tend, and ltend:
   - tend land with multiple target ships now tends each land unit
     just once.
   - tend land now reports "not on ship" only when the land unit was
     explicitly selected by UID.  Matches unload.
   - Fix load, unload, lload, lunload to suppress messages and skip
     foreign ships / land units when the second argument starts with a
     digit.
   - load and unload no longer prompt for land units to unload when a
     carrier can't carry any.
   - Fix load plane to reject satellites in orbit.  The stock game
     does not have loadable satellites.
 * Fix launch to take satellites off their carrier when moving to
   orbit.
 * Shield embarked planes and land units from sector damage, like
   their ships.
 * Rewrite much of fairland to make it more robust and more fair:
   - Show a more useful map.
   - Fix error handling, fix and tidy up error and progress messages.
   - Drop option -a.  Deities can find continents with ?dterr<N, where
     N is the number of continents.
   - Drop option -o.  Deities can unset resources with "edit l * i 0 g
     0 f 0 c 0 u 0".
   - Drop a useless warning about small world.  It was downgraded from
     error in 4.3.25.
   - Check arguments more thoroughly, and don't silently "correct" bad
     arguments.
   - Fix checking of minimal distance arguments.  Has always been
     broken, but 4.3.15 made it worse.
   - Ensure the continents' first two sectors obey minimal distance.
   - Correct bias in placement of expansion islands.  These were more
     often placed to the right of existing land.  Players could
     exploit that to guide their search for land.
   - Fix island growth and correct its bias.  Growth could fail even
     when there was space to grow.  Islands tended to curve around
     obstacles in a counterclockwise direction.  Besides fixing that,
     the new code also gives better results for high spike
     percentages: it produces shorter spikes extending from a core
     rather than long snakes that tend to curve into themselves, and
     is less prone to put capitals on the coast.
   - Size and place expansion islands fairly: ensure each start island
     "owns" the exact same set of expansion islands, where "owns"
     means it's closer than any other start island.
   - To make that work, fairland now requires the number of expansion
     islands to be a multiple of the number of continents, and fails
     when it can't place all the islands the deity asks for.  You
     can't fill the world with islands anymore by asking for
     impossibly many of them.
   - Fix silent failure to place place mountains.  Fairness issue.
   - Fix unfair mountain resources.  Neglected when Empire 3 made
     mountains produce gold dust.
   - Drop undocumented, silent limit of 1000 mountains per island.
   - Location of resources is more varied, in particular for islands
     with few mountains, and for sea.  Resource remain pretty much the
     same overall.
   - Performance improves for large minimal distances, especially for
     crowded worlds.  It can suffer for huge worlds and small minimal
     distances.
   - Manual page improvements.
   - Code refactoring and cleanup.
 * Test suite improvements:
   - Fix empdump-test for "make check-accept".  Has been broken its
     addition in 4.3.33.
   - Improve give and edit coverage.
   - Cover load and tend.  There are known gaps.
   - Improve fairland coverage.
 * Game customization improvements:
   - Configuration tables now have more extra space deities can use
     for customizing their games: 31 product types, 63 sector types,
     127 ship, plane and land unit types, and 63 nuke types.
   - Work to produce stuff is now independently configurable, and
     "show product" shows it.  Before, it was a function of raw
     materials and resource use.
 * New product selector "bwork" is work to build one unit of the
   product.
 * Fix build with compilers that require the depcomp wrapper for
   dependency tracking.  Broken in 4.3.31.
 * Fix LWP signal code.  Messed up in 4.3.6, some more in 4.3.10.
 * Improve POSIX conformance for portability.  Observed to fix the
   build on Solaris 10.
 * The client's password prompt now behaves the same on all POSIX
   systems: read from standard input in canonical mode with echo
   turned off.  Before, it commonly read from /dev/tty in noncanonical
   mode.
 * The server crashed on systems when bomb, launch and interdiction
   miss the target.  Fix that.
 * Fix truncated "info Empire4.3" on systems with a losing nroff.
 * Make client's fancy line editing and persistent history work on
   more systems.
 * Fix ground combat not to wipe out commodities supplied to defending
   land units.  Broken in 4.3.33.
 * Don't permit SAMs on escort missions.  They work, apart from a
   recoverable internal error, but it makes no sense.
 * Don't permit marine missiles on support missions.  They can't
   actually go.
 * Drop support for ABM and a-sat missiles consuming shells.  The
   shell use is logistical busy-work, and economically irrelevant.
   The stock game's abm type didn't consume any, but its asat type
   did.
 * Stricter plane configuration validation.
 * Increase mountain process efficiency from 75% to 100%.  Gold
   resources in mountains are now worth the same as elsewhere, and a
   third more than before.
 * Fix crash when a deity runs neweff or production on a sea sector.
   Broken in 4.4.0.
 * Don't let embarked engineers work.  Screwed up when the command was
   added in Empire 2.
 * Fix buy error messages on concurrent lot change.
 * Fix MOB_ACCESS mobility update for ships owned by countries with
   even country number.  Broken in 4.4.0.
 * Journal entries for output are no longer flushed to disk right
   away.
 * Code refactoring and cleanup.
 * Info page, manual page and documentation fixes and clarifications.

Changes to Empire 4.4.0 - Sun Sep  3 2017
 * Don't increase negative ship and land unit mobility to zero on
   trade.
 * Client improvements:
   - Support fancy line editing and persistent history when compiled
     with the GNU readline library.  Contributed by Martin Haukeli.
   - Fix misdetection of input EOF in obscure circumstances.
   - Tighten protection against rogue server abusing redirection and
     execute.
   - Improve some error and warning messages.
 * Fix wire for deities to honor announcement rejection.  Broken in
   Empire 3.
 * Fix excessively fast contact aging with option LOSE_CONTACT.
   Broken in 4.3.0.
 * The edit command no longer shows contact with option HIDDEN.
 * Grant security units a military control bonus similar to their
   shoot and convert mobility bonus.
 * Embarked land units no longer contribute to military control.
 * Fix convert and shoot not to put foreign and embarked land units to
   work.  Screwed up when land units were added in Chainsaw.
 * A security unit's combat strength bonus for fighting guerrilla is
   now proportional to its efficiency.
 * The mobility bonus a security unit provides to shooting and
   converting civilians is now proportional to its efficiency.
 * Fix a missing newline in the edit command's "teleported" message.
   Messed up in 4.3.32.
 * Don't permit boarding of embarked land units.
 * Guerrilla fighting fixes:
   - Let rebels liberate old-owned sectors again.  Broken in 4.2.6.
   - Fix overeager destruction of defending land units.  Screwed up
     when land units were added in Chainsaw.
   - Reduce under-strength land unit damage for consistency with
     ordinary ground combat.
   - Land units without military can no longer die fighting che.  They
     can now get captured when the rebels take over the sector, just
     like in ordinary ground combat.
   - Fix the way casualties are applied to land units, because it is
     utterly bonkers.  Screwed up when land units were added in
     Chainsaw, made worse when military became loadable in 4.0.0.
   - Fix guerrilla shootout body counts.  Screwed up when land units
     were added in Chainsaw.
 * Fix ground combat not to disclose retreat of defending land by
   printing newlines.  Messed up in Empire 3.
 * Fix bad grammar in message reporting destruction of an attacking
   land unit.
 * Fix a missing newline in intelligence reports on land units.
   Broken in 4.3.33.
 * Spy unit changes:
   - Spies can no longer assault foreign sectors together with non-spy
     units.  This matches attack and board.
   - Ground attack can no longer detect spies in the target sector.
     They can still get caught when the attack succeeds.
   - Spies that get caught when their sector is taken over are now
     summarily executed.
   - Change the chance for assaulting spies to "sneak ashore"
     undetected from 10% regardless of efficiency to 40% at 100%
     efficiency.  The chance drops to 0% at 20% efficiency.
   - Assaulting spies now use mobility and can hit landmines even when
     they "sneak ashore".
   - Plug memory leak in spies sneaking ashore.  Screwed up when spy
     units were added in 4.0.0.
 * Plug memory leak when missile defense sinks attacking ships.  The
   leak goes back to flawed bug fix in Empire 2.
 * Fix lost and xdump lost to report more than one lost sector.
   Broken since lost item tracking was added in 4.0.7.
 * Merged news reports now saturate "times" at 65535.  Before, merging
   stopped when "times" reached 127.
 * More detailed version information, looks like this:
       V            Unmodified release V (same as before)
       V.N-H        Modified release built from a clean git tree
                    N is the number of additional commits, and
                    H is the abbreviated commit hash
       V.N-H-dirty  Same, but the working tree is dirty
       V-dirty      Modified release built from a tarball
   Visible in output of commands version, xdump version, and in
   program output for option -v.
 * Update, budget, neweff, production and work improvements:
   - Delivery screws up when it abandons a sector.  Fix by making it
     not abandon sectors.
   - Plague people in sectors only after taxes and feeding, for
     consistency with ships and land units.  Sectors were messed up in
     Empire 3.
   - Reorder the update sequence not to prefer countries with lower
     country numbers when the same sector repairs ships, planes or
     land units owned by multiple countries.  Broken since Chainsaw
     introduced budget and foreign repairs as options.  Either option
     worked sanely in isolation back then, but not their combination.
     They both became standard in Empire 2.  Restore the pre-budget
     order.
   - Fix budget for ship, plane and land unit repairs in foreign
     sectors.  Also broken since Chainsaw.
   - Fix the update to use new available work for ship, plane and land
     unit repairs even when the sector owner has a higher country
     number.  Also broken since Chainsaw.
   - Pilots are now paid at the same time as other military.  Can
     matter only when the country goes broke during the update.
   - Make ships produce after eating and building efficiency, like
     sectors.  The starvation command assumed that, and the change
     "fixes" it.
   - Don't let stopped ships produce.
   - Fix sector maintenance to charge for stopped sectors and when the
     owner is broke.  Broken in 4.3.23.
   - Fix plane maintenance to charge for satellites in orbit.  Broken
     in 4.2.2.
   - Change TECH_POP to count civilians in ships and land units.
   - Fix happiness and education to count civilians in ships before
     plague (same as in sectors), and to count civilians in land
     units.  No such land units exist in the stock game.
   - Revise sector production to avoid rounding intermediate values.
     This makes commands production and budget a bit more accurate.
   - Don't round income and expenses before adding them up.  Fixes
     numerous crazy artifacts, such as military getting paid less when
     spread over many sectors.
   - Fix budget for nations going broke or becoming solvent.
   - Fix military count in budget.
   - Zap available work when the sector is stopped or its owner is
     broke.  Broken in 4.0.0.
   - Limit "rollover" of unused available work to about half the
     work done by people at the update.
   - Round the people's work randomly rather than down, to reduce
     micromanagement incentives.  This is how things worked until
     Empire 3.
   - Make sector and unit building more predictable.  The budget
     command is now a bit more accurate.
   - Don't let repair of foreign ships, planes and land units use
     materials and available work destroyed by che or the plague.
   - Stopped sectors no longer repair foreign ships.
   - Fix unowned uw to eat, procreate and produce normally.
   - Fix budget not to predict production when the required level is
     too low.
   - Fix use of stale owner after revolt or revert to deity.  In
     particular, a bank now pays interest to the rebels.  Research and
     tech level for plague were also off.  Has always been wrong.
   - neweff and production now simulate the effect of insufficient
     food, like budget does.
   - Fix double-rounding of new work that made the update sometimes do
     less than neweff and production predicted.  Broken in Empire 3.
   - Enforce sector population limit only right after growth.  Before,
     it was also enforced when the sector changed from big city to not
     big city (since Empire 2 added option BIG_CITY), but not for
     other changes of the population limit during the update.
   - Limit work in big cities to the population limit at 0%
     efficiency, to get rid of ugly special cases in the code.
   - Code refactoring to undo much of the damage done in Empire 3 to
     make budget reuse the update's code.
 * Make plains more like wilderness: efficiency costs no money,
   but doesn't reduce mobility cost.
 * Changes to collect:
   - Tweak value of sectors.
   - You can no longer collect an active capital.
   - Don't disclose the value of a sector when it exceeds the amount
     owed.
 * Changes to xdumps, breaking compatibility:
   - Split tables relat, reject, contact off table nat.  Saves disk
     I/O and xdump bandwidth.
   - Table nat's field "hostname" was deprecated in 4.3.33 and is now
     gone.
   - Meta-type numbers now map to meta-type symbols 1:1.  Meta-type
     "c" is gone; the selectors use type "s" instead.
   - New sect-chr and item field "power".  Field "value" is gone.
   - New sect-chr and infrastructure fields "l_build", "h_build"
     replace "lcms", "hcms".  The replacements track materials for
     building 100% instead of 1%.
   - New plane-chr field "m_build" replaces "crew".
   - New sect-chr, ship-chr, plane-chr, land-chr, nuke-chr and
     infrastructure fields for arbitrary build materials, named
     "g_build" for guns, "p_build" for petrol, and so forth.  For now,
     these are all zero.
   - sect-chr field "cost" is now money to build 100% (was: cost to
     designate).  It replaces field "build" (money to build 1%).
   - New sect-chr, ship-chr, plane-chr, land-chr and nuke-chr field
     "bwork" is work to build 100% sector efficiency.  Tearing down a
     sector costs one fourth as much.
   - New sect-chr field "flags", backed by new table sect-chr-flags.
     The only flag so far is "deity", which marks sectors mortals
     can't designate.
   - New infrastructure fields "cost" and "bmobil" are money and
     mobility to build 100%.  They replace fields "dcost" and "mcost"
     (money and mobility to build 1%).
   - meta-flags "extra" and "const" are gone.  They were of no concern
     to clients anyway.
 * Game customization improvements:
   - The power value of commodities is now configurable, and "show
     item" shows it.  The collect value of commodities and sectors is
     no longer independently configurable, but derived from power
     value.
   - More flexible infrastructure material and mobility cost
     configuration.
   - More flexible sector build material and cost configuration,
     consistent with unit building.
   - Work to build sectors, ships, planes, land units and nukes is now
     independently configurable.  Before, it was fixed at 100 for
     sectors, and a function of build materials for ships, planes,
     land units and nukes.
   - The designate command can no longer be made to cost money.
 * Fix obscure corner cases where we failed to reset work percentage,
   loyalty or old owner in sectors without civilians.  Lingering old
   owner can prevent airlifting and unloading civilians.  The fix
   partly mitigates the bug where airlifted and unloaded civilians
   adopt the sector's work percentage and loyalty.  It also sets work
   percentage to 100% for unowned sectors without civilians.
 * Even planes without crew can now spread the plague.
 * The transport command now computes plane weight from build material
   weight rather than making it up.  The stock game's planes become a
   bit easier to transport, except for Zeppelins.
 * Tweak the "power factor", which determines power chart order:
   - Nukes now contribute to power just like other units.
   - Tweak power value of items, sectors ships, planes and land units:
     construction materials, oil and civilians are worth less, shells,
     guns, bars and petrol are worth more.  Cheap and old (low tech)
     ships, planes and land units are worth less, expensive and new
     ones are worth more.  Cheap and "small" (low maximum population)
     sectors are worth less, expensive and big ones are worth more.
   - Tweak tech's contribution to power so it's less dominant at low
     tech.
   - Research no longer contributes to power with option RES_POP.
   Note that with the old formula, producing and building certain
   things could be a net loss of "power".
 * The edit command now lets you edit land unit plague stage and time.
 * Takeoff and landing in mountains now require capability helo or
   missile.  The flying commands reject fixed-wing planes based in
   mountains.  When flying to a mountain, they select only helicopters
   and silently ignore the rest, exactly like they select only VTOL
   planes for flying to a non-airfield.  Similarly, missions ignore
   fixed-wing planes in mountains.  Before, any plane could bomb,
   drop, paradrop and fly missions out of a mountain, but no plane
   could fly to a mountain one-way, with fly, recon or sweep.  Has
   always been that way.
 * Add missing newlines to "while it is carrying a nuclear weapon"
   messages.  Screwed up in 4.3.23.
 * Disallow bombing spy units.  Bombers spotting spies makes no sense.
   Worse, they could bomb spies that weren't spotted.  Screwed up when
   spy units were added in 4.0.0.
 * Fix march's check for sector abandonment not to ignore land units
   that evade spy detection or are loaded on ships, but to ignore land
   units loaded on land units marching out.  Broken in 4.3.33.
 * Fix damage and ammunition use when multiple defenders with the same
   UID (say a ship and a land unit) return fire to multiple attackers.
   Broken in 4.3.12.
 * Fix march not to wipe out concurrent updates when the player
   declines to abandon the sector.  Broken in 4.3.33.
 * Make the savecore example script report more nicely when there's no
   core dump.
 * Enable various compiler options for hardening the programs against
   certain kinds of attacks when available.
 * Tell compilers not to rely on strict aliasing and signed overflow
   rules.  These rules are subtle, and compilers can't reliably
   diagnose violations.
 * Test suite improvements:
   - Add news to the smoke test.
   - Enable GNU libc memory allocation error checking.
   - Cover much of the update.  Notable gaps are fallout, delivery,
     distribution, ALL_BLEED, LOSE_CONTACT, and parts of guerrilla.
   - Cover version, show and xdump of configuration.
   - Improve navigate and march coverage.
 * Plug memory leaks in navigate and march.  Broken in 4.3.33.
 * Fix read beyond buffer in navigate, march and automatic retreat.
   Could theoretically crash the server.  Broken in 4.3.33.
 * Fix out-of-bounds array read in bomb, drop, fly, paradrop, recon,
   sweep.  Could theoretically crash the server, or be abused to gain
   plane mobility.  Broken in 4.3.27.
 * Enable more compiler warnings.
 * Code refactoring and cleanup.
 * Info page, manual page and documentation fixes and clarifications.

.FI
.s1
.SA "Server"
