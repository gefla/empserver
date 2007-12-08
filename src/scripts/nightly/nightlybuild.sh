#!/bin/sh
#
# Blame it on marcolz
#
# Skip certain parts of this script by exporting the variable
# "NIGHTLY_SKIP_STEP" containing the following possible substrings,
# preventing the named behaviour:
#
# REDIRECT	-	Redirect all output to a logfile
# CHECKOUT	-	Fill the sandbox with a fresh cvs checkout
# PATCH		-	Apply the patches for this system (global + specific)
# BUILD		-	Build everything
# GENERATE	-	Generate a new world
# SERVERSTART	-	Start the server
# TESTSCRIPT	-	Run the testscript
# SERVERSTOP	-	Stop the server if it was started by this script
# CLEANUP	-	Remove the contents of the sandbox
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
mkdir "${WORKDIR}" || warn "Could not create ${BOXDIR}/${WORKDIR}"
cd "${WORKDIR}" || err "Could not cd to ${BOXDIR}/${WORKDIR}"

echo "Getting source from CVS:"
# Extract source
export CVS_RSH=${CVS_RSH:=ssh}
export CVSROOT=${CVSROOT:=:pserver:anonymous@empserver.cvs.sourceforge.net:/cvsroot/empserver}
RETR=0
while ! cvs -z3 co empserver >/dev/null
do
	sleep "`expr 5 + ${RETR}`"
	RETR="`expr 1 + ${RETR}`"
	[ "${RETR}" -gt 5 ] && err "CVS Timeout after ${RETR} retres."
done
echo "Done (CVS)."
echo ""

		;;
esac
#
# END CHECKOUT
#

#
# START PATCH
#
case "${NIGHTLY_SKIP_STEP}"
in
	*PATCH*) ;;
	*)

