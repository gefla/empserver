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

# For some reason, solaris sh exits as soon as both stderr and stdout
# are redirected to file at the exec, so if we run on solaris, use ksh
# for this script.
#
INTERPRETER="$_"

case "$INTERPRETER"
in
	*/ksh|ksh)
		;;
	*)
		if  [ "`uname`" = "SunOS" ]
		then
			exec ksh "$0" "$@"
		fi
	;;
esac

PROGNAME="$0"
CVS_RSH=ssh
export CVS_RSH
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
WORKDIR="${EMPTARGET}.${ARCH}"
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
RETR=0
while ! cvs -z3 -d:ext:"${SFLOGIN}"@cvs.sourceforge.net:/cvsroot/empserver co empserver >/dev/null
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

echo "Applying global patches from patches/All:"
for i in "${SCRIPTDIR}/patches/All"/*.patch
do
	[ -r "${i}" ] || continue
	if patch -Np0 < "${i}" >/dev/null
	then
		echo "${i}: OK"
	else
		echo "========== ${i}: NOT OK! ${?} =========="
	fi
done
echo "Done (patch All)."
echo ""

# Run local patches ${LOCALPATCHDIR}/*.patch
case "${LOCALPATCHDIR}"
in
	/*)
		;;
	*)
		LOCALPATCHDIR="${SCRIPTDIR}/${LOCALPATCHDIR}"
		;;
esac

if [ -n "${LOCALPATCHDIR}" -a -d "${LOCALPATCHDIR}/." ]
then
	echo "Applying system specific patches from ${LOCALPATCHDIR}:"
	for i in "${LOCALPATCHDIR}"/*.patch
	do
		[ -r "${i}" ] || continue
		if patch -Np0 < "${i}" >/dev/null
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

# Prep build.conf
echo "Preparing build.conf"
sed	-e "s,^USERNAME = .*$,USERNAME = ${EMPLOGIN}," \
	-e "s,^HOSTNAME = .*$,HOSTNAME = localhost," \
	-e "s,^IPADDR = .*$,IPADDR = 127.0.0.1," \
	-e "s,^PORTNUM = .*$,PORTNUM = ${EMPPORT}," \
	-e "s,^EMPDIR = .*$,EMPDIR = ${BOXDIR}/${WORKDIR}/emp4," \
	< build.conf > build.conf.new && \
	mv build.conf.new build.conf || \
	err "Could not prep build.conf"
echo "Done (build.conf)."
echo ""

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

# TODO: this should be fixed another way...
echo "Generating empty Makedepends."
touch src/client/Makedepend src/doconfig/Makedepend src/lib/as/Makedepend src/lib/commands/Makedepend src/lib/common/Makedepend src/lib/empthread/Makedepend src/lib/gen/Makedepend src/lib/global/Makedepend src/lib/lwp/Makedepend src/lib/player/Makedepend src/lib/subs/Makedepend src/lib/update/Makedepend src/server/Makedepend src/util/Makedepend || err "Could tot touch Makedepends"
echo "Done (touch)."
echo ""

# Start the build
echo "Building server"
if make "${EMPTARGET}" >/dev/null
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

	if [ -d ../emp4 -a -d ../emp4/bin -a -d ../emp4/data ]
	then
		echo "Directory structure is ok"
	else
		warn "Directory structure is NOT ok"
		break
	fi

	cd ../emp4/bin || err "Could not cd to ../emp4/bin"

	echo "Determining type of files in bindir"
	file *
	echo "Done (file *)."
	echo ""

	echo "Running files and fairland"
	echo y | ./files || warn "Error running files"
	./fairland -R 0 10 30 >/dev/null || { warn "Error running fairland" ; break ; }
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
	./emp_server -d &
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
expl c 2,0 1 bnh
expl c 2,0 1 bbh
expl c 0,0 1 bgh
expl c 0,0 1 ggh
expl c 0,0 1 gyh
expl c 0,0 1 yyh
expl c 2,0 1 yyh
expl c 2,0 1 uyh
expl c 2,0 1 uuh
desi * ?ne=- +
expl c 2,0 1 bbnh
expl c 2,0 1 uuuh
expl c 2,0 1 yyyh
expl c 0,0 1 yyyh
expl c 0,0 1 yygh
expl c 0,0 1 yggh
expl c 0,0 1 gggh
expl c 0,0 1 ggbh
desi * ?ne=- +
expl c 0,0 1 ggggh
expl c 0,0 1 gggbh
expl c 0,0 1 ggbbh
desi * ?ne=- +
mov u 0,0 75 jh
demob 0:2,0 55 y
desi 2,0 m
mov c 0,0 767 -3,-1
desi -3,-1 g
mov c 0,0 270 1,-1
mov c 2,0 274 1,-1
desi 1,-1 m
deliver i 2,0 120 u
dist 2,-2 2,0
thre i 2,-2 1000
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
real 0 -12:10,-5:5
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
desi -3,-1 b
mov i 1,-1 120 2,-2
mov i 1,-1 4 jh
mov c -3,-1 435 2,-2
deli i 2,0 0 u
deli i 1,-1 0 u
mov c -3,-1 80 3,-1
mov c 1,-1 256 4,-2
mov c 2,0 230 3,-1
mov c 1,-1 409 3,-1
desi 2,-2 k
desi 3,-1 j
dist # 4,-2
thre h 2,-2 1
thre l 3,-1 1
desi 4,-2 h
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
build sh 4,-2 frg
mov l 3,-1 1 -8,0
mov l 4,-2 193 -8,0
mov i 1,-1 1 2,-2
mov c 2,-2 377 -8,0
desi -8,0 l
thre l -8,0 150
mov c -3,-1 627 -2,0
mov c 3,-1 139 -2,0
mov c 4,-2 292 uh
mov c 2,-2 36 -7,1
mov c 3,-1 29 -7,1
mov c 2,0 191 -7,1
mov c 2,0 125 5,-3
mov u 2,0 99 yh
deliver o 5,-3 0 b
deliver d -2,0 0 y
desi -7,1 h
desi 5,-3 o
desi -2,0 g
budg h 1
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
cd "${BOXDIR}" || err "Could not cd back to sanbox root !"
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
