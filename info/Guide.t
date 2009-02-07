.TH Introduction "User's Guide to Empire"
.NA Guide "User's Guide to Empire"
.LV Basic
.nf
.tr \&
|                                                                              |
|    ***       ***                                                             |
|    ***       ***                                   *                         |
|    ***       ***    ******    ******    ** ****     *    ******              |
|    ***       ***   **        **    **   ****  **   *    **                   |
|    ***       ***    *****    *******    **               *****               |
|    ***       ***        **   **         **                   **              |
|    ***       ***   ******     ******    **              ******               |
|     ***     ***                                                              |
|      *********     ++++  +   +  +++++  ++++   +++++      +++++   +++         |
|                   +      +   +    +    +   +  +            +    +   +        |
|                   +  ++  +   +    +    +   +  +++          +    +   +        |
| ************      +   +  +   +    +    +   +  +            +    +   +        |
| ***********        +++    +++   +++++  ++++   +++++        +     +++         |
| ***                                                                          |
| ***           *** ****  ****    ********    *********   ********    *******  |
| *******       *****  ****  ***  ***    **     *****     ***   ***  ***   *** |
| ***           ****    **    **  ***     **     ***      ***   ***  ********  |
| ***           ***     **    **  ***    **      ***      ********   ***       |
| ***********   ***     **    **  ********      *****     ***   ***  ***   *** |
| ************  ***     **    **  ***         *********   ***    ***  *******  |
|                                 **                                           |
|                                                           By Geoff Cashman   |


User's Guide to Empire							 1

                    Preface to the Wolfpack Edition

This is Geoff Cashman's venerable User's Guide to Empire.  It was written
in 1991, and Empire has evolved quite a bit since then.  Nevertheless, many
players have found it useful through the years, so here it is.  We
reproduce it verbatim except for this preface and corrections of a few
editing mistakes.


			       Copyright

Copyright (C) 1991-2009 Geoff Cashman.  This is free documentation; see the
source for copying conditions.  There is NO warranty; not even for
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
.\" See COPYING in the root of the source tree for details.


User's Guide to Empire							 2

				Preface

This guide's intended audience is the beginning player and those players
who have played a game or three and would like to have some insight on some
more subtle aspects of the game.  Some experienced players may find some 
benefit from reading this guide as well.  While you may feel you are an
expert, it is always possible to learn.  Nothing in this guide is intended 
to be difficult to understand, and every effort has been made to make this
guide an easy learning tool. 
				   
This publication is current to Berkeley version 1.1 patch level 5




User's Guide to Empire							 3

				Foreword

It has been over a year now since I first sat down and thought in a serious
way about writing this guide.  It has been a long trip with many interesting
turns and developments.  When I first started, the version of Empire that
this guide has ultimately been written for had not yet been released.  Now,
with this project nearing release I find that yet another new version of
Empire is coming out.  I look on in near despair as I realize that this guide
may very well soon be outdated.  Nevertheless, I feel compelled to release
this information in what might be considered a final form.  

However, I hope that this is not the final form.  With new versions of
Empire coming out in the very near future I hope that some other potential
author steps forward and takes up the banner of educating the new player. 
I caution you though that such a work as this takes quite an effort. 

To the reader and any critics or even cynics that might be reading, I say
that I am not writer.  I never was, and I never intend to be.  There are
certainly enough writers in the world to write almost everything the world
could want to know about in print.  I was never skilled in the art of proper
style for any sort of writing.  Please, be kind and keep this in mind.

I wrote this guide because I felt there was a great need for something of
this nature.  I have watched so many players come to the network looking
for information on Empire, such as the rules, or information on how to play.
One can get information about the various commands available to oneself
within the game, but to understand how to use those commands effectively
and get skilled at running a country is a far greater task.  Many new
players have despaired after playing their first game for lack of the
requisite knowledge to play effectively.  Some come back fighting hard
in the next game, but many just disappear never to be heard from again.
This guide was written with the hope that if the new player is supplied
with the basics, they can advance in skill more quickly than previously
possible and not despair at their first defeat. 

I am hoping that within the near future two other companion guides to 
this one will be written and released to the net.  These two guides will
be intended to make the average player a better player or even expert,
and to teach fledgling deities how to compile and establish a, solicit 
entries for , and effectively run, a game.  Some people have commented
to me that the time spent on these guides would be better spent if I 
were to spend time programming for Empire instead.  It is my belief that
in teaching players to be better players, and teaching deities how to be 
a deity should that guide be released, the potential pool for
programmers of Empire will increase.  

I can not take credit for the idea of this guide.  The need for such
a guide I think was painfully obvious to a great many players.  Some in
fact did engage in projects to fill that need.  Every such attempt has
fallen short, in my humble opinion.  Witness the large number of people


User's Guide to Empire							 4

who still ask the network, "How do I play?" and even more importantly
the lack of sufficient answer to such people's questions.  In my highest
hopes, I dream that someday soon that answer will be "Get a copy of the
User's Guide to Empire, which is available via FTP from xx.xx.xx".

This work was made possible by the contributions of several Empire players.
This work would not have been completed without the insights and assistance 
of many Empire players and deities.  To the Empire community as a whole,
I say thank you for your priceless contributions. 




--------------------------------------------------------------------------------


User's Guide to Empire							 5

	       		   *** Contents ***

Preface
Foreword
1.  What is Empire? . . . . . . . . . . . . . . . . . . . . . . .  6
2.  How to get into a game of Empire. . . . . . . . . . . . . . .  8
3.  The primitive basics. . . . . . . . . . . . . . . . . . . . . 16
4.  Geography in Empire.  . . . . . . . . . . . . . . . . . . . . 18
5.  The beginning phases of Empire. . . . . . . . . . . . . . . . 21
6.  Diplomacy.  . . . . . . . . . . . . . . . . . . . . . . . . . 28
7.  Setting up your defenses. . . . . . . . . . . . . . . . . . . 33
8.  Nuclear warfare.  . . . . . . . . . . . . . . . . . . . . . . 38
9.  Technology. . . . . . . . . . . . . . . . . . . . . . . . . . 42
10. The economy.  . . . . . . . . . . . . . . . . . . . . . . . . 45
11. War.  . . . . . . . . . . . . . . . . . . . . . . . . . . . . 54
12. Other helpful or not so helpful hints.  . . . . . . . . . . . 57
13. Some tricks and subtle tactics. . . . . . . . . . . . . . . . 59
14. Tools.  . . . . . . . . . . . . . . . . . . . . . . . . . . . 63

Appendices
 a) glossary
 b) known bugs
 c) ftp sites
 

Figures:
	1.  Sample of how to FTP and compile a client program
	2.  Killing a process on a remote Empire host
	3.  Example of an Empire map
	4.  Realistic version of an Empire map
	5.  Table of sector types and designations 
	6.  Unexplored island prior to breaking sanctuary
	7.  Island showing exploration using theory 1
	8.  Island showing exploration using theory 2
	9   Island showing exploration using theory 3
	10. Results from 'show planes capability'
	11. Results from a 'nation' report
	12. Map of region hit by nuclear weapon
	13. Table of types of commodities in Empire
	14. Sample output of 'prod' command
	15. Results of a 'census' command of a library
	16. Map of area requiring some distribution
	17. Results of a distribution command
	18. Map of an area requiring delivery path


User's Guide to Empire							 6

Chapter 1 :-: What is Empire?

	Empire is a game in which you are pitted against several other
	players in a country vs. country format.  There are economies to
	run, armies to support, policies to generate, and allies and
	enemies to make.  The game supports various features that make
	certain aspects of the game seem to parallel the "realistic"
	world.  These include satellites, planes, banks, parks, etc.
	
	Empire, in the version herein described, is a very complex 
	game.  The closest game that most people will know that is 
	analogous to it is the game known as Risk.  However, Empire is 
	more complex by a factor of at least one hundred.  Nevertheless, 
	as a result of the computer aiding you every step of the way, the 
	game is not difficult to play, though it can be difficult to 
	learn.  If you read this guide thoroughly, and use it as a 
	continuing resource, you can help make learning this game a lot 
	easier. 

	It should be noted that in learning to play this game, you
	may become frustrated or feel daunted by its complexity. 
	Several players have noted that they almost gave up on Empire,
	until one day when everything seemed to start to make sense.
	I call it the "threshold level of understanding" after which
	learning the art of Empire becomes several grades easier, and
	players learn substantially faster.  I urge you to stay with
	it until you get to that level.  It may take some time, but it
	is well worth the effort.  This guide is intended to get you
	to that threshold level of understanding.

	Games usually take two to three months depending on various factors 
	that are changeable by the person who runs the game prior to its 
	inception.  Some games, known as blitz games, last a day or three 
	and will wear out your finger tips from typing too much, will cause 
	your family and loved ones to be tremendously upset with you for 
	missing breakfast, lunch and dinner for three days straight, will cause
	your stomach to argue constantly with you about its right to
	food, and are a tremendous amount of fun.  Blitz games are not
	for the light of heart nor are they meant for the beginner. 
		
	For more information concerning the basic scope of Empire,
	refer to the game itself.  Within the game, there are 
	various help entries accessible through the command 'info'.
	This guide is intended to go well beyond the scope of help
	available from 'info'.

	Empire, the game, has evolved through many different forms
	over the last decade.  There are still numerous versions
	for various computers around.  The version herein discussed
	is BSD Empire, written for the most part by Dave Pare along
	with a lot of help from net people.  As release time approaches
	I am aware that Mr. Pare is working on BSD Empire V2.0. 
	This guide is only current to version V1.1.5 of BSD.  I wish


User's Guide to Empire							 7
	
	the future author of the guide to V2.0, whomever that might be,
	the best of luck and bequeath that person a family size bottle
	of aspirin for personal use. 

Summary: Empire is a very complex game that has a rather steep learning
	 curve.  Most of the complexity of the game can be handled by
	 the commands available to you, the game server itself, or tools
	 that you use to better your country.  Tools are described later
	 in this text. 


User's Guide to Empire							 8
	
Chapter 2 :-: How to get into a game of Empire.

	The most common way of finding about about games that are about to 
	happen is by monitoring the newsgroup rec.games.empire on your local
	news reading program. Other ways are via e-mail by getting to know 
	several players with whom you correspond over the network.  Some 
	games are by invitation only but these are few and far between as 
	usually a game is better with more people in it.  If you do not know
	how to use your local news reading program, ask a local site consultant
	or find a brochure about the news programs available.  One of the most
	common newsreaders can be reached by typing 'rn' from your Unix
	prompt.  If you have VMS for an operating system this may not be true.
	To go directly to the rec.games.empire newsgroup you could type in
	'rn rec.games.empire'.  

	Before entering a game of Empire, consider the amount of time that
	you have available to play the game.  Two hours a day for a four
	update per day game is usually the norm.  If you are inexperienced,
	and have not fought in wars before, that two hours will dramatically
	increase should war break out between you and another country.

	Note that it is easy to get addicted to the game.  The player should
	be aware of this prior to playing.  Play in moderation.  This is 
	obviously not meant to scare you off.  It is merely a word of caution 
	to some people.  Remember to play in moderation.

	If you do decide to join in an upcoming game of Empire, then you will
	have the pleasure of playing one of the best games around.  Empire has
	been described as being the most complicated game ever created.
	Despite the fact that the game was created for computers and has
	been in existence for several years, as yet no robot player has been
	developed nor is it anticipated.  It is possible to build such a 
	robot, but it would be a huge task.  Do not be dismayed though.
	Most of the complexity of Empire is handled by the server and client.
	There is a fair amount of what is called "micro-managing" that may
	need to be done to be a highly successful nation. However, it is
	not a necessary thing to do.  Furthermore, there are many tools 
	(described later) that can handle the micromanaging aspects to
	a great degree. 

	After you find out about a game, and decide that you do want to 
	play in that upcoming game, you need to contact the "Deity".  The
	"Deity" is the person who runs the game.  You should mail him/her
	with information about yourself and your country.  A customary 
	basic form for registering your country to play might contain
	the following information:

	Country name: (any name you like, from Brianville to USofA)
	Rep: (this is your *password*)
	Net address: (the address at which you can be contacted over the net)
	Skill level: (1-10..10 being the best, 1 being beginner)
	
	It might be necessary depending on the Deity to include all addresses


User's Guide to Empire							 9
	
	from which you might be playing.  With the proliferation of suns as
	general user machines this is becoming more and more a chore, and 
	few deities require it.  It might suffice to say "mickey@*.buffalo.edu"
	which might cover all machines you play from if you played from
	SUNY-Buffalo.  Mickey, of course, is just an arbitrary username chosen
	for this example.

	Empire players connect to games in progress through a program known
	as a "client".  This program handles communication between your
	computer and the computer that the game is running on. 

	There are several versions of clients available.  Some are more
	"capable" than others in that they can make decisions easier for 
	you or make the decisions themselves.  They are intended to reduce
	the amount of work you have to do to maintain your country though
	in some cases they actually increase the amount of work you have
	to do.  I have listed places from which you can get other types
	of clients from in the FTP listings in appendix C.

	Sources for basic client program is available by FTPing to 
	ucbvax.berkeley.edu.  For the user's benefit, I have included
	a copy of a session in which a user FTPs to ucbvax, retrieves the
	source, compiles the source and establishes the appropriate
	environment variables to connect to a game.  

	Note that the following example will work ONLY on a Unix,
	Ultrix, or other flavor of Unix operating system.  This will
	not work for the VMS client. 

-----------------------------<begin sample>---------------------------------

(PLEASE NOTE: Walk through comments in this sample will be enclosed
 in [** and **].  They are not part of what actually appeared on screen
 in collecting this sample.  They are meant only as guides.)

[** First, I create a directory to put the client in.  I then cd into that
    directory and then proceed with the ftp to ucbvax. **]

autarch{tmp}!>mkdir myclient
autarch{tmp}!>cd myclient 
autarch{myclient}!>ftp ucbvax.berkeley.edu
Connected to ucbvax.berkeley.edu.
220 ucbvax.Berkeley.EDU FTP server (Version 5.30 Tue Mar 28 18:11:37 PST 1989) ready.
[** Type in anonymous for your name **]
Name (ucbvax.berkeley.edu:mickey): anonymous
[** Send your real identity for the password **]
331 Guest login ok, send ident as password.
Password:
230 Guest login ok, access restrictions apply.
[** I am now logged in to ucbvax's public access FTP area.  Now 
    I cd into the "pub" directory, then the "games" directory, and then 
    the "bsd-empire" directory where I know the Empire client source 
    resides. **]


User's Guide to Empire							 10
	
