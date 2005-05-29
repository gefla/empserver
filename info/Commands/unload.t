.TH Command UNLOAD
.NA unload "Inverse of load (above)"
.LV Basic
.SY "unload <COMMODITY> <SHIPS> <AMOUNT>"
.SY "unload plane <SHIPS> <PLANES>"
.SY "unload land <SHIPS> <UNITS>"
The unload command is the opposite of load and follows the
exact same syntax ...
so why look here for documentation?
.s1
The only thing to know is that unload won't unload more of a thing if the
sector already has 999. For example, if a ship has 10 food, and the harbor
has 994, you can only unload 5 of the 10 food.
.s1
Also, you must have Friendly relations with the owner of the harbour
in order to unload into it.
.SA "load, Ships, , Moving"
