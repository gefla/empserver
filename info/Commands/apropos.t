.TH Command APROPOS
.NA apropos "Searches info pages for information on a topic"
.LV Basic
.SY "apropos <searchword> [maxhits]"
.s1
The Empire info pages are organized by subject. It can sometimes be hard to
find the exact page with the information in which you are interested.
To get a list of subjects containing the word you are looking for, type:
.EX apropos <word>
.s1
without the < and > brackets.
.s1
You can then type:
.EX apropos bridge
to get a list of Empire commands and info topics which contain the
word bridge, as well as the line that mentions it.
.s1
Apropos is not case sensitive, thus the above example will find
pages with Bridge or bridge (or even bRiDgE).
.s1
By default, apropos exits when 100 hits are found.  You can change this value
with an optional second parameter.
.s1
.SA "info, Playing"
