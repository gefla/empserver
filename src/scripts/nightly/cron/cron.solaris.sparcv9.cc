#!/bin/sh
cd /home/marcolz/src/empire/nightlybuild/
NIGHTLY_SKIP_STEP=TESTSCRIPT ksh ./nightlybuild.sh conf/solaris.sparcv9.cc.config
ksh ./report.sh conf/solaris.sparcv9.cc.config | mutt -s 'empserver solaris.sparcv9.cc' empserver-devel@lists.sourceforge.net
