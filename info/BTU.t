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
you a certain number of BTUs.  The same happens at the update.
.s1
Here is the procedure for determining how many BTUs you get:
.nf
If the game is a blitz, then as soon as your BTUs go below zero, then
they are automatically set back to the maximum.

(1) Count the number of civs generating BTUs.
  civs = maximum(1000, number of civs in your capital sector)
  Note that if you don't have a capital, then you will get _no_ BTUs.

(2) Find out how many civs are required to make one BTU in one time unit.
The "version" command will tell you how many civilians are required to
produce one BTU in one Empire Time Unit (ETU) in a perfectly efficient
capital.

(3) Calculate how efficiently your capital makes BTUs.  A proper
capital's BTU efficiency is sector efficiency times work percentage,
but at least 0.5%.  A mountain capital's BTU efficiency is always
0.5%.

(4) Calculate how many BTUs your capital produces in one time unit.
Divide the number of civs generating BTUs (step 1) by the number of
civs required to produce one BTU in one time unit (step 2).  Multiply
by efficiency (step 3).

(5) Find out how many time units have passed since you last got BTUs.
A fixed number of ETUs elapse between updates.  The \*Qversion\*U
command shows how many.

(6) Calculate how many BTUs you get.
Multiply the number of BTUs your cap produces in one ETU (step 4) by
the number of ETUs which have passed (step 5).  This is how many BTUs
you get.  Note that there is a limit to how many BTUs you can have at
any given time.  This number is usually 640 but can be changed by the deity.

EXAMPLE: say your capital is in perfect condition (100% efficiency and
work), and has 500 civilians.  Suppose that version said:

It takes 25.00 civilians to produce a BTU in one time unit

Then first you would divide 500 by 25 to get 20.  Now since your cap
is 100% efficient and 100% working, its BTU efficiency is 100% * 100%
= 100%, i.e. your capital makes 20 BTUs per ETU.  Now suppose that
version said:

An update consists of 60 empire time units.

and suppose that exactly that many had passed since the last time you
got BTUs.  You multiply 20 by 60 and get 1200 BTUs.  But since the
maximum is 640, you'd have 640 BTUs.  Note that if your capital were a
mountain sector, it would make only 20 * 0.5% = 0.1 BTUs per time
unit.  You'd get only 6 BTUs then.
.fi
.s1
.SA "Innards, version, Time, Playing"
