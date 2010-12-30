#!/bin/sh

set -e

if [ -z $1 ] || [ ! -d $1 ]; then
    echo "You should specify the src dir"
    exit 1
fi

if [ -z $2 ] || [ ! -d $2 ]; then
    echo "You should specify the target dir"
    exit 1
fi

PATCH_SRC_DIR=`dirname $1`/`basename $1`
PATCH_TARGET_DIR=`dirname $2`/`basename $2`

for src in `find $PATCH_SRC_DIR \( -name *.svn* -prune \) -o -print`; do
    target=$PATCH_TARGET_DIR${src#$PATCH_SRC_DIR}
    if [ -d $src ]; then
        if [ ! -d $target ]; then
            echo "creating dir $target"
            mkdir -p $target
        fi
    else
        echo "copying $src to $target"
        cp $src $target
    fi
done
echo "done."
