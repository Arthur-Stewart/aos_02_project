#!/bin/bash

netid=acs170004

# Root directory of your project
PROJDIR=02_Project

CONFIGLOCAL=$HOME/Computer_Science/Courses/UTD/Advanced_Operating_Systems/Homework/$PROJDIR/Config_Files/config.txt

CONFIGREMOTE=$PROJDIR/Config_Files/config.txt 

# Your executable binary 
PROG=main

n=0

cat $CONFIGLOCAL | sed -e "s/#.*//" | sed -e "/^\s*$/d" |
(
    read i
    echo $i
    while [[ $n -lt $i ]]
    do
    	read line
    	host=$( echo $line | awk '{ print $1 }' )
        port=$( echo $line | awk '{ print $2 }' )

		#echo $host.utdallas.edu
		#echo $port
		#echo $n
	
	urxvt -e sh -c "ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no $netid@$host.utdallas.edu ./$PROJDIR/$PROG $CONFIGREMOTE $n; exec bash" &

        n=$(( n + 1 ))
    done
)
