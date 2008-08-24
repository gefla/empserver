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
	! [ -d "${WORKDIR}"/empserver/.git ]  || err "Invalid sandbox, missing .git directory"
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
	while ! git clone $GITROOT empserver >/dev/null
	do
		sleep "`expr 5 + ${RETR}`"
		RETR="`expr 1 + ${RETR}`"
		[ "${RETR}" -gt 5 ] && err "git-clone Timeout after ${RETR} retres."
	done
	cd empserver || err "Could not cd to ${BOXDIR}/${WORKDIR}/empserver."
else
	cd empserver || err "Could not cd to ${BOXDIR}/${WORKDIR}/empserver."
	while ! git pull $GITROOT master >/dev/null
	do
		sleep "`expr 5 + ${RETR}`"
		RETR="`expr 1 + ${RETR}`"
		[ "${RETR}" -gt 5 ] && err "GIT pull Timeout after ${RETR} retres."
done

fi

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

git-pull
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
mov c 0,0 600 -1,-1
desi -1,-1 g
mov c 0,0 275 1,-1
mov c 2,0 274 1,-1
desi 1,-1 m
deliver i 2,0 230 u
deliver i 1,-1 0 j
dist 4,0 2,0
thres i 2,0 1
des 0,-2 c
capital 0,-2
des 0,0 g
thres d 0,0 1
EOF

	echo "Run an update"
	runfeed POGO peter << EOF
power new
cen * ?own#0
comm * ?own#0
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
comm * ?own#0
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
mov c -1,-1 300 4,0
mov c -1,-1 300 3,-1
mov c 1,-1 175 3,-1
deli i 2,0 0 j
deli i 1,-1 0 j
mov c 2,0 230 5,-1
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
comm * ?own#0
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
mov l 5,-1 102 6,-2
mov c 4,0 255 6,-2
desi 6,-2 l
thre l 6,-2 150
mov c 2,0 370 -2,2
deliver i 2,0 0 j
deliver i 1,-1 0 j
thres d 1,1 1
thres o -2,2 1
thres i 2,0 0
thres i 1,-1 0
desi -2,2 o
desi 1,1 g
thres c -1:5,-1 768
thres c -2:4,0 768
thres c 6,-2 250
thres c 4,-2 300
thres l 4,-2 100
thres o 4,-2 50
thres d 4,-2 10
thres d -1,-1 50
thres c 1,1 768
prod *
EOF
	echo "Turn 3 for player 8"
	runfeed 8 << EOF
cen *
map #
read y 
exp c 0,0 50 gyyygh
des -7,-3 )
lost *
EOF

	echo "Run an update"
	runfeed POGO peter << EOF
power new
report *
cen * ?own#0
comm * ?own#0
reso * ?own#0
enable
force 1
disable
EOF
	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 4 for player 1"
	runfeed 1 << EOF
cen *
ship *
map #
read y
des 4,-2 t
enlist 3,-1 50
mov m 3,-1 50 5,-1
load m 0 50
nav 0
j
j
h
assault 11,-1 0
25
prod *
EOF
	echo "Turn 4 for player 8"
	runfeed 8 << EOF
cen *
map #
read y 
lost *
EOF
	echo "Run an update"
	runfeed POGO peter << EOF
power new
report *
cen * ?own#0
comm * ?own#0
reso * ?own#0
enable
force 1
disable
EOF
	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 5 for player 1"
	runfeed 1 << EOF
real 0 -8:16,-4:4
cen *
ship *
map #
read y
prod *
EOF
	echo "Run an update"
	runfeed POGO peter << EOF
power new
report *
cen * ?own#0
comm * ?own#0
reso * ?own#0
enable
force 1
disable
EOF

	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 6 for player 1"
	runfeed 1 << EOF
real 1 -16:24,-8:8
convert 11,-1 76
thres c -2,2 769
mov c -2,2 47 0,2
des 0:2,2 g
thres d 0:2,2 1
thres c 0:2,2 769
des 3,1 g
thres d 3,1 1
thres c 3,1 769
des 0,0 m
deliver i 0,0 0 g
thres d -1,-1 75
thres d 0,0 0
mov d 0,0 1 -1,-1
thres c 0,-2 100
thres c 4:6,-2 210
thres c -1,-1 100
thres i 4,0 999
thres i 3,-1 999
des -2,0 j
thres c -2,0 769
thres l -2,0 1
bmap #1
nav 0 njnh
look 0
radar 0
radar 11,-1
cen *
ship *
map #
read y
prod *
EOF
	echo "Turn 6 for player 8"
	runfeed 8 << EOF
