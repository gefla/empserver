.TH Command LEDGER
.NA ledger "Report on outstanding loans"
.LV Expert
.SY "ledger"
The ledger command prints the information on all outstanding loans
in which you are involved.
.s1
The loan number, lender's name, loanee's name, principal, interest rate,
expiration date, amount due, etc. will be printed for each loan.
.s1
For example:
.EX ledger
.NF
        \&... Groonland Ledger \&...

Loan #0 from Groonland to Dog Dish
Amount paid to date $0
Amount due (if paid now) $5046 (if paid on due date) $5250
Due date is Thu Mar 19 16:44:35 1981

Loan #1 from Doctor Fate to Groonland
(proposed) principal=$3000 interest rate=10% duration(days)=30
Loan must be accepted by Tue Feb 17 16:43:27 1981
.FI
.s1
.SA "consider, financial, offer, Loans"