ftp> cd pub
250 CWD command successful.
ftp> cd games
250 CWD command successful.
ftp> cd bsd-empire
250 CWD command successful.
[** Now, I set my file transfer mode to "binary" and set hash on so that
    I can see transmit progress on screen (# = 1024 bytes.) **]
ftp> bin
200 Type set to I.
ftp> hash
Hash mark printing on (1024 bytes/hash mark).
[** Now, I view the directory listing to confirm the name of the client
    source. **]
ftp> ls
200 PORT command successful.
150 Opening ASCII mode data connection for ..
empire-1.1.tar.Z
empire-1.1.patch.5
empire-1.1.patch.4
empire-1.1.patch.3
contrib.tar.Z
client.tar.Z
empclient-1.1.tar.Z
[** ^^^^^^^^^^^^^ it's this one. **]
README
emptools.tar.Z
#
226 Transfer complete.
143 bytes received in 0.03 seconds (4.7 Kbytes/s)
[** Now, I "get" the file and the transmission to my directory begins. **]
ftp> get empclient-1.1.tar.Z
200 PORT command successful.
150 Opening BINARY mode data connection for empclient-1.1.tar.Z (20859 bytes).
#########################################
226 Transfer complete.
local: empclient-1.1.tar.Z remote: empclient-1.1.tar.Z
20859 bytes received in 1.8 seconds (11 Kbytes/s)
[** The client source now resides in my directory, so I log out of Berkeley **]
ftp> quit
221 Goodbye.
[** I next confirm that the client is in my directory. **]
autarch{myclient}!>ls
empclient-1.1.tar.Z
[** Next, I have to uncompress the file (the Z postfix means it is in 
    compressed format).  I again ls to confirm it's presence. **]
autarch{myclient}!>uncompress empclient-1.1.tar.Z
autarch{myclient}!>ls
empclient-1.1.tar
[** Next, I have to tar the file (tar postfix means it's in tar format).
    Again, I ls to confirm operation completed. **]
autarch{myclient}!>tar -xf empclient-1.1.tar
autarch{myclient}!>ls


User's Guide to Empire							 11
	
client/			empclient-1.1.tar
[** Ah, ok.  I tar'ed the file and it created a directory for itself. 
    So, I cd into that directory and ls it. **]
autarch{myclient}!>cd client
autarch{client}!>ls
Makefile	empire.6	host.c		main.c		saveargv.c
README		expect.c	hpux.c		misc.h		servercmd.c
bit.c		fnlist.h	ioqueue.c	proto.h		serverio.c
bit.h		globals.c	ioqueue.h	queue.c		termio.c
dtable.c	handle.c	login.c		queue.h		termlib.c
[** There's the README file up there.  That will tell me what to do next. **]
autarch{client}!>more README

			BSD Empire 1.1


This is the empire client for BSD Empire.

To make the program, edit globals.c and change localhost to
point at the machine where the game is running.

Type "make".

You can change hosts on-the-fly by using environment variables -- see
the man page for details.

[** Simple enough.  I don't bother modifying globals.c since I can modify
    the environment variables using the Unix command 'setenv'.  So now,
    I just type 'make' and hope everything goes ok.  NOTE: This was done
    on a sun4.  Your results may vary slightly. **]
autarch{client}!>make
[** Now, it does the compilation automatically without me having to 
    do it by hand, I just typed the make and it does it by itself. **]
cc -g  -sun4 -c  bit.c
cc -g  -sun4 -c  dtable.c
cc -g  -sun4 -c  expect.c
cc -g  -sun4 -c  globals.c
cc -g  -sun4 -c  handle.c
cc -g  -sun4 -c  host.c
cc -g  -sun4 -c  hpux.c
cc -g  -sun4 -c  ioqueue.c
cc -g  -sun4 -c  login.c
cc -g  -sun4 -c  main.c
cc -g  -sun4 -c  queue.c
cc -g  -sun4 -c  saveargv.c
cc -g  -sun4 -c  servercmd.c
cc -g  -sun4 -c  serverio.c
cc -g  -sun4 -c  termio.c
cc -g  -sun4 -c  termlib.c
cc -g -o empire bit.o dtable.o expect.o globals.o handle.o host.o  hpux.o 
ioqueue.o login.o main.o queue.o saveargv.o  servercmd.o serverio.o termio.o 
termlib.o -ltermlib
autarch{client}!>ls


User's Guide to Empire							 12
	
Makefile	expect.c	hpux.c		misc.h		serverio.c
README		expect.o	hpux.o		proto.h		serverio.o
bit.c		fnlist.h	ioqueue.c	queue.c		termio.c
bit.h		globals.c	ioqueue.h	queue.h		termio.o
bit.o		globals.o	ioqueue.o	queue.o		termlib.c
dtable.c	handle.c	login.c		saveargv.c	termlib.o
dtable.o	handle.o	login.o		saveargv.o
empire*		host.c		main.c		servercmd.c
empire.6	host.o		main.o		servercmd.o
[** Walla, it compiled, and now there is a file in the directory called
    'empire' with a * next to it (meaning it's an executable.  Not all
    ls's will do this, see 'man ls' for further info on parameters for ls).
    That file is the client.  I could delete all of the '.c', '.h', and 
    '.o' files right now to clean things up, but I am anxious to get into
    the Empire game that is already in progress.  So, I next setup my
    environment variables so my client knows which computer to call and
    which port to use on that computer. **]
autarch{client}!>setenv EMPIREHOST oleander.cs.odu.edu
autarch{client}!>setenv EMPIREPORT 1617
[** EMPIREHOST and EMPIREPORT variables will have been given to you by
    the deity running the game. 
    With that done, I can now run my client and connect to the game. **]
autarch{client}!>empire
Country name? [** I type in my country name **]
Your name? [** And my password, NOT my real name. **]

    -=0=-

You have six new telegrams waiting...

[0:255] Command:
[** And you are now connected to the game and are ready to play dictator **]


				Figure 1
	    Sample of how to FTP and compile a client program

-------------------------------<end sample>-----------------------------------


	Since you are playing across a network of computers, it is possible
	that there will be problems along the way in connecting to the server
	from the computer you play on.  What follows are some common connection
	errors.

Problem 1:
autarch{client}!>empire
connect: Connection refused

Problem	  This generally means that you have either set your environment
	  variables wrong (see 'setenv' from earlier in this chapter), or
	  the server at the host site is not currently running.
Solution  (1) Check your environment variables by issuing a 'setenv' command.


User's Guide to Empire							 13
	
	  (2) Wait for the server to come up.

Problem 2:
autarch{client}!>empire
connect: Connection timed out

Problem   This generally means that there is a network outage somewhere
	  along the network.  
Solution  (1) You can monitor the network by using two Unix commands located in
	  /usr/etc on most Unix systems.  These are 'ping' and 'traceroute'.
	  /usr/etc/ping 128.205.7.3 for instance will send a ping at the
	  host with IP address 128.205.7.3.  If that machine can be reached
	  from your machine across the network it will return:
	  128.205.7.3 is alive
	  traceroute 128.205.7.3 will request responses from each site along
	  the way.  This is useful for seeing which site along the way is 
	  causing the outage. 
	  (2) This solution is easier, but more frustrating.  Wait. 

Problem 3:
autarch{client}!>empire
Country name? <your country name>
Your name? <your password>

        -=O=-
3 country in use by mickey@lictor.acsu.buffalo.edu (pid=5797)

Problem	  This one is potentially nasty.  It means that another process is
	  logged into your country.  This could be from multiple sources.
	  1: Your friend who is playing your country as well.
	  2: A process of yours that got accidentally hung.
	  3: The network hung your remote process even though your local
	     process is now terminated. 
Solution  (1) Go kick your friend in the head :-)
	  (2) Kill your process 'nicely'.  To find the process execute a 
	  'ps -x' command and find the appropriate process number:
	  autarch{client}ps -x
	    PID TT STAT   TIME COMMAND
	  3555 t1 S      0:00 -usr/new/csh (csh)
	  3575 t1 R      0:00 ps -x
	  3559 ?  S      0:00 -usr/new/csh (csh)
	  3560 ?  S      0:00 (empire)
	  The offending process number in this list is 3560.  To kill it,
	  simply type the command 'kill 3560'.  After that, all should be
	  well.  Try entering the game again. 
	  (3) If your process is hung on the server machine, you can
	  do one of two things.  First, you can mail the Deity with the
	  "pid", or process identification number.  This number is listed
	  in the original error message and in our case is 5797.  Secondly,
	  you can log in to the remote site and kill the process yourself
	  via the process illustrated below.  Note that in the first line
	  of the illustrated process that the name storm.tamu.edu is the
	  host site for the particular game I am using for this example. 


User's Guide to Empire							 14
	
	  Also, 1617 is the port on that machine that Empire is being run
	  through.  Commands that you should type in are indicated 
	  by a "<----" at the end of the line.

-------------------------------------------------------------------------------

% telnet storm.tamu.edu 1617     <----
Trying 128.194.15.98 ...
Connected to storm.tamu.edu.
Escape character is '^]'.
2 Empire server ready            <---- [** your country name in quotes **]
coun "Terminus"
0 country name Terminus
pass "busyemperor"               <---- [** your password in quotes **]
0 password ok
kill				 <---- [** this terminates the hung process **]
3 process 1056 (from mickey@autarch.acsu.buffalo.edu) terminated
[** next type in 'ctrl-]' and then type 'close' **]
^]
telnet> c
Connection closed.
%

				Figure 2
		Killing a process on a remote Empire host

-------------------------------------------------------------------------------

	  Your process is now unhung, and your country should be ready to play
	  once again. 

Problem 4:
autarch{myclient}empire
Country name? <your country name>
Your name? <your password>

Bad password

Problem   The server does not accept the password you typed as being correct.
Solution  Try typing it again.  Try using capital letters at the beginning of
	  the password as well, or the entire password in caps.  If this does
	  not work, mail the Deity and inform him or her of the problem
	  and what the password should be.

Problem 5:
autarch{myclient}empire
Alarm clock

Problem   Your connection to the server is not responding.  
Solution  The delay time for waiting for a response that is built into 
	  the client is not exceedingly long.  The theory behind this being
	  that if it is taking you that long to get a response from the 
	  Empire server then it is not worth playing, since response times


User's Guide to Empire							 15
	
	  for information from the game are going to be pretty unbearable. 
	  Keep trying is probably the best solution. 


	These are most of the problems that you will encounter in connecting to
	the game server.  There are some more problems that you might 
	encounter, but generally they are not common.  If you can not reach 
	your game server for more than a day, contact your deity and let 
	him/her know that you can not connect.  

Summary: Getting into a game of Empire is not terribly difficult. Setting up
	 a client program should be a fairly painless task.  If it is not,
	 do not be dismayed.  Get help from a consultant or another player
	 if you can.  Some problems with the network may temporarily prevent 
	 you from playing.  Be patient, the network is a horse that should 
	 be shot, but generally it comes around after a while. 


User's Guide to Empire							 16
	
Chapter 3 :-: The primitive basics.


	There are a few basic commands and ideas that will help you get 
	acquainted with Empire.  These are here summarized.

Concept: Sanctuary. 
	All players begin a game of Empire in what is known as a sanctuary.
While you are in a sanctuary state you can not be attacked, overrun, or 
otherwise hurt.  However, while you are in sanctuary you can not do
very many things.  In order to really begin playing the game you need
to do what is known as "break sanctuary".  You can do this using the 
\&'break' command.  When you do this, your two sanctuary sectors (which
everyone starts with) will become capital sectors.  You are no longer
protected from attacks from enemy countries. 

Concept: Movement in Empire.
	Paths of movement are indicated by several keys.  Path indicators
exist in a hex shape around the letter H on your keyboard.  Thus, y means
travel North-West, u for North-East, j for East, n for South-East, b for
South-West, and g for West.  Also, h means "stop" which terminates movement
of whatever you are moving.  There is one other key, v which "views" the
sector you are currently moving through.  Thus if I gave a movement path
of gggvgh the game would move whatever I am moving three sectors West, view
the sector at three sectors west of my starting point, then move one more 
sector west and then stop. 

Concept: Updates
	Empire games have "turns" known as updates.  When an update occurs,
your country goes through economic growth or decline, interests are paid
on gold bars, military costs are paid for, mobility increases in your
sectors, etc.  You can do a great many things between updates, an update
merely does book keeping more or less, at a predetermined time.  To find
out when the next update is going to happen, you can type 'update' as
a command.

Command: map -10:10,-7:7
	This command will give you a map of the area immediately surrounding
your beginning two sectors.  Specifically, it will give you a map from ten
sectors West of your 0,0 to ten sectors East and from seven sectors North to
seven sectors South of your 0,0.

Command: explore civ 0,0 1
	This command allows you to move one civilian from your 0,0 and 
"explore" into wilderness and mountain sectors.  

Command: info commands
	This command will list the various commands available in Empire.
It is a very good idea to go through the information on each command
by typing 'info <name of command>'.  This will help you get more acquainted
with the game.  Though this guide shows many commands and how to use them,
it is no substitute for the game's help itself. 


User's Guide to Empire							 17

Command: designate 1,1 +
	This command will enable you to designate a sector you own into 
whatever you desire.  Usually, in the beginning of the game you will designate
sectors that you will not be using actively for a while into highways.
The above shown example of the command will designate your 1,1 sector into
a highway.  

Command: telegram <country name>
	Countries in Empire communicate with each other via telegrams.
These are private letters sent just like a telegram would be.  They have
a size limit, but there is usually more than enough room to say what you
want to say.  There is no limit to the number of telegrams you may send.

Command: announce
	Countries broadcast to the world via the command 'announce'.  These
announcements are similar to telegrams except instead of one country reading
the telegram the entire world can read it. 

Command: read 
	This command allows you to read any incoming telegrams or
announcements.  Empire will notify you when you have new incoming traffic.

Command: news {x number of days}
	This command will allow you to peruse the Empire newspaper.  It does
not make for great reading.  However, it is useful for intelligence gathering.
If you just enter 'news' as a command, it will print all the news since you
last read the news.  Entering 'news 3' will give you the news for the last
three days.  In big games, the news file gets quite large.

Summary: These are the basic ideas and commands of Empire.  Empire has a great
	 many commands however, and the player should endeavor to learn them.
	 Empire can be played with only a few commands but the additional 
	 commands are there for your best interests.  If you do not understand
	 a command shown here, simply type 'info <command>' while in the 
	 game itself. 


User's Guide to Empire							 18

Chapter 4 :-: Geography in Empire.

	Maps in Empire are based on a simple hex design.  For example:

.eo
---------------------------------- There are three basic sector types in
				   Empire (note: "type" is different than
     / \   / \   / \   / \   / \   "designation".  Designation refers to what
    /   \ /   \ /   \ /   \ /   \  that sector has on it, a factory or school
-4 |  .  |  ^  |  -  |  ^  |  .  | for example.  Type refers to what it is 
    \   / \   / \   / \   / \   /  built on).  There are:
     \ /   \ /   \ /   \ /   \ /   seas	: .
-3    |  -  |  -  |  .  |  .  |    mountains	: ^
     / \   / \   / \   / \   / \   wilderness  : -
    /   \ /   \ /   \ /   \ /   \  When using the Empire map, you use an "X,Y"
-2 |  .  |  -  |  -  |  -  |  .  | system.  The X coordinate refers to how far
    \   / \   / \   / \   / \   /  along the horizontal you are, the Y referring
     \ /   \ /   \ /   \ /   \ /   to the vertical.  Thus, there are seas on 
-1    |  -  |  -  |  .  |  .  |    example map at  5,-1  7,-1  0,-2  8,-2
     / \   / \   / \   / \   / \   5,-3  7,-3  0,-4  and 8,-4.  Negative X is
    /   \ /   \ /   \ /   \ /   \  to the left (to the East) of 0,0 just as 
      0  1  2  3  4  5  6  7  8    in your high school math courses.  However,
				   negative Y is above (to the North of) 0,0.
	     Figure 3		   This is different from high school math. 
     Example of an Empire map      Why the maps have coordinates in this way
				   is one of the great mysteries of Empire.
----------------------------------  
.ec
				    
	Figure 4 below is a real map of Empire, in its real size and form.
	Figure 3 is "zoomed in".  Note that the '?' marks on the right part of 
	the map belong to a neighboring country.  In this figure all wilderness
	type sectors have been designated into something other than a 
	wilderness There are several mountains, and a lot of ocean visible as 
	well. 



User's Guide to Empire							 19

--------------------------------------------------------------------------------

[309:112] Command : map #1
     33333322222222221111111111---------00000000001111111111222222
     5432109876543210987654321098765432101234567890123456789012345
  -3 + + + + + . . . . . . . . + . . . . . . .                     -3
  -2  a ^ + m ^ + . . . . o . + . . . . . . . .                    -2
  -1 + + + + + + + + . . . a o a . + + + . . . .                   -1
   0  + + + + + + + g o o o + + + . + o c m . . .                  0
   1 + + ^ o + a j m g w g ^ ^ m + j j + . + . . .                 1
   2  + + a + + + + + + + + . b j + m + w + . . . . . .            2
   3 + a . a a + b + m o j + t + + + + + + . + . + + + . . ? ? ?   3
   4  . . . . . a g + ^ + m + + l ^ ^ ^ + o + + + + + + + + ? + ?  4
   5 + . . + + + + + ^ + ^ m + j ^ ^ c + m + + + + + + + ^ + + + ^ 5
   6  .   . . + + ^ + + + + m + + o o + k + + + + + + ^ ^ + + + +  6
   7       . + + + + + + + + o m + a o + e + + + ^ ^ a + + + + + ? 7
   8      . + + + + a + ^ + g d m + w m + m + + + + + ^ + + + + ?  8
   9     . + + + + + + + + + + + k + g t + + + + + + + + ? ? ? ?   9
  10      . . + + + + ^ + ^ ^ g + + + p g + i g + + + + ?          10
  11     . + + + + + ^ + + + ^ + + m + g o + g + + + + + ? ? ? ?   11
  12      . + + + + + + + + + + + g g + + + + g + + + + + + + + ?  12
  13     . + + + + + + + + + + + + + g + g m + w + + + + + + + + ? 13
  14      . . . + + + + + + + + + + + g + + m + + + + + + + + + +  14
  15     . + + + + + + a + + + + + + + + m + + + + + + + a + + + + 15
     33333322222222221111111111---------00000000001111111111222222
     5432109876543210987654321098765432101234567890123456789012345

				Figure 4
	     	     Realistic version of an Empire map

-------------------------------------------------------------------------------

	The following listing is taken directly from the command 
	'info sector-types' in the game.  There is a lengthy description of 
	each sector type also in this command, but I have omitted it here 
	for brevity.


User's Guide to Empire							 20


--------------------------------------------------------------------------------

    
BASICS                   INDUSTRIES                MILITARY / SCIENTIFIC
    .  sea                   d  defense plant          t  technical center
    ^  mountain              i  shell industry         f  fortress
    s  sanctuary             m  mine                   r  research lab
    /  wasteland             g  gold mine              n  nuclear plant
    -  wilderness            h  harbor                 l  library/school
    c  capital               w  warehouse              e  enlistment center
    p  park                  u  uranium mine
    COMMUNICATIONS           *  airfield               FINANCIAL
    +  highway               a  agribusiness           b  bank
    )  radar installation    o  oil field
                             j  light manufacturing
    #  bridge head           k  heavy manufacturing
    =  bridge span           %  refinery

				Figure 5
		  Table of sector types and designations 

-------------------------------------------------------------------------------

	As you can see, there are a wide variety of sector types in Empire.
	However, until your country becomes populated the most useful sector 
	is the highway ('+'), for it allows you to move things around easier, 
	even if the highway has only begun being constructed 
	(ie, 0% efficient). 

Summary: Empire maps are based on a simple hex design, with the movement
	 keys based similarly around the letter h on standard keyboards. 
	 These movement keys are a basic tool in Empire and must be learned.


User's Guide to Empire							 21

Chapter 5 :-: The beginning phases of Empire.


	Typically a game starts with a `speed' phase. If it does not, then
	initial play can get boring quickly.  A `speed' opening also demands
	more time for the period when the updates are frequent, but hopefully,
	that will be balanced by your efficient country and the less time
	spent running it when your classes/real life demand more time. 

	There are several opening strategies for Empire players.  I am going
	to go through a few of them here.  If you feel you can make a better
	strategy feel free to do so. 

	Basically, all strategies for the opening phases revolve around a 
	few basic ideas.  First, you want to secure as much land for yourself
	as possible.  Secondly, if you share an island with some other person,
	you want to leave yourself in a strategically advantageous position
	against your opponent, optimally to surround him/her before s/he breaks
	sanctuary.  These two ideas are the most important things to consider 
	prior to the first update.  There are other things to consider, which 
	will be reviewed later.

	It is highly important for you, the player, to begin playing the game
	as soon as the game begins.  If you do not, you risk being surrounded
	before the first update.  Furthermore, not breaking sanctuary prior to
	the first update can cost you a great deal in terms of population 
	growth and the establishment of your economy.

	Some players will use tools, which are reviewed in chapter 14, to 
	break sanctuary and occupy land from the beginning.  There are other 
	means to aid your land occupation as well.  If you have a programmable 
	terminal, or terminal program, you can program a key to contain a 
	particular command such as "explore civ 0,0 1 jjjujuuu".  This could 
	be a path along a line of land that you have already occupied, and 
	wish to expand beyond.  If you had to repeatedly type in that command 
	and path you would take a much greater amount of time to occupy land.
	This can also be done through X windows, using the mouse and cut/paste
	capabilities. 

	There are a few different theories on how to explore out from your
	sanctuaries to claim as much land as possible for yourself. 

	In the following maps, the square island shown is entirely wilderness
	except for the two sanctuaries in the middle.  It is important to note
	that prior to exploring the entire island, the only thing you would 
	be able to see on your map would be this:


User's Guide to Empire							 22


	    - - - - -
	   - - - - - -
	  - - - - - - -
	 - - - s s - - -
	  - - - - - - - 
	   - - - - - -
 	    - - - - -

	It is your job to explore the island, while occupying, or securing for 
	yourself, as much land as possible.  Note that for the sake of 
	simplicity I have not placed another country on this example island.  
	If a sector is a "+","c", or "s" it is an occupied (by you) sector.  
	If it's a "-" it's a wilderness sector, and if you can move into it, 
	it is unoccupied.  This is an important piece of information.  If you 
	can note move into a wilderness sector it means that it is occupied by 
	another country.  Note that "." are seas, as indicated in chapter 5.
	The basic commands to complete this exploration are 'explore' and 
	'designate'.  Learn about those commands either from chapter 4 or 
	the game before attempting these theories. 

	First, I show the entire island prior to your breaking sanctuary and
	exploring:

--------------------------------------------------------------------------------

	  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	 . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	 . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	 . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	 . - - - - - - - - - - - - - s s - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	 . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	 . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	 . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

				Figure 6
		Unexplored island prior to breaking sanctuary

--------------------------------------------------------------------------------

	Next, I show theory 1, occupying in a concentric circle. 
	Note that once you have broken sanctuary (using the 'break'
	command) that your two sanctuary sectors immediately become
	capitals, with your real capital at 0,0.


User's Guide to Empire							 23


--------------------------------------------------------------------------------

	  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	 . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	 . - - - - - - - - - - - + + + + + + - - - - - - - - - - - - - .
	  . - - - - - - - - - + + + + + + + + + - - - - - - - - - - - - .
	 . - - - - - - - - - + + + + + + + + + + - - - - - - - - - - - .
	  . - - - - - - - - + + + + + + + + + + + - - - - - - - - - - - .
	 . - - - - - - - - + + + + + c c + + + + + - - - - - - - - - - .
	  . - - - - - - - - + + + + + + + + + + + - - - - - - - - - - - .
	 . - - - - - - - - - + + + + + + + + + + - - - - - - - - - - - .
	  . - - - - - - - - - + + + + + + + + + - - - - - - - - - - - - .
	 . - - - - - - - - - - + + + + + + + + - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	 . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

				Figure 7
		Island showing exploration using theory 1

--------------------------------------------------------------------------------

	So far, 84 sectors have been occupied.  Now I show theory 2, 
	occupying in spokes with concentric hexes spaced every few sectors 
	apart:

--------------------------------------------------------------------------------

	  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	 . - - - - - - - - - - + + + + + + + + - - - - - - - - - - - - .
	  . - - - - - - - - - + + - - - - - + + - - - - - - - - - - - - .
	 . - - - - - - - - - + - + - - - - + - + - - - - - - - - - - - .
	  . - - - - - - - - + - - + + + + + - - + - - - - - - - - - - - .
	 . - - - - - - - - + - - + + - - + + - - + - - - - - - - - - - .
	  . - - - - - - - + - - + - + + + - + - - + - - - - - - - - - - .
	 . - - - - - - - + + + + + + c c + + + + + + - - - - - - - - - .
	  . - - - - - - - + - - + - + + + - + - - + - - - - - - - - - - .
	 . - - - - - - - - + - - + + - - + + - - + - - - - - - - - - - .
	  . - - - - - - - - + - - + + + + + - - + - - - - - - - - - - - .
	 . - - - - - - - - - + - + - - - - + - + - - - - - - - - - - - .
	  . - - - - - - - - - + + - - - - - + + - - - - - - - - - - - - .
	 . - - - - - - - - - - + + + + + + + + - - - - - - - - - - - - .
	  . - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - .
	   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

				Figure 8
		Island showing exploration using theory 2

--------------------------------------------------------------------------------



User's Guide to Empire							 24

	Now, 84 sectors have once again been occupied.  However, an additional
	56 sectors are now within the borders of your country.  Thus, securing
	a total of 140 sectors for yourself.  The disadvantage?  You have three
	lines of occupation ( the "+" sectors) between the center of your 
	country and unsecured wilderness.  In theory 1, the same number of 
	sectors provides for 4 lines for most of the country.  Another 
	disadvantage is that it costs more mobility to use theory 2.  The more
	lines of occupation you have between the central areas of your 
	country and the unexplored wilderness or an enemy the easier it is
	to defend your country. 

	A third theory is the idea of coastal occupation as a means of securing
	a large island.  The idea being to secure the land immediately around
	you and then explore in one direction until you find seas, and then
	proceed to occupy the coast:
 
-------------------------------------------------------------------------------

	  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	 . + + + + + + + + + + + - - - - - - - - - - - - - - - - - - - .
	  . + - - - - - - - - - + - - - - - - - - - - - - - - - - - - - .
	 . + - - - - - - - - - - + - - - - - - - - - - - - - - - - - - .
	  . + - - - - - - - - - - + - - - - - - - - - - - - - - - - - - .
	 . + - - - - - - - - - - - + + + + - - - - - - - - - - - - - - .
	  . + - - - - - - - - - - + + + + + - - - - - - - - - - - - - - .
	 . + - - - - - - - - - - + + c c + + + + + + + - - - - - - - - .
	  . + - - - - - - - - - - + + + + + - - - - - + - - - - - - - - .
	 . + - - - - - - - - - - - + + + + - - - - - - + - - - - - - - .
	  . + - - - - - - - - - - - - - - - - - - - - - + - - - - - - - .
	 . + - - - - - - - - - - - - - - - - - - - - - - + - - - - - - .
	  . + - - - - - - - - - - - - - - - - - - - - - - + - - - - - - .
	 . + - - - - - - - - - - - - - - - - - - - - - - - + - - - - - .
	  . + + + + + + + + + + + + + + + + + + + + + + + + + - - - - - .
	   . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

				Figure 9
		Island showing exploration using theory 3

--------------------------------------------------------------------------------

	Here, once again, 84 sectors have been occupied.  In addition, 179 
	sectors of wilderness have been secured, ie cut off from another 
	country.  This brings a total of 263 sectors occupied or secured.  
	This is obviously impressive.  However, it has its drawbacks as well.
	If there was a country bordering this country, that country would 
	need only to attack and occupy one of your sectors and then they 
	could explore into all of the "secured" sectors.  In addition, there 
	are only 2-3 lines of occupied sectors between the borders and the 
	center of the country.  This also costs more in terms of mobility than
	either theory 1 or 2. 

	The next section contains a strategy submitted by Scott Yelich.  Scott
	adheres in this summary to theory 1 as his method of exploration:


User's Guide to Empire							 25


  "Designate one of the two initial [sanctuary] capital sectors to
an agricultural center.  Then move a civ out from either sector to
either 1,-1 or 1,1 and designate it a +.  Then, I continue to move out
one civilian at a time from both of my initial sectors.  Only move
the civ one spot into the wilderness and then I designate that sector
as a highway (+).  Then, do a map to see if there is anything
especially interesting... such as, a mountain, more land or ANOTHER
PLAYER.  Of course, Only do that map at certain times... it's not
necessary to do a map after EVERY explore since explore will show you
a one sector radius!

[Editor's note: exploring with one civilian across a wilderness sector
takes .8 mobility.  Across a highway, even a 0% highway, it will cost
only .2 mobility.  Thus, the reason for designating new land as soon
as you occupy it.  You will conserve mobility if you only explore into
lands bordering your new highways.]

  "Here is where quick action comes into play.  If you spot any controlled
sectors (?) or a (-) that can't be entered (if you can note move into the
sector it is occupied by someone else! hurry!), try to make a boarder as
quick as possible by following the pattern above... move one civ, look,
repeat.  If you don't find any controlled sectors, simply [try] to
AVOID MOUNTAINS at ALL costs...  especially during the first couple of
updates.  Typically, from 30 to 70 sectors can be grabbed this way
within the first minute or two.

  "To sum up, the strategy for this is: 1) You want to see who is out there, 
2) You want to see what land is out there, 3) when you take more sectors 
during the initial updates, you will get more mobility each update 
(typically 8, 'version' will tell...).  Anyway, the more sectors you grab 
with the first update, the more you can grab the second.  It's like an 
exponential function."


	This next section is a strategy submitted by Geoff Cashman.  Geoff 
	adheres in this summary to theory 2 as his method for exploration:

  "The basic strategy of moving one sector into the wilderness and then 
designate them with a command such as this:  designate #1 ?des=- +  which will 
designate all sectors in realm #1 that are wilderness into highways, is a 
common one.  Using it wisely is not always done.  My personal method is to 
expand only in a few directions, ie 3-6 different directions from your 0,0 2,0 
area.  The idea here being to make "spokes" of exploration radiating out from 
your 0,0 and 2,0 area.  Using this method, you can use less mobility and see 
more land though not actually own more land until after the first update.  
Another advantage is you might reach an opponent before s/he reaches you.  A 
weakness is that an opponent can explore inbetween your spokes.  If I feel 
this is possible inbetween two spokes then I connect the spokes via concentric 
(on 2,0 0,0 ) arc several sectors out from the 0,0.  If you do not use this 
method, then you would use up all of your mobility to achieve the same number 
of sectors by the second update.



User's Guide to Empire							 26

  "Why save mobility?  Mobility in the 0,0 sector is especially critical.
The reason is because the 0,0 starts out with 999 civs.  If a sector has
999 or more civs it will not give birth to more civilians.  Half of the
battle in winning Empire is population explosion at the beginning of the
game...ESPECIALLY if you get involved in a low tech war with a neighbor
early on.  More population means more resources mined, more supportable
military, and more taxpayers.  If you are going to be a tech producer then
it may be desirable to limit population, but early on I do not recommend
it.  So, with 999 civs in your 0,0 sector you need to be able to move out
many civs (depends on the number of ETUs per update (see the version command)
as to how many you should move out, see number of births per 1,000 civs per
etu in the version command).  Thus, my strategy is slightly different. 
I concentrate on getting a 100% highway built at 1,-1 or 1,1.  Why those
sectors?  because I can then use the mobility in 1,-1 or 1,1 to move civs
or products into 2,0."


	The use of the third theory requires a player with some experience 
	to deal with its shortcomings.  For the beginner, I do not recommend
	it. 

	The following was submitted by another player to point out their means
	of establishing their production and the beginning stages of their 
	economy:

"Setting up a distribution network, even with inefficient warehouses, is
fairly critical early on.  If you starve civilians in the beginning of the
game, you will fall behind in production.  It is not uncommon to lose 
contact with the game for a day or more due to net failures.  Do not 
depend on your capability to connect to the game to feed your civilians.

"Some people never leave their 0,0 or 2,0 designated as a capital during the 
beginning of the game.  They check the resources in 0,0 and then determine if 
it's suitable for being a mine or a gold mine.  If neither, then they designate
2,0 into a mine.  If mine, then 2,0 becomes a gold mine.  Using this method
they can get maximum production from their civilians that are essentially 
trapped for the time being in their 0,0 and 2,0 sectors due to lack of 
mobility.  Note that in doing this that you must conserve BTUs.  Having
a 0% efficient capital somewhere else will cause a lack of production of
BTUs.  You must be careful not to run out of BTUs completely, or it could
cause you a great deal of grief. 


"NOTE: IT IS EXTREMELY DANGEROUS TO LEAVE A CAPITAL ON THE COAST AFTER
ANYONE PRODUCES A BOAT.  Until that time, a capital on the coast is
an advantage in some ways.  It is easier to defend (less places that
it can be attacked from).  You MUST get some civilians into the capital
at -2,0 (about 50) before the first update along with some food.  
In general, your capital will not produce BTUs for a while, but neither
do you need them for a while.  However, leave your 0,0 sector as your
capital until JUST prior (like 2-4 minutes, make it 10 to be safe) to
the update.  Does this all sound complicated?  Yep.  But it works. 



User's Guide to Empire							 27


"After the first or second update I will occupy the remaining wilderness
that I can see, using the mobility from the sectors that I explored into
in the previous update.   I will then continue to move civs out of 0,0 and
into 1,-1 or 1,1 (the highway that I am trying to make 100%) and move from
there.  If I am not going for tech production then I don't need the library
or parks (you can still make education, it helps in some regards, but for
the most part you don't need it if you are not going the way of tech.

You can then pretty much do as you please and work towards the goals you
think are reasonable for your country.  "

	One matter that can not be overlooked by any player is the simple
	fact that people need food to survive.  If a sector has less than
	60 people in it total, including civs, soldiers, and uws, then it
	will not starve.  But if after the update it has more than 60 total
	people will starve.  Make sure to feed your people!

	Ultimately, you want to take over every sector on your island.
	If you have neighbors on your island, you will probably have to share.
	Just try to get a bigger portion of it than your neighbors do.

Summary: There are several basic ideas expressed in this chapter:
	 1) Break sanctuary as soon as the game begins. 
	 2) Secure as much land for yourself as possible.
	 3) Use your civilians efficiently.
	 4) Do not allow your 0,0 sector to have a lot of civs in it.
	 5) Get a distribution system set up early (explained later).