exp c 0,0 1 gh
exp c 0,0 1 yh
exp c 0,0 1 ygh
exp c 0,0 1 yyh
exp c 0,0 1 yyyh
exp c 0,0 1 yyyyh
exp c 0,0 1 yygh
exp c 0,0 1 yygyh
exp c 0,0 1 yygyyh
exp c 0,0 1 bh
exp c 0,0 1 bgh
exp c 0,0 1 nh
exp c 0,0 1 njh
exp c 0,0 1 yuh
exp c 0,0 1 yuyh
exp c 0,0 1 yuuh
exp c 0,0 1 yuuyh
exp c 2,0 1 jh
exp c 2,0 1 jjh
exp c 2,0 1 jjjh
exp c 2,0 1 uh
exp c 2,0 1 ujh
exp c 2,0 1 ujjh
exp c 2,0 1 uyh
exp c 2,0 1 uuh
exp c 2,0 1 uujh
cen *
map #
read y
prod *
EOF
	echo "Run an update"
	runfeed POGO peter << EOF
power new
report *
cen * ?own#0
comm * ?own#0
reso * ?own#0
enable
force 1
disable
EOF

	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 7 for player 1"
	runfeed 1 << EOF
build bridge 5,-1 j
explore c 5,-1 1 jh
mov c 5,-1 76 7,-1
thres c 7,-1 77
dist * 5,-1
des 1,1 m
deliver i 1,1 0 g
thres d 1,1 0
mov d 1,1 1 -1,-1
des -1,1 k
thres h -1,1 1
thres c -1,1 769
des -4:-2,-2 o
thres o -4:-2,-2 1
thres c -4:-2,-2 769
des 1,-3 o
thres o 1,-3 1
thres c 1,-3 769
des 4:8,2 g
thres d 4:8,2 1
thres c 4:8,2 769
thres c * ?c_dist=768 769
bmap #1
radar 11,-1
cen *
ship *
map #
read y
prod *
EOF
	echo "Turn 7 for player 8"
	runfeed 8 << EOF
des 7,-1 c
capital 7,-1
des * ?gold>1 g
thres d * ?newdes=g 1
thres d * ?des=g 1
thres d 4,0 0
thres d 1,-1 1
des -5,-3 o
thres o -5,-3 1
des -6,-4 o
thres o -6,-4 1
des 1,-1 m
thres i 1,-1 1
des 3,1 w
dist * 3,1
thres c 0:2,0 769
thres c 1,-1 769
thres c 4,0 350
thres c 3,-1 769
mov c 2,0 231 jh
mov c 0,0 231 jjh
mov c 1,-1 231 jh
bmap #
cen *
map #
read y
prod *
EOF
	echo "Run an update"
	runfeed POGO peter << EOF
power new
report *
cen * ?own#0
comm * ?own#0
reso * ?own#0
enable
force 1
disable
EOF

	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 8 for player 1"
	runfeed 1 << EOF
des 0,2 m
deliver i 0,2 0 j
thres d 0,2 0
mov d 0,2 1 -1,-1
des 3,1 m
deliver i 3,1 0 j
thres d 3,1 0
mov d 3,1 1 -1,-1
des 2,2 k
thres h 2,2 1
thres d 2,2 0
mov d 2,2 1 -1,-1
thres d 4,-2 15
thres o 4,-2 75
thres l 4,-2 150
des 5:7,1 g
thres d 5:7,1 1
thres c 5:7,1 769
des 10,2 g
thres d 10,2 1
thres c 10,2 769
radar 11,-1
bmap #1
cen *
ship *
map #
read y
prod *
EOF
	echo "Turn 8 for player 8"
	runfeed 8 << EOF
