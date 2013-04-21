.TH Concept Flak
.NA Flak "How flak works"
.LV Basic
This page describes flak, and how it works.
.s1
Flak is created from gunfire when enemy planes fly overhead.
.s1
The way that flak works when planes fly over a sector is:
.nf
 * Sector flak is fired.  Up to a maximum of 14 guns can fire.
   They're then scaled by twice your country's tech factor.  So, for
   example, if you had 14 guns firing, and have a tech factor of 35%,
   you would have a total of 14 * .35 * 2 = 10 (after rounding) guns
   firing.
 * Land unit flak is then fired.  Only 'flak' capable units will fire
   in this volley (see "info show"), and each point of aaf counts as
   1.5 flak guns.  Flak saturates at 14 guns, and is then scaled by
   unit tech factor.
 * Ship flak is then fired.  Flak saturates at 14 guns, and is then
   scaled by unit tech factor.
.fi
.s1
In addition, if you are pin-bombing a land unit or a ship, when
you make your bombing run, the specific unit/ship you are bombing
gets to fire flak at you again.  This number of guns firing flak
in these cases does NOT saturate.
Land units do not need capability flak to fire here.
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
        guns += min(guns, guns able to fire)
    if (guns > 14)
        guns = 14;

Land unit:
    guns = 0;
    for (each land unit in the sector)
        guns += (aaf rating) * 1.5
    if (guns > 14)
	guns = 14;
.fi
.s1
Then, for each of the above general flak volleys, the # of guns is
scaled like so:
.nf
    firing = guns * (average techfact) * 2;

where average techfact is the average tech factor of everything that
could fire in this volley, regardless of saturation.  Sectors use
their owner's tech factor.

On pinpoint bombing runs, the # of guns firing is determined by:

Ship:
    guns = min(guns, guns able to fire) * (techfact of ship) * 2;

Land unit:
    guns = (aaf rating) * (techfact of unit) * 3;

techfact is determined by:
    techfact = (50.0 + tech) / (200.0 + tech);
.fi
.s1
Once the number of flak guns firing has been determined, the planes
have to fly through it.  The plane may have to fly through flak up
to 4 times in each sector, depending on circumstances.  They are:
.nf

    1) Fly through general sector flak (if any)
    2) Fly through general ship flak (if any)
    3) Fly through general unit flak (if any)
    4) If pin-bombing, fly through specific unit or ship flak (if any)
.fi
.s1
To figure out the damages that a plane takes each time it flies through
flak, the following formula's are used:
.nf

    flak = # of guns firing.
    flak = flak - (planes defense);
    if (plane is not tactical)
        flak = flak - 1;
    if (plane is stealthy)
        flak = flak - 2;
    if (plane is half stealthy)
        flak = flak - 1;
    if (flak > 8)
        mult = flaktable[16];
    else if (flak < -7)
        mult = flaktable[0];
    else {
        flak += 8;
        mult = flaktable[flak];
    }
    mult *= flakscale;
    damage = ((random number from 1 to 8) + 2) * mult;
    if (damage > 100)
        damage = 100;

    plane's eff = (plane's eff) - dam;
    if (plane's eff < 10%)
        plane is shot down
    else if (chance((80 - (plane's eff)) / 100))
        plane aborts mission
    otherwise the plane continues on it's mission

For the above, use this table:

    flaktable[0] = 0.132
    flaktable[1] = 0.20
    flaktable[2] = 0.20
    flaktable[3] = 0.25
    flaktable[4] = 0.30
    flaktable[5] = 0.35
    flaktable[6] = 0.40
    flaktable[7] = 0.45
    flaktable[8] = 0.50
    flaktable[9] = 0.50
    flaktable[10] = 0.55
    flaktable[11] = 0.60
    flaktable[12] = 0.65
    flaktable[13] = 0.70
    flaktable[14] = 0.75
    flaktable[15] = 0.80
    flaktable[16] = 1.1305

and a flakscale of 1.75
.fi
.SA "bomb, fly, recon, paradrop, nation, Planes, Interception, Combat"
