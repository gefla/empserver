export EMPIREHOST=127.0.0.1 LOGNAME=tester

if [ -x ./config.status ] && ./config.status --version | grep -q '^Wolfpack Empire'
then :
else echo "$0: Must be run in root of build tree" >&2; exit 1
fi

test=${0##*/}
test=${test%-test}

failed=
trap 'if [ "$pid" ]; then kill -9 "$pid" 2>/dev/null || true; fi; [ "$failed" ] && exit 1' EXIT

# Abbreviations
testdir="$srcdir/tests/$test"
econfig=sandbox/etc/empire/econfig
schedule=sandbox/etc/empire/schedule
data=sandbox/var/empire
empdump="src/util/empdump -e $econfig"
files="src/util/files -e $econfig"
fairland="src/util/fairland -e $econfig"

# GNU libc memory allocation checking, see mallopt(3)
# $RANDOM isn't portable
malloc_perturb=${EMPIRE_CHECK_MALLOC_PERTURB:-$(perl -e 'print int(rand(255))')}
env="MALLOC_CHECK_=3 MALLOC_PERTURB_=$malloc_perturb"

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
    mkdir -p sandbox/etc/empire sandbox/share/empire/builtin $data
    echo $malloc_perturb >sandbox/malloc-perturb
    touch $schedule
    cat >$econfig <<EOF
data "../../var/empire"
info "../../../../info.nr"
builtin "../../share/empire/builtin"
listen_addr "$EMPIREHOST"
keep_journal 2
GODNEWS 0
running_test_suite 1
EOF
    cp `perl "$srcdir"/src/scripts/ls-sources.pl "$srcdir"/src/lib/global '\.config$'` sandbox/share/empire/builtin
}

copy_tables()
{
    local t

    for t
    do [ -e sandbox/etc/empire/"$t".config ] || cp "$srcdir"/src/lib/global/$t.config sandbox/etc/empire
    done
}

# sed -i isn't portable...
sed_i()
{
    local script="$1"
    shift
    for i
    do sed "$script" "$i" >sandbox/$$ && mv sandbox/$$ "$i"
    done
}

customize()
{
    local key
    for key
    do
	case $key in
	big-city)
	    copy_tables sect
	    sed_i '/"c" .* "capital"/d;/^#.*"c" .* "city"/s/^#/ /;/"c" .* norm/d;/^#.*"c" .* cana/s/^#/ /' sandbox/etc/empire/sect.config
	    ;;
	trade-ship)
	    copy_tables ship
	    sed_i '/"ts   trade ship"//^#/ /' sandbox/etc/empire/ship.config
	esac
    done
    echo "custom_tables \"`cd sandbox/etc/empire && echo *.config`\"" >>$econfig
}

run_and_cmp()
{
    run "$@"
    cmp_run "$1"
}

run()
{
    local name=$1 ret
    shift
    set +e
    env $env "$@" >>sandbox/$name.out 2>>sandbox/$name.err
    ret=$?
    set -e
    echo $ret >>sandbox/$name.status
}

now()
{
    # date +%s isn't portable...
    perl -e 'printf "%s\n", time'
}

start_server()
{
    local pidfile=$data/server.pid
    local timeout

    #
    # Currently expected to work only with thread package LWP,
    # because:
    #
    # - Thread scheduling is reliably deterministic only with LWP
    # - Shell builtin kill appears not to do the job in MinGW
    # - The Windows server tries to run as service when -d isn't
    #   specified
    # - The Windows server does not implement -E crash-dump
    #
    # TODO address these shortcomings.
    #
    check_empthread

    pid=
    env $env src/server/emp_server -e $econfig -R 1 -s -E crash-dump
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
    local i

    for i
    do
	c="${i##*/*-}"
	r=`echo $c | sed 's/^POGO$/peter/'`
	feed_input "$c" "$r" "$i"
    done
}

feed_dir()
{
    feed_files `perl "$srcdir"/src/scripts/ls-sources.pl "$@" '/[0-9][0-9]-[^/]*$'`
}

begin_test()
{
    $files -f >/dev/null
    local xd=
    case "$1" in
    *.xdump)
	xd="$1"
	$empdump -i "$xd"
	shift
	;;
    esac
    if [ -z "$xd" ] || [ "$#" -ne 0 ]
    then
	cp -r $data/tel $data/empty.tel
	start_server
	if [ "$#" -eq 0 ]
	then feed_input POGO peter
	else feed_files "$@"
	fi
	echo 'edit c * ?tgms>0 t 0' | feed_input POGO peter
	stop_server
	mv $data/tel $data/setup.tel
	mv $data/empty.tel $data/tel
	mv $data/news $data/setup.news
	>$data/news
	mv $data/lostitems $data/setup.lostitems
	>$data/lostitems
	mv $data/journal.log $data/setup.journal.log
	normalize sandbox/var/empire/setup.journal.log
	mv $data/server.log $data/setup.server.log
	normalize sandbox/var/empire/setup.server.log
    fi
    start_server
}

end_test ()
{
    stop_server
    $empdump -x >sandbox/final.xdump
}

cmp_run()
{
    local i j exp

    for i
    do
	for j in status out err
	do cmp_out "$i.$j"
	done
    done
}

cmp_logs_xdump()
{
    cmp_out var/empire/server.log var/empire/journal.log final.xdump
}

cmp_out()
{
    local i

    for i
    do cmp_out1 "$i"
    done
}

normalize()
{
    local act="$1"
    local nrm="${2-sandbox/normalized-${1##*/}}"

    case "$act" in
    *journal.log)
	perl "$srcdir"/tests/normalize.pl -j "$act" ;;
    *server.log)
	perl "$srcdir"/tests/normalize.pl -s "$act" ;;
    *.xdump)
	perl "$srcdir"/tests/normalize.pl "$act" ;;
    *.err)
	perl -pe 's/\s+$/\n/;' -e "s,\Q$srcdir/tests\E,tests," "$act" ;;
    *)
	perl -pe 's/\s+$/\n/;' "$act" ;;
    esac >"$nrm"
}

cmp_out1()
{
    local i=$1 master="${2-$testdir/${1##*/}}"
    local exp="$master"
    local act="sandbox/$i"
    local nrm="sandbox/normalized-${i##*/}"

    if [ ! -e "$exp" ]
    then
	case "$i" in
	*.status)
	    exp=sandbox/ok.status
	    echo 0 >sandbox/ok.status
	    ;;
	*)
	    [ ! -e "$act" ] && return
	    exp=/dev/null
	    ;;
	esac
    fi

    normalize "$act" "$nrm"
    if diff -u "$exp" "$nrm" >"$nrm.diff"
    then
	echo "$i OK"
    elif [ "$EMPIRE_CHECK_ACCEPT" ]
    then
	echo "$i CHANGED"
	cp "$nrm" "$master"
    else
	failed=y
	echo "$i FAIL"
    fi
}
