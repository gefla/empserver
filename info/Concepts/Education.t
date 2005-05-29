.TH Concept Education
.NA Education "How Education is calculated at the update"
.LV Expert
.s1
Education is important for both technology and research production.
Your country will not be able to produce technology or research unless
your civilians have an education of at least 5.  (See "info Products"
for more details).
.s1
The \*Qnation\*U command will display, among other things,
your current education level.
.s1
.nf
This is how your new education level is determined every update:

(1) Graduates are produced in schools
See "info <Products.html>" or the "production" command to find out how many
Graduates you are going to produce.  Your update telegram will tell
you the class of graduates produced in your various schools in a line
like this:
  a class of graduates (135.00) produced in 3,1

(2) Calculate your education production efficiency
The higher your population, the lower your education p.e., as it takes
more graduates to maintain the same overall level of education.  Note that
with low population you education p.e. can be greater than 1.
The version output will contain a line like this:
  Education p.e. requires 1 class of graduates per 10000 civ.

Your education p.e. is now calculated as follows:
education p.e. = "civ. per graduate" /
		 "total pre-update civilian population"

number of graduates produced = class of graduates * education p.e

For example, lets say you have a pre-update civilian population of
40,000 and have produced a class of 260 graduates.  Your education p.e.
would be 10000 / 40000 = .25 and your number of graduates would be
260 * .25 = 65

 (3) Calculate edu delta from the number of graduates
"edu log base" is always 4.0 and "easy edu" is always 5.0.
To find out what your edu delta for the update will be, subtract "easy
edu" from the number of graduates you are producing and call
this number "over easy".  Now divide "over easy" by the log of ("over
easy" + "edu log base") base "edu log base", and then add "easy edu" back.
Simple eh?  ;-)

OK, here's an example:
Say you are producing 65 graduates.  Then your "over easy" would be 65
- 5.0 = 60.  Now the log of (60 + 4.0) base 4.0 is 3 (since 4 to the
power 3 is 64), so our edu delta will be 60/3 + 5 = 25.

(4) Calculate your new education as a weighted average
The version output will contain a line like this:
  Education is averaged over 192 time units
And the number of etus per update is printed in the version output as:
  An update consists of 60 empire time units.

Your new education is now calculated as follows:
"new edu" = ("old edu" * "average etus" + "edu delta" * "etus per update") /
            ("average etus" + "etus per update")

So if, for example, your old education level was 40 and your "edu
delta" was 25, then your new edu would be:
"new edu" = (40 * 192 + 25 * 60) / (192 + 60)
          = 36.43
.fi
.s1
.SA "Happiness, Innards, nation, Products, Research, Technology, Nations, Producing, Updates"