User's Guide to Empire							 28

Chapter 6 :-: Diplomacy.

	It's your own ruin or success.  One of the best players of the game
	(who shall go un-named) has had severe problems due to lack of skill 
	in diplomacy.  Remember, there is another person on the other end of 
	that telegram.  They are human like you are, and like to be treated 
	like it for the most part.  There are jerks, and there are non-jerks.  
	Determine for yourself and go along with what you think is best, but 
	remember, it is ok to lose a few battles so long as you win the war.  
	A diplomatic loss can be an overall victory.  

	One of the most powerful weapons in the game is information. 
	What types of information can you use to your benefit?  Information on;
	the locality of other countries, the plans of opponents, the trading 
	going on between certain countries, etc.  There is also a very powerful
	tool right here in this document: how to play the game well.  There 
	was a classic case of a country (myself) who accidentally announced 
	his attention to attack a bordering country.  The bordering country 
	happened to be on, an began an attack.  The attack stalemated for the 
	most part because of the obvious inexperience of the person running 
	that country.  I offered information to him on how to play the game 
	better in exchange for his surrender and some other terms of agreement.
	I turned a horrendous diplomatic mistake into a diplomatic success 
	within two hours.  This is obviously the rare case (surprised me too), 
	but is a good example of the power of information.

	Another aspect of diplomacy is style.  You project a style in your
	telegrams and announcements whether it is intentional or not.  If 
	people like you they tend to side with you more than the next guy.  
	Of course, if you are a jerk but a powerful one people tend to 
	reluctantly side with you.  Then again, they are more likely to 
	backstab you. 

	While I am on that subject:  Backstabbing is a bad idea in general.
	It leads to distrust of many players with regards to yourself and tends
	to give yourself backstabbist's wrist if you do it too much.  Even if 
	you change your country name from game to game people tend to pick 
	out who the major jerks are and their previous names or even net 
	addresses.  Trust is important.  If you have allies you can trust 
	implicitly you will sleep a lot better at night and also have people 
	who you can trust to run your country for you while you are away for a 
	few days pursuing real world goals.  If you are going to backstab 
	someone crush them while you can and make sure their allies can not
	get at you.  Furthermore, make the world see that the person was a 
	fool for trusting you in the first place (like, due to previous war 
	or some such).  Sometimes, that extra attack you get without 
	interference from automatic defenses due to a declaration of alliance 
	with your opponent can be critical.  You can also map an opponent 
	country if you are allied with them and they will never know it.  
	Thus another good reason why you should be trustable and can trust 
	other close allies.  Having allies is important, especially 
	in the current market games. 



User's Guide to Empire							 29

