.TH Command REPORT
.NA report "List all countries and their status"
.LV Basic
.SY "report <NATS>"
The report command displays a list of nations in <NATS>.  It shows
their status (in-flux, visitor, active, etc), and their estimated tech,
education, research and happiness levels.
.s1
If <NATS> is a country name or number, it will only display information
on that country; if <NATS> is * then information on all countries will
be listed.
.s1
An example of a report command for the country \*QFroom 93\*U would
look something like this:
.EX report *
.NF
Thu Sep 14 16:10:40 1995
 #    name                tech      research   education   happiness  status
 2     Artifax           0 - 147    79 - 99      0 - 23      0 - 9    In flux
 3     Billy             0 - 137    79 - 99      0 - 23      0 - 9    In flux
 4     CCCP            150 - 200   120 - 150    23 - 30     20 - 25   Active
 8     Khazad'ur        86 - 108   120 - 150    23 - 30     12 - 17   Active
 9     Lorphos          96 - 121   120 - 150     0 - 23     12 - 17   Active

This is how the ranges are determined.
(1) If your value of the statistic is 0, then n/a (not applicable)
    will be printed in that column.

(2) If the value for both countries is > 100, then shift them down so
    the lowest one is at 100.

(3) Each of the following tests is made in order.  As soon as one of them
    is true, then  that will be the range for that statistic:
      2.0 * yours < theirs
      1.5 * yours < theirs < 2.0 * yours
      1.2 * yours < theirs < 1.5 * yours
      1.1 * yours < theirs < 1.2 * yours
      yours / 1.1 < theirs < 1.1 * yours
      yours / 1.2 < theirs < yours / 1.1
      yours / 1.5 < theirs < yours / 1.2
      yours / 2.0 < theirs < yours / 1.5
                    theirs < yours / 2.0

(4) Once the range has been determined, it is shifted back up to where
    it should be.

(5) There is a "minimum range" for the different characteristics:
      tech: 20
      res:  10
      edu:   5
      hap:   5
    If the distance between the high and low end of the range is smaller
    than the "minimum range", then the range is expanded (equally in both
    directions but never crossing zero) to be equal to the minimum range.
    However, it will never be expanded beyond twice your value of that
    statistic.
.FI
.s1
.SA "country, Diplomacy, Nation"
