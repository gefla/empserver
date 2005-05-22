REM Example Backup script to be run from pre_update_hook
REM Written by Ron Koenderink, 2005
REM This script is in the public domain.
REM
REM Use: edit configuration variables below to taste, then set
REM   pre_update_hook = c:\home\wolfpack\emp4\scripts\backup.bat
REM in econfig.
REM
REM Rules when running as pre_update_hook:
REM Data directory is working directory.
REM stdin, stdout, stderr are redirected to /dev/null
REM Exit code other than zero cancels update!
REM
REM This script is as simple as possible, since failure cancels the
REM update.
xcopy /E /Y c:\home\wolfpack\emp4\data c:\home\wolfpack\emp4\backup
