#!/bin/sh

DELIM=">>>>>>>>>>"

# see Lib/dummy_compileall.py
while read MARK
do
    if [[ $MARK = $DELIM ]]
    then
        read FILENAME
        read FILELEN
        dd bs=1 count=$FILELEN > $FILENAME 2>-
        read
    fi
done