A)  What is diplomacy? 
	The dictionary defines diplomacy as "1: The art and practice of 
	conducting negotiations between nations.  2: Skill in handling
	affairs without arousing hostility."  The second definition may
	not be entirely accurate in that in Empire, diplomacy can sometimes
	be used to cause battles, rather than avoid them, to gain a particular
	end.  I will go into this later.  

B)  What purpose, diplomacy?
	Since Empire is a multi-user game the diplomacy side of the game
	is one of the main factors inherent to its playing.  Being able
	to interact with other players, make bargains, treaties, alliances,
	sales, war, etc. leads to an incredibly variety of possible
	scenarios among various countries.  In asking, what purpose is
	there to diplomacy, it must first be realized that diplomacy in
	the game of Empire is virtually unavoidable.  To what degree
	you involve yourself in diplomacy is the true measure.

	Diplomacy can be utterly disastrous for you.  It can also be
	very beneficial.  It can also be a mediocre tool that goes mostly
	unused.  If you master the techniques of diplomacy you will find
	yourself with a very valuable and powerful tool.  If you fail
	at grasping the basic concepts of diplomacy, then you may find
	yourself the quick victim of a diplomatic trick.

C)  General types of relations amongst nations in Empire
	1) No contact.  The two countries have never been in contact.
	2) Allies.  Several types:
		a) Old friends, or local site friends.
		b) People whom you trust more than other people
		   in the game, but are not entirely comfortable with.
		   Nevertheless, you ally with them for lack of allies.
		c) Enemy allies.  People whom you ally with whom you
		   intend to attack.  
		d) Undeclared allies.  You conduct a lot of business with
		   the country, but you do not officially declare your 
		   alliance. 
	3) Hostile.  Generally speaking, any country who is unfriendly
	   towards you.  May not necessarily be declared hostile. 
	4) At war.  If you have not figured out what this means, stop
	   reading, never play Empire, and go back to elementary school :-)
	5) Neutral.  The broadest category.  You've dealt with this person
	   but have not decided one way or another your relations.  It might
	   be a person from a previous game or a total unknown.  

d)  Types of diplomacy
	1) Active diplomacy
		You interact with the country on a very high level.  Whatever
		their relations with you, a lot of correspondence is going on.
		This is probably the easiest form of diplomacy to invoke.
	2) Passive diplomacy
		Low level contact only.  In essence, you do not work with
		this country.  Some goals served by this:  Country in question


User's Guide to Empire							 30

		learns little about you; Country in question may give you more
		info than you give him or her; etc..
	3) Indirect diplomacy
		Diplomacy through a middle country to another country.  This
		is most often seen in situations where a known friend is an 
		ally of a country you want to be allied with.  You attempt 
		to use your friend to convince the prospective ally to ally
		with you. 
	4) Median diplomacy
		Being the middle person in a triad or more of allies that
		may not be already allied.  This can be a VERY dangerous
		position to be in.  The other sides of the triad or more
		may have certain expectations of you, which go against what
		another country may want from you.  This is essentially 
		sitting on a fence. 

E)  Some goals that can be achieved through diplomacy:
	1)  Getting another country to ally with you so that you can
	    overfly his country at will without planes scrambling.
	2)  Getting two countries whom you dislike to go to war against
	    each other. 
	3)  Getting another country to perform an act that is beneficial
	    to you or your allies. 
	4)  Getting a reduction in price of certain commodities or articles
	    from another country.
	5)  Gaining an ally.
	6)  Assisting in destroying an enemy.



	One of the most dangerous means by which diplomacy can be used was 
	written about in the book "Red Storm Rising" by Tom Clancy.  If you 
	have not already read this book and you are an Empire buff, it is 
	strongly recommended reading.  This method is referred to as "The 
	Maskirovka" or "Masquerade".  It is often referred to as the "big lie" 
	by the victims of this trick.  Essentially it works as follows:

	a) You wish to attack another country, but would like to have things
	   sightly different before the attack begins.  
		o  You may want that person to be your ally before the attack 
		   so you can overfly him/her without being shot down. 
		o  You might want to get him/her to give you a loan prior to 
		   your attack.  
		o  You might want them to move their fleet off in another 
		   direction or in closer to your forts.
		o  You might want them to give you certain materials of war
		   that you can use in your attack against them.
	   All of these things and much more can be accomplished with 
	   the concept of maskirovka.  Decide what you want to achieve.
	b) You set out to give the impression to the target country that you
	   are in a particular state of matters.  This can be accomplished
	   in various ways depending on what you are trying to achieve.
	   You can have another country attack you for instance to make it


User's Guide to Empire							 31

	   appear that you need military arms or supplies to hold them off.
	   This is one small example of what can be done.  Ensure that the
	   image you are presenting can ultimately lead to getting what you
	   want to achieve accomplished. 
	c) You must be convincing in your impression.  Use all means reasonable
	   to ensure the target country is getting the right image.  Using 
	   e-mail with collaborating countries to coordinate such matters 
	   is often best. 
	d) As you begin feeling that you have achieved the correct image,
	   begin asking the target country for whatever it is you are trying
	   to achieve.  Do not come right out and say it.  That may cause
	   the whole trick to collapse on you.  Approach it sideways so to
	   speak, and try to get the target country to offer whatever it is
	   without you directly asking for it.  
	e) If the country agrees to the transaction of events you are seeking,
	   all is well and good.  You have achieved your goal.  Keep in mind
	   that if you have played the maskirovka well, you have not yet 
	   committed yourself in any particular direction with any sense of
	   strong commitment.  At this point, you can always back down from
	   your further goals for which the maskirovka was designed to help.
	   The maskirovka at this point will have given you a more favorable
	   situation without having committed yourself.  It may also have
	   enabled you to see more into the mind of your opponent, often
	   a valuable commodity.  
	f) If you have failed in your maskirovka, you may still have made
	   some gains.  You again might know more about your opponent than
	   before.  You might have reached some of your goals that the
	   maskirovka set out to accomplish.  Lastly, you have not committed
	   yourself in any particular direction, yet you have tried to
	   enhance your position.  Not a bad proposition. 

	There are many variations to maskirovkas.  The guides listed above are 
	very general at best, and are meant more to give you an idea of how to
	build a plan for maskirovka than anything else.

	Chess players who have ever played in matches or tournaments will 
	probably feel right at home in scheming a maskirovka.  You must try to 
	get into the mind of your opponent, understand what s/he sees from 
	his/her vantage point and get a feel for how you think s/he will 
	respond.  You must think as if you were your opponent.  Understand 
	their vantage point as best as you can.  Do not think of what you 
	would do in a given situation, think what they would do.  Think 
	consequences to every action you can take. 

	This usage of diplomacy is "active diplomacy", sometimes mixed with 
	"indirect diplomacy".  

	Another oft seen ploy of sorts is one in which you befriend a dying
	rich country.  They may be on there way out, and there may not be a 
	whole hell of a lot you can do for them.  However, befriending them 
	towards the end may bring you gifts of money or lower market prices 
	from them when they dissolve.  Show sympathy for their situation.  
	You can be sincere, or you may chose not to be.  Sometimes a dying 


User's Guide to Empire							 32

	country has no friends to whom they will leave their money.  If your 
	the first to show sympathy, they may be willing to give you much in 
	return.  This is an active form of diplomacy, but is rarely seen 
	except after a long period of passiveness relative to that country. 

	Posturing.  Probably the most common form of diplomacy.  This basically
	means presenting an image to a possible attacker that you are more 
	capable than you appear.  In general, people tend to believe it.  
	Consider the simple fact that generally speaking a good Empire player 
	spends a lot of time working on his or her country.  Then, let them 
	believe through posturing that they are at a greater risk than
	they actually are.  They may tend to get possessive about their 
	country and not want to risk its destruction and the eventual loss 
	of so many sleepless nights and hours building the country in the 
	first place. 


Summary: Diplomacy is a powerful tool.  Some players consider it the
	 most powerful aspect of the game.  In many ways that can be shown 
	 to be true.  However, do not let other facets of your country 
	 decline and spend a huge amount of time building an excellent 
	 diplomatic situation.  The success or failure of diplomacy depends a 
	 great deal on the other players perception of your words.  If your 
	 diplomacy fails, you may find that you will be forced to back up what 
	 you say with military force.  



User's Guide to Empire							 33

Chapter 7 :-: Setting up your defenses. 

	There are various means of "automatic" defense that Empire employs 
	against enemy invaders.  These can be outlined in three major areas:

	1: Home air defenses

		Empire has numerous planes and plane capabilities.  The higher
		your tech level, the better the planes you can build.  For 
		automatic defenses you are concerned with planes that have the
		"intercept" capability listed next to them in the command, 
		'show planes c'.
		
		Generally speaking, as some games diverge from this list, the
		planes that have intercept capability are: 
			a: Fighter 1       (also has VTOL capability)
			b: Fighter 2 
			c: Escort 2        (in SOME games)
			d: Jet fighter
			e: Jet fighter 2   (also has VTOL capability)
		
		These planes are based out of airbases (except for the VTOL
		planes which can scramble from airbases and non-airbase 
		sectors) and will automatically intercept incoming enemy 
		planes.  

		notes: 1: Planes will only scramble if there is petrol in
			  the sector they are scrambling from.
		       2: Airbases must be at 60% or better efficiency to
			  scramble.  This does not apply to other sectors
			  from which VTOL planes are scrambling, nor does it
			  apply to VTOL planes in airports.

		In this version of Empire it is VERY important to have a good
		home air defense network.  Having a great many planes with
		intercept capability without proper planning is not a good
		defense.  You must learn to use the planes wisely.

		Hints for good home air defense: 
		1:  Have airports within intercept range of each other.
		    A plane's intercept range is 1/2 of its range indicated
		    in the command 'show planes c'.  For example:


User's Guide to Empire							 34


-------------------------------------------------------------------------------

[513:36] Command : show planes c
                     acc load att def ran fuel abilities
Sopwith Camel, fight  30    1   3   2   6    1 tactical intercept VTOL light
TBD-1 Devastator, lt  30    2   1   2   8    1 bomber tactical VTOL light
C-56 Lodestar, trans   0    7   0   1  15    2 cargo
B-26B Marauder, hvy   85    6   2   3  15    2 bomber

				Figure 10
		  Results from 'show planes capability'

-------------------------------------------------------------------------------

		    In this list of planes, only the Sopwith Camel plane is 
		    capable of interception.  It has a range of 6, and thus
		    it can intercept enemy planes up to 3 sectors away from
		    the sector it is based in.  Have your planes based in
		    sectors that are three sectors away from each other. 
		2:  Always have a substantial number of military in your
		    airports.  It is all too easy for an enemy to track
		    down your airport and capture it, thus possibly destroying
		    all your planes, and in the very least taking them 
		    from you. 
		3:  Ensure that all sectors that you have planes based in
		    have a sufficient amount of petrol for them.  Note that in
		    the above figure, the Sopwith Camel uses 1 fuel per 
		    mission.  Thus, having 10 or more petrol per Sopwith
		    would be sufficient. 
		4:  Do not base your entire defense on non-VTOL capable 
		    interceptors.  If you do, then your enemy can remove
		    your air defenses just by taking out your airports. 
		    Sopwith Camels (Fighter 1s) and Jet Fighter 2s are 
		    the VTOL intercept capable planes.  Sopwith Camels 
		    generally become useless for intercept duty as the 
		    game wears on.  After all, a paper airplane can not 
		    put up with cannon fire from an F-15 for very long!
		5:  It is a good idea to have each sector in your country
		    covered by at least two airbases (or non airbases from 
		    which your are basing VTOL planes).  This makes it more 
		    difficult for your enemy to locate where your airbases are.

	2: Forts and guns

		Forts will automatically fire on any enemy army units that
		attack land of the defending nation, assuming that the guns
		in the fort have the range to reach the aggressor.  This 
		also assumes that the fort has shells and military.
	
		Fort gun range is determined by multiplying the number of guns
		in the fort by the technology factor of the nation.  This
		number can be found in the nation report [see command: nation].


User's Guide to Empire							 35

		Here's an example of calculating your gun range:

--------------------------------------------------------------------------------

[77:144] Command : nation

        Terminus Nation Report  Sun Oct 20 18:27:08 1991
Nation status is ACTIVE     Bureaucratic Time Units: 144
100% eff capital at -3,5 has 999 civilians & 0 military
# 0  -70:-40,-30:-16    # 1  -35:25,-3:15    # 2  -30:15,-4:7
# 3  -45:5,7:22    # 4  -45:5,7:17    # 5  -65:-20,-31:0
# 6  -65:-20,-12:0    # 7  -80:-30,-5:10    # 8  -8:10,-5:5
# 9  -32:32,11:22    #10  -50:18,-12:10    #11  -40:-10,-5:5
#12  -38:29,-3:23    #13  -65:30,-18:6    #14  -20:20,16:22
#15  -30:30,-3:23    #16  -40:20,7:25    #17  -80:30,-30:23
 The treasury has $57485.00     Military reserves: 1676
Education.......... 17.78       Happiness....... 13.42
Technology......... 52.78       Research........  1.44
Technology factor : 40.66%      Plague factor :   1.51%

				Figure 11
		      Results from a 'nation' report

--------------------------------------------------------------------------------

		To calculate the gun range, multiply the number of guns in
		the fort (up to a maximum of seven) by the number listed in
		"Technology factor" which is shown in this figure as 40.66.
		Then, divide the resulting number by one hundred and that
		is the number of sectors away the gun can fire at.  In this
		case, if you have seven guns the resulting range is 2.85 
		sectors.  Effectively, the fort can fire just short of three
		sectors away. 

		There is an additional aspect of forts which make them 
		useful as defensive sectors.  Military forces inside of 
		a fort have four times the defensive factor of military in
		most other sectors.  Ie, 100 soldiers in a fort is equal to
		400 regular troops if they are defending the fort.  

		Also note that forts can not be taken by paratroops dropped
		from planes.  They can only be taken via amphibious assault
		and overland attack. 

		Forts are generally useful along your border sectors early
		in the game and later on in the game throughout your country.
		Ensure that they each have sufficient (seven or more) guns
		and shells each, when you are able to devote that many guns
		and shells to them.  Forts are sometimes very critical when
		your country is being over run.  



User's Guide to Empire							 36

	3: Naval support
		
		Your navy will defend itself if it has the capability to do
		so.  Ships do not automatically come with guns when they are
		built; they must be provided with guns and shells by loading
		them onto the respective ship.  Most ships have the capability
		to fire guns.  Should a ship of your nation be attacked, it
		will defend itself if it can.  In addition, ships within range
		will also retaliate.  Note that submarines will *not* fire
		no matter how often they are fired at.

		Ships will not defend your land sectors. 

		If ships are attacked from the air, they will fire their
		anti-aircraft guns (which are the same as your regular guns,
		and again are not automatically installed upon their 
		construction).  Anti-aircraft guns on ships are pretty much
		worthless on defense.  Additionally, if enemy planes fly
		over your ships but do not attack those ships, your ships
		will not fire on them.  Thus having a line of ships as
		radar pickets will not work. 

	In addition to these three major modes of automatic defense, there
	are a few more minor ones.  
	
	Any gun in any sector of yours that is overflown by enemy planes 
	will fire on those planes if it has shells, guns,  and military 
	available. 
	
	Military in any sector of yours that is attacked will automatically
	fight back to the best of its ability.  There is no such thing as
	retreat in automatic military fighting.  In addition, there are three
	sectors which multiply the effectiveness of military within them.
	We have already mentioned forts in this capacity, in that they 
	multiply the effectiveness of troops within them by four.  Capitals
	and Banks multiply troops effectiveness by two, when on the defensive.

	Also note that Capitals and Forts, as previously mentioned, can not
	be taken by airborne paratroopers.  Neither can mountains. 

	Lastly, should sectors of yours be taken, the enemy will have to deal
	with your patriotic guerrillas.  Guerrillas will attempt to retake 
	sectors from the enemy every update until they are defeated (which 
	takes some time).  They are large in numbers, but have been known to 
	reach strengths of a hundred or more soldiers, though this is rare.  
	If they can not hope to retake the sector they may try to blow things 
	up in the sector the enemy has conquered.  This results in loss of 
	efficiency to the sector as well as reduction in the work capability 
	of the sector. 

	You have *no* control over your guerrillas whatsoever.  They are 
	completely automatic.  Should guerrillas win a sector back from your
	enemy then the surviving guerrillas become members of your regular


User's Guide to Empire							 37

	military, and you can use them as such. 

	If your guerrillas manage to take a sector back behind enemy lines
	it is possible for you to enlist reservists in that sector.  In
	so doing, you may throw a major thorn into the side of your attacker.
	This is a major reason for maintaining a large amount of reserves.

Summary: Automatic defenses in Empire are a critical part of every country. 
	 If your country can not defend itself then you can expect to be 
	 quickly over run.  Use the automatic defenses described in this 
	 chapter, and you will become a more difficult opponent.  Remember 
	 the three basic areas of automatic defense.  In addition, keep in 
	 mind that your airforce is probably the most important component of 
	 your automatic defenses. 



User's Guide to Empire							 38

