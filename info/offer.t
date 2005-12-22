.TH Command OFFER
.NA offer "Offer a loan or treaty to another country"
.LV Expert
.SY "offer loan <CNUM/CNAME> <AMOUNT> <DURATION> <RATE>"
.SY "offer treaty <CNUM/CNAME>"
The offer command is used to offer either a loan or a treaty
to another country.
.s1
In the case of a loan:
.in +4
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
.in -4
In the case of a treaty:
.s1
.in +4
<CNUM/CNAME> is the name or number of the country to whom
you wish to offer the treaty.
.s1
You will be asked to set both the conditions for yourself
and for the other country.
.s1
The conditions from which you may choose are:
.s1
.NF
Condition                 Command(s) affected
no attacks on land units  lboard
no attacks on ships       board
no sector attacks         assault, attack, paradrop
no shelling ships         fire
no depth-charging subs    fire
no shelling land          fire
no enlistments            enlist
no building               build
.FI
.s1
Note that the first six terms apply to interactions
between the two parties to the treaty while the last two apply
to ANY actions of the specified type.
.s1
Once you have offered the treaty,
a telegram will be sent to the other country indicating the offer.
If the other country accepts the treaty,
(using the \*Qconsider\*U command),
you will be notified that it has gone into effect.
.s1
Once signed, the treaty is not binding!
However, any violation of the treaty will be pointed out
to the violator in time for him/her to reconsider the action and will
find its way into the news.
.in -4
.s1
.SA "collect, consider, ledger, repay, treaty, Loans"
