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
