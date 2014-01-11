.TH Command OFFER
.NA offer "Offer a loan to another country"
.LV Expert
.SY "offer loan <CNUM/CNAME> <AMOUNT> <DURATION> <RATE>"
The offer command is used to offer a loan
to another country.
.s1
<CNUM/CNAME> is the number or name of the country to whom
you are offering the loan,
.s1
<AMOUNT> is the size of the loan.
(NOTE: do not use a dollar sign),
.s1
<DURATION> is the length of the loan in days,
.s1
<RATE> is the interest rate, (per <DURATION>), on the loan.
.s1
The program prompts with various questions concerning any
terms of the loan not provided on the command line
and sends a telegram to <CNUM/CNAME> announcing the loan offer.
.s1
If the other country accepts the loan,
(using the \*Qconsider\*U command),
the money will be transferred from your account
and you will be notified that the loan duration has started.
.s1
Note that the dependence of rate on the duration makes a 30 day loan
at 10% a worse deal than a 60 day loan at 15%.
.s1
.SA "collect, consider, ledger, repay, Loans"
