.TH Command RADAR
.NA radar "Perform radar scan from ship or sector"
.LV Basic
.SY "radar [<SHIP/FLEET> | <SECTS>]"
The radar command bears some resemblance
to modern high-resolution radar.
It has a circular range dependent upon its efficiency.
.s1
To run land radar type:
.EX radar <SECTS>
where <SECTS> must be the sector where the radar station resides,
or an area that contains one or more radar stations.
.s1
The program will respond with the station's efficiency and range
and then display the area.
.s1
What is displayed is dependent upon the range.
Sectors and ships within 1/3 of the range
are displayed with their sector or ship designations.
Sectors at greater distances are indicated by a '?'
if they are inhabited by another country.
.s1
A 100% radar station in a country with infinite technology
on a clear day has a range of 16.
The range is linearly related to efficiency;
thus a 50% radar station can see half as far
as a 100% radar station.
The range is related to technology level in the \*Qusual\*U way,
i.e., it varies linearly with \*Qtechnology factor\*U
as indicated in the nation report;
see \*Qinfo nation\*U and \*Qinfo technology\*U for details.
.s1
Satellites will appear on the map as a '$'.
.s1
WARNING!
Land radar cannot spot submarines.
.s1
FURTHER WARNING!
Even worse, should your cunning enemy move troops right to your
doorstep and leave the sector designated a wilderness it would
show up on your very efficient radar as '-'.
Thus you would never know he was there....
Well, almost never. 
.s1
An example of land radar:
.EX radar 5,1
.NF
5,1 efficiency 100%, max range 6
       . a a - ^ ? ?       
      T k o ! - ^ ? ?      
     a h j a ^ ^ ^ ^ ^     
    h . a w a ^ - ? a ?    
   . . a m a b f a ^ ^ -   
  . . m c * a a - ^ - ^ -  
 . - . a + . 0 a - - ^ - . 
  - ^ ^ a + ) ^ a - - ^ .  
   - - a m + a f . . - ^   
    ^ a - a + a ^ - - -    
     - ^ - - ^ - - - -     
      - ^ - - - - - -      
       - - - - - - -       
.FI
.s1
To run unit radar type:
.EX lradar <UNIT/ARMY>
where <UNIT/ARMY> is the number of a unit or an army designation.
.s1
To run naval radar type:
.EX radar <SHIP/FLEET>
where <SHIP/FLEET> is the number of a ship or a fleet designation.
.s1
Sea radar is highly dependent upon what kind of ship you
have and what kind the opponent has.
(e.g., Battleships can see battleships far away
but fishing boats must be close together to see each other).
.s1
The maximum distance at which ship A can see ship B
is dependent on two factors:
the \*Qspying\*U ability of ship A,
and the \*Qvisibility\*U of ship B.
Battleships have sophisticated communications equipment
which give them a maximum spying range of 6 while fishing boats,
having little such equipment,
have a maximum range of 2.
This factor is shown in a table in \*Qinfo Ship-types\*U
under the heading \*Qspy\*U.
On the other hand, the ships vary in size and consequently,
vary in visibility.
Battleships, oil derricks, and carriers are the easiest to see;
next come cargo ships, heavy cruisers and tenders, and so on.
See \*Qinfo Ship-types\*U for a chart.
.s1
Ship A will see ship B if the distance between them is less than:
.NF + 0.3i
             spy factor of ship A * visibility of ship B
efficiency * ------------------------------------------- * technology factor
                               20
.FI
.s1
Note that, although subs can not be found via radar,
destroyers automatically use sonar so they can see subs.
Destroyer A will see submarine B if the distance between them is less than:
.NF + 0.3i
             spy factor of ship A * visibility of ship B
efficiency * ------------------------------------------- * technology factor
                               20
.FI
.s1
An example of ship radar:
.EX radar 50
.NF
patrol boat #50 at -1, -3 efficiency 100%, max range 4
     . . . . .   
    . . . . - -  
   . . - . a a - 
  . . . T k o ! -
 . - F a P j a ^ ^
  . . - . a w a ^
   . . . a m a b 
    . . m c * a  
     - . a + .   
.FI
.s1
.SA "skywatch, coastwatch, census, map, nation, Ship-types, Technology, lradar, Detection, Ships, Sectors"
