.TH Command CAPITAL
.NA capital "Change the location of your capital"
.LV Basic
.SY "capital <SECTOR>"
This command changes the location of your nation's capital.
The new sector must be of type "capital" or "mountain",
and must be owned by you.  Your capital produces BTUs.  See "info BTU"
for the exact rate of BTU production.
.s1
This command does not change your coordinate origin
(where 0,0 is located),
so if you prefer your coordinates centered on your nation's capital,
you must use the origin command.
For example:
.EX capit -10,-4
.NF
Your capital is now at -10,-4.
.FI
.EX orig -10,-4
.NF
Designation of new origin requires revision of sector numbers.
Origin at -10,-4 (old system) is now at 0,0 (new system).
.FI
.s1
If the unthinkable should come to pass, and an enemy captures your capital,
bad things happen. Some happen once, immediately:

.NF
1) You will lose half your cash, or $3K, whichever is more.
2) Your enemy will gain half your cash times (1/5 + 4/5 * sector efficiency)
.FI
.s1
Re-taking a previously taken capital has no further effect. Once a new capital
has been declared with this command,
you are again in peril of these two awful things.
.s1
Some effects happen right away, and last until you use
the \*Qcapital\*U command to create a new capital. The effects are:

.NF
3) You will not be able to use many commands (use the
   'list of commands' command to see the pitiful few you may still use)

4) The rate at which you gain BTUs will dramatically decrease (see
   info BTU).
.FI
.s1
.SA "origin, designate, BTU, Nations"
