#!/bin/sh
command="$1 $2 $3 $4 $5"
dirs=`find . -maxdepth 1 -mindepth 1 -type d`
for d in $dirs;
do
    echo "exec command $command in $d"
    cd $d
    $command
    cd ..
done

    
