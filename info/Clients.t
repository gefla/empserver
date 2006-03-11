.TH Introduction "Empire4 Compatible Clients"
.NA Clients "Clients which communicate well with the Empire4 Server"
.LV Basic
.NF
What follows is a list of clients which support the new Empire4
protocol.  All of these clients may be found on the Wolfpack Empire
Archives located at:
  http://www.wolfpackempire.com

empclient-2.8 and later - full support

If you are using one of these clients, then you should type
"toggle inform" once you have broken sanctuary.  These clients
are fully asynchronous, which means that they are able to respond
immediately when an unexpected message arrives from the server.  So,
for example, if someone sends you a "flash" message, then the message
will be printed on your screen immediately.  Similarly, with "inform"
toggled on, you will be informed the moment a telegram arrives.  Note,
the server remembers your toggle flags when you log out, so you do not
have to type "toggle inform" again the next time you connect.

Most other clients should work fine with the Empire4 server, however
they may get confused if a flash message comes in.  Users of other
clients should keep the "inform" flag toggled off, and ask a friend to
send them a "flash" message to see how the client handles it (note,
you must declare friendly relations towards them in order to receive
the flash message).  If the flash message confuses your client, then I
recommend you type "toggle flash" to turn flash mode off.
.FI
.s1
In addition, there are a list of commands which have been added to the
server to help the development of clients.  They are:
.NF
   xdump - Dump everything, designed to replace the following
   dump - Dump sector information
   ldump - Dump land unit information
   sdump - Dump ship information
   pdump - Dump plane information
   ndump - Dump nuclear stockpile information
   lost - Report lost items
.FI
.s1
See the various info pages on these for complete documentation on how they
work and how you can use them to help improve your clients.
.s1
In addition, there is a "timestamp" field on each object (sectors, ships,
land units, planes, nuclear stockpiles, lost items) that you can use to 
compare against to keep data between clients and the database in sync with
each other.  These timestamps are kept in systems seconds, so they should
be accurate down to 1 second.  Every time an object is changed, it's
timestamp is updated.
.SA "toggle, xdump, dump, ldump, sdump, pdump, ndump, lost, Empire4, Communication, Introduction"
