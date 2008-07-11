#!/bin/sh
#
# dbxmld.sh - startup script for dbxmld on FreeBSD
#
# This goes in /usr/local/etc/rc.d and gets run at boot-time.

case "$1" in

    start)
    if [ -x /usr/local/sbin/dbxmld_wrapper ] ; then
	echo -n " dbxmld"
	/usr/local/sbin/dbxmld_wrapper &
    fi
    ;;

    stop)
    kill -USR1 `cat /var/run/dbxmld.pid`
    ;;

    *)
    echo "usage: $0 { start | stop }" >&2
    exit 1
    ;;

esac
