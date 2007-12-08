#!/bin/sh
#
# Generate report from the last two build logs
#
# Blame it on marcolz
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

cd "${LOGDIR}" || err "Could not cd to ${LOGDIR}"

BUILDTYPE="${INSTANCE}"
[ -n "${EXTRASUFFIX}" ] && BUILDTYPE="${BUILDTYPE}.${EXTRASUFFIX}"

echo "This was generated using: ${0} ${1}"
echo ""
echo "Sections: <diff> <full>"
echo ""
echo "Environment:"
echo "uname -a: $(uname -a)"
echo "gcc -v: $(gcc -v 2>&1)"
echo ""
echo "========================================="
echo "===== Differences since last build: ====="
echo "========================================="
echo ""
diff -u `ls "${BUILDTYPE}."* | tail -2`
echo ""

echo "========================================="
echo "================ Full log: =============="
echo "========================================="
echo ""
cat `ls "${BUILDTYPE}."* | tail -1`
