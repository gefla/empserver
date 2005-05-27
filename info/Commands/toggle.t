.TH Command TOGGLE
.NA toggle "Toggle client flags"
.LV Expert
.SY "toggle [inform|flash|beep|coastwatch|sonar|techlists] [on|off]"
You use the toggle command to set various user-interface flags for
your country.  The server will remember which flags you have set
between sessions.  If you do not specify the second argument ("on" or
"off"), then the server will just flip the switch from on to off or
vice versa.
.EX "toggle inform"
Will toggle the "inform" flag (default off) for your country.  When
inform is on, the server will inform you immediately whenever you
receive a telegram.
.s1
.EX "toggle flash"
Will toggle the "flash" flag (default on) for your country.  When
flash is on, you can receive instantaneous messages from your allies when
they use the "flash" and "wall" commands.
.s1
.EX "toggle beep"
Will toggle the "beep" flag (default on) for your country.  When beep
is on, your terminal will beep whenever there's a big explosion in
the game (shells, bombs, torpedoes...).  You might want to turn beep
off if you don't want to disturb others.
.s1
.EX "toggle coastwatch"
Will toggle the "coastwatch" flag (default on) for your country.  When
coastwatch is on, then you will receive telegrams informing you of any
new ship sightings.  Note that allied and friendly ships do not show up
in these automatic sightings.
.s1
.EX "toggle sonar"
Will toggle the "sonar" flag (default on) for your country.  When
sonar is on, then you will receive telegrams whenever any of your
sonar-equipped ships detect a sonar ping.
.s1
.EX "toggle techlists"
Will toggle the "techlists" flag (default off) for your country.  When
techlists is on, when you do a "show" command, the lists shows will be
in order of tech instead of grouping order.
.s1
.EX "toggle"
Will list all your flags and show whether they are on or off.
.s1
.SA "telegram, flash, wall, Communication"
