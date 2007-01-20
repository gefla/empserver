.TH Command SHUTDOWN
.NA shutdown "Shut down the server"
.LV Expert
.SY "shutdown <minutes> <disable update?>"
Shutdown the server in <minutes> minutes.  If <minutes> is 0,
then any previous shutdown request will be canceled.  If
the second argument is a "n" then updates will not be disabled
(otherwise they will be).
.s1
All players currently logged on will be sent flash messages when the
shutdown is started, every hour the day of the shutdown, every 10
minutes the hour of the shutdown, and every minute for the last five
minutes of the shutdown.  When the shutdown actually
happens, the server will abort player commands in the same way that
the update does.  The players connection is closed in the same way
that idle connections are closed.
.SA "enable, disable, force, turn, Deity"
