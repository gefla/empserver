#!/bin/sh
#
# Blame it on marcolz
#
# Skip certain parts of this script by exporting the variable
# "NIGHTLY_SKIP_STEP" containing the following possible substrings,
# preventing the named behaviour:
#
# REDIRECT	-	Redirect all output to a logfile
# CHECKOUT	-	Fill the sandbox with a fresh checkout
# PATCH		-	Apply the patches for this system (global + specific)
# BUILD		-	Build everything
# GENERATE	-	Generate a new world
# SERVERSTART	-	Start the server
# TESTSCRIPT	-	Run the testscript
# SERVERSTOP	-	Stop the server if it was started by this script
# CLEANUP	-	Remove the contents of the sandbox
# REMOVE_REPOSITORY	Removes the git repository when cleaning up
#

PROGNAME="$0"
TERM="${TERM:-vt100}"
export TERM

usage() {
	echo "Usage: ${PROGNAME} <configfile>" >&2;
	exit 1;
}

[ $# -lt 1 ] && usage

[ -f "$1" ] || usage

# Source config file
case "$1"
in
	*/*)
		. "$1"
		;;
	*)
		. ./"$1"
		;;
esac

[ -f "${SCRIPTDIR}/common.sh" ] || { echo "Broken config ?" >&2; exit 1; }
. "${SCRIPTDIR}"/common.sh
export SCRIPTDIR


STAMP="`date +%Y%m%d%H%M%S`"
WORKDIR="${INSTANCE}"
[ -n "${EXTRASUFFIX}" ] && WORKDIR="${WORKDIR}.${EXTRASUFFIX}"
LOGFILE="${LOGDIR}/${WORKDIR}.${STAMP}"

#
# START REDIRECT
#
case "${NIGHTLY_SKIP_STEP}"
in
	*REDIRECT*) ;;
	*)

# Log everything
exec > "${LOGFILE}"
exec 2>&1

		;;
esac
#
# END REDIRECT
#

case "${BOXDIR}"
in
	/*)
		;;
	*)
		BOXDIR="${SCRIPTDIR}/${BOXDIR}"
		;;
esac

cd "${BOXDIR}" || err "Could not chdir to ${BOXDIR}"

echo "Nightly build starting at `date`"


#
# START CHECKOUT
#
case "${NIGHTLY_SKIP_STEP}"
in
	*CHECKOUT*) ;;
	*)

# Make sandbox
if [ -d "${WORKDIR}" ]
then
	[ -d "${WORKDIR}"/empserver/.git ] || err "Invalid sandbox, missing .git directory"
else
	echo making directory
	mkdir "${WORKDIR}" || warn "Could not create ${BOXDIR}/${WORKDIR}"
fi
cd "${WORKDIR}" || err "Could not cd to ${BOXDIR}/${WORKDIR}"

echo "Getting source from GIT:"
# Extract source
export GITROOT=${GITROOT:= git://git.pond.sub.org/~armbru/empserver}
RETR=0
if ! [ -d empserver ]
then
	while ! git clone --quiet $GITROOT empserver >/dev/null
	do
		sleep "`expr 5 + ${RETR}`"
		RETR="`expr 1 + ${RETR}`"
		[ "${RETR}" -gt 5 ] && err "git clone Timeout after ${RETR} retres."
	done
	cd empserver || err "Could not cd to ${BOXDIR}/${WORKDIR}/empserver."
else
	cd empserver || err "Could not cd to ${BOXDIR}/${WORKDIR}/empserver."
	while ! git pull --quiet $GITROOT master >/dev/null
	do
		sleep "`expr 5 + ${RETR}`"
		RETR="`expr 1 + ${RETR}`"
		[ "${RETR}" -gt 5 ] && err "GIT pull Timeout after ${RETR} retres."
done

fi

echo "Commit `git describe --match 'v*' --always`"
echo "Done (GIT)."
echo ""
		;;
esac
#
# END CHECKOUT
#

#
# START PATCH
#
cd "${BOXDIR}/${WORKDIR}/empserver" || err "Could not cd to ${BOXDIR}/${WORKDIR}/empserver"
case "${NIGHTLY_SKIP_STEP}"
in
	*PATCH*) ;;
	*)

echo "Applying global patches from ${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/patches/All"
for i in "${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/patches/All"/*.patch
do
	[ -r "${i}" ] || continue
	if git apply "${i}" >/dev/null
	then
		echo "${i}: OK"
	else
		echo "========== ${i}: NOT OK! ${?} =========="
	fi
done
echo "Done (patch All)."
echo ""

LOCALPATCHDIRECTORY="${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/patches/${INSTANCE}"
if [ -n "${LOCALPATCHDIRECTORY}" -a -d "${LOCALPATCHDIRECTORY}/." ]
then
	echo "Applying system specific patches from ${LOCALPATCHDIRECTORY}:"
	for i in "${LOCALPATCHDIRECTORY}"/*.patch
	do
		[ -r "${i}" ] || continue
		if git apply "${i}" >/dev/null
		then
			echo "${i}: OK"
		else
			echo "========== ${i}: NOT OK! ${?} =========="
		fi
	done
	echo "Done (patch specific)."
	echo ""
fi
echo "Apply controlled time patch."
for f in `git-ls-files | grep -E '\.[ch](\.in)?$' | xargs grep -l \>`
do
 n=`grep -n '^[     ]*#[     ]*include\>' $f | tail -n 1 | sed 's/:.*//'`
if [ $n ]
then
sed "$n"'a\
#include "emptime.h"\
#undef time\
#define time(timer) emp_time((timer), __FUNCTION__)' $f >$f.patched
mv $f.patched $f
fi
done
git add include/emptime.h src/lib/gen/emptime.c
echo "Done applying controlled time patch."
echo ""
git pull --quiet
sh ./bootstrap
./configure --prefix ${BOXDIR}/${WORKDIR}/emp4 ${CONFIGURE_OPTIONS}

		;;
esac
#
# END PATCH
#

#
# START BUILD
#
case "${NIGHTLY_SKIP_STEP}"
in
	*BUILD*) ;;
	*)