Chapter 8 :-: Nuclear warfare.

	Nuclear war in Empire is not currently the dreaded end of the world
	doomsday machine it is in the real world.  Nuclear weapons in the 
	standard release of Empire have no fallout, no radiation, no dust
	clouds, etc.  Nuclear weapons in Empire are essentially big 
	conventional bombs.  They are "clean" weapons in that the only 
	difference between them and conventional bombs is that they are
	bigger and create nuclear wasteland.

	As such, a nuclear war is much more survivable.

	There are several things that you need in order to wage nuclear
	war against an enemy.  First, you need the nuclear warheads themselves.
	You will also need a missile or plane of sufficient type and range to 
	launch the warheads on.  Lastly, you need to have target information, 
	ie where to land the nuclear warhead to cause the damage you want it 
	to cause. 

	Next, you need to decide if it is worth the cost to wage a nuclear
	war.  Should the enemy country know where you are, and have the means
	to retaliate in kind, s/he will most likely do so.  This can cause
	grave damage to your own country, and eliminate any gains you may have
	achieved by beginning a nuclear war. 

	There is also the large chance of great diplomatic cost.  Most 
	countries do not tend to favor countries that rely on nuclear weapons 
	for victory, since it is the "easy" way out. 

	However, the scenario that the beginner needs to be most concerned
	about is being the victim of a nuclear attack.  Most beginners never
	come close to achieving enough strength to have an effective nuclear
	force.  It may take several games before you become practiced enough
	to maintain an economy and military that is capable of having the
	resources, etc. necessary for such a force.  Nuclear weapons can be
	bought from larger powers, however the beginner still rarely tends
	to have these. 

	In the event that you are attacked with nuclear weapons you need
	to take immediate steps in order to ensure your survival.  You must
	first decide if it is in fact worth it for you to continue.  Most
	nuclear attacks are survivable.  In many cases, you may be hit by
	only one warhead.  Depending on the size of your country, a small
	number of warheads hitting you is survivable.  This all assumes the
	warheads are of the 5mt variety.  5mt warheads create a wasteland
	that is 5 sectors long from left to right.  3mt warheads create 
	wasteland that is 3 sectors from left to right.  Diameter in both
	cases is the same north-south as east-west.  These are the only
	nuclear weapons that can really create serious problems for you.
	Other nuclear weapons cause problems, but are essentially nuisances
	that can be easily overcome. 

	The following is a map of your -20:0,-10:3 region.  You have been 


User's Guide to Empire							 39

	hit with a 3mt nuclear warhead at your sector -12,-6.  It wastelanded 
	7 of your sectors and damaged nearly the entire area.

.eo
--------------------------------------------------------------------------------

			   21111111111----------
			   098765432109876543210
			10 . . . . . . . . . . . 10
			 9  . . ) . . h f . . .  9
			 8 . . a m g o + m . . . 8
			 7  . a + \ \ + * m . .  7
			 6 = # + \ \ \ w b ) . . 6
			 5  . . h \ \ + e f . .  5
			 4 . . f + + c f l . . . 4
			 3  . . k j + f m m . .  3
			   211111111119876543210
			   09876543210----------
			
				Figure 12
		  Map of area hit by 3mt nuclear warhead

--------------------------------------------------------------------------------
.ec

	Here is a basic outline of what you will need to do should you be hit
	with one or more 3mt or greater warheads:
	
	1: Get food to the people in areas that need food.  Especially areas
	   that are partially or totally cut off from the rest of your country.
	   ON THE MAP ABOVE:  This would probably be necessary for all sectors
			      east of -12 and/or north of -4. 

	2: Distribution routes *will* be cut off by a nuclear hit.  Redo your
	   distribution routes around the nuclear wasteland areas.  Make sure
	   that your new distribution center has enough food to feed the people
	   you are trying to support with it. 
	   ON THE MAP ABOVE:  This would probably be necessary for all sectors
			      in the region. 
	3: Move people into the area affected by the blast.  Get highways in 
	   the area back up to efficiency as soon as possible.  This will 
	   allow for increased movement in the immediate area of the blast 
	   which could be critical to the survival of the people in that area 
	   as well as defense of the area. 
	   ON THE MAP ABOVE:  You will probably need to restructure your 
		              highway system in the region.  Move people in 
			      from the south-east (off the map).
	4: Lastly, try to get production back online in the area.  This is
	   not as critical as the first three things.

	Those are the essentials for recovering from a nuclear blast.  The


User's Guide to Empire							 40

	first three items should be done as soon as possible top to bottom,
	ie number one is priority one. 

	There are also defense considerations after you have been hit by
	a nuclear blast.  Your air defenses in the area might be wiped out
	which would allow your opponent to take over your land in that area.
	There are also steps you can use to shore up your defenses in the
	area while you are getting the area back on its feet:

	1: Enlist military in the region.  Looking at the map in figure 12,
	   You might do something like:

	[23:210] Command: enlist -20:0,-10:3 -25

	   That would enlist military in the region so that every sector had
	   at least 25 military in it.  Note that you can only enlist half of
	   the population in a sector at one time, but you can enlist in the
	   same sector over and over again.

	   Doing this would hamper an enemy's progress should they attempt a
	   paradrop assault in the region.

	2: Move undamaged or nearly undamaged VTOL fighter planes from the 
	   south-east (off the map) into the area.  Do not put them on
	   the airbase.  Provide them with petrol as well.  This will
	   shore up your air defenses which were damaged by the blast.  It
	   will also make it much more difficult for an enemy power to attack
	   by air in the region. 

	3: Move guns and shells into the forts on the map if they are not
	   already there.  Make sure there is at least 7 guns and a lot of 
	   shells in each fort.  Also make sure there are substantial military
	   forces in each.  This will help prevent extended military action
	   on land by an enemy. 

	4: Move naval forces into the region if possible.  This will help
	   to prevent amphibious assaults.

	If you do all of these things, recovery from the blast will not
	take long at all.  These things will minimize the effect of the
	blast and get the area back on its feet again.  

	Do not be dismayed by loss of resources etc.  You *can* recover.
	I have been a victim of nuclear war probably half a dozen times,
	and so far have found only one case in which I was crippled by 
	the effects.  My entire homeland (of ~100 sectors) was 
	wastelanded except for less than 10 sectors.  No, I could no 
	longer hope to win the game, but I did hang on long enough to 
	purchase a nuclear warhead and missile and attack the person 
	who nuked me.  Doing something like this will make the player 
	who nuked you respect you more in future games, and consider 
	the consequences before pushing that button. 



User's Guide to Empire							 41

	It has become fairly routine lately for games to end with a world
	wide nuke-off.  Generally speaking, nations having nuclear weapons
	threaten the world with destruction if they are not acknowledged
	as victors.  Other people protest of course, and nuclear war happens.
	A game that has taken eight weeks to evolve to that point can be
	destroyed in less than an hour in the hottest nuclear wars.  This
	of course does not lead to any clear cut winner, and is rather 
	pointless but people do it anyway.  It is after all, a game, and
	people want to have fun.  Nuclear war, to many people in Empire,
	is fun. 

Summary: Do not be demoralized by a nuclear attack.  Powers using nuclear 
	 weapons expect their opponents to get demoralized. Recovery from a 
	 nuclear attack of larger degree is possible as well.  It takes 
	 perseverance and sometimes hard work.  Don't be dismayed by mass 
	 starvations.  Populations can recover if you work at it.  Your 
	 reputation may be greatly improved if you do recover from a nuclear
	 blast.



User's Guide to Empire							 42

Chapter 9 :-: Technology.

	By most people's definitions technology or the means to purchase
	technological weapons from another country is one of the most 
	critical elements in doing well in Empire. 

	Having technology allows you to:
	1.  Have increased ranges on your land based radar.
	2.  Build higher capability planes.
	3.  Build more advanced types of planes.
	4.  Build higher capability ships (not as great an effect as in planes)
	5.  Build more advanced types of ships.
	6.  Build bridges.
	7.  Build nuclear weapons.
	8.  Have increased ranges on your guns in forts.

	Thus, having a high technology country is a very useful thing.
	However, attaining a high technology level is very difficult for
	the beginner.  Running a country that has an economy balanced well
	enough to produce technology at a rate sufficient to stay in the
	technology race is a very difficult task, even for players who are
	more experienced and/or have a lot of allies supporting their 
	economies as well. 

	Producing tech is expensive.  It costs a lot in terms of money
	and materials.  Not only do you have to produce tech, but you
	have to educate your people to a high enough degree that tech
	production can be done efficiently.  Doing this takes a great
	many light construction materials (lcms) for education, and a 
	large amount of oil, gold dust and lcms for tech production.
	
	It is not uncommon for tech nations to be one of the smallest
	and poorest nations around.  Most of their time and resources
	have to be devoted to tech production rather than ships for 
	exploration and military to defend outlying colonies. 

	Do not fantasize.  If you are a beginner, forget about producing
	tech no matter how good you think you are.  You will be wasting
	your resources in doing so and crippling your economy while other
	countries will run far far ahead of you in production. 

	In general, until you have played a few games it is far better to
	purchase high technology items from other players.  Diplomacy, 
	discussed at length in chapter 7, is an effective tool for
	purchasing technologically superior items.

	Should you decide to run tech, there are several things you should
	know.  If you are able to take these into account, and balance these
	factors against each other you may be able to produce tech at a 
	sufficient rate to stay with the leaders. 

	Education is important to produce tech.  You have to have an
	education level of at least five before you can produce any.  However,


User's Guide to Empire							 43

	an education level of five will produce tech only in an inefficient 
	manner.  The higher your education level, the more efficiently tech
	will be produced.  This means that a single tech center can produce
	more tech per update, and cost you less in raw materials, though it
	will cost the same per unit of tech produced.  The savings in raw
	materials is critical. 

	Some countries feel it is better to keep your population low when
	they are planning on having a high education level.  The reasoning
	behind this is that it takes less effort to educate a small number
	of people and costs less money as well.  There is a bad side to this
	plan though.  With a small population it is hard to produce things
	in large quantities, and exploring into and occupying new lands is
	very difficult. 

	Technology production demands a large amount of resources.  Especially
	oil.  However, the higher tech you are the more oil you are able to
	refine per amount of crude oil in the ground.  Thus, it is better to 
	only take out of the ground what oil you need to keep your country
	running, and leave the rest for later on when you have higher tech,
	and get more per barrel of crude.  Thus, if you were to do the 
	following:

	[23:512] Command: resource 4,0

	You might see that your 4,0 sector has 100 units of oil in the ground.
	At tech 0 you'll be lucky if you get 500 barrels of oil out of that.
	However, at tech 100 you will probably get somewhere around 700-750. 
	That's a big difference.  So, only take approximately what oil you will
	need for each update out of the ground, and leave the rest for later.

	Tech production also requires gold dust.  Gold dust mining is not
	affected by tech like oil wells are.  Thus, mining the gold dust
	right away or saving it later has no impact on how much is produced 
	per unit of gold dust in the ground.  There are other considerations
	though.  If you have a lot of gold dust sitting around in your 
	warehouse or other storage place, other countries will see via the
	power report that you have a lot of dust.  If they are low on dust they
	just might look at you as a possible target.  So, if you are going to
	mine a lot of your dust right away you might want to consider putting
	a good portion of it into banks where the dust gets converted into
	bars, which will give you more money per update.  This especially 
	applies to the Kent State version of Empire, where you can grind up
	your gold bars into gold dust at a loss of 20% of the original amount
	of dust put into the bars.  

	Uncompensated workers are also useful for tech production, as well
	as many other things.  However, it has been noticed that a tech
	center with 999 uws will produce tech less efficiently than one with
	500 uws.  The loss in efficiency is greater than what is expected 
	from the tech limitation function, which is a non-linear function
	designed to limit fast tech production.  Most players do not feel that
	the loss in efficiency is worth it for the greater amount of tech


User's Guide to Empire							 44

	production, especially in small countries where having two or even
	three tech centers is unrealistic due to the small amount of land
	available.

	As your technology increases, the possibility of there being an 
	outbreak of plague in your country increases.  This is not of great 
	concern until such time as your plague factor (found by the command
	"nation") increases above a factor of 3.0 or more.  Even if plague 
	does break out in your country, if the plague factor is less than
	3.0 or so, the plague will have minimal effect on your country. 
	During your first outbreaks of plague, you will lose some civilians
	but the birth rate of the civilians will be greater than the death
	rate.  Thus, increasing your research level (by opening and operating
	a research center, which requires the same materials as a tech center)
	prior to your plague factor reaching 3.0 is actually a waste of 
	resources, time, money and effort.  This fact has been lost on a
	great many people and it is not uncommon to find many players 
	boasting of a high research level long before you would have a 
	plague factor of 3.0 without any research production.  While we
	have emphasised "3.0" as a point at which things start getting
	bad with plague, that is a vague figure.  Things start getting
	bad before that point, and get worse afterwards.  Its a matter of
	how much you want to put up with civilians dying off.

	There is a command called "report" which will list other countries,
	their status, and their levels of tech, research, and education 
	relative to you.  If your tech is significantly higher than 
	say a country called 'Canada' then report will indicate that
	Canada's technological level is 'backward' relative to you. 
	If Canada's technological level is significantly higher, than
	report will indicate 'advanced', meaning Canada is advanced
	relative to you.  If its roughly the same, 'similar' will be
	shown.  There are two other possibilities, 'primitive' and 
	'superior' which are used for extreme differences in levels.
	It should be noted that report is inaccurate at best, and only
	gives a very vague idea of levels.  

Summary: Technology is one of the basic aspects of having a country that
	 lasts to the end of the game.  Generally, the higher tech you are
	 the greater chance of your survival.  However, if you have a high
	 technology friend, you chances of survival are still increased
	 greatly.  Empire is a classic case of cold war as far as 
	 technological development is concerned.  As a beginner, you are 
	 probably better off not producing a great deal of tech until you 
	 become well versed in how to run a strong economy.



User's Guide to Empire							 45

Chapter 10 :-: The economy.

	There are several products produced in Empire economies.

--------------------------------------------------------------------------------

	 Group 1		  Group 2
	 1. Gold dust		   8. Light Construction Materials (LCM)  (3)
	 2. Oil			   9. Heavy Construction Materials (HCM)  (3)
	 3. Iron		  10. Gold bars (1)
	 4. Rads		  11. Petrol (2)
	 5. Food		  12. Soldiers (6)
	 6. Civilians		  
	 7. Uncompensated workers

	Group 3			  Group 4
	13. Education (8)	  17. Ships (8,9)
	14. Research  (1,2,8,13)  18. Planes (8,9)
	15. Technology (1,2,8,13) 19. Bridges (9)
	16. Money (6 or 7 or 10)  20. Nuclear weapons (2,4,8,9) ????

	Special items:  Bureaucratic Time Units (BTUs), Available work (Avail)


				Figure 13
		  Table of types of commodities in Empire

--------------------------------------------------------------------------------

	Bureaucratic Time Units (BTUs) are used when executing a command, such 
	as "build" for example.  They are also used to shuffle paperwork 
	around, say for soldiers killed in combat. 

	Available Work (Avail) is used up by producing sectors and those 
	sectors in which you build things (planes, for example).  It is an 
	indices of the amount of labor hours available for work. 

	Running your economy efficiently as possible is one of the keys to 
	being a good Empire player.  Without a good economy it is impossible
	to wage a long lasting war, especially an offensive one.  It is 
	possible to last a long time defensively even with a weak economy 
	but your chances are greatly increased if you have a strong economy. 

	So, what defines an efficient and strong economy?  An efficient
	economy is one in which the amount of a product being produced in a 
	sector is nearly equal to the amount that the civilians can produce in 
	one turn.  For example, if a light construction materials (LCM) plant 
	can produce two hundred LCMs an update, but is only producing one 
	hundred, then fifty percent of the people in that plant are basically 
	doing nothing.  They could be used elsewhere on something that needs 
	more civilians or you could put more iron into the LCM plant to allow 
	the civilians to make more LCMs.



User's Guide to Empire							 46

	A strong economy is an economy that produces a lot of materials,
	resources, etc relative to the size of the country.  If your country
	has only say ten thousand civilians but is producing a large amount
	of materials per update then your country may in fact be a strong 
	country.  This is a very unlikely scenario, but it is a good example.  
	A strong economy is usually the result of an efficient economy, 
	though there are other factors, those mainly referring to population 
	growth.  To find out how much work your civilians are going to perform 
	at an update you can issue the "prod" command to get a production 
	simulation.  Using this you can see if all the civilians in a given 
	sector will be making something or if some will be sitting around and 
	doing nothing.  Here is a sample output from a prod command:

--------------------------------------------------------------------------------

[138:250] Command: prod #1
Mon Sep 30 20:18:37 1991
PRODUCTION SIMULATION
   sect  des eff wkfc will make- p.e. cost  use1 use2 use3  max1 max2 max3  max
  -2,0    o  39%  422   39 oil   0.50 $0                                     39
   2,0    m 100%  807  387 iron  1.00 $0                                    387
 -10,2    b 100%  653   36 bars  1.00 $360  180d            310d             62
  -8,2    j 100%  576  138 lcm   0.50 $0    276i            276i            138
  -4,2    g  27%  615   79 dust  1.00 $0                                     79
 -11,3    t 100%  620 0.00 tech  0.00 $0      0d   0o   0l    0d   0o   0l 0.00
  -8,4    l 100%  499   40 edu   1.00 $360   40l            239l            239
    7 sectors

				Figure 14
		     Sample output of the 'prod' command

