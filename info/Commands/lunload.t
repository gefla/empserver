.TH Command LUNLOAD
.NA lunload "Unload things from land units"
.LV Basic
.SY "lunload <COMMODITY> <UNITS> <AMOUNT>"
.SY "lunload plane <UNITS> <PLANES>"
The lunload command is the opposite of lload and follows the
exact same syntax ...
so why look here for documentation?
.s1
The only thing to know is that lunload won't unload more of a thing if the
sector already has 999. For example, if a unit has 10 food, and the sector
has 994, you can only unload 5 of the 10 food.
.s1
.SA "lload, LandUnits, Moving"
