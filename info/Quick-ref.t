.TH Concept "Quick Reference Card"
.NA Quick-ref "A quick reference card for sector types and production"
.LV Basic
.NF

Sector-types:
    BASICS                   INDUSTRIES                MILITARY / SCIENTIFIC
    .  sea                   d  defense plant          t  technical center
    ^  mountain              i  shell industry         f  fortress
    s  sanctuary             m  mine                   r  research lab
    /  wasteland             g  gold mine              n  nuclear plant
    -  wilderness            h  harbor                 l  library/school
    c  capital/city          w  warehouse              e  enlistment center
    p  park                  u  uranium mine   	       !  headquarters
    COMMUNICATIONS           *  airfield               
    +  highway               a  agribusiness           FINANCIAL
    )  radar installation    o  oil field              b  bank
                             j  light manufacturing
    #  bridge head           k  heavy manufacturing
    =  bridge span           %  refinery

(Petrol is per 10 units; others per unit)

Products:
Item      $    Lcm  Hcm Iron Dust  Oil  Rad   Tech   Production Eff.
Shells:   3     2    1    0    0    0    0     20    (tech-20)/(tech-10)
Guns:    30     5    10   0    0    1    0     20    (tech-20)/(tech-10)
Iron:     0     0    0    0    0    0    0      0    0
Dust:     0     0    0    0    0    0    0      0    0
Bars:    10     0    0    0    5    0    0      0    0
Food:     0     0    0    0    0    0    0      0    (tech+10)/(tech+20)
Oil:      0     0    0    0    0    0    0      0    (tech+10)/(tech+20)
Petrol    1     0    0    0    0    1    0     20    (tech-20)/(tech-10)
Lcm:      0     0    0    1    0    0    0      0    (tech+10)/(tech+20)
Hcm:      0     0    0    2    0    0    0      0    (tech+10)/(tech+20)
Rad:      2     0    0    0    0    0    0     40    (tech-40)/(tech-30)
Educate:  9     1    0    0    0    0    0      0    0
Happy:    9     1    0    0    0    0    0      0    0
Tech:  5*ETUS  10    0    0    1    5    0      0    (edu-5)/(edu+5)
Resrch:  90    10    0    0    1    5    0      0    (edu-5)/(edu+5)
.FI
.SA "Sectors, Producing"