--------------------------------------------------------------------------------

	The important columns to note here are the "will" and "max" columns.
	If the number indicated in the "will" column is less than the "max" 
	column by more than 1, then you have civilians who are sitting around 
	doing nothing.  The way you calculate the number of civilians you have 
	sitting around is fairly simple.  First, it is important to note that 
	the number indicated in	the "wkfc" column is the number of civilians 
	and uncompensated workers (UWs) you will have in the sector *after* the
	next update.  You need to reduce the number of civs and/or UWs in the 
	sector to optimum *before* the next update.

	For this example, and for sake of ease, I am going to assume that
	there are no UWs or soldiers in this sector, which both produce work.  
	We will assume that there are only civilians, and happy, content ones 
	at that.  Civilians that are starved, or recently starved do not work
	as hard. 
	
	In the above shown production simulation there are two sectors which
	have civilians sitting around idle.  These two sectors are the bank 
	('b') at -10,2, and the library ('l') at -8,4.  We will use the library 
	for our example.



User's Guide to Empire							 47

	The library is going to produce 40 education, but it is capable of
	producing 239.  It doesn't have enough lcms (the only material needed 
	to produce education) to produce 239 education.  Thus, only the civs 
	necessary to produce 40 education are actually working.  The remaining 
	number of civilians, as a percentage, is calculated by this simple 
	formula:

	((# in max column) - (# in 'will' column)) / (# in max column)

	Thus, the percentage of civilians in the library not actually doing 
	work is (239 - 40) / 239, or 83.26%.  That's a large percentage!  Now, 
	to calculate how many civilians you can move out of the sector without 
	reducing your production beneath 40 education in that sector, multiply 
	.8326 * the number of civilians currently in that sector.  So, you need
	to do a census command:

--------------------------------------------------------------------------------

[162:166] Command : cen -8,4 
Mon Sep 30 20:19:07 1991
CENSUS               del dst
  sect        eff mob uf uf *  civ  mil   uw food work avail ter fallout 
  -8,4    l  100% 127 .. .0    403    0    0   49 100%   54   0   0

				Figure 15
		Results of a 'census' command of a library

--------------------------------------------------------------------------------

	There are 403 civilians listed in the 'civ' column for the library at
	sector -8,4.  Thus, you multiply 403 by .8326 and you find that you can
	safely remove 355 civilians from that sector and still be producing
	40 education.  That's 355 civilians that can be doing valuable work
	elsewhere instead of sitting around doing nothing! 
 
	The other alternative to get the sector producing efficiently
	is to move lcms into the library up to the number that the library can
	possibly process with the current number of civilians.  Once again, we
	refer to the production report and note the 'use1' and 'max1' columns.
	The library will use 40l (40 lcms) at the next update unless you put
	more lcms into the library.  It can use up to 239 (239l in 'max1') 
	lcms.  Thus, it can process an additional 199 lcms.  So, you find lcms 
	somewhere else in your country and move the 199 lcms into the library.
	At the next update your library will produce 239 education.  Once 
	again, all of the civilians in the sector are now doing something. 

	Of course, you can also use a combination of these two methods to
	achieve an equitable balance. 

	There are other aspects to running an efficient economy as well.
	Ideally, you do not want to have an abundance of raw materials lying 
	around especially in the early stages of the game.  The best situation 
	is to have your economy using the same amount of resources per update 


User's Guide to Empire							 48

	as it is making, thus having a zero delta situation with respect to 
	all materials in your economy, you neither gain nor lose surplus of 
	materials.  Keep in mind however that it is important to create and
	maintain a surplus of materials from which to draw on in times of war,
	especially later on in the game.  Without spare materials, you can not
	wage war very effectively.  In doing this you are getting the best 
	balance of production, and are producing finished items at a faster 
	pace then if you spent more work producing raw materials.  If you 
	spent more work producing finished materials, you would run out of 
	raw materials, and find that you have to move your civilians around 
	to produce more raw materials or have them sitting idle doing nothing 
	while your other civilians produce sufficient raw materials for them 
	to do work.  The benefits of a balanced economy are obvious. 

	There are additional tasks that are somewhat rewarding in running an 
	efficient economy, but they become increasingly technical in nature 
	and their impact as a whole against your economy is significantly less.

	As you become more experienced you will no doubt discover these thing 
	yourself, or hear about them in a game or on the network.  These basic 
	elements are more than enough for you to run smooth production if you 
	adhere to them.  

	There are still more elements of an economy which, while very useful, 
	are not exactly related to the strength or efficiency with which your 
	economy is running.  Your country, should it live for some time and 
	develop a well balanced economy, will produce a huge amount of 
	materials.  It is impossible for you to keep track of everything in
	each and every sector of your country.  Of course, the Empire creators
	realized this and have developed some critical commands for reducing
	by a very large amount the quantity of time you need to spend adjusting
	your economy.  These are the 'distribute' and 'deliver' commands.

	'Distribute' and 'deliver' are commands in Empire which allow
	you to set up your economy to run automatically, with little 
	intervention from yourself.  Distribution is the most commonly used 
	command in doing this.  However, deliver has certain features which 
	are useful as well. 

	Distribute is based on the central warehouse scheme of production
	and consumption of materials.  An iron mine produces iron, which it 
	ships off to the central warehouse.  A light construction materials 
	plant needs iron to make lcms, and thus requests iron to be sent to it 
	from the warehouse.  The lcm factory produces lcms, and sends them 
	off to the warehouse.  A library plant needs lcms to produce 
	education, and requests lcms from the warehouse to do so.  The 
	warehouse handles regional traffic for your economy.  Some smaller 
	countries only have one warehouse. 

	Deliver is based on the local delivery truck idea.  A nearby farm
	produces a certain amount of food per update, and sends it directly to 
	the neighboring sector.  You know that the neighboring sector can 
	consume more food per update than that farm can produce (this is very 


User's Guide to Empire							 49

	rarely true, this is just an example).  So, the farm sends all the 
	food it produces to the neighboring sector.  The neighboring sector 
	notes how much food has been delivered from the farm, and then requests
	from the warehouse enough food to feed the rest of the civilians in 
	the sector.  In so doing things in this manner, you are possibly saving
	mobility by not having the food automatically move to the warehouse 
	from the neighboring farm and then to the neighboring sector.  The same
	result would be achieved however, though as stated at a possible 
	greater loss of mobility.  Nevertheless, most people prefer to depend 
	entirely on 'distribute' to handle automatic distribution of materials 
	around their country. 

	To set up a distribution system, or network, you have to set a 
	distribution 'path' *from* every sector *to* the warehouse.  I 
	emphasize the direction that the path is moving in because all 
	distribution paths go *to* the warehouse, not from the warehouse 
	to the sector, though materials can and will go from the warehouse 
	to the sector along that same path.  The idea here is say a farm 
	outside of St. Louis about 50 miles produces 500 units of food, the 
	farm needs to know the route by which it should send its trucks to get 
	to a warehouse in St. Louis before it can send the food to that 
	warehouse (this of course being a real life scenario). The best path 
	to choose is across highways.  A 100% highway requires no mobility to 
	move anything in to it.  Obviously you don't want to travel the 50 
	miles to St. Louis across dirt roads.  You would prefer an interstate
	if you were a truck.  In Empire, a 100% highway is essentially an 
	interstate.  Your goal is to have a distribution path from each sector 
	to the warehouse travel the route over the least mobility cost path.  
	The greater the percentage of that path that is 100% highways the 
	better.  It is important to note that distribution paths have a maximum
	length of 10 sectors.  For example:


User's Guide to Empire							 50


--------------------------------------------------------------------------------

[529:32] Command : map -20:10,-3:6
     21111111111---------00000000001
     0987654321098765432101234567890
  -3  . . . . . ) . . . . . . .      -3
  -2 . . . f . o . . . . . . . .     -2
  -1  . . . a k a . a a h . . . .    -1
   0 k l k a + + + . + + c m . . .   0
   1  m w o ^ ^ m + j j + . + . . .  1
   2 + + + + . b j + m + w + . . . . 2
   3  m j j + t + + + + + + . ) . +  3
   4 + ^ j m + + l ^ ^ ^ + * a a o m 4
   5  ^ + ^ m + j ^ ^ c + m m + + +  5
   6 + + + g m + + m b + k o + j w g 6
     21111111111---------00000000001
     0987654321098765432101234567890

[529:32] Command :

				Figure 16
		  Map of area requiring some distribution

--------------------------------------------------------------------------------

	Let's say that in the previous figure you wanted to set up a 
	distribution path from the oil well at -15,1 (the 'o' sector left of 
	the middle of the map) to the warehouse at 0,2 (the 'w' sector right 
	of the middle of the map).  The command to set up the distribution 
	path from that oil well to the warehouse would be: 

--------------------------------------------------------------------------------

[529:32] Command : distribute -15,1 nnnjujjjuj
oil field at -15,1 has no distribution path.
<10: 0,2>h
New path distributes to 0,2

[534:31] Command :

				Figure 17
		    Results of a distribution command

--------------------------------------------------------------------------------

	Now there is a path along highways from the oil well to the warehouse.
	If you do not understand the nnnjujjjuj command above, refer to chapter
	3 and the section under "movement in Empire".

	Next, you need to set thresholds for the sector.  This sector is an 
	oil well, and it produces, you guessed it, oil.  So, you want the oil
	that is produced to be shipped to the warehouse.  You have to tell the


User's Guide to Empire							 51

	people in the oil well that all oil over 1 unit is to be shipped.
	This is called a "threshold", a level at which any quantity of that
	unit over that amount if shipped to the warehouse.  Note that the 
	lowest threshold you can have and still have the item shipped is 1.  
	If the threshold is 0, then it will not be shipped.  The command to set 
	the threshold in the sector -15,1 for oil is:

[534:31] Command : threshold oil -15,1 1

	Next, you want to make sure that food gets out to the oil well.  The 
	other aspect of thresholds is that if there is less than the threshold 
	of a commodity, then the warehouse will send out enough of that 
	commodity to equal the threshold, assuming it has such commodities in 
	the warehouse.  The command to set the threshold for for in the sector
	so that food will go to the sector from the warehouse could be this:

[534:30] Command: threshold food -15,1 50

	Which means at each update, the warehouse will attempt to ensure 
	that there is 50 food in the oil well at -15,1 by sending enough 
	food, if it can, to raise the stockpile of food in that sector to
	50 units.  

	Again, distribution paths must be set up for every sector that you 
	intend on doing anything with.  Essentially, as the game progresses, 
	this means all of your sectors.  You must also set appropriate 
	thresholds as well.  Some sectors need iron from the warehouse to 
	produce, and thus a threshold will be required for iron.  Some need 
	more than one commodity, so thresholds must be set for each of those 
	commodities.  EVERY sector should have a threshold value set for food.
	You do not want to starve your civilians!  To find out what 
	commodities a sector needs to produce an item, use the 'production' 
	command discussed earlier in this chapter.

	Some things to note about distribute: 

1) No distribution path may be longer than 10 sectors.
2) A distribution path must go through sectors controlled by you.
3) Sometimes it may seem that the warehouse is not getting the commodities
   to the sectors requesting it.  Empire updates sectors one at a time, and 
   as such the warehouse may actually run out of a commodity, receive the
   request for that commodity, be unable to fill it and move on the the
   sectors it updates after that sector.  Those sectors in turn may send
   commodities to the warehouse, thus giving it enough to fill the previous
   request.  However, since it has already updated that sector, it will not
   fulfill the backlogged request.  Empire updates the world left to right
   and top to bottom.  Thus in a map of your country the most north-west
   sector gets updated first and the most south-east sector gets updated last.
4) The warehouse has to spend mobility to send commodities to the requesting
   sectors.  Warehouses do this at a great reduction in the cost to the 
   normal mobility.  However, if the warehouse runs out of mobility it
   will NOT distribute any more commodities for that update. 
5) Do NOT set thresholds for all commodities in a sector.  Only set the 


User's Guide to Empire							 52

   thresholds for the commodities you KNOW that sector will need.  Distribute
   will not distribute commodities to sectors with the thresholds set for 
   every commodity. 


	Deliver is much more basic.  As a result however, it is much less 
	powerful, though it is useful.  Deliver works as explained earlier in
	that it does local deliveries of a specified commodity.  Deliver is
	capable of delivering an item to only neighboring sectors.  During the 
	update, deliver is done before distribute.  It is important to note 
	that.  Let's say that a farm has a lot of food to send to the 
	warehouse.  It may not have the mobility to send it all to the 
	warehouse, and thus you get a large surplus left over in the farm.  
	Eventually, the farm will not have room to store the surplus anymore.  
	This is of course bad, since the farm will no longer produce very much 
	food.  It will produce only enough food to replace what was taken from 
	the storage at the farm.  

	The solution to this is to have the farm sends its surplus to a 
	neighboring 100% efficient highway.  This will cost the farm no 
	mobility and will open up its storage areas for new production at the 
	farm.  The highway can then spend mobility sending the food to the 
	warehouse.  Once again, we use the map: 

--------------------------------------------------------------------------------

[529:32] Command : map -20:10,-3:6
     21111111111---------00000000001
     0987654321098765432101234567890
  -3  . . . . . ) . . . . . . .      -3
  -2 . . . f . o . . . . . . . .     -2
  -1  . . . a k a . a a h . . . .    -1
   0 k l k a + + + . + + c m . . .   0
   1  m w o ^ ^ m + j j + . + . . .  1
   2 + + + + . b j + m + w + . . . . 2
   3  m j j + t + + + + + + . ) . +  3
   4 + ^ j m + + l ^ ^ ^ + * a a o m 4
   5  ^ + ^ m + j ^ ^ c + m m + + +  5
   6 + + + g m + + m b + k o + j w g 6
     21111111111---------00000000001
     0987654321098765432101234567890

				Figure 18
		   Map of an area requiring delivery path

-------------------------------------------------------------------------------

	Let's say the farm at -5,-1 (just north of the middle) has a huge 
	surplus of food.  You can send that surplus to the highway at -4,0 by 
	issuing this command: 

[566:115] Command : deliver food -5,-1 n (50)



User's Guide to Empire							 53

	Which means, deliver all food in the sector at -5,-1 in excess of 50 
	units in the 'n' direction.  Note the deliver threshold in parentheses 
	must be in parentheses and that deliver thresholds are different than 
	distribute thresholds.  

	While I have described how to use the basics of deliver, I do not
	recommend it for the beginner.  Using a distribution network is quite
	sufficient for the beginning player, and you will not notice any great
	difficulty in running your economy without using the deliver command.
	In fact, some of the best players in Empire do not use deliver at all.


Summary: The things discussed in this chapter may seem time consuming, and 
	 may seem to require a lot of detail.  However, these things are 
	 important to you as an Empire player to learn and understand.  They 
	 become easy to use after only a short while.  Having a strong 
	 economy is the basic element in having a strong country.  If you 
	 have not understood this chapter, read it again.  Better yet, read 
	 it while experimenting with a couple of your own sectors in a game.  
	 While understanding 'deliver' is not very important, understanding 
	 'distribute' is. 



User's Guide to Empire							 54

Chapter 11 :-: War

	The art of waging war is certainly beyond the scope of this guide.
	There are a great number of books written on the history of man's wish 
	to destroy themselves.  In this chapter, I only intend to give the 
	player some rudimentary basics for waging Empire style wars.  Even more
	pertinent in the case of the beginner, how to defend against or 
	hopefully repulse an invader.

	Keep in mind that as a beginner, you stand a very good chance of losing
	your first war.  Consider it a learning experience.  The person who
	defeated you is better than you are in most circumstances, and just by 
	their actions you can learn how best to wage an offensive or defensive 
	war. 

	There are several things that you can do to defend your country
	in the event of an attack.  However, it is always best to be prepared 
	for an attack before it happens.  There are some basics already 
	discussed in this guide on these preparations.  Refer to chapter 8
	for information on automatic defenses, and how they can help your 
	country.  I will review the essentials here.

	1) Have a strong airforce with many fighter planes with the intercept
	   capability.  Make sure they are used as indicated in chapter 8.
	2) Have forts with overlapping fields of fire. 
	3) Have a large number of reserves (typically 1/10 of your population).
	4) Have at least two enlistment centers ready to produce more fresh
	   recruits.  Ie, at 100%.  You can "shut off" enlistment centers, that
	   is stop them from producing recruits, by leaving 1 civ in them
	   or having a military population in the sector equal to one half
	   of the civilian population.  To turn them on, put in more civs or
	   reduce the number of military.
	5) Have a strong navy to guard your coast.
	6) Mine your own coast.  This prevents many amphibious assaults.
	   It also helps to prevent people from mapping your coastline. 
	7) Have a good highway network.  This allows you to move materials
	   and armies around your country quickly and with ease.  
	8) Have more than one capital.  If your real one is taken, you will
	   need a back up immediately. 
	9) Have a lot of food in all of your sectors if possible.  This allows
	   sectors that are cut off from your warehouse to be self sufficient
	   at least with respects to feeding the people with food for a few 
	   updates until you can open up a path to them. 

	These are just *basic* hints.  Some more hints are given later in this 
	guide.  These are meant to get you started. 

	Of course, it is better to not be attacked at all.  But, how do you
	avoid being attacked?  This is not simple, but there are some things
	that you can do to make yourself less of a target. 

	One, do NOT allow people to map your coast using their ships, 
	especially patrol boats.  Maps of your coastline are deadly.


