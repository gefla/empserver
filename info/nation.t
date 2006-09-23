.TH Command NATION
.NA nation "The state of your nation"
.LV Basic
.SY "nation [as-COUNTRY]"
The nation report command displays various data about a country.
The as-COUNTRY defaults to yourself.
Only a deity can request a nation report for another country.
.s1
The data displayed by the nation command is formatted as a single page
report and should be self explanatory.
For example:
.EX nation
.NF
	MGM Nation Report	Sat Nov  4 15:01:52 1986
Nation status is ACTIVE         Bureaucratic Time Units: 250
Capital is 100% efficient and has 87 civilians and 80 military
#0  -10:10,-5:5    #1  -10:0,-64:64    #2  -10:64,-64:64    #3  -10:64,-64:64
#4  -10:20,-10:10    #5  -10:10,-5:5    #6  -10:20,-25:25    #7  -10:30,-20:0
 The treasury has $16800.00     Military reserves: 152
Education..........  9.20       Happiness.......  2.01
Technology......... 16.81       Research........  19.82
Technology factor : 30.81%      Max population : 556
 
Plague factor :   1.05%
Max safe population for civs/uws: 427/483
Happiness needed is 4.385283
.FI
.s1
The max safe population is the highest population in an ordinary
sector such as a mine
that will NOT forfeit some population growth during an update.
In the example above, if you have 427 civs in a sector, you are
guaranteed not to have more than 556 after population growth.
If you have more than 427, population growth would make you go
over 556, and the excess babies would be eliminated when the
population was reduced to 556.
.s1
If you have at least the happiness mentioned in the last line,
your sector's loyalty won't have a chance to go down. Otherwise,
each update, the chance is the number above - your happiness / 5.0.
For the country above, the chance would be (4.385283-2.01)/5.0 = .475 = 47%
chance that any individual sector's loyalty would go down.
.s1
.SA "power, Nations"
