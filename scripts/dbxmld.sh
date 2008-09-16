#!/bin/sh
#
# xdbms.sh - startup script for xdbms on FreeBSD
#
# This goes in /usr/local/etc/rc.d and gets run at boot-time.

case "$1" in

    start)
    if [ -x /usr/local/sbin/xdbms_wrapper ] ; then
	echo -n " xdbms"
	/usr/local/sbin/xdbms_wrapper &
    fi
    ;;

    stop)
    kill -USR1 `cat /var/run/xdbms.pid`
    ;;

    *)
    echo "usage: $0 { start | stop }" >&2
    exit 1
    ;;

esac
