#
# Common functions
#
# Blame it on marcolz
#
err() {
	echo "ERROR: $@" >&2
	exit 1
}

warn() {
	echo "WARNING: $@" >&2
}

trykill() {
	[ -n "$1" ] || { warn "INTERNAL ERROR: trykill: no argument ?" ; return 2 ; }
	kill -TERM "$1" || { warn "Could not kill pid '${1}'" ; return 1 ; }
	sleep 1
	/bin/kill -KILL "$1" 2>/dev/null && { warn "Process ${1} would not die" ; }
	sleep 1
	/bin/kill -KILL "$1" 2>/dev/null && { warn "Process ${1} would not die after -KILL" ; return 1 ; }

	return 0
}

runfeed() {
	[ -n "$1" ] || { warn "INTERNAL ERROR: No coun/repr given ?" ; return 2 ; }
	REP="$2"
	[ -n "$REP" ] || REP="$1"
	{
		cat
		echo "quit"
	} | ./emp_client "$1" "$REP" || { warn "emp_client not ok ?" ; return 1 ; }
	return 0
}
