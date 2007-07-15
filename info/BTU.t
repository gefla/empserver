.TH Concept "Bureaucratic Time Units"
.NA BTU "How BTUs are generated"
.LV Expert
.s1
It takes time for decisions to be made by top level officials, and it
takes time for their orders to reach those who execute them.
The potential of your government to process new information is
measured in Bureaucratic Time Units (BTU).
.s1
Most commands consume BTUs.  The \*Qlist\*U command shows how many.
Some commands consume additional BTUs, depending on what they actually
do.
.s1
BTUs are generated in real-time based on the number of civilians that
are in your capital.  Every time you log out and in again, the server
calculates how long you've been away, and based on that time awards
you a certain number of BTUs.
.s1
Here is the procedure for determining how many BTUs you get:
.nf
If the game is a blitz, then as soon as your BTUs go below zero, then
they are automatically set back to the maximum.

(1) Count the number of civs generating BTUs.
  civs = maximum(999, number of civs in your capital sector)
  Note that if you don't have a capital, then you will get _no_ BTUs.

(2) Find out how many civs are required to make one BTU in one time unit.
The "version" command will tell you how many civilians are required to
produce one BTU in one Empire Time Unit (ETU).

(3) Calculate how many BTUs your cap produces in one time unit.
Divide the number of civs generating BTUs (step 1) by the number of
civs required to produce one BTU in one time unit (step 2).  If your
capital is in a mountain or has zero efficiency, then multiply by
1/200.  Otherwise, multiply by (sector efficiency) / 100.  Note that
0% capitals and mountains generate BTUs as if they were 0.5% capitals.

(4) Find out how many time units have passed.
A fixed number of ETUs elapse between updates.  The \*Qversion\*U
command shows how many.

(5) Calculate how many BTUs you get.
Multiply the number of BTUs your cap produces in one ETU (step 3) by
the number of ETUs which have passed (step 4).  This is how many BTUs
you get.  Note that there is a limit to how many BTUs you can have at
any given time.  This number is usually 640 but can be changed by the deity.

EXAMPLE: say you had a 100% capital containing 500 civs.
Suppose that version said:

It takes 25.00 civilians to produce a BTU in one time unit

Then first you would divide 500 by 25 to get 20.  Now since your cap
is 100% efficient, you would multiply 20 by (100/100) and so the civs
in your cap would produce 20 BTUs per time unit.  Now suppose that
version said:

An update consists of 60 empire time units.

and suppose that exactly that many had passed since the last time you
logged on.
  Lastly, we multiply 20 by 60 to get 1200 BTUs.  But since the
maximum is 640, we would have 640 BTUs.  Note that if your capital had
been in a mountain sector, then you would have only gotten 6 BTUs in
24 hours.
.fi
.s1
.SA "Innards, version, Time, Playing"
