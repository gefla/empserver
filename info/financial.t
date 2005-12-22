.TH Command FINANCIAL
.NA financial "Show all the outstanding loans in the world"
.LV Expert
.SY "financial"
The financial report shows all of the outstanding loans in the world,
so that the loan sharks can better calculate
the ability of a prospective client to pay.
.s1
It does not show loan offers pending, however,
just those loans which have been accepted.
.s1
If a loan is past due (i.e. the loanee has defaulted on the loan) the
"(in arreras)" is printed after the line of that loan.  Loans in default
may be "sharked" up and collected upon.
.s1
.EX financial
.NF
             -= Empire Financial Status Report =- 
                  Mon Oct  6 11:12:30 1986
Loan      From            To      Rate   Dur    Paid    Total
 6   ( 2) Domino    ( 3) Oz         0%    10        0   5000
 8   ( 4) Zyf       (14) Id         0%    30        0   2000 (in arreras)
 11  ( 2) Hodedo    ( 3) Oz        13%    15    18000  25000
.FI
.s1
.SA "offer, ledger, shark, Loans"
