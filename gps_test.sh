#!/bin/bash
cd

start() {
    ./gas_kill.sh
    export DISPLAY=:0.0
    gnome-terminal -- bash -c "./gps_test -n"
}

stop() {
    pkill gnome-terminal
}

case "$1" in
    'start')
            start
            ;;
    'stop')
            stop
            ;;
    'restart')
            stop
            sleep 1
            start
            ;;
    *)
            echo
            echo "Usage: $0 { start | stop | restart }"
            echo
            exit 1
            ;;
esac

exit 0