echo "Applying global patches from ${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/patches/All"
for i in "${BOXDIR}/${WORKDIR}/empserver/src/scripts/nightly/patches/All"/*.patch
do
	[ -r "${i}" ] || continue
	if ${PATCH:=patch} -Np0 < "${i}" >/dev/null
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
		if ${PATCH:=patch} -Np0 < "${i}" >/dev/null
		then
			echo "${i}: OK"
		else
			echo "========== ${i}: NOT OK! ${?} =========="
		fi
	done
	echo "Done (patch specific)."
	echo ""
fi

cd empserver || err "Could not cd to ${BOXDIR}/${WORKDIR}/empserver."

cvs up -d
sh ./bootstrap
./configure --prefix ${BOXDIR}/${WORKDIR}/emp4

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
if make install >/dev/null
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

	if [ -d ../emp4 -a -d ../emp4/bin -a -d ../emp4/sbin -a -d ../emp4/var/empire ]
	then
		echo "Directory structure is ok"
	else
		warn "Directory structure is NOT ok"
		break
	fi

	cd ../emp4/bin || err "Could not cd to ../emp4/bin"

	echo "Determining type of files in bin directory"
	file *
	echo "Done (file *)."
	echo ""

	cd ../../emp4/sbin || err "Could not cd to ../../emp4/sbin"

	echo "Determining type of files in sbin directory"
	file *
	echo "Done (file *)."
	echo ""

	echo "Running files and fairland"
	echo y | ./files || warn "Error running files"
	./fairland -R 1 10 30 >/dev/null || { warn "Error running fairland" ; break ; }
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

	echo "Starting server with -d in the background"
	./emp_server -R 1 -d &
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

	ln -s ../bin/empire emp_client
	echo "Running newcap_script through emp_client"
	runfeed POGO peter < newcap_script >/dev/null 2>&1 ||
		{
			warn "Could not run newcap_script"
			echo "Stopping server"
			trykill $PID
			break
		}
	echo "Done (newcap_script / emp_client)."
	echo ""
	
	echo "TODO: Replace this with a real test script."
	echo "Just do some rudimentary testing for now."
	echo ""

	echo "Prevent updates from happening without our consent."
	runfeed POGO peter << EOF
disable
EOF
	echo "Done (update stop)."
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

	for PLAYER in 2 3 4 5 6 7 8 9 10
	do
		echo "explore for player ${PLAYER}"
		runfeed $PLAYER << EOF >/dev/null 2>&1
break
expl c 0,0 1 uh
desi 1,-1 +
mov c 0,0 205 uh
desi 1,-1 g
cen *
EOF
		echo "Done (explore)."
		echo ""
	done

	# Something more elaborate for player 1
	echo "explore and more for player 1"
	runfeed 1 << EOF >/dev/null 2>&1
break
expl c 0,0 1 uh
expl c 2,0 1 jh
expl c 2,0 1 uh
expl c 2,0 1 nh
expl c 2,0 1 bh
expl c 0,0 1 yh
expl c 0,0 1 gh
expl c 0,0 1 bh
desi * ?ne=- +
expl c 2,0 1 njh
expl c 2,0 1 nnh
expl c 2,0 1 bnh
expl c 0,0 1 bbh
expl c 0,0 1 yyh
expl c 0,0 1 yuh
expl c 0,0 1 bnh
expl c 2,0 1 yuh
expl c 2,0 1 uuh
expl c 2,0 1 juh
desi * ?ne=- +
expl c 2,0 1 nnnh
expl c 2,0 1 nnjh
expl c 2,0 1 njjh
expl c 2,0 1 uujh
expl c 0,0 1 bbnh
expl c 0,0 1 bnnh
expl c 0,0 1 yygh
expl c 0,0 1 yuuh
desi * ?ne=- +
expl c 2,0 1 nnjjh
desi * ?ne=- +
expl c 2,0 1 nnjjjh
desi * ?ne=- +
mov u 0,0 75 jh
demob 0:2,0 55 y
desi 2,0 m
mov c 0,0 767 -1,-1
desi -1,-1 g
mov c 0,0 275 1,-1
mov c 2,0 274 1,-1
desi 1,-1 m
deliver i 2,0 230 u
deliver i 1,-1 0 j
dist 4,0 2,0
thres i 2,0 1
EOF

	echo "Run an update"
	runfeed POGO peter << EOF
power new
cen * ?own#0
reso * ?own#0
enable
force 1
disable
EOF
	echo "Done (force)."
	echo ""

	sleep 10
	echo "Check player 1"
	runfeed 1 << EOF
real 0 -8:12,-4:4
cen *
map #
read y
EOF
	echo "Done (check)."
	echo ""

	echo "Check whether the update did anything"
	runfeed POGO peter << EOF
power new
cen * ?own#0
reso * ?own#0
read
y
EOF
	echo "Done (check update)."
	echo ""

	echo "Continue some updates for player 1"
	echo ""

	echo "Turn 2 for player 1"

	runfeed 1 << EOF >/dev/null 2>&1
desi -1,-1 b
mov i 2,0 200 4,0
mov i 1,-1 4 jh
mov c -1,-1 435 4,0
deli i 2,0 0 j
deli i 1,-1 0 j
mov c -1,-1 80 3,-1
mov c 1,-1 256 4,0
mov c 2,0 230 5,-1
mov c 1,-1 409 3,-1
desi 4,0 k
desi 3,-1 j
dist # 5,-1
thre h 4,0 1
thre l 3,-1 1
desi 5,-1 h
EOF

	echo "Run an update"
	runfeed POGO peter << EOF
power new
cen * ?own#0
reso * ?own#0
enable
force 1
disable
EOF
	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 3 for player 1"
	runfeed 1 << EOF
cen *
map #
read y
build sh 5,-1 frg
mov l 5,-1 134 6,-2
mov c 4,0 377 6,-2
desi 6,-2 l
thre l 6,-2 150
mov c -1,-1 600 1,1
mov c 2,0 370 -2,2
deliver i 2,0 0 j
deliver i 1,-1 0 j
thres d 1,1 1
thres o -2,0 1
thres i 2,0 0
thres i 1,-1 0
desi -2,2 o
desi 1,1 g
prod *
EOF

	echo "Run an update"
	runfeed POGO peter << EOF
power new
cen * ?own#0
reso * ?own#0
enable
force 1
disable
EOF
	echo "Done (force)."
	echo ""
	sleep 10

	echo "Done (player 1)."
	echo ""
	echo "TODO: turn 4/5 (tech/assault)..."

	echo "Done (Rudimentary tests)."
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
	echo ""
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
rm -r "${WORKDIR}" || warn "Directory ${WORKDIR} could not be cleanly removed !"
rm -rf "${WORKDIR}" || warn "Directory ${WORKDIR} could not be forcibly removed !"
[ -d "${WORKDIR}/." ] && warn "Directory ${WORKDIR} still present"
echo "Done (cleaning)."

		;;
esac
#
# END CLEANUP
#

echo "Nightly build finished at `date`"

exit 0
