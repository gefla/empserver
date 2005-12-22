.TH Command FOLLOW
.NA follow "Cause one ship to automatically follow another"
.LV Expert
.SY "follow <SHIP/FLEET>"
You use the \*Qfollow\*U command to get a ship to use the same
sailing path as another ship.
At each update, ships with a sailing path \*Qautonavigate\*U along
the path that was set for them, and ships that are set
to follow that ship will also be navigated along the same
path.  Telegrams notify you of arrivals
or problems encountered while attempting to follow the path.
.s1
Radar operates continuously as the ship moves along the path, constantly
adding sector information to the known world map.
.s1
Ship movement is performed before mobility is increased.
This insures that after the update, ships with a sailing path will still
have some mobility so that you can always manually navigate
to some extent.
.s1
You may follow any ship which is owned by you or a friendly country.
.s1
.SA "sail, unsail, mquota, Ships"
