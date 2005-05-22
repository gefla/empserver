REM Example Blitz script which can be ran as a scheduled task.
REM Written by Ron Koenderink, 2005
REM This script is in the public domain.
REM
REM You will need to set the EMPIREHOST and EMPIREPORT if the 
REM emp_client is not compiled with the correct defaults.
REM set EMPIREHOST=localhost
REM set EMPIREPORT=6665
REM
REM The script can be simplified by setting the execute
REM directory to the bin directory.
REM The -e is not necessary unless you are not using the
REM the default game directory.
REM
echo power new | c:\home\wolfpack\emp4\bin\emp_client -2 power.out POGO peter
net stop "Empire Server"
c:\home\wolfpack\emp4\bin\files -f -e c:\home\wolfpack\emp4\data\econfig
c:\home\wolfpack\emp4\bin\fairland -e c:\home\wolfpack\emp4\data\econfig 10 30
net start "Empire Server"
c:\home\wolfpack\emp4\bin\emp_client POGO peter <newcap_script
echo enable | c:\home\wolfpack\emp4\bin\emp_client POGO peter
