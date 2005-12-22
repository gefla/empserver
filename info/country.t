.TH Command COUNTRY
.NA country "Find out country status"
.LV Expert
.SY "country <NATS>"
The country command displays status of the countries you specify.
.s1
.EX country *
.NF
Sat Oct 15 20:54:30 2005
  #   last access                         status     name
  0   Now logged on                       DEITY      POGO
  1   Now logged on                       Active     Khazad'ur
  2   Unknown                             Active     Spectre
  3   Unknown                             Sanctuary  3
  4   Unknown                             Sanctuary  4
  5   Unknown                             Visitor    visitor
.FI
.s1
Deities get more information:
.s1
.EX country *
.NF
Sat Oct 15 20:54:30 2005
  #   last access                         BTU  status     name
  0   Now logged on                       640  DEITY      POGO
  1   Now logged on			  480  Active     Khazad'ur
  2  Sat Oct 15 17:56 - Sat Oct 15 18:11  638  Active     Spectre
  3   Never logged on			  640  Sanctuary  3
  4   Never logged on			  640  Sanctuary  4
  5  Sat Oct 15 20:08 - ?                 640  Visitor    visitor
.FI
.s1
.SA "players, Communication, Playing"
