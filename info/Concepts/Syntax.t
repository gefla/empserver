.TH Concept "Command Syntax"
.NA Syntax "The Empire command syntax"
.LV Basic
.de AB
.in 0.3i
.ti -0.3i
..
.s1
The Empire Shell (command interpreter)
expects input in the form:
.EX VERB ARG1 ARG2 ...
VERB is any one of the Empire commands on the \*Qcommand list\*U
(what you see when you type \*Qlist\*U-- e.g. 
\*Qmap\*U, \*Qmove\*U, \*Qinfo\*U, etc).
.s1
The number of ARGs varies from command to command.
In most cases the ARGs need not be supplied on the command line;
any that are not supplied will be obtained by prompting.
The exception to this rule are those ARGs
that are enclosed in [square brackets]
in the command list.
The presence of these ARGs changes the way the command is carried out.
.s1
Certain basic argument types recur often and are abbreviated in the
following ways.
.s1
.AB
<TYPE> ::= 'se', 'sh', 'l', or 'p' for sector, ship, land unit, or
plane respectively.  If "sector" is not a possibility, then 's' will
likely suffice for "ship".
.s1
.AB
<CNUM/CNAME> ::= either a country number or a country name
.s1
.AB
<COMM> ::= any one, (or unambiguous abbreviation), of the list of
commodities found in \*Qinfo Selector\*U:
.AB
<LOAN> ::= loan number
.AB
<SECT> ::= sector coordinates in the form: x,y
.AB
.ne 4
<SECTS> ::= sector(s) in the form:
.NF
lox:hix,loy:hiy ?cond&cond&...
.FI
\*Qlox\*U, \*Qhix\*U, \*Qloy\*U, \*Qhiy\*U are coordinates bounding
the rectangular area to be considered
.s1
\*Qcond\*U is a condition of the form:
.NF
<VALUE><OPERATOR><VALUE>
.FI
<VALUE> is either a <COMM>, as above,
a number in the range 0 to 65536,
a sector designation,
(\*Qm\*U for mine, \*Qc\*U for capital, etc), a type of unit
(\*Qfrigate\*U, \*Qcavalry\*U, etc), or a selector found in \*Qinfo
Selector\*U.
.br
<OPERATOR> can be any one of:
.in +\w'#\0\0'u
.L <
less than
.L =
equal
.L >
greater than
.L #
not equal
.in
.s1
Thus \*Qmob=100\*U, \*Qore#0\*U, \*Q7=guns\*U, \*Qciv#mil\*U,
\*Qdes=b\*U are examples of legitimate conditions,
(note lack of embedded spaces),
and
.EX census -3:3,-3:3 ?des=+&gun=5&mil>civ
will list all highways
within 3 of your capital
with exactly 5 guns and more military than civilians.
.sp
Similarly,
.EX deliver f -9:9,-9:9 ?f_del=0
will allow you to set up food delivery
in all the sectors that don't already
have food delivery.
.sp
Note that \*Qhix\*U, \*Qhiy\*U, and \*Q?cond\*U are all optional.
Also, the entire \*Qlox:hix,loy:hiy\*U section may be replaced
by either \*Q#\*U, \*Q#0\*U, \*Q#1\*U, \*Q#2\*U,
or \*Q#3\*U, ... which refer to 
\*Qrealm\*Us which you can define.
(See \*Qinfo realm\*U.)
Note that \*Q#\*U and \*Q#0\*U are equivalent.  Also \*Q*\*U refers to
all sectors, and \*Q@x,y:dist\*U refers to all sectors within dist of x,y.
.AB
<SHIP> ::= one ship number
.AB
<FLEET> ::= fleet designation,
which may be a list of ships separated by slashes (`/'),
a fleet letter,
the character tilde (`~') which means all ships not in a specific fleet,
or a rectangular sector area.
.AB
<SHIP/FLEET> ::= either a single ship number or a fleet designation.
.sp
Note that <FLEET> specifications may have an optional ?cond&cond...
argument like that used for <SECTS> arguments.
For instance:
.EX ship ~ ?des=d&mil>5
will list all destroyers that have more than 5 military
which are not currently in any fleet.
.s1
.in 0
.s1
The output from commands may also be sent to a file or another
process by utilizing these alternate syntaxes:
.s1
.ne 2
.EX VERB ARG1 ... > FILE
which sends the output to the file \*QFILE\*U.
.s1
.ne 2
.EX VERB ARG1 ... >> FILE
which appends the output to the file \*QFILE\*U.
.s1
.ne 2
.EX VERB ARG1 ... >! FILE
which sends the output to the file \*QFILE\*U,
replacing the file it if it already exists.
.s1
.ne 2
.EX VERB ARG ... | PROGRAM [ARGS...]
which send the output of the command to the program \*QPROGRAM\*U
with optional arguments \*QARGS\*U.
This is most useful when \*QPROGRAM\*U is some type of
pager like \*(iFmore\*(pF(1) or \*(iFless\*(pF(1).
.s1
The Empire shell also recognizes a few control characters:
.s1
.in +\w'interrupt\0\0'u
.L \f2name\fP
\f2meaning\fP
.L interrupt
Abort command and return to Empire command level
.L quit
Exit Empire
.L EOT
Exit Empire
.in
.s1
.SA "realm, Move, Selector, Playing"