User's Guide to Empire							 55

	The opposing country knows where your country lies, and can furthermore
	send that information on to another player who might be interested
	in attacking you in exchange for some commodity or assistance.
	Maps are a critical piece of intelligence gathering.  Some of the 
	best players in the game sink ships on sight without warning.  If you
	warn a ship off it might be able to use some of its mobility to
	make a final swing along your coast before going away, and seemingly
	out of your hair without having caused further trouble.  Every piece
	of information about your coast is valuable military intelligence.
	Additionally, it is a good idea to try to map other people.  Do not
	get upset when your ships get sunk.  They will get sunk.  Just consider
	them expendable commodities that are doing a temporary job with 
	permanent results.  

	Two, if you appear tough people generally will not attack you.  
	Posturing can be an effective tool.  Furthermore, if you have a large 
	airforce and/or army, people tend to avoid you.  It does not matter if 
	your airforce only has wimpy planes in it, your opponent can only 
	tell the quantity of planes that you have, not the quality.  They 
	might have a general idea of the quality of your airforce, but a large 
	airforce is still an intimidating presence. 

	Three, do not make yourself a target by having a lot of valuable
	commodities showing up on your power report.  Having lots of money
	in a money poor game for example makes you a juicy target.  The same
	holds true with gold dust or oil.  You need not worry about iron and 
	food, since those are renewable resources.  Try to use gold dust and
	oil as you make it.  Having some surplus around is fine, but the more
	you have sitting around the more of a target you become. 

	If you are attacked, it is always best if you are logged on to the game
	so that you can actively defend your country rather than relying 
	entirely on automatic defenses.  However, as can obviously be seen, you
	probably will not know that your enemy is going to attack you.  Neither
	will s/he broadcast that they are going to attack unless they are very 
	foolish or extremely confident. 

	In light of this, your opponent will try to attack you at a time when
	they know you are not logged on.  Thus, a great many Empire wars begin 
	in the wee hours of the morning, when most people are asleep.  What can
	you do to overcome this shortcoming?  Have allies for one.  If one of 
	your allies is going to be up most of the night you can have them 
	telephone you if you are being attacked.  Of course, this is for the 
	more diehard types :-)

	There are many tactics that an enemy will use against you.  Again, 
	these tactics are beyond the scope of this guide, but there are some 
	general tactics that you should be aware of.  An opponent may try one 
	or more (usually more) of the following tactics:

	1) Remove your airforce as an effective retaliatory force.
	2) Remove your airforce as an effective defensive force. 
	3) Take your capital (you lose half your money).


User's Guide to Empire							 56

	4) Take your banks (you could lose all your gold bars, very valuable!).
	5) Destroy your highway system.
	6) Take your warehouses (lose your stored commodities. OUCH!).
	7) Bomb your enlistment centers.
	8) Take your radar stations.

	
	In addition, your opponent may try to destroy your distribution and/or
	delivery system (if you so opt to use deliver).  After you take a 
	sector back from your enemy check the thresholds and distribution path 
	in the sector.  Sometimes if you take a great deal of sectors back it 
	is best to just use the 'wipe' command and start from scratch.  
	Remember to check your delivery thresholds as well, by issuing a 
	'level' command.  Your opponent will probably also redesignate the 
	sector to something useless or even detrimental to you.

	It is important to be on around update times when you are at war,
	either defensively or offensively.  Things can go very quickly right 
	before and after an update. 

	One of the best ways of being a good military tactician is to be
	imaginative.  Think of ways to do things that might surprise your 
	opponent.  Think of ways to do things that will cost you less lives, 
	or less resources than your original way.  

	There is a saying that sometimes the best defense is a good offense.
	This can be very true in Empire.  If you are attacked it might be a 
	good idea to launch a counter attack against your opponent's homeland
	if you are able to.  This will certainly surprise them, and may lead 
	them to think you are stronger than you actually are.  This can be a 
	very powerful tool.  

	If you want to win the war, be resilient and tenacious.  Fight as hard
	as you can with every last soldier and resource.  If you make your 
	enemy pay a heavy price they may withdraw.  Sometimes breaking an 
	opponent's will to fight is the most important factor in winning a 
	tactical or strategic situation.  Even if they do not withdraw, or you 
	do not win, you may at least be helping to defend the future of your 
	allies.  That in itself is an honorable achievement.  There is also the
	point that if you are a tenacious fighter your opponent will respect 
	you more in future games than if you just roll over and play dead while
	his/her armies pillage your country.  Reputation does count for a lot in
	Empire.  You may lose now, but your defiance in losing can help you a 
	great deal in future games.  Some countries simply will not attack
	another country because they know that country's reputation as a good 
	player.  Alternatively, some countries have a poor reputation and
	are attacked in every game they play in!


Summary: The art of Empire war is one that is different from the real life
	 world that we live in.  However, many tactics from the real world
	 can be applied to Empire.  Remember to be imaginative, learn from
	 your mistakes.  Learn from your allies and your opponents.  Fight
	 as if it was a real world nation, and you are the leader.  




User's Guide to Empire							 57

Chapter 12 :-: Other helpful or not so helpful hints:


	Here are some hints on how you might be better able to run
	your country.  Note that a lot of these hints were garnered through
	many player years of play.  You are free to disagree with them, and
	are certainly encouraged to come up with your own ideas.  However,
	keep in mind these recommendations.

	- Build your military reserves to a reasonably high level
	  (I try to aim for 5-10% of my total population).  Reserves are
	  not expensive to maintain, and it is a commodity that can not
	  be destroyed by any military action.  
	- As soon as someone starts building ships guard your coastal
	  sectors with at least 1 military in each.  This prevents an
	  opponent from making landings in mass quantity on your homeland
	  with measly cargo ships.  If you have military in a coastal sector
	  then opponent ships really need the "land" capability (see 
	  show ship capability command for further insight) in order to be
	  truly effective.  Guard harbors with more military, say 50 or
	  so.  Else an opponent can assault a harbor, move his or her invasion
	  fleet in, unload the ships and walla several hundred foreign military
	  are sitting in your former harbor. 
	- As soon as planes start being built, try to get at least 10 military
	  into every sector.  This makes it more difficult for a country to
	  wipe you out quickly.  In this manner they usually need 2 or more
	  transports to take a single sector.  This makes it costly in air
	  mobility for them and softens the blow of their attack.  In addition
	  reinforce capitals with a lot of military.  Same with banks, airbases,
	  and warehouses.  
	- Do not base your entire air defenses on non-VTOL (vertical take off
	  and landing) planes.  Base your VTOL planes in non airport sectors.
	  In doing this, you prevent your enemy from knocking out your airbases
	  through any number of means and then flying at liberty over your 
	  country.  There are two VTOL fighters.  Fighter 1s, which are 
	  essentially biplanes with very brave souls in them and Jet Fighter 2s
	  which are akin to the British Harrier jet.  Fighter 1s are not 
	  necessarily a bad plane.  Against low tech transports they do not
 	  fair too badly.  There are two other means of air defense if your
	  airbases are knocked out.  Anti-aircraft guns exist in any sector
	  that has guns and shells.  They will fire if the sector in question
	  is at war with the planes in question.  They are mostly worthless.
	  The other method is SAMs (surface to air missiles).  These can be
	  quite effective but are shoot once only weapons.  
	- Forts are an integral part of any home auto-defense scheme.  They
	  will fire on any enemy aggressors that are within range whenever
	  any sector of yours within range is attacked. 
	- Naval units become essentially useless later on in the game except
	  for some actions.  Nevertheless, be careful to have naval units in
	  appropriate places around your coast for coastal defense.  Especially
	  until you have planes that can pinpoint bomb well (jet lt bombers,
	  jet f/bs, helicopters). 
	- Always have several fake capitals in reserve.


User's Guide to Empire							 58

	- If you are not using planes that are 100% and are not planes used
	  in air defense, get them off your airports.  Otherwise they become
	  easy targets for air assault and bombs.
	- Bridge span sectors are effective means of food production and
	  are especially useful in that role in countries with few sectors
	  or countries with limited fertility.  They are usually somewhat
	  expensive to build, but pay for themselves quickly.
	- Keep in mind that allies are important.  You might want to go it 
	  alone, but at some point along the way you will need an ally or 
	  three.  Allies can do many things for you, not the least of which
	  is call you over the phone when you are under attack at four in the
	  morning.  Its nice to know that even though your country is not
	  being played, someone is watching it from a distance and seeing
	  whether it gets attacked or not.  You sleep easier :-)
	- If you are positive there is no one else on your island with you
	  at the beginning of the game then demobilise your military and
	  put them on reserve.  This will save you a lot of money, and help
	  in population growth. 
	- Never put airports, banks, or capitals on your coast.  They are 
	  easy targets for your enemies if you do.  Sometimes it is necessary
	  to place an airport on your coast to get that extra one sector
	  of range for your planes in an attack.  If you do that, don't leave
	  your planes in that sector for very long.  You might be surprised.

Summary:  Many of these ideas can only be gained through experience.  Do not
	  be come distraught for lack of knowledge.  Experience is a wonderful
	  teacher.  These hints cover some of the major pitfalls of beginners,
	  and that is all.  There are far more things to be learned that can
	  only be taught through experience. 


User's Guide to Empire							 59

Chapter 13 :-: Some tricks and subtle tactics

	I have listed here some of the tricks used by the more advanced
	players in victimizing the beginning players.  I  divulge this hard won
	information because I feel it will make for a better Empire game for 
	everyone.  In reading these, I hope that you will be encouraged to 
	develop tactics of your own.  It is not hard to be imaginative. 

	Note that not all tricks and subtle tactics will work as advertised.  
	Some games have slightly different implementations that have a 
	negative effect on these hints.

	- Sacking a warehouse.  This is an old tactic that is useful
	  only in those games that have a market.  Essentially the trick is to 
	  take an opponent's warehouses or distribution points and sell off 
	  everything that is in the sector to the market.  You don't need a 
	  buyer immediately, just sell everything off.  If you want the items 
	  for yourself, sell them to the market at an exorbitantly high price 
	  and wait for a friend to buy them and sell them back.
	- Tracking an airbase.  This is a less known trick.  Non-VTOL planes
	  intercept from airbases.  They also intercept from airbases in a hex 
	  pattern.  Ie, if the plane has a six sector intercept range it will 
	  intercept up to six sectors west, six sectors north-west, etc in all 
	  the six directions.  If you can find two corners of that plane's 
	  interception hex you can triangulate to find the sector from which it
	  is intercepting.  This does work for VTOL planes as well, but isn't 
	  as useful since airbases usually have far more planes in them to be 
	  dealt with.  Once you have tracked the originating sector of the 
	  intercepting planes, what you do to that sector is your option.  Most
	  often, a concerted effort will be made to crack the defensive fighter
	  cover and blast the airbase.  Sometimes countries will use their 
	  surface to surface missiles (which do not get intercepted) to take 
	  out the airbase.  For non-VTOL planes taking out an airbase just
	  means getting it below 60% efficiency.
	- Fort "walking".  If you have a land border with a country, you can
	  use your forts to demolish a bordering sector with that country.
	  By demolishing it, your are completely destroying every living 
	  civilian and soldier in that sector.  When that is accomplished you
	  can explore into that sector as if it were wilderness.  You can then
	  designate that sector into a 0% fort, which it will become 
	  immediately, and then move guns and shells into that sector.  
	  Then, you can do the same thing to a sector bordering your new
	  sector.  In this manner you can slowly invade another country
	  beyond a one sector advance in one update without the use of
	  planes or ships.  This costs A LOT of BTUs and shells, but in
	  critical situations is very useful. 
	- Fire base.  This tactic has some relation to the tactic used in
	  the Vietnam conflict by U.S. Forces.  The idea is to take an enemy
	  sector in an area that you want to wreak havoc on.  Next, use your
	  pinpoint bombing capable planes to bomb your own position's sector
	  efficiency.  You will have some collateral damage on your troops,
	  but it is well worth it.  When the efficiency drops below 5% you can
	  then designate the sector into a 0% fort, place guns and shells 


User's Guide to Empire							 60

	  in it using air transports, and then have a base from which to 
	  blast away at your enemy without the use of much airpower.  It is 
	  possible to use the fort walking tactic from this position.
	- Messing up a country's distribution paths and or thresholds.
	  This is most often an "annoyance" tactic.  Most players know about it
	  and check the distribution paths and thresholds of sectors they take
	  back from an invading country.  The idea is to set the thresholds so 
	  that a lot of commodities will be distributed to the sector from the 
	  warehouse of your opponent after your opponent takes the sector 
	  back.  Then you can take the sector again and get the commodities 
	  if s/he has not noticed they are there.  Even if s/he notices it is 
	  a pain getting the commodities back to the warehouse.  
	  Alternatively, you can screw up the distribution path to run through
	  a mountain and then to his warehouse, and also set the thresholds 
	  of the commodities quite high.  This will cause mobility in her/his 
	  warehouse to be used in great quantities to get the commodities to 
	  the recaptured sector.  Either of these ideas can get particularly 
	  nasty.  Say you conquer a sector, and set the thresholds on food to 
	  9000 units.  Your opponent takes back the sector, and neglects to 
	  reset the threshold.  At the next update the warehouse moves 9000 
	  food (or at least as much as it can) into the sector.  This might 
	  leave your warehouse without food to deliver to the rest of the 
	  country.  Food stocks begin to drop, with obvious repercussions.
	- Messing up a country's delivery routes and thresholds.
	  Same as above except using deliver.  Many countries do not use 
	  deliver at all, and as a result are susceptible to this tactic.  
	  You can cause mass starvation in a country that you attacked, but 
	  no longer control without the leader ever knowing how. 
	- Destroyer trap.  Move a submarine near a destroyer, say one sector
	  away.  Lay some mines from the submarine, and then send out an active
	  sonar ping using the "sonar" command.  Enemy destroyer rushes over to
	  see what it is and KAWHOMP! hits a mine.  You can leave a sector that
	  has mines in it safely but can not necessarily enter safely.  Every
	  mine laid in a sector increases the chance of a ship hitting a mine
	  while entering that sector by five percent.
	- Determining if someone is on.  You can get an idea of when someone 
	  is usually logged on by doing something like this:

[65:255] Command : news 10 | grep <name of country you want to learn about>

	  This will scan the news file from the server for the last ten days
	  (or however many days up to ten are being preserved in the news).
	  You can then get an idea of when your opponent is logged on.
	  There are tools that can do this for you, but so far none has
	  reached public distribution. 
	- Attacking any sector with one soldier repeatedly.  Within the code
	  there is a small chance that against incredible odds a small 
	  collection of soldiers might win.  Taken to the extreme, there is
	  a chance for one soldier to win, even against 400 soldiers, or so
	  says the game.  Thus a player who wants a particular sector bad
	  enough, may spend thirty minutes attacking a sector of yours 
	  over and over again with one military.  They might win eventually,
	  and if it is important enough then they have gained a major sector.


User's Guide to Empire							 61

	- A player might offer to sell you a type of commodity in several
	  batches.  Or, you might just start buying batches of a commodity
	  from any given commodity.  If the player you are buying from is on,
	  they might change the price on you between batches that you are
	  purchasing.  For example, DoDo starts buying food in batches of
	  9000 from WabbleHead at 10 cents per food.  This costs DoDo $900
	  per batch of food.  Suddenly between DoDo's 2nd and 3rd purchase
	  of 9000 food, WabbleHead changes the price on the food from 10 cents
	  to 10 dollars per unit.  DoDo never realizes it and pays $90,000
	  for the 3rd batch of food.  Moral:  Don't purchase anything without
	  checking the price first, and never buy in several batches at once.
	  This applies to market and trade.
	- It is possible to move small quantities of a commodity across any
	  sector in your country and not have it cost mobility.  To do this,
	  you have to have the commodity moving from a warehouse to whatever
	  sector you want to move it to.  The basic trick, is to move the
	  commodity in batches of 5 or less.  Note this does not work for
	  bars, and some other commodities.  It does work for food, shells,
	  etc; items that do not take a great deal of mobility to move to
	  begin with.  This trick is very effective for getting supplies
	  to a region cut off from your main area without highway access
	  and possibly even a mountain range separating you from it. 
	- Mine your coast.  This prevents your opponents from effectively
	  mapping your coast, and you can catch their ships in the act since
	  a damaged ship moves a lot slower than a 100% ship.  Some people
	  might even scuttle the ship in question to avoid detection.
	- Be aware that submarines will try to map your coast.  Use your
	  submarines and destroyers to find them.  Use minefields as a 
	  preventative measure. 
	- Do not give out your net address to anyone except the Deity unless
	  people offsite can not finger you, or you have an account on 
	  another machine, and can regularly play from it.  People can finger
	  you over the net to see if you are logged in or not.  If you're not,
	  it's a good time to attack probably. 
	- Beware of people offering you loans.  Make sure you know the terms
	  of the loan.  Someone might offer you a loan that looks nice, say
	  for $30,000.  But, the loan might expire in one day and have an
	  interest rate of 127%.  The lending country can collect sectors
	  directly from you without your permission if you are in arrears
	  on the loan. 
	- Defend your radar stations.  Always have at least 20 military in
	  each of your radar stations, especially ones that are on the coast.
	  Otherwise a passing ship might decide to capture it and take a 
	  snapshot of your country. 
	- If a sub attacks one of your ships, it is possible to wander around
	  the area with a ship that is not capable of sonar or depth charging,
	  and still find that sub.  How?  Try boarding the sub by its hull
	  number.  If a sub attacks you, you will get a message saying that
	  sub number so and so did such and such to your ship.  Use a ship
	  in the area to wander from sector to sector trying to board the 
	  sub.  If you find it, you will get a message saying that you can not
	  board a sub.  Walla, you have found the sub without sonar, which
	  other ships can hear anyways.  This is a nice quiet form of anti-