# Start the build
echo "Building server"
if make -k install >/dev/null
then
	warn "make did not return 0"
fi
echo "Done (make)."
echo ""

		;;
esac
#
# END BUILD
#

cd "${BOXDIR}/${WORKDIR}" || err "Could not cd to ${BOXDIR}/${WORKDIR}"
# Try to run startup utilities
for onetime in 1
do
	#
	# START GENERATE
	#
	case "${NIGHTLY_SKIP_STEP}"
	in
		*GENERATE*) ;;
		*)

	if [ -d emp4 -a -d emp4/bin -a -d emp4/sbin -a -d emp4/var/empire ]
	then
		echo "Directory structure is ok"
	else
		warn "Directory structure is NOT ok"
		break
	fi

	if [ ! -f emp4/etc/empire/schedule ]
	then
		warn "schedule file is missing"
		break
	fi

	if [ ! -f emp4/etc/empire/econfig ]
	then
		warn "econfig file is missing"
		break
	fi

	if ! emp4/sbin/pconfig >emp4/etc/empire/econfig
	then
		warn "pconfig failed to create econfig file"
		break
	fi
echo "Applying global econfig patches from ${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/patches/All"
for i in "${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/patches/All"/*.econfig
do
	[ -r "${i}" ] || continue
	if "${i}" >>emp4/etc/empire/econfig
	then
		echo "${i}: OK"
	else
		echo "========== ${i}: NOT OK! ${?} =========="
	fi
done
echo "Done (econfig patch All)."
echo ""

LOCALPATCHDIRECTORY="${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/patches/${INSTANCE}"
if [ -n "${LOCALPATCHDIRECTORY}" -a -d "${LOCALPATCHDIRECTORY}/." ]
then
	echo "Applying system specific econfig patches from ${LOCALPATCHDIRECTORY}:"
	for i in "${LOCALPATCHDIRECTORY}"/*.econfig
	do
		[ -r "${i}" ] || continue
		if "${i}" >>emp4/etc/empire/econfig
		then
			echo "${i}: OK"
		else
			echo "========== ${i}: NOT OK! ${?} =========="
		fi
	done
	echo "Done (econfig patch specific)."
	echo ""
fi

	cd emp4/bin || err "Could not cd to emp4/bin"

	echo "Determining type of files in bin directory"
	file *
	echo "Done (file *)."
	echo ""

	cd ../sbin || err "Could not cd to ../sbin"

	echo "Determining type of files in sbin directory"
	file *
	echo "Done (file *)."
	echo ""

	echo "Running files and fairland"
	echo y | ./files -e ../etc/empire/econfig || warn "Error running files"
	./fairland -R 1 -e ../etc/empire/econfig 10 30 >/dev/null || { warn "Error running fairland" ; break ; }
	[ -s "newcap_script" ] || { warn "fairland did not produce newcap_script" ; break ; }
	echo "Done (files & fairland)."
	echo ""

			;;
	esac
	#
	# END GENERATE
	#

	#
	# START SERVERSTART
	#
	case "${NIGHTLY_SKIP_STEP}"
	in
		*SERVERSTART*) ;;
		*)

	echo "Removing existing server.log and journal.log"
	if [ -f "../var/empire/server.log" ]
	then
	    rm "../var/empire/server.log"
	fi
	if [ -f "../var/empire/journal.log" ]
	then
	    rm "../var/empire/journal.log"
	fi
	echo "Removing existing schedule"
	if [ -f "../etc/empire/schedule" ]
	then
	    >../etc/empire/schedule
	fi
	echo "Starting server with -d in the background"
	./emp_server -R 1 -e ../etc/empire/econfig -d 2>/dev/null &
	PID="$!"
	sleep 1
	kill -0 "${PID}" || { warn "emp_server not running ?" ; break ; }
	echo "Done (emp_server)."
	echo ""

			;;
	esac
	#
	# END SERVERSTART
	#

	#
	# START GENERATE (2nd part)
	#
	case "${NIGHTLY_SKIP_STEP}"
	in
		*GENERATE*) ;;
		*)

export EMPIRE_PATH=${BOXDIR}/${WORKDIR}/emp4/bin
	echo "Running newcap_script through empire"
	runfeed POGO peter < newcap_script >/dev/null 2>&1 ||
		{
			warn "Could not run newcap_script"
			echo "Stopping server"
			trykill $PID
			break
		}
	echo "Done (newcap_script / empire)."
	echo ""

			;;
	esac
	#
	# END GENERATE (2nd part)
	#

	#
	# START TESTSCRIPT
	#
	case "${NIGHTLY_SKIP_STEP}"
	in
		*TESTSCRIPT*) ;;
		*)

echo "Applying tests from ${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/tests"
cd ${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/tests
for i in *
do
	[ -d "$i" ] || continue
	cd  "$i"
	if [ "$i" -ne "00" ]
	then
		echo "Update Turn $i starting"
		if ../update
		then
			echo "Update Turn $i completed successfully"
		else
			echo "Update Turn $i failed"
		fi

	fi
	for j in *
	do
		[ -x "$j" ] || continue
		echo "Player $j Turn $i starting"
		if ./"$j"
		then
			echo "Player $j Turn $i completed successfully"
		else
			echo "Player $j Turn $i failed"
		fi
	done
	cd ..
done
echo "Done tests."
echo ""

			;;
	esac
	#
	# END TESTSCRIPT
	#

	#
	# START SERVERSTOP
	#
	case "${NIGHTLY_SKIP_STEP}"
	in
		*SERVERSTOP*) ;;
		*)
			case "${NIGHTLY_SKIP_STEP}"
			in
				*SERVERSTART*) ;;
				*)

	echo "Stopping server"
	trykill "${PID}"
	echo "Done (kill)."
cd "${BOXDIR}/${WORKDIR}/emp4/var/empire" || err "Could not cd to ${BOXDIR}/${WORKDIR}/emp4/var/empire"
	echo "-- Start Server Log --"
	cat server.log
	echo "-- End of Server Log --"
	echo "-- Start Journal Log --"
	cat journal.log
	echo "-- End of Journal Log --"
	echo "Server stopped"
					;;
			esac
			;;
	esac
	#
	# END SERVERSTOP
	#
done

#
# START CLEANUP
#
case "${NIGHTLY_SKIP_STEP}"
in
	*CLEANUP*) ;;
	*)

echo "Cleaning sandbox"
cd "${BOXDIR}" || err "Could not cd back to sandbox root !"
case "${NIGHTLY_SKIP_STEP}"
in
	*REMOVE_REPOSITORY*)
rm -rf `find "${WORKDIR}" -maxdepth 1 ! -name .git` || warn "Directory ${WORKDIR} could not be forcibly removed !"
		;;
	*)
rm -rf "${WORKDIR}" || warn "Directory ${WORKDIR} could not be forcibly removed !"
[ -d "${WORKDIR}/." ] && warn "Directory ${WORKDIR} still present"
		;;
esac
echo "Done (cleaning)."
		;;
esac
#
# END CLEANUP
#

echo "Nightly build finished at `date`"

exit 0