thres c -3:-1,-1 769
des 0:2,0 m
thres i 0:2,0 1
bmap #
cen *
map #
read y
prod *
EOF

	echo "Run an update"
	runfeed POGO peter << EOF
power new
report *
cen * ?own#0
comm * ?own#0
reso * ?own#0
enable
force 1
disable
EOF

	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 9 for player 1"
	runfeed 1 << EOF
mov u 2,0 10 7,-1
des -2,2 i
thres l -2,2 600
thres h -2,2 300
thres s -2,2 1
thres o -2,2 0
mov o -2,2 1 4,-2
thres c * ?des#= 769
bmap #1
cen *
ship *
map #
read y
prod *
EOF
	echo "Turn 9 for player 8"
	runfeed 8 << EOF
thres d 1,-1 0
mov d 1,-1 1 4,0
des 3,-1 m
thres i 3,-1 1
thres d 3,-1 0
mov d 3,-1 1 4,0
thres c -6,-4 769
thres c 6,-2 769
thres c 1,-3 769
des 4,0 b
thres d 4,0 100
thres d 0:2,0 0
mov d 0,0 1 4,0
mov d 2,0 1 4,0
bmap #
cen *
map #
read y
prod *
EOF

	echo "Run an update"
	runfeed POGO peter << EOF
power new
report *
cen * ?own#0
comm * ?own#0
reso * ?own#0
enable
force 1
disable
EOF

	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 10 for player 1"
	runfeed 1 << EOF
mov h 5,-1 100 7,-1
build bridge 7,-1 j
explore c 7,-1 1 jh
thres c 9,-1 77
mov u 7,-1 12 4,0
thres u 2,0 579
thres u 4,0 869
thres u 11,-1 1
thres l 6,-2 250
des 7,1 d
thres l 7,1 200
thres h 7,1 100
thres o 7,1 20
thres g 7,1 1
thres d 7,1 0
mov d 7,1 1 -1,-1
des 5,1 j
thres l 5,1 1
thres d 5,1 0
mov d 5,1 1 -1,-1
des 4,2 r
thres l 4,2 100
thres o 4,2 50
thres d 4,2 10
des 6,2 w
thres d 6,2 0
mov d 6,2 1 -1,-1
des 8,2 e
thres d 8,2 0
mov d 8,2 1 -1,-1
thres m 8,2 1
des 10,2 !
thres l 10,2 200
thres h 10,2 200
thres s 10,2 200
thres g 10,2 25
thres d 10,2 0
mov d 10,2 1 -1,-1
des 5,3 p
thres l 5,3 75
dist #1 5,-1
spy 11,-1
bmap #1
cen *
ship *
map #
read y
prod *
EOF
	echo "Turn 10 for player 8"
	runfeed 8 << EOF
des -3,-1 j
thres i -3,-1 999
thres l -3,-1 1
thres c 6,0 769
thres d -3,-1 0
mov d -3,-1 1 4,0
bmap #
cen *
map #
read y
prod *
EOF
	echo "Run an update"
	runfeed POGO peter << EOF
power new
report *
cen * ?own#0
comm * ?own#0
reso * ?own#0
enable
force 1
disable
EOF

	echo "Done (force)."
	echo ""
	sleep 10

	echo "Turn 11 for player 1"
	runfeed 1 << EOF
thres l 6,-2 300
thres l 5,-1 200
thres h 5,-1 200
thres i 5,-1 1
thres o 5,-1 1
thres d 5,-1 1
dist #1 6,2
bmap #1
cen *
ship *
map #
read y
prod *
EOF
	echo "Turn 11 for player 8"
	runfeed 8 << EOF
des -1,-1 m
thres i -1,-1 1
thres d -1,-1 0
mov d -1,-1 1 4,0
bmap #
cen *
map #
read y
prod *
EOF

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
rm -r "${WORKDIR}" || warn "Directory ${WORKDIR} could not be cleanly removed !"
rm -rf "${WORKDIR}" || warn "Directory ${WORKDIR} could not be forcibly removed !"
[ -d "${WORKDIR}/." ] && warn "Directory ${WORKDIR} still present"
echo "Done (cleaning)."
		;;
esac
		;;
esac
#
# END CLEANUP
#

echo "Nightly build finished at `date`"

exit 0
