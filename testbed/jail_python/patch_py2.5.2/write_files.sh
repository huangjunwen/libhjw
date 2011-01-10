#!/bin/sh

# see Lib/dummy_compileall.py
MARK=">>>>>>>>>>"

set -e

while read line
do
    if [ "$line" = "$MARK" ]; then
        read FILENAME
        read FILELEN
        dd bs=1 count=$FILELEN > $FILENAME 2>&-
        read
    else
        echo $line
    fi
done

