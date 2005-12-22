.TH Concept Technology
.NA Technology "How technology level is calculated each update"
.LV Expert
.s1
Your technology level affects the range that guns can fire,
the range over which radar is effective,
the range that planes can fly
and the range that torpedoes can travel, among other things.
.s1
The \*Qnation\*U command will display your current technology level
and technology factor.
.s1
.nf
This is how your new technology level is determined every update:

(1) Technological breakthroughs are produced in tech centres.
See "info Products" or the "production" command to find out how many
technological breakthroughs you are going to produce.  Your update
telegram will tell you the number of technological breakthroughs
produced in your various tech centres in a line like this:
  technological breakthroughs (7.2) produced in 3,1

(2) Calculate tech delta from technological breakthroughs
In the output of the "version" command, you will see a line like this:
  Tech Buildup is limited to logarithmic growth (base 2.00) after 3.00.

In this example, the "tech log base" is 2.00 and the "easy tech" is
3.00.  To find out what your tech delta for an update will be, subtract
"easy tech" from the number of technological breakthroughs you are
producing and then add 1, then take the logorithm with base "tech log
base" and then add the "easy tech" back on.

For example, say you had a number of tech centres producing a total of
18.0 tech units.  Then to calculate your delta, first subtract "easy
tech" from it and then add 1 to obtain 16.0, then take the log base 2.0
of that which is 4.0 (since 2 to the power 4 is equal to 16), and then
add the "easy tech" back on to get a tech delta of 7.0.

(3) Tech bleed
If the ALL_BLEED option is enabled, then you will gain extra technology from
all countries producing tech, and there will be a line in the
"version" output like this:
  The technology/research boost you get from the world is 50.00%.

If the ALL_BLEED option is turned off, then you will gain extra
technology from the countries with whom you share a two-way alliance,
and the line in the "version" output will look like this:
  The technology/research boost you get from your allies is 50.00%.

In both cases, the amount of tech you get is 50% of the average amount
of tech delta for all eligible "bleeders".  Note that a fundamental
difference between these two cases is that in the ALL_BLEED case, only
countries which are producing tech are counted in the average, whereas
in the case where ALL_BLEED is turned off, the amount of tech bleed is
averaged among all of your allies whether they are producing tech or not.

Your net tech delta will be printed in your update telegram as:
  3.000 technology (2.0000 + 1.0000)
where the 3.0000 is your net tech level gain, and that is broken down into
how much you produced (2.0000) and how much you got from tech bleed (1.0000).

(4) Tech decay
After your net tech increase has been added to your country's
technology level, then your technology level will decay.  The amount of tech
decay is printed in the version output as:
  Nation levels (tech etc.) decline 1% every 96 time units.
The number of time units in an update is printed in the version output as:
  An update consists of 60 empire time units.
Every update, your tech is reduced by
  decay = (tech level) * (decay rate) * (etu per update) / (decay etus).
So if your tech level was 200.0, and the decay rate was as above, then
you would lose:
  decay = 200 * 0.01 * 60 / 96 = 1.25
and so your tech would go down to 198.75 from decay.

(5) Tech leak
If your tech level is much lower than the tech level of the highest
tech country in the world, then every update, there is a chance that
one of their high tech secrets will get leaked into your country.

Take the tech level of the highest tech country in the world and
divide by 5.  Call this the "minimum tech".  If your tech level is
lower than the "minimum tech", then every update, there is a 20%
chance that your tech level will increase by one third of the
difference between your tech level and the minimum tech.
.fi
.s1
Note that if the TECH_POP option is enabled, technology costs 
proportionately more $$$ to make as your civilian population grows
past 50,000 civilians.
.s1
.SA "Innards, nation, Research, Nations, Producing, Updates"
