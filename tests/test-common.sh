export EMPIREHOST=127.0.0.1 LOGNAME=tester

if [ -x ./config.status ] && ./config.status --version | grep -q '^Wolfpack Empire'
then :
else echo "$0: Must be run in root of build tree" >&2; exit 1
fi

test=${0##*/}
test=${test%-test}

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
keep_journal 1
EOF
    cp "$srcdir"/src/lib/global/*.config sandbox/share/empire/builtin
}

start_server()
{
    pid=
    trap 'if [ "$pid" ]; then kill "$pid" 2>/dev/null || true; fi' EXIT
    src/server/emp_server -e sandbox/etc/empire/econfig -R 1
    while src/client/empire red herring 2>&1 | grep -q "Connection refused"
    do :	       # FIXME hangs here if server crashes on startup
    done
    pid=`cat sandbox/var/empire/server.pid`
}

stop_server()
{
    kill "$pid"
    while kill -0 "$pid" 2>/dev/null
    do :		    # FIXME hangs here if server fails to exit
    done
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
	    echo "$exp OK"
	else
	    ret=$?
	    echo "$exp FAIL"
	    echo $msg
	fi
    done
    return $ret
}
