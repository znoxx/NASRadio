#!/bin/sh
export LANG=ru_RU.UTF-8
export LC_ALL='ru_RU.UTF-8'
export PATH=$PATH:.
HOME=/root
/etc/init.d/mDNSResponder start
/etc/init.d/mpd start
mpc load radio.pls
echo "AVR Start!" >/dev/ttyS0
sleep 2
mpc play
trap 'exit 1' SIGINT    # exit on ctrl-c, useful for debugging
while true        # loop forever
 do
 echo "currentsong" | nc localhost 6600 | grep -e "^Name: " >/dev/ttyS0 
 echo "currentsong" | nc localhost 6600 | grep -e "^Title: " >/dev/ttyS0
 sleep 1
done
 