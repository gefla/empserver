#!/bin/sh
#
# Blame it on marcolz
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

# Log everything
exec > "${LOGFILE}"
exec 2>&1

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
	[ "${RETR}" -gt 100 ] && err "CVS Timeout after ${RETR} retres."
done
echo "Done (CVS)."
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
	echo "Applying local patches from ${LOCALPATCHDIR}:"
	for i in "${LOCALPATCHDIR}"/*.patch
	do
		if patch -Np0 < "${i}" >/dev/null
		then
			echo "${i}: OK"
		else
			echo "========== ${i}: NOT OK! ${?} =========="
		fi
	done
	echo "Done (patch)."
	echo ""
fi

cd empserver || err "Could not cd to ${BOXDIR}/${WORKDIR}/empserver."

# Prep build.conf
echo "Preparing build.conf"
sed	-e "s,^USERNAME = .*$,USERNAME = ${EMPLOGIN}," \
	-e "s,^HOSTNAME = .*$,HOSTNAME = localhost," \
	-e "s,^IPADDR = .*$,IPADDR = 127.0.0.1," \
	-e "s,^EMPDIR = .*$,EMPDIR = ${BOXDIR}/${WORKDIR}/emp4," \
	< build.conf > build.conf.new && \
	mv build.conf.new build.conf || \
	err "Could not prep build.conf"
echo "Done (build.conf)."
echo ""

# TODO: this should be fixed another way...
echo "Generating empty Makedepends."
touch src/client/Makedepend src/doconfig/Makedepend src/lib/as/Makedepend src/lib/commands/Makedepend src/lib/common/Makedepend src/lib/empthread/Makedepend src/lib/gen/Makedepend src/lib/global/Makedepend src/lib/lwp/Makedepend src/lib/player/Makedepend src/lib/subs/Makedepend src/lib/update/Makedepend src/server/Makedepend src/util/Makedepend || err "Could tot touch Makedepends"
echo "Done (touch)."
echo ""

# Start the build
echo "Building server"
if make "${EMPTARGET}" >/dev/null
then
	warn "make did not return 0."
fi
echo "Done (make)."
echo ""

# Try to run startup utilities
if [ -d ../emp4 -a -d ../emp4/bin -a -d ../emp4/data ]
then
	cd ../emp4/bin || err "Could not cd to ../emp4/bin"
	echo "Determining type of files in bindir"
	file *
	echo "Done (file *)."
	echo ""
	echo "Running files and fairland"
	echo y | ./files || warn "Error running files"
	./fairland 10 30 >/dev/null || warn "Error running fairland"
	echo "Done (files & fairland)."
	echo ""
fi

# Clean sandbox
echo "Cleaning sandbox"
cd "${BOXDIR}" || err "Could not cd back to sanbox root !"
rm -r "${WORKDIR}" || warn "Directory ${WORKDIR} could not be cleanly removed !"
rm -rf "${WORKDIR}" || warn "Directory ${WORKDIR} could not be forcibly removed !"
[ -d "${WORKDIR}/." ] && warn "Directory ${WORKDIR} still present"
echo "Done (cleaning)."

echo "Nightly build finished at `date`"

exit 0
