.TH Command REPAY
.NA repay "Repay a loan (all or part)"
.LV Expert
.SY "repay <LOAN>"
The repay command is used to repay loans from other countries.
.s1
The <LOAN> argument is the number of the loan you are repaying
and if not included on the command line
it will be solicited by the program.
.s1
You can repay all or part of the loan with this command;
however beware of letting the loan become overdue --
the lender may decide to force collection
(see \*Qinfo collect\*U).
.s1
When you repay on a loan a telegram is sent to the lender
informing her/him of the amount repaid.
.s1
.EX repay 12
.NF
You presently owe $4515    payment : $2000
.FI
or,
.EX repay 12
.NF
You presently owe $4515   payment : $4515
Congratulations, you've paid off the loan!
.FI
.s1
.SA "collect, consider, ledger, offer, Loans"
