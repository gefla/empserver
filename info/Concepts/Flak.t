.TH Concept Flak
.NA Flak "How flak works"
.LV Basic
This page describes flak, and how it works.
.s1
Flak is created from gunfire when enemy planes fly overhead.
.s1
The way that flak works when planes fly over a sector is:
.nf
 * Sector flak is fired.  Up to a maximum of 14 guns can fire
   before being scaled.
 * Land unit flak is then fired.  Up to a maximum of 14 guns can
   fire before being scaled.  Only 'flak' capable units will fire
   in this volley (see "info show"), and only the guns loaded onto
   a land unit are counted in the # of guns firing.
 * Ship flak is then fired.  Up to a maximum of 14 guns can fire
   before being scaled.
 * Defensive planes then intercept.
.fi
.s1
In all of the above flak rounds, once the maximum # of guns firing
has been determined, the # of guns is then scaled by your country's
tech factor, and then doubled.  So, for example, if you had 14 guns
firing, and have a tech factor of 35%, you would have a total of
(14 * .35) * 2 = 10 (after rounding) guns firing.
.s1
In addition, if you are pinbombing a land unit or a ship, when 
you make your bombing run, the specific unit/ship you are bombing
gets to fire flak at you again.  This number of guns firing flak
in these cases is NOT scaled after the number of guns is determined.
.s1
When a sector fires flak, the amount of shells required is the # of
guns fired divided by 2.  If not enough shells are available, either no flak
is fired, or the flak is reduced to the number of shells available.
Note that the shells are not consumed during flak fire, they just need
to be available.
.s1
When a ship fires flak, at least 1 shell must be available.
If no shells are available, no flak is fired.  Note that the shell is
not consumed during flak fire, it just needs to be available.
.s1
Land units use no shells when firing flak.  Any land unit with an
aaf rating of > 0 will fire flak when pin-bombed.
.s1
The formulas for determining the # of guns fired in a general volley
is:
.nf
Sector:
    guns = (guns in sector);
    if (guns > 14)
        guns = 14;

Ship:
    guns = 0;
    for (each ship in the sector)
        guns += min(guns, guns able to fire) * (techfact of ship) * 2;
    if (guns > 14)
        guns = 14;

Land unit:
    guns = 0;
    for (each land unit in the sector)
        guns += (aaf rating) * (techfact of unit) * 3;
    if (guns > 14)
	guns = 14;
.fi
.s1
Then, for each of the above general flak volleys, the # of guns is
scaled like so:
.nf
    firing = guns * (techfact of nation) * 2;

On specific bombing runs, the # of guns firing is determined by:

Ship:
    guns = min(guns, guns able to fire) * (techfact of ship) * 2;

Land unit:
    guns = (aaf rating) * (techfact of unit) * 3;

The # of guns in these cases are NOT scaled.

techfact is determined by:
    techfact = (50.0 + tech) / (200.0 + tech);
.fi
.s1
Once the number of flak guns firing has been determined, the planes
have to fly through it.  The plane may have to fly through flak up
to 4 times in each sector, depending on circumstances.  They are:
.nf

    1) Fly through general sector flak (if any)
    1a) fight in a possible dog-fight
    2) Fly through general ship flak (if any)
    3) Fly through general unit flak (if any)
    4) If pinbombing, fly through specific unit or ship flak (if any)
.fi
.s1
To figure out the damages that a plane takes each time it flys through
flak, the following formula's are used:
.nf

    flak = # of guns firing.
    flak = flak - (planes defense + 1);
    if (plane is stealthy)
        flak = flak - 2;
    if (plane is half stealthy)
        flak = flak - 1;
    if (flak > 8)
        mult = flaktable[15] * 1.33;
    else if (flak < -7)
        mult = flaktable[0] * 0.66;
    else {
        flak += 7;
        mult = flaktable[flak];
    }
    mult *= flakscale;
    damage = ((random number from 1 to 8) + 2) * mult;
    if (damage > 100)
        damage = 100;

    plane's eff = (plane's eff) - dam;
    if (plane's eff < 10%)
        plane is shot down
    else if (chance((100 - (plane's eff)) / 100))
        plane aborts mission
    otherwise the plane continues on it's mission

For the above, use this table:

    flaktable[0] = 0.20
    flaktable[1] = 0.20
    flaktable[2] = 0.25
    flaktable[3] = 0.30
    flaktable[4] = 0.35
    flaktable[5] = 0.40
    flaktable[6] = 0.45
    flaktable[7] = 0.50
    flaktable[8] = 0.50
    flaktable[9] = 0.55
    flaktable[10] = 0.60
    flaktable[11] = 0.65
    flaktable[12] = 0.70
    flaktable[13] = 0.75
    flaktable[14] = 0.80
    flaktable[15] = 0.85

and a flakscale of 1.75
.fi
.SA "bomb, fly, recon, paradrop, nation, Planes, Interception, Combat"

