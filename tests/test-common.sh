export EMPIREHOST=127.0.0.1 LOGNAME=tester

if [ -x ./config.status ] && ./config.status --version | grep -q '^Wolfpack Empire'
then :
else echo "$0: Must be run in root of build tree" >&2; exit 1
fi

test=${0##*/}
test=${test%-test}

empthread=`sed -n 's/empthread *:= *\(.*\)/\1/p' <GNUmakefile`
warn_empthread=y

check_empthread()
{
    if [ "$warn_empthread" ] && [ "$empthread" != "LWP" ]
    then
	echo "Warning: test not expected to work with thread package $empthread!" >&2
	warn_empthread=
    fi
}

create_sandbox()
{
    rm -rf sandbox
    mkdir -p sandbox/etc/empire sandbox/share/empire/builtin sandbox/var/empire
    touch sandbox/etc/empire/schedule
    cat >sandbox/etc/empire/econfig <<EOF
data "../../var/empire"
info "../../../../info.nr"
builtin "../../share/empire/builtin"
listen_addr "$EMPIREHOST"
keep_journal 2
GODNEWS 0
EOF
    cp `git ls-files "$srcdir"/src/lib/global | grep '\.config$'` sandbox/share/empire/builtin
}

now()
{
    # date +%s isn't portable...
    perl -e 'printf "%s\n", time'
}

start_server()
{
    local pidfile=sandbox/var/empire/server.pid
    local timeout

    #
    # Currently expected to work only with thread package LWP,
    # because:
    #
    # - Thread scheduling is reliably deterministic only with LWP
    # - Shell builtin kill appears not to do the job in MinGW
    # - The Windows server tries to run as service when -d isn't
    #   specified
    #
    # TODO address these shortcomings.
    #
    check_empthread

    pid=
    trap 'if [ "$pid" ]; then kill -9 "$pid" 2>/dev/null || true; fi' EXIT
    src/server/emp_server -e sandbox/etc/empire/econfig -R 1 -s
    timeout=$((`now`+5))
    until pid=`cat $pidfile 2>/dev/null` && [ -n "$pid" ]
    do
	if [ `now` -gt $timeout ]
	then
	    echo "Timed out waiting for server to create $pidfile" >&2
	    exit 1
	fi
    done
    while src/client/empire red herring 2>&1 | grep -q "Connection refused"
    do
	if [ `now` -gt $timeout ]
	then
	    echo "Timed out waiting for server to accept connections" >&2
	    exit 1
	fi
    done
}

stop_server()
{
    local timeout
    kill "$pid"
    timeout=$((`now`+5))
    while kill -0 "$pid" 2>/dev/null
    do
	if [ `now` -gt $timeout ]
	then
	    echo "Timed out waiting for server to terminate" >&2
	    exit 1
	fi
    done
}

feed_input()
{
    c=$1
    r=$2
    shift 2
    sed '/^|/d' "$@" | src/client/empire "$c" "$r" >/dev/null
}

feed_files()
{
    for i
    do
	c="${i##*/*-}"
	r=`echo $c | sed 's/^POGO$/peter/'`
	feed_input "$c" "$r" "$i"
    done
}

feed_dir()
{
    feed_files `git ls-files "$@" | grep '/[0-9][0-9]-[^/]*$'`
}

begin_test()
{
    src/util/files -e sandbox/etc/empire/econfig -f >/dev/null
    local xd=
    case "$1" in
    *.xdump)
	xd="$1"
	src/util/empdump -e sandbox/etc/empire/econfig -i "$xd"
	shift
	;;
    esac
    if [ -z "$xd" ] || [ "$#" -ne 0 ]
    then
	cp -r sandbox/var/empire/tel sandbox/var/empire/empty.tel
	start_server
	feed_input POGO peter "$@"
	stop_server
	mv sandbox/var/empire/tel sandbox/var/empire/init.tel
	mv sandbox/var/empire/empty.tel sandbox/var/empire/tel
	mv sandbox/var/empire/journal.log sandbox/var/empire/init.journal.log
	mv sandbox/var/empire/server.log sandbox/var/empire/init.server.log
    fi
    start_server
}

end_test ()
{
    stop_server
    src/util/empdump -e sandbox/etc/empire/econfig -x >sandbox/final.xdump
}

cmp_out()
{
    local opt exp act nrm msg ret=0
    for i
    do
	case "$i" in
	*/journal.log)	opt=-j ;;
	*/server.log)	opt=-s ;;
	*)		opt= ;;
	esac
	exp="$srcdir/tests/$test/${i##*/}"
	act="sandbox/$i"
	nrm="sandbox/normalized-${i##*/}"
	perl "$srcdir"/tests/normalize.pl $opt "$act" >"$nrm"
	if msg=`diff -q "$exp" "$nrm"`
	then
	    echo "$i OK"
	else
	    ret=$?
	    echo "$i FAIL"
	    echo $msg
	fi
    done
    return $ret
}