User's Guide to Empire							 62

	  submarine warfare. 
	- You can load a missile cruiser full of anti-shipping missiles, and
	  then create a script file with firing commands for every ship in
	  existence.  Remove your own ships of course.  Then execute the
	  script, and the missile cruiser will fire at only those ships 
	  that are in range, regardless of whether the ships are in radar
	  range of the cruiser or not.  They need only be in range of the
	  anti-shipping missiles.  This is an effective, if blundering, way
	  of removing approaching fleets before you see them on your radars.
	  Note: anti-shipping missiles are not available in standard BSD code, 
	  but are available in some other versions, namely the Kent State 
	  code.


Summary: Be sneaky.  Be imaginative.  Think of tricks you can pull on your
	 opponent.  Quite often, they will in fact work.  Most players of this
	 game are not Rommels, Pattons, or Montgomerys.  Many can be duped 
	 by very simple tricks. 



User's Guide to Empire							 63

Chapter 14 :-: Tools

	Tools are programs used by players on their local machines to 
	assist them in developing their country, analyzing another country, 
	conducting war on another country, etc.  Tools are very powerful 
	devices at times.  As such, many players have tools that they consider 
	to be military secrets and they do not release copies of them.  As a 
	result, many programs doing basically identical tasks have been 
	written.  I encourage those players that are reading this that have 
	such secret tools to release them to an anonymous FTP site with a 
	directory for Empire.  Not only does this make the game better for the 
	beginners, but it also helps to keep the experts on their toes, and 
	give a challenge to those of us who have been playing for quite some 
	time. 

	Some players feel that playing with tools takes something away from
	the playing aspect of the game, and feel that in so doing they are not
	really playing the game anymore.  This has lately turned into a 
	religious war of sorts in the Empire community, with many people taking 
	both sides of the coin.  

	You do not need tools other than your client to play Empire. 
	Empire is designed such that any player can play the game without 
	hindrance without the use of a tool other than the basic client itself. 

	What follows is a list of tools that are most commonly found
	among players of the game.  Most of these are publicly available at
	an FTP site listed in Appendix C. 

Tool		Purpose
autodist	Automatically sets distribution paths for an entire
		country or realm within the country.
autofeed	automatically feeds the people in your country.
pmvr,civmvr	Automatically redistributes your population so that sectors
		will be able to have population growth rather than maxing
		out at the limit of 999 civilians.
econ		Provides an analysis of your country's economic progress
		at the next update.  
GEET		Gnu-Emacs Empire Tool.  An Emacs interface for Empire that
		has a great many capabilities and functions.  Provides for
		better automatic defenses as well.
EIF		A more advanced client than the basic client.  Allows
		for aliasing commands, command editing, etc. 
XEMP		Empire client for X windows.  Provides many nice features
		including an on screen map constantly displayed.  A very
		nice interface that unfortunately still has several bugs.

	These are just a few of the Empire tools out there.  There are still 
	many more programs not listed.  Some cover news analyses, country
	analyses over several updates, etc. 

	Most tools come with documentation on how to use them, and it is
	beyond the scope of this guide to illustrate how each one of them 


User's Guide to Empire							 64

	is used.

	There are other aids that are often called tools which are called
	scripts.  Scripts can do a great many things for you.  The most common
	of these applications are simply files containing a list of commands 
	that you want to execute in Empire.  

	For example, let's say that at each update you wanted to dump the 
	current news into a file in your local directory, get a new power
	report and dump that into another file, and dump all current telegrams
	into yet another file.  This would be three commands that you could 
	consolidate into one command by doing this:

	Construct a file in your local directory that has the appropriate
	commands in it.  In this case you could write a file with three lines:

news >>news.file
power new >>powerfile
read >>old.tele

	You could call this file update.do

	While in Empire you could then request the Empire client to access
	that file and execute those commands by using the exec command followed
	by the file name.  Thus:

[102:253] Command: exec update.do

	Building such a simple script is useful for doing a set of commands
	over and over again without typing it in each and every time.

	Lastly, remember to exercise wisdom in the use of tools.  It is 
	possible that the execution of a simple tool can damage your country 
	irreparably.  Use tools, especially those written by others, with 
	caution.  Try them out on small sections of your country first.  Look 
	at the source code prior to running it if you have access to the 
	source code.  It is theoretically possible for someone to write
	a tool that will scan the list of countries in a game, and if the
	author's country name is in that list, send valuable information
	about your country to him or her.  Yes, it is true, some people
	have no morals believe it or not.


Summary: Tools are very powerful items if used wisely.  It is not necessary
	 to have tools to play Empire effectively.  Some experts of the game
	 refuse to use tools of any type, and they remain among the best 
	 players.  Tools are no replacement for skill.  Be careful when using
	 tools, they can ruin your country.



User's Guide to Empire							 65

******************************* Appendices ************************************


Appendix A :-: Glossary of terms, names, and etc.

Amphibious assault - Attacking a sector using ships, usually landing ships.
Avail	    - A unit of measuring available labor hours in a sector.
Backstab    - To harm another country with whom you had prior treaties
	      and or alliances that were in effect at the time of your
	      harmful action.  Usually a severe action.
Blitz game  - An implementation of an Empire game that is very fast paced.
	      Usually these games last no more than two days. 
BSD 	    - Berkeley Standard Distribution.  A term borrowed from Unix.  
	      It indicates that the version is a release version from Berkeley,
	      ie Dave Pare. 
BTU 	    - Bureaucratic Time Unit.  A unit of government activity.  Used 
	      by various commands as cost for executing that command.
Capital     - The government center of a country.  BTUs are produced by
	      only one capital in a country, regardless of how many are
	      designated.  Only the actual capital of the country produces.
Client	    - A program that allows you to connect to the Empire server.
Commodity   - Just as in the real world; any of a plethora of raw and refined
	      materials that can be traded, stolen, etc. 
Dave Pare   - Not the original author of Empire, but the person who has 
	      made it possible for it to be what it is.  THE major author
	      of Empire.  Also known as Mr. Frog.
Deity       - Person (yes, a real life human) who runs/referees a game.
Deliver     - A means of managing economic resource allocation via orders
	      from neighboring sectors. 
Designation - What the sector has been ordered to be, ie a highway for example.
Distribution- A means of managing economic resource allocation via a central
	      warehouse scheme.  Often referred to as a "distribution
	      network"
Education   - Activity on the part of a country to raise the overall 
	      intelligence level of a country, to promote research and
	      technological development. 
Eif	    - "Empire Interface".  A version of the client with certain
	      enhancements.
Empire host - The computer the server is running on. 
ETU 	    - Empire Time Unit - A subdivision of an update.  An update's 
	      value is measured in terms of ETUs.
FTP 	    - "File Transfer Program".  This is a means of transferring
	      files over the computer networks.  Most computers are capable
	      of using FTP.  For more information you may refer to your 
	      host computer's help.  For example, "man ftp" or "help ftp".
GEET	    - Gnu-Emacs Empire Tool.  An Emacs based client, also known 
	      (to the chagrin of the author of GEET) as GEEK.
Guerrilla   - A patriot behind enemy lines.  They will attempt to do damage
	      to the enemy's military or the sector the enemy is occupying.
	      You can not control guerrilla actions. 
HCM         - Heavy construction material.  A unit of material used in
              Empire economies for bridges, planes, etc.
KSU, Kent State code - refers to the version of Empire based on the Berkeley


User's Guide to Empire							 66

	      release.  There are several enhancements in this version, and
	      many think it is better than the Berkeley release.
LCM         - Light construction material.  A unit of material used in
              Empire economies for education, technology, research, planes,
              boats, etc. 
Megatons    - A unit of measure of nuclear warhead yield.  One megaton 
	      equals one million tons of TNT explosive power.  Often 
	      represented as 'mt' as in '5mt'. 
Mobility    - A number indices indicating the available amount of moving
	      capability in a sector, for a plane, or for a ship. 
Plague	    - An all encompassing term representing disease in the Empire
	      world.  Can be kept in check by the country doing research.
Representative,Rep - Your password for your country in the game.  NOT your 
	      real name. 
Research    - Activity on the part of a country that keeps plague in check.
Reserves    - Civilian soldier.  A soldier not on active duty who is paid
	      a small sum to retain training in case of times of need.
Sanctuary   - Your starting area in any game.  Usually comprised of two
	      sectors.  Can not be attacked, or otherwise affected by
	      any opponent. 
Script	    - A file containing several Empire commands that can be 
	      automatically run or run manually from the client.
Sector	    - A unit of geography, comprising an area represented by a 
	      single character on maps. 
Server	    - The program (running usually at a remote site) that handles
	      the functioning of the game.  In essence, the game itself. 
Spy	    - A member of your military forces sent across neighboring
	      country borders to gather information.
Technology  - The scientific development of a country.  Relates most directly
	      to a country's capability to create better weapons.  Also known
	      as 'tech'.
Threshold   - A break off point for the 'deliver' and 'distribute' commands
	      that indicates a quantity beyond which every unit of a 
	      specific commodity is moved automatically at an update.
Tool	    - A program used on a player's local machine to aid in there
	      goals in Empire.  
Update	    - A 'turn' in Empire.  Mobility is increased in sectors, 
	      civilians multiply and eat food, military is paid for, etc.
UW	    - Uncompensated worker.  A nicer name for slave. 
VTOL	    - Vertical Take Off and Landing capability for planes.  Requires
	      no runways or other developed airport like structures.
Wasteland   - A permanent sector designation caused by a nuclear blast of
	      one megaton or larger.  Can not be traveled across by land,
	      but can be flown over. 
Warehouse  - A central collection and distribution point for products
	      of an economy.
Wilderness  - Undesignated areas other than mountains and ocean.  Represented
	      by a '-' character on maps.  
XEMP	    - An X windows based client.



User's Guide to Empire							 67

Appendix B :-: Known exploitable bugs

	As a result of Empire having several thousand lines of code, there
are numerous bugs.  Some are simply annoying, others you never notice.  Some,
like the following, are bugs that you can be used against you.

	a) "Intercept" bug - Non-VTOL planes can and will rise and intercept
	   hostile planes from any sector as long as there is petrol in the
	   sector, the plane has the intercept capability (see show plane c),
	   it is at least 40% efficient, and it has mobility. 

	b) "Escort" bug - Planes that are capable of escort duty can escort
	   themselves on various missions, as long as they are capable of
	   the mission in question (such as a bombing raid).  Say for example
	   that you want to fly a reconnaissance mission over enemy territory,
	   but you know that enemy planes are going to intercept you.  
	   When the game asks you for the plane that will be doing the recon,
	   you can give it plane 345 for example, which for the sake of 
	   this guide is a jet fighter.  When it asks you for the planes
	   that are going to escort it, you can say 345 again.  The game will
	   respond as if two planes are flying the mission when in fact only
	   one is.  Enemy planes will think there are two planes.  Enemy planes
	   will first do battle with the escort.  Let's say they did 16% damage
	   to the escort, and the escort did 14% damage to one of the enemy
	   interceptors.  Next, the enemy interceptor attacks the recon plane,
	   which is the same plane, and inflicts 21% damage.  The recon plane
	   inflicts 13% damage on the interceptor, and the recon plane aborts
	   the mission.  It looks like you lost the dogfight.  In actuality,
	   you did not.  The intercepting plane suffered a total of 27% damage.
	   It looks like you suffered 37% damage.  However, you actually only
	   suffered the *last* damage inflicted, which was to the recon plane.
	   Thus, you only suffered 21% damage and you won the dogfight. 

	c) "Flying Bridge" bug - Say for example you have a 100% bridgehead
	   sector with enough HCMs, workers, and avail to build a bridge. 
	   Normally, you would say "build bridge x,y" and then it would ask
	   you for a direction to build the bridge.  If after you built the 
	   bridge you were to redesignate the bridgehead, the bridge would 
	   collapse.  However, if you redesignate the bridgehead sector 
	   before you build the bridge, the bridge will be built and will 
	   not collapse.  At the next update, the bridgehead will become
	   the sector you redesignated it to (say, a highway) and the bridge
	   will still be there.  Note: It is believed that most current
	   games of Empire have a new patch that prevents this bug.

	d) "Resource sucker" bug - Let's say you have a sector with 23 gold
	   content left, and it is a gold mine.  Normally it might produce
	   say, 111 gold dust at the next update.  If you were to put 9000
	   civilians into that sector, it would produce over 900 dust at 
	   the next update.  The sector's resource content of gold will go
	   to 0, but instead of getting a roughly 115 dust from the sector
	   with the remaining content, you have gotten over 900 dust.  
	   Civilians reproduce quickly enough that losing 8000 civs this way


User's Guide to Empire							 68

	   is really of no concern especially when you consider what you
	   have gained.  This bug works also for oil wells and uranium
	   mines. 

	I have listed the bugs here because it is important that players 
	know about these bugs in case they are used against them.  I do not 
	condone the use of these bugs, nor do many players of the game.  
	Many deities (the people who run the game) take action against 
	players using these bugs, such as taking away your technology or 
	your money.  Do not use them.  Simply be aware that others may use 
	them and come up with ways to counter their use.  If you are aware 
	that another player is using one of these bugs, report it to the 
	deity of the game.  S/he might give you a reward of some sort. 

	Also note that since these bugs have been published> here, it is likely
	that some programmer has written bug fixes for these.  Most, and 
	possible all of these bugs might not work in a game you are 
	participating in. 



User's Guide to Empire							 69

Appendix C :-: FTP sites and other sources for Empire related items 

	Herein listed are the net address of computers that have files
	related to Empire in there anonymous FTP directories.  An example of 
	anonymous FTP is found in chapter 2 of this guide, and that method
	with appropriate substitutions for file names can be used to retrieve
	files off of these FTP sites.  If you have trouble using anonymous FTP
	seek out your local site consultant for more information.

Host			Directory		Files available
ucbvax.berkeley.edu  -  pub/games/empire/bsd	Empire client, server,
						various tools, this guide.
empire.bbn.com	     -  pub/Emp			Various tools, VMS client,
						this guide. 
star.cs.vu.nl	     -  xemp			Xemp		
hylka.helsinki.fi    -	public/vax/games-etc/empireclient.dir 
						VMS client	
uunet.uu.net	     -	games			Tools, server.
alcazar.cd.chalmers.se - pub/emp		Various tools, patches, eif
						eddie, this guide.


User's Guide to Empire							 70

Index

Agriculture centers 	     25,49,51	Naval operations
Announcements			   17		Defensive  	     36,54,57
Avail 				   45		In mapping 		55,61
Backstabbing 			   28		Anti-submarine 		   61
Banks 				   56		Amphibious 	  35,40,54,57
Bridges 		  42,45,58,67		Mining		     54,60,61
BTUs 			        26,45	Nuclear weapons
Capitals 	    16,26,36,54,55,57		Damage from 		   38
Clients 			    9		Ending games		   41
	Basic client 		    9		Getting 		38,42
	Compiling 		 9-12		Recovering from 	   39
Deliver 	       48,52-53,56,60		Defense of areas hit by    40
Designate 		     17,21,25	Oil		 42,43,45,50,54,55,67
Diplomacy 			28-32	Paratroops 
Distribution network   27,48-52,56,60		In nuclear war 		   40
Education 	    42-43,44,45,47,48		Against capitals 	   36
Empire						Against forts 		35,36
	Addictive		    8		Against defended sectors   57
	Blitz games 		    6	Plague 				   44
	Compared to Risk 	    6	Planes 
	Description 		    6		In defense 	  33-34,54,57
	Game length	    	    6		In mapping		   28
	Time needed to play 	    6		VTOL 		     33,34,40
Enlistment  		        40,54		Out of airbases      33,34,59
ETU, Empire Time Units 		   26		In nuclear war 		38,40
Exploring 		     16,21-25		Defending from 		   57	
Food in sectors 		27,54		In escort		   67
Forts 	      30,34-35,40,54,57,59-60	Population 			26,43
FTP				 9,63	Posturing 			   32
Gold dust 	 26,42,43,45,54,55,67	Registering
Guerillas 			36-37		Hearing of games            8
Guns			  34-35,36,40		Standard registration 	  8-9
HCMs				45,67	Rec.games.empire 		    8
Help				16,17	"Red Storm Rising" 		   30
Kent State Empire 	       43, 62	Research 			44,45
LCMs			  42,45,47,48	Reserves 		     37,54,57
Loans			 	30,61	Sanctuary 	       16,21,22,25,27
Mapping 		  28,54,55,61	Scripts 			   64
Maps 			     16,18-20	Sectors				   20
Maskirovka 			30-31	Server, connecting problems  12-15,26
Missiles				Starvation		  26,46,51,54
	In nuclear war 		   38	Technology 	    26,27,33,34,42-44
	Against planes 		   57	Telegrams			   17
	Against airbases 	   59	Thresholds 		  50-53,56,60
	Against ships 	   	   62	Tools 			 7,8,21,63-64
Mobility 	    24,25,26,49,51,61	Uncompensated workers 		43,46
Mountains  			25,36	Updates	   16,21,25,26,27,36,46,51,56
Movement 			16,20	Warehouse 26,43,48,49-51,56,59,60,61
					Wilderness 	    20,21,22,23,24,25

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
.fi
.SA "Introduction"
