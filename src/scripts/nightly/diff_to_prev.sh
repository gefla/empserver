#!/bin/sh
#
# Check differences between the last two logs
#
# Blame it on marcolz
#

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

diff -u $(ls "${EMPTARGET}.${ARCH}."* | tail -2)
