.TH Command DECLARE
.NA declare "Formally declare alliance, friendly, neutrality, hostile or war"
.LV Basic
.SY "declare <POSITION> <NATS>"
The declare command allows you to officially declare
your diplomatic relations with other countries.
.s1
<POSITION> may be \*Qalliance\*U, \*Qfriendly\*U, \*Qneutrality\*U,
\*Qhostile\*U, or \*Qwar\*U;
.s1
<NATS> can be the name or number of
a country you want to declare relations towards, or it can be * in
which case you will declare <POSITION> towards all nations.
.s1
For a complete list of the meanings of all the different types of
relations, see "info relations".
.s1
GOING TO WAR
.s1
(Note: the following stuff is only in effect if the SLOW_WAR option is selected)
With the SLOW_WAR option in use, you cannot attack the land of someone you are
not at war with. When you declare war on
someone, your relation goes to \*Qmobilizing\*U. You still can't attack them.
After the next update, your relation goes to \*Qsitzkrieg\*U, and you still
can't attack them. After the NEXT update, you're at war with them, and anything
goes.
.s1
If someone is \*Qmobilizing\*U or in \*Qsitzkrieg\*U, or at war with you,
and you
declare war against them, your relation is set to be the same as theirs, so
there is no way to gain any advantage through this stuff. It just means that
you can't sneak-attack people.
.s1
The first person who declares war pays $1000. Return declarations are
free.
.s1
There is an exception to the SLOW_WAR restrictions. If you are listed as
the old owner of a sector, you are still allowed to attack it. This is to
prevent someone from declaring war with you, taking some sectors, then
declaring neutrality before you declare war. (Assuming you didn't notice
their declaration of war)
.s1
Also, if the SNEAK_ATTACK option is enabled, you make make one attack upon
someone you're not at war with. You'll pay $5K for the privilege, however.
Once the sneak attack is over (win or lose), war is automatically declare.
(The normal $1K cost is waived in this case)
.s1
The newspaper is very interested in declarations of war or alliance --
don't expect to keep such declarations secret.
.s1
Deities can will be prompted for a third parameter, the country to make
the declaration for. The default is to make it for the deity's country.
.s1
.SA "fly, nation, spy, relations, Diplomacy"